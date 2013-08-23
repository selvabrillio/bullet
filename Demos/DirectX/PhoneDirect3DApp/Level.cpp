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
#include "Level.h"
#include "CubeMotionState.h"

Level::Level(void)
{
	m_physics = new Physics();
}

void Level::Initialize(Renderer^ renderer)
{
	auto grnd = renderer->CreateGround();
	m_physics->AddPhysicalObject(grnd->GetGroundShape(), grnd->GetMotionState(), 0, btVector3(0,0,0));

	for(float height = 0; height < 10.0f; height += 1.0f)
	{
		for(float i = -2.0f; i < 3.0f; i += 1.0f)
		{
			auto cube = renderer->CreateCube();
			m_cubes.push_back(cube);

			btScalar x = 3;
			btScalar y = height;
			btScalar z = i;

			auto fallShape = new btBoxShape(btVector3(btScalar(0.5), btScalar(0.5), btScalar(0.5)));
			btMotionState* fallMotionState = new CubeMotionState(cube, btTransform(btQuaternion(0,0,0,1),btVector3(x,y,z)));
			btScalar mass = 5;
			btVector3 fallInertia(0,0,0);
			fallShape->calculateLocalInertia(mass,fallInertia);
			m_physics->AddPhysicalObject(fallShape, fallMotionState, mass, fallInertia);
		}
	}

	auto cube = renderer->CreateCube();
	m_cubes.push_back(cube);
	
	auto fallShape = new btBoxShape(btVector3(btScalar(0.5), btScalar(0.5), btScalar(0.5)));

	btMotionState* fallMotionState = new CubeMotionState(cube, btTransform(btQuaternion(0,0,0,1),btVector3(-40,15,0)));
	btScalar mass = 50;
	btVector3 fallInertia(0,0,0);
	btVector3 linearVelocity(30,0,0);
	fallShape->calculateLocalInertia(mass,fallInertia);

	m_physics->AddPhysicalObject(fallShape, fallMotionState, mass, fallInertia,linearVelocity);
}

Level::~Level(void)
{
	delete m_physics;
}

void Level::Update()
{
	m_physics->Update();
}