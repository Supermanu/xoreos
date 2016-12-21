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

namespace Common {

float triangleArea2(Common::Vector3 vertA, Common::Vector3 vertB, Common::Vector3 vertC) {
	return (vertB - vertA).cross(vertC - vertA)._z;
}

bool getIntersection(Common::Vector3 segStart1, Common::Vector3 segEnd1, Common::Vector3 segStart2, Common::Vector3 segEnd2, Common::Vector3 &intersect) {

	// 	warning("getIntersection: segStart1(%f, %f), segEnd1(%f, %f), segStart2(%f, %f), segEnd2(%f, %f)",
	// 			segStart1._x, segStart1._y, segEnd1._x, segEnd1._y, segStart2._x, segStart2._y, segEnd2._x, segEnd2._y
	// 	);
	Common::Vector3 r = segEnd1 - segStart1;
	Common::Vector3 s = segEnd2 - segStart2;
	// 	warning("r: (%f, %f)", r._x, r._y);
	// 	warning("s: (%f, %f)", s._x, s._y);
	float rs = (r * s)._z;
	// 	warning("rs: %f", rs);
	float qpr = ((segStart2 - segStart1) * r)._z;
	// 	warning("qpr: %f", qpr);

	if (rs == 0 && qpr == 0) {
		// The two segments are parallel
		if (qpr == 0) {
			// The two segments are colinear.
			float t0 = (segStart2 - segStart1).dot(r) / r.dot(r);
			float t1 = t0 + s.dot(r) / (r.dot(r));

			if ((t0 <= 1 && t0 >= 0) || (t1 <= 1.f && t1 >= 0.f)) {
				// Segments are overlaping.
				float lenghtS1 = segStart1.length();
				// 				float lenghtE1 = segEnd1.length();
				float lenghtS2 = segStart2.length();
				float lenghtE2 = segEnd2.length();

				// segStart1 or segEnd2 must be in seg2.
				if (lenghtS1 <= MAX(lenghtS2, lenghtE2) && lenghtS1 >= MIN(lenghtS2, lenghtE2)) {
					intersect = segStart1;
				} else {
					intersect = segEnd1;
				}
				return true;
			}
		}
		return false;
	}

	float u = qpr / rs;
	float t = ((segStart2 - segStart1) * s)._z / rs;

	// 	warning("u: %f, t: %f", u, t);
	if (u <= 1 && u >= 0 && t <= 1 && t >= 0) {
		// The segments are intersecting.
		intersect = segStart1 + (segEnd1 - segStart1) * t;
		// 		warning("Intersect: (%f, %f)", intersect._x, intersect._y);
		return true;
	}

	return false;
}

bool inCircle(Vector3 center, float radius, Vector3 startSegment, Vector3 endSegment) {
	Vector3 seg = (endSegment - startSegment).norm();
	// We use scalar projection to find the closest point to the center of the circle.
	return (startSegment + seg * center.dot(seg) - center).length() < radius;
}

bool inCircle(Vector3 center, float radius, Vector3 point) {
	return (center - point).length() < radius;
}

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

	// 	warning("In face %f, %f", u, v);
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
