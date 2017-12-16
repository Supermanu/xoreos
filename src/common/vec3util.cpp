/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Utility/math functions for working with Vector3.
 */

#include "src/common/util.h"
#include "src/common/vector3.h"
#include "src/common/vec3util.h"

namespace Common {

bool inFace(Vector3 point, Vector3 vertA, Vector3 vertB, Vector3 vertC) {
	// Use Barycentric Technique.
	Vector3 v0 = vertC - vertA;
	Vector3 v1 = vertB - vertA;
	Vector3 v2 = point - vertA;

	float dot00 = v0.dot(v0);
	float dot01 = v0.dot(v1);
	float dot02 = v0.dot(v2);
	float dot11 = v1.dot(v1);
	float dot12 = v1.dot(v2);

	// Compute barycentric coordinates
	float denom = 1.f / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * denom;
	float v = (dot00 * dot12 - dot01 * dot02) * denom;

	// Check if point is in triangle
	return (u + 0.0001 >= 0) && (v + 0.0001 >= 0) && (u + v - 0.0001 < 1);
}

bool inFace(Vector3 vA, Vector3 vB, Vector3 vC, Vector3 lineStart, Vector3 lineEnd, Vector3 &intersect) {
	float epsilon = 0.000001;

	Vector3 D = lineEnd - lineStart;
	Vector3 e1, e2;  //Edge1, Edge2
	Vector3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	//Find vectors for two edges sharing V1
	e1 = vB - vA;
	e2 = vC - vA;

	//Begin calculating determinant - also used to calculate u parameter
	P = D.cross(e2);

	//if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
	det = e1.dot(P);
	//NOT CULLING
	if (det > -epsilon && det < epsilon)
		return false;

	inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = lineStart - vA;

	//Calculate u parameter and test bound
	u = T.dot(P) * inv_det;
	//The intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
		return false;

	//Prepare to test v parameter
	Q = T.cross(e1);

	//Calculate V parameter and test bound
	v = D.dot(Q) * inv_det;
	//The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f)
		return false;

	t = e2.dot(Q) * inv_det;

	if (t > epsilon) { //ray intersection
		intersect = lineStart + D * t;
		return true;
	}

	// No hit, no win
	return false;
}

} // End of namespace Common
