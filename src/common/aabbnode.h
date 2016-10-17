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
 *  An axis-aligned bounding box node.
 */

#ifndef COMMON_AABBNODE_H
#define COMMON_AABBNODE_H

#include <vector>

#include "src/common/types.h"
#include "src/common/boundingbox.h"

namespace Common {

class AABBNode : public BoundingBox {
public:
	AABBNode(float min[], float max[], int32 property = -1);
	~AABBNode();

	bool hasChildren() const;
	void setChildren(AABBNode *leftChild, AABBNode *rightChild);

	void rotate(float angle, float x, float y, float z);
	void setOrientation(uint8 orientation);
	void translate(float x, float y, float z);
	void absolutize();

	AABBNode *getNode(float x, float y);
	AABBNode *getNode(float x1, float y1, float z1, float x2, float y2, float z2);
	void getNodes(float x1, float y1, float z1, float x2, float y2, float z2, std::vector<AABBNode *> &nodes);
	void getNodes(float x1, float y1, float x2, float y2, std::vector<AABBNode *> &nodes);
	void getNodes(float x, float y, std::vector<AABBNode *> &nodes);

	int32 getProperty() const;

private:
// 	bool isIn(float x1, float y1, float x2, float y2) const;
	AABBNode *_leftChild;
	AABBNode *_rightChild;
	int32 _property;
};

} // End of namespace Common

#endif // COMMON_AABBNODE_H
