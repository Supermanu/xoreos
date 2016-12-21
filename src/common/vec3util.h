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

#ifndef COMMON_VEC3UTIL_H
#define COMMON_VEC3UTIL_H

namespace Common {

class Vector3;

/** Compute the squared area of the triangle formed by three vectors.
 *
 * Can also be used to know if the last vector is on the right or left side
 * relatively to the segment formed by the others vertices. If the result is
 * positive, it means the third vector is on the left and on the right side
 * if negative.
 */
float triangleArea2(Common::Vector3 vertA, Common::Vector3 vertB, Common::Vector3 vertC);

bool getIntersection(Common::Vector3 segStart1, Common::Vector3 segEnd1,
                     Common::Vector3 segStart2, Common::Vector3 segEnd2, Common::Vector3 &intersect);
bool inCircle(Common::Vector3 center, float radius, Common::Vector3 startSegment, Common::Vector3 endSegment);
bool inCircle(Common::Vector3 center, float radius, Common::Vector3 point);
bool inFace(Vector3 point, Vector3 vertA, Vector3 vertB, Vector3 vertC);
bool inFace(Vector3 vA, Vector3 vB, Vector3 vC, Vector3 lineStart, Vector3 lineEnd, Vector3 &intersect);

} // End of namespace Common

#endif // COMMON_STRUTIL_H
