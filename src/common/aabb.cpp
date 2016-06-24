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
 *  AABB (Axis-Aligned Bounding Box).
 */

#include "src/common/util.h"

#include "src/common/aabb.h"

namespace Common {
AABBNode::AABBNode(float min[], float max[]) : BoundingBox(min, max),
     _hasChildren(false), _left(0), _right(0), _selected(false) {
}

AABBNode::~AABBNode() {
	if (_hasChildren) {
		delete _left;
		delete _right;
	}
}

void AABBNode::translate(float x, float y, float z) {
	BoundingBox::translate(x, y, z);

	if (!_hasChildren)
		return;

	_left ->translate(x, y, z);
	_right->translate(x, y, z);
}

void AABBNode::rotate(float angle, float x, float y, float z) {
	if ((int) angle % 90 != 0)
		error("AABB must be axis-aligned");

	BoundingBox::rotate(angle, x, y, z);

	if (!_hasChildren)
		return;

	_left ->rotate(angle, x, y, z);
	_right->rotate(angle, x, y, z);
}

void AABBNode::setOrientation(uint8 orientation) {
	rotate(orientation * 90.f, 0, 0, 1);
}

void AABBNode::absolutize() {
	BoundingBox::absolutize();

	if (!_hasChildren)
		return;

	_left->absolutize();
	_right->absolutize();
}

void AABBNode::setProperty(int32 property) {
	_property = property;
}

int32 AABBNode::getProperty() const {
	return _property;
}
void AABBNode::setChildren(AABBNode *left, AABBNode *right) {
	if (left == 0 && right == 0)
		error("AABB children cannot be empty");

	_left = left;
	_right = right;
	_hasChildren = true;
}

bool AABBNode::hasChildren() const {
	return _hasChildren;
}

AABBNode *AABBNode::getNode(float x, float y) {
	if (!isIn(x, y))
		return 0;

	if (!_hasChildren)
		return this;

	AABBNode *leftLeaf = _left->getNode(x, y);
	if (leftLeaf)
		return leftLeaf;

	return _right->getNode(x, y);
}

AABBNode *AABBNode::getNode(float x1, float y1, float z1, float x2, float y2, float z2) {
	if (!isIn(x1, y1, z1, x2, y2, z2))
		return 0;

	if (!_hasChildren)
		return this;

	AABBNode *leftLeaf = _left->getNode(x1, y1, z1, x2, y2, z2);
	if (leftLeaf)
		return leftLeaf;

	return _right->getNode(x1, y1, z1, x2, y2, z2);
}

AABBNode *AABBNode::getLeft() const {
	return _left;
}

AABBNode *AABBNode::getRight() const {
	return _right;
}

bool AABBNode::isSelected() const {
	return _selected;
}

void AABBNode::select(bool select) {
	_selected = select;

	if (_hasChildren) {
		_left->select(select);
		_right->select(select);
	}
}

} // End of namespace Common
