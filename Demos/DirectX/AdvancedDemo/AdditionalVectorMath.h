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
#pragma once

using namespace DirectX;

namespace Math
{
	static XMVECTOR InitXMVector3(float i, float j, float k)
	{
		XMVECTOR result;

		result.VItem(0) = i;
		result.VItem(1) = j;
		result.VItem(2) = k;

		return result;
	}

	static XMVECTOR GetNormalizedVector(const XMVECTOR& vector)
	{
		float s = vector.VItem(0) * vector.VItem(0) 
				+ vector.VItem(1) * vector.VItem(1) 
				+ vector.VItem(2) * vector.VItem(2);
		s = sqrt(s);

		float x = vector.VItem(0) / s;
		float y = vector.VItem(1) / s;
		float z = vector.VItem(2) / s;
		
		return InitXMVector3(x,y,z);
	}

	static XMVECTOR VectorMultiply(const FXMVECTOR& vect1, const FXMVECTOR& vect2)
	{
		float x1 = vect1.VItem(0);
		float y1 = vect1.VItem(1);
		float z1 = vect1.VItem(2);
		float x2 = vect2.VItem(0);
		float y2 = vect2.VItem(1);
		float z2 = vect2.VItem(2);
		
		return InitXMVector3(z1 * y2 - y1 * z2, x1 * z2 - z1 * x2, y1 * x2 - x1 * y2);
	}

	static XMVECTOR SubstractVectors(const FXMVECTOR& item1, const FXMVECTOR& item2)
	{
		float x = item1.VItem(0) - item2.VItem(0);
		float y = item1.VItem(1) - item2.VItem(1);
		float z = item1.VItem(2) - item2.VItem(2);

		return InitXMVector3(x,y,z);
	}

	static bool IsVectorsEqual(const FXMVECTOR& item1, const FXMVECTOR& item2)
	{
		bool isXEqual = item1.VItem(0) == item2.VItem(0);
		bool isYEqual = item1.VItem(1) == item2.VItem(1);
		bool isZEqual = item1.VItem(2) == item2.VItem(2);

		return isXEqual && isYEqual && isZEqual;
	}

	static XMVECTOR RotateVector(const FXMVECTOR& vector, float angle, bool isRight)
	{
		if (isRight)
		{
			angle = -angle;
		}

		float v11 = cos(angle);
		float v12 = -sin(angle);
		float v21 = sin(angle);
		float v22 = cos(angle);

		return InitXMVector3(v11 * vector.VItem(0) + v12 * vector.VItem(2),
							vector.VItem(1),
							v21 * vector.VItem(0) + v22 * vector.VItem(2));
	}
};