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
//--------------------------------------------------------------------------------------
// File: Renderer.cpp
//
// This is a simple Windows Store app showing use of DirectXTK
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "pch.h"

#include "Renderer.h"
#include "Effects.h"
#include "DDSTextureLoader.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

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
}

Renderer::~Renderer()
{
	delete m_batch;
}

Cube^ Renderer::CreateCube()
{	
	auto cube = ref new Cube();

	cubes.push_back(cube);
	cube->Create(m_d3dContext);
	return cube;
}

void Renderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources();

    // Create DirectXTK objects
    auto device = m_d3dDevice.Get();
    m_states.reset( new CommonStates( device ) );
    m_fxFactory.reset( new MyEffectFactory( device ) );

	customEffect.reset( new BasicEffect( device ) );
	customEffect->SetVertexColorEnabled(false);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

		customEffect->GetVertexShaderBytecode( &shaderByteCode, &byteCodeLength );

        DX::ThrowIfFailed(
			device->CreateInputLayout( VertexPositionColor::InputElements,
                                        VertexPositionColor::InputElementCount,
                                        shaderByteCode, byteCodeLength,
                                        inputLayout.ReleaseAndGetAddressOf() ) 
                         );
    }


    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile( device, L"assets\\seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf() )
                     );
	
	m_spriteBatch.reset(new SpriteBatch(m_d3dContext.Get()));
    m_spriteFont.reset(new SpriteFont(device, L"assets\\italic.spritefont"));

	m_batch = new PrimitiveBatch<VertexPositionColor>(m_d3dContext.Get());
}

void Renderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// Note that the m_orientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

    XMMATRIX projection = 
			XMMatrixMultiply(
				XMMatrixPerspectiveFovRH(
					fovAngleY,
					aspectRatio,
					0.01f,
					100.0f
					),
				XMLoadFloat4x4(&m_orientationTransform3D)
			);

	customEffect->SetProjection(projection);

	XMStoreFloat4x4( &m_projection, projection );

	//m_projection = XMFLOAT4X4(1.f, 0.f, 0.f,0.f, 0.f,0.f, 0.f,0.f, 0.f,0.f, 0.f,0.f, 0.f,0.f, 0.f, 0.f);
}

void Renderer::Update(float timeTotal, float timeDelta)
{
    UNREFERENCED_PARAMETER( timeDelta );

	fpsCounter.Update(timeDelta);

	XMVECTOR eye = XMVectorSet(30, 15.0f, 10, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtRH(eye, at, up);
    XMMATRIX world = XMMatrixRotationY(-XM_PIDIV2);

	customEffect->SetView( view );
	customEffect->SetWorld( XMMatrixIdentity() );

	XMStoreFloat4x4(&m_view, view);
	XMStoreFloat4x4(&m_world, world);
}

Ground^ Renderer::CreateGround()
{
	this->ground = ref new Ground();
	return this->ground;
}

void Renderer::RenderGround( FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color )
{
    m_batch->Begin();

    xdivs = std::max<size_t>( 1, xdivs );
    ydivs = std::max<size_t>( 1, ydivs );

    for( size_t i = 0; i <= xdivs; ++i )
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = ( fPercent * 2.0f ) - 1.0f;
        XMVECTOR vScale = XMVectorScale( xAxis, fPercent );
        vScale = XMVectorAdd( vScale, origin );

        VertexPositionColor v1( XMVectorSubtract( vScale, yAxis ), color );
        VertexPositionColor v2( XMVectorAdd( vScale, yAxis ), color );
        m_batch->DrawLine( v1, v2 );
    }

    for( size_t i = 0; i <= ydivs; i++ )
    {
        FLOAT fPercent = float(i) / float(ydivs);
        fPercent = ( fPercent * 2.0f ) - 1.0f;
        XMVECTOR vScale = XMVectorScale( yAxis, fPercent );
        vScale = XMVectorAdd( vScale, origin );

        VertexPositionColor v1( XMVectorSubtract( vScale, xAxis ), color );
        VertexPositionColor v2( XMVectorAdd( vScale, xAxis ), color );
        m_batch->DrawLine( v1, v2 );
    }

    m_batch->End();
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

	customEffect->Apply( m_d3dContext.Get() );
	m_d3dContext->IASetInputLayout( inputLayout.Get() );
   
	m_spriteBatch->Begin();
		m_spriteFont->DrawString(m_spriteBatch.get(), fpsCounter.ToString().c_str(), XMFLOAT2(40, 10));
	m_spriteBatch->End();

	const XMVECTORF32 xaxis = { 35.f, 0.f, 0.f };
    const XMVECTORF32 yaxis = { 0.f, 0.f, 10.f };
    RenderGround( xaxis, yaxis, g_XMZero, 50, 15, Colors::Gray );

	XMMATRIX world = XMLoadFloat4x4( &m_world );
	XMMATRIX view = XMLoadFloat4x4( &m_view );
    XMMATRIX projection = XMLoadFloat4x4( &m_projection );

	for(auto& cube : cubes)
	{
		cube->Draw(world, view, projection, Colors::White, m_texture1.Get());
	}
}
