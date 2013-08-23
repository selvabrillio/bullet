#include "pch.h"

#include "Renderer.h"
#include "Effects.h"
#include "DDSTextureLoader.h"

using namespace Microsoft::WRL;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

using namespace Math;

class MyEffectFactory : public EffectFactory
{
public:
    MyEffectFactory( _In_ ID3D11Device* device ) : EffectFactory( device ) {}

    virtual void CreateTexture( _In_z_ const WCHAR* name, _In_opt_ ID3D11DeviceContext* deviceContext, _Outptr_ ID3D11ShaderResourceView** textureView ) override
    {
        WCHAR path[MAX_PATH] = {0};

        wcscpy_s( path, L"Assets\\" );
        wcscat_s( path, name );

        EffectFactory::CreateTexture( path, deviceContext, textureView );
    }
};

Renderer::Renderer()
{
	cameraLookAt.xPosition = 0;
	cameraLookAt.yPosition = 5;
	cameraLookAt.zPosition = 0;
	Position cameraPos(-15,5,0);
	this->cameraPosition = cameraPos;
}

Renderer::~Renderer()
{
}

Cube^ Renderer::CreateCube()
{	
	auto cube = ref new Cube();

	cubes.push_back(cube);
	cube->Create(m_d3dContext);
	return cube;
}

Sphere^ Renderer::CreateSphere()
{	
	auto sphere = ref new Sphere();

	spheres.push_back(sphere);
	sphere->Create(m_d3dContext);
	return sphere;
}

void Renderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources();

    auto device = m_d3dDevice.Get();
    m_states.reset( new CommonStates( device ) );
    m_fxFactory.reset( new MyEffectFactory( device ) );

	m_customEffect.reset( new BasicEffect( device ) );

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

		m_customEffect->SetFogEnabled(false);
		m_customEffect->SetVertexColorEnabled(false);
		m_customEffect->EnableDefaultLighting();
		m_customEffect->SetTextureEnabled(true);

		m_customEffect->GetVertexShaderBytecode( &shaderByteCode, &byteCodeLength );

        DX::ThrowIfFailed(
			device->CreateInputLayout( VertexPositionNormalTexture::InputElements,
                                        VertexPositionNormalTexture::InputElementCount,
                                        shaderByteCode, byteCodeLength,
                                        m_inputLayout.ReleaseAndGetAddressOf() ) 
                         );
    }

    DX::ThrowIfFailed(
		CreateDDSTextureFromFile( device, L"assets\\seafloor.dds", nullptr, m_cubeTex.ReleaseAndGetAddressOf() )
                     );

	DX::ThrowIfFailed(
    CreateDDSTextureFromFile( device, L"assets\\windowslogo.dds", nullptr, m_sphereTex.ReleaseAndGetAddressOf()  )
                    );

	DX::ThrowIfFailed(
	CreateDDSTextureFromFile( device, L"assets\\grass.dds", nullptr, m_groundTex.ReleaseAndGetAddressOf()  )
					 );

	DX::ThrowIfFailed(
	CreateDDSTextureFromFile( device, L"assets\\Scope.dds", nullptr, m_scopeTex.ReleaseAndGetAddressOf()  )
					 );

	m_customEffect->SetTexture(m_groundTex.Get());

	m_spriteBatch.reset(new SpriteBatch(m_d3dContext.Get()));
    m_spriteFont.reset(new SpriteFont(device, L"assets\\italic.spritefont"));
}

void Renderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	//m_scope = ref new Scope((int)m_windowBounds.Width,(int)m_windowBounds.Height,m_d3dContext.Get());

	// Note that the m_orientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

    DirectX::XMMATRIX projection = 
			XMMatrixMultiply(
				XMMatrixPerspectiveFovRH(
					fovAngleY,
					aspectRatio,
					0.01f,
					100.0f
					),
				XMLoadFloat4x4(&m_orientationTransform3D)
			);

	m_customEffect->SetProjection(projection);

	DirectX::XMStoreFloat4x4( &m_projection, projection );
}

void Renderer::Update(float timeTotal, float timeDelta)
{
    UNREFERENCED_PARAMETER( timeDelta );
	DirectX::XMVECTOR seeVector = Math::GetNormalizedVector(cameraLookAt - cameraPosition);
	DirectX::XMVECTOR yAxis = Math::InitXMVector3(0,1,0);
	float angle = (float) (0.3 * 180 / XM_PI);

	fpsCounter.Update(timeDelta);

	DirectX::XMVECTOR eye = XMVectorSet(cameraPosition.xPosition, cameraPosition.yPosition, cameraPosition.zPosition, 0.0f);
	DirectX::XMVECTOR at = XMVectorSet(cameraLookAt.xPosition, cameraLookAt.yPosition, cameraLookAt.zPosition, 0.0f);
	DirectX::XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	if (Math::IsVectorsEqual(Math::SubstractVectors(eye,at),Math::InitXMVector3(0,0,0)))
	{
		eye.VItem(0) += 0.01f;
		eye.VItem(2) += 0.01f;
	}

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eye, at, up);
    DirectX::XMMATRIX world = DirectX::XMMatrixRotationY(-XM_PIDIV2);

	m_customEffect->SetWorld( DirectX::XMMatrixIdentity() );
	m_customEffect->SetView( view );

	DirectX::XMStoreFloat4x4(&m_view, view);
	DirectX::XMStoreFloat4x4(&m_world, world);
}

Ground^ Renderer::CreateGround()
{
	this->ground = ref new Ground(m_d3dContext.Get());
	return this->ground;
}

void Renderer::Render()
{
	m_d3dContext->ClearRenderTargetView(
		m_renderTargetView.Get(),
		Colors::MidnightBlue
		);

	m_d3dContext->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);

	m_d3dContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
		);
	m_spriteBatch->Begin();
		m_spriteFont->DrawString(m_spriteBatch.get(), fpsCounter.ToString().c_str(), XMFLOAT2(m_window->Bounds.Right - 400, 10));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Touch - fire", XMFLOAT2(10, 50));
	m_spriteBatch->End();

	m_customEffect->Apply( m_d3dContext.Get() );
	m_d3dContext->IASetInputLayout( m_inputLayout.Get() );
	this->ground->Render();

	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4( &m_world );
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4( &m_view );
    DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4( &m_projection );	

	for(auto& cube : cubes)
	{
		cube->Draw(world, view, projection, Colors::White, m_cubeTex.Get());
	}

	for(auto& sphere : spheres)
	{
		sphere->Draw(world, view, projection, Colors::White, m_sphereTex.Get());
	}

	//m_scope->Render(m_scopeTex.Get());
}
