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
#include "Ground.h"
#include "SpriteBatch.h"

Ground::Ground(ID3D11DeviceContext1* deviceContext)
{	
	this->batch = new PrimitiveBatch<VertexPositionNormalTexture>(deviceContext);
	this->groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
	this->groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
}

void Ground::Render()
{
	XMFLOAT2 leftTopTex(0 , 0);
	XMFLOAT2 leftBotTex(1 , 0);
	XMFLOAT2 rightBotTex(1 , 1);
	XMFLOAT2 rightTopTex(0 , 1);
	
	XMFLOAT3  normal(0, 1 ,0);
	batch->Begin();
	for (int i = -10 ; i < 30 ; i += 2)
	{
		for (int j = 10; j > -10 ; j -= 2)
		{
			XMFLOAT3 leftTop((float)i, 0.0f, (float)(j - 2));
			XMFLOAT3 leftBottom((float)(i + 2), 0.0f, (float)(j - 2));
			XMFLOAT3 rightBottom((float)(i + 2), 0.0f, (float)j);
			XMFLOAT3 rightTop ((float)i, 0.0f, (float)j);

			VertexPositionNormalTexture vp1(leftTop, normal, leftTopTex);
			VertexPositionNormalTexture vp2(leftBottom, normal, leftBotTex);
			VertexPositionNormalTexture vp3(rightBottom, normal, rightBotTex);
			VertexPositionNormalTexture vp4(rightTop, normal, rightTopTex);

			batch->DrawQuad(vp1,vp2,vp3,vp4);
		}
	}
	batch->End();
}

Ground::~Ground()
{
	delete batch;
	//do not delete pointers - bullet will delete it itself
	//delete this->groundMotionState;
	//delete this->groundShape;
}