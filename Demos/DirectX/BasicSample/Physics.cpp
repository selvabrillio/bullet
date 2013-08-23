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
#include "Physics.h"

#include <algorithm>

using namespace std;

Physics::Physics(void) :
	m_broadphase(new btDbvtBroadphase()),
	m_collisionConfiguration(new btDefaultCollisionConfiguration()),
	m_solver(new btSequentialImpulseConstraintSolver)
{
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);

	m_dynamicsWorld->setGravity(btVector3(0,-10,0));
}

Physics::~Physics(void)
{
	for_each( begin(m_rigidBodies), end(m_rigidBodies), [&](btRigidBody& rigidBody )
	{
		m_dynamicsWorld->removeRigidBody(&rigidBody);
	} );

	m_rigidBodies.clear();

	//do not delete pointers - bullet will delete it itself
/*	delete m_broadphase;
	delete m_collisionConfiguration;
	delete m_dispatcher;
	delete m_solver;
	delete m_dynamicsWorld;*/
}

void Physics::Update()
{
    m_dynamicsWorld->stepSimulation(1/60.f,10);
}

void Physics::AddPhysicalObject(btCollisionShape* collisionShape, btMotionState* motionState, btScalar mass, const btVector3& inertia)
{
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, motionState, collisionShape, inertia);

	auto groundRigidBody = new btRigidBody(groundRigidBodyCI);
	
	m_dynamicsWorld->addRigidBody(groundRigidBody);

	m_shapes.push_back(std::unique_ptr<btCollisionShape>(collisionShape));
	m_rigidBodies.push_back(*groundRigidBody);
}

void Physics::AddPhysicalObject(btCollisionShape* collisionShape, btMotionState* motionState, 
								btScalar mass, const btVector3& inertia,const btVector3& linVelocity)
{
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, motionState, collisionShape, inertia);

	auto groundRigidBody = new btRigidBody(groundRigidBodyCI);
	groundRigidBody->setLinearVelocity(linVelocity);
	m_dynamicsWorld->addRigidBody(groundRigidBody);

	m_shapes.push_back(std::unique_ptr<btCollisionShape>(collisionShape));
	m_rigidBodies.push_back(*groundRigidBody);
}
