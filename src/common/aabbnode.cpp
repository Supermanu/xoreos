
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

#include "src/common/util.h"
#include "src/common/aabbnode.h"

namespace Common {

AABBNode::AABBNode(float min[], float max[], int32 property) : BoundingBox(), _property(property) {
	_min[0] = min[0];
	_min[1] = min[1];
	_min[2] = min[2];

	_max[0] = max[0];
	_max[1] = max[1];
	_max[2] = max[2];

	_leftChild = 0;
	_rightChild = 0;
	_empty = false;
}

AABBNode::~AABBNode() {
	delete _leftChild;
	delete _rightChild;
}

bool AABBNode::hasChildren() const {
	// Assume that there are always two children.
	return _leftChild != 0;
}

void AABBNode::setChildren(AABBNode *leftChild, AABBNode *rightChild) {
	if (leftChild == 0 || rightChild == 0)
		error("AABB must have two or no child");

	_leftChild = leftChild;
	_rightChild = rightChild;
}

void AABBNode::rotate(float angle, float x, float y, float z) {
	if (fmod(angle, 90.f) != 0.f)
		error("AABB can only rotate from one axis to the other");

	BoundingBox::rotate(angle, x, y, z);

	if (!hasChildren())
		return;

	_leftChild->rotate(angle, x, y, z);
	_rightChild->rotate(angle, x, y, z);
}

void AABBNode::setOrientation(uint8 orientation) {
	rotate((float) orientation * 90.f, 0.f, 0.f, 1.f);
}

void AABBNode::translate(float x, float y, float z) {
	BoundingBox::translate(x, y, z);

	if (!hasChildren())
		return;

	_leftChild->translate(x, y, z);
	_rightChild->translate(x, y, z);
}

void AABBNode::absolutize() {
	BoundingBox::absolutize();

	if (!hasChildren())
		return;

	_leftChild->absolutize();
	_rightChild->absolutize();
}

AABBNode *AABBNode::getNode(float x, float y) {
	if (!isIn(x, y))
		return 0;

	if (!hasChildren())
		return this;

	AABBNode *leftLeaf = _leftChild->getNode(x, y);
	if (leftLeaf)
		return leftLeaf;

	return _rightChild->getNode(x, y);
}

AABBNode *AABBNode::getNode(float x1, float y1, float z1, float x2, float y2, float z2) {
	if (!isIn(x1, y1, z1, x2, y2, z2))
		return 0;

	if (!hasChildren())
		return this;

	AABBNode *leftLeaf = _leftChild->getNode(x1, y1, z1, x2, y2, z2);
	if (leftLeaf)
		return leftLeaf;

	return _rightChild->getNode(x1, y1, z1, x2, y2, z2);
}

void AABBNode::getNodes(float x1, float y1, float z1, float x2, float y2, float z2, std::vector<AABBNode *> &nodes) {
	if (!isIn(x1, y1, z1, x2, y2, z2))
		return;

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodes(x1, y1, z1, x2, y2, z2, nodes);
	_rightChild->getNodes(x1, y1, z1, x2, y2, z2, nodes);
}

int32 AABBNode::getProperty() const {
	return _property;
}

} // End of namespace Common
