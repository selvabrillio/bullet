//--------------------------------------------------------------------------------------
// Copyright © Microsoft Corporation. All rights reserved.
// Portions Copyright © Microsoft Open Technologies, Inc. All rights reserved.
// Microsoft Public License (Ms-PL)
//
// This license governs use of the accompanying software. If you use the software, you accept this license. If you do not accept the license, do not use the software.
//
// 1. Definitions
//
// The terms "reproduce," "reproduction," "derivative works," and "distribution" have the same meaning here as under U.S. copyright law.
//
// A "contribution" is the original software, or any additions or changes to the software.
//
// A "contributor" is any person that distributes its contribution under this license.
//
// "Licensed patents" are a contributor's patent claims that read directly on its contribution.
//
// 2. Grant of Rights
//
// (A) Copyright Grant- Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
//
// (B) Patent Grant- Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free license under its licensed patents to make, have made, use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.
// 
// 3. Conditions and Limitations
// 
// (A) No Trademark License- This license does not grant you rights to use any contributors' name, logo, or trademarks.
// 
// (B) If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your patent license from such contributor to the software ends automatically.
// 
// (C) If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
// 
// (D) If you distribute any portion of the software in source code form, you may do so only under this license by including a complete copy of this license with your distribution. If you distribute any portion of the software in compiled or object code form, you may only do so under a license that complies with this license.
// 
// (E) The software is licensed "as-is." You bear the risk of using it. The contributors give no express warranties, guarantees or conditions. You may have additional consumer rights under your local laws which this license cannot change. To the extent permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular purpose and non-infringement.
//--------------------------------------------------------------------------------------
#include "pch.h"

#include "Renderer.h"
#include "Effects.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

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
	Position cameraPos(-45,5,0);
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

	m_scope = ref new Scope((int)m_windowBounds.Width,(int)m_windowBounds.Height,m_d3dContext.Get());

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

void Renderer::Update(float timeTotal, float timeDelta, const VirtualKey keyPressed)
{
    UNREFERENCED_PARAMETER( timeDelta );
	DirectX::XMVECTOR seeVector = Math::GetNormalizedVector(cameraLookAt - cameraPosition);
	DirectX::XMVECTOR yAxis = Math::InitXMVector3(0,1,0);
	DirectX::XMVECTOR perpSeeVector;
	DirectX::XMVECTOR rotatedSeeVector;
	float angle = (float) (0.3 * 180 / XM_PI);

	fpsCounter.Update(timeDelta);

	switch (keyPressed)
	{
		case VirtualKey::W: 
			cameraPosition += seeVector;
			break;
		case VirtualKey::S: 	
			seeVector = -seeVector;
			cameraPosition += seeVector;
			break;
		case VirtualKey::A: 
			perpSeeVector = Math::VectorMultiply(seeVector,yAxis);

			cameraPosition += perpSeeVector;

			cameraLookAt.xPosition += perpSeeVector.VItem(0);
			cameraLookAt.zPosition += perpSeeVector.VItem(2);
			break;
		case VirtualKey::D:
			perpSeeVector = -Math::VectorMultiply(seeVector,yAxis);

			cameraPosition += perpSeeVector;

			cameraLookAt.xPosition += perpSeeVector.VItem(0);
			cameraLookAt.zPosition += perpSeeVector.VItem(2);
			break;
		case VirtualKey::Up: 
			cameraLookAt.yPosition += 0.3f;
			break;
		case VirtualKey::Down: 
			cameraLookAt.yPosition -= 0.3f;
			break;
		case VirtualKey::Left: 
			rotatedSeeVector = Math::RotateVector(seeVector, angle, false);
			cameraLookAt += rotatedSeeVector;
			break;
		case VirtualKey::Right: 
			rotatedSeeVector = Math::RotateVector(seeVector, angle, true);
			cameraLookAt += rotatedSeeVector;
			break;
		default:break;
	}

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
		m_spriteFont->DrawString(m_spriteBatch.get(), L"W - move forward", XMFLOAT2(10, 10));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"S - move back", XMFLOAT2(10, 50));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"A - move camera left", XMFLOAT2(10, 90));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"D - move camera right", XMFLOAT2(10, 130));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Up - look up", XMFLOAT2(10, 170));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Down - look down", XMFLOAT2(10, 210));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Left - turn left", XMFLOAT2(10, 250));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Right - turn right", XMFLOAT2(10, 290));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Alt + F4 - exit", XMFLOAT2(10, 330));
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Spacebar - fire", XMFLOAT2(10, 370));
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

	m_scope->Render(m_scopeTex.Get());
}
