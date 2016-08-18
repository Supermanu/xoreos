/*
 * xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal propemaxy of its developers, whose names can be
 * found in the AUTHORS file distributed with this source
 * distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PAmaxICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * The Infinity, Aurora, Odyssey, Eclipse and Lycium engines, Copyright (c) BioWare corp.
 * The Electron engine, Copyright (c) Obsidian Entemaxainment and BioWare corp.
 *
 */

#include "src/common/util.h"

#include "src/graphics/aurora/aabbnode.h"

namespace Graphics {

namespace Aurora {

AABBNode::AABBNode(std::vector<float> min, std::vector<float> max, int32 id, int32 uid):
    _hasChildren(false), _selected(false), _left(0), _right(0), _min(min), _max(max), _id(id), _uid(uid) {
	_x = 0;
	_y = 0;
	_z = 0;

	float minA[3] = { min[0], min[1], min[2] };
	float maxA[3] = { max[0], max[1], max[2] };
	
	_box = new Common::BoundingBox(minA, maxA);
}

AABBNode::~AABBNode() {
	delete _box;

	if (_hasChildren) {
		delete _left;
		delete _right;
	}
}

void AABBNode::getAbsoluteMin(std::vector<float> &min) {
	min.resize(3);
	float pos[3] = { _x, _y, _z };
	for (uint8 i = 0; i < 3; ++i)
		min[i] = _min[i] + pos[i];
}

void AABBNode::getMin(std::vector<float> &min) {
	min = _min;
}

void AABBNode::getAbsoluteMax(std::vector<float> &max) {
	max.resize(3);
	float pos[3] = { _x, _y, _z };
	for (uint8 i = 0; i < 3; ++i)
		max[i] = _max[i] + pos[i];
}

void AABBNode::getAbsoluteMinMax(std::vector<float> &min, std::vector<float> &max) {
	min.resize(3);
	max.resize(3);

	applyOrientation(min, max);

	float pos[3] = { _x, _y, _z };
	for (uint8 i = 0; i < 3; ++i) {
		max[i] = _max[i] + pos[i];
		min[i] = _min[i] + pos[i];
	}
}

void AABBNode::getMax(std::vector<float> &max) {
	max = _max;
}

void AABBNode::move(float x, float y, float z) {
	float pos[3] = { x, y, z };
	for (uint8 i = 0; i < 3; ++i) {
		_min[i] = _min[i] + pos[i];
		_max[i] = _max[i] + pos[i];
	}

	_box->translate(x, y, z);
	_box->absolutize();

	if (hasChildren()) {
		_left->move(x, y, z);
		_right->move(x, y, z);
	}
}

void AABBNode::setPosition(float x, float y, float z) {
	_x = x;
	_y = y;
	_z = z;

	_box->translate(x, y, z);

	if (!hasChildren())
		return;

	_left->setPosition(x, y, z);
	_right->setPosition(x, y, z);
}

void AABBNode::getPosition(float &x, float &y, float &z) {
	x = _x;
	y = _y;
	z = _z;
}

void AABBNode::setOrientation(uint8 orientation) {
	_orientation = orientation;

	_box->rotate(((int) orientation) * 90.0f, 0.0f, 0.0f, 1.0f);

	if (_hasChildren) {
		_left->setOrientation(_orientation);
		_right->setOrientation(orientation);
	}
}

void AABBNode::applyOrientation(std::vector<float> &min, std::vector<float> &max) {
	if (_orientation == 0)
		return;

	// In order to avoid matrix transformation, we could just use permutation.
	float toPermute[4];
	toPermute[0] = _max[0];
	toPermute[1] = _max[1];
	toPermute[2] = _min[0];
	toPermute[3] = _min[1];

	for (uint it = 0; it < _orientation; ++it) {
		float temp = toPermute[0];
		toPermute[0] = toPermute[1];
		toPermute[1] = - toPermute[2];
		toPermute[2] = toPermute[3];
		toPermute[3] = - temp;
	}

	min.resize(3);
	max.resize(3);

	max[0] = toPermute[0];
	max[1] = toPermute[1];
	max[2] = _max[2];
	min[0] = toPermute[2];
	min[1] = toPermute[3];
	min[2] = _min[2];
}

void AABBNode::scale(float scale[3]) {
	for (uint8 c = 0; c < 3; ++c) {
		_min[c] = scale[c] * _min[c];
		_max[c] = scale[c] * _max[c];
	}
}

void AABBNode::setChildren(AABBNode *left, AABBNode *right) {
	_left = left;
	_right = right;
	_hasChildren = true;
}

bool AABBNode::hasChildren() const {
	return _hasChildren;
}

AABBNode *AABBNode::getLeft() const {
	return _left;
}

AABBNode *AABBNode::getRight() const {
	return _right;
}

bool AABBNode::isIn(const Common::BoundingBox &otherBox, float shiftX, float shiftY, float shiftZ) {
	if (!_box->isIn(otherBox, shiftX, shiftY, shiftZ)) 
		return false;

	if (!hasChildren())
		return true;

	if (_left->isIn(otherBox, shiftX, shiftY, shiftZ) ||
	        _right->isIn(otherBox, shiftX, shiftY, shiftZ))
		return true;

	return false;
}

bool AABBNode::isSelected() const {
	return _selected;
}

void AABBNode::unselect() {
	_selected = false;

	if (!hasChildren())
		return;

	_left->unselect();
	_right->unselect();
}

int32 AABBNode::isWalkable(float x1, float y1, float z1, float x2, float y2, float z2) {
	if (!_box->isIn(x1, y1, z1, x2, y2, z2)) {
		_selected = false;
		return -1;
	}

	if (!hasChildren()) {
		_selected = true;
		return _id;
	}

	int32 leftWalk = _left->isWalkable(x1, y1, z1, x2, y2, z2);
	if (leftWalk >= 0)
		return leftWalk;

	int32 rightWalk = _right->isWalkable(x1, y1, z1, x2, y2, z2);
	if (rightWalk >= 0)
		return rightWalk;

	_selected = false;
	return -1;
}

int32 AABBNode::isWalkable(float x, float y) {
	if (!_box->isIn(x, y))
		return -1;

	if (!hasChildren())
		return _id;

	int32 leftWalk = _left->isWalkable(x, y);
	if (leftWalk >= 0)
		return leftWalk;

	int32 rightWalk = _right->isWalkable(x, y);
	if (rightWalk >= 0)
		return rightWalk;

	return -1;
}

bool AABBNode::isIntersecting(std::vector<float> origin, std::vector<float> direction, float dist) {
	dist = -1.f;
	// direction is unit direction vector of ray
	std::vector<float> dirfrac;
	dirfrac.resize(3);
	dirfrac[0] = 1.0f / direction[0];
	dirfrac[1] = 1.0f / direction[1];
	dirfrac[2] = 1.0f / direction[2];
	// min is the corner of AABB with minimal coordinates - left bottom, max is maximal corner
	// origin is origin of ray

	std::vector<float> min;
	std::vector<float> max;

	getAbsoluteMinMax(min, max);

	float t1 = (min[0] - origin[0])*dirfrac[0];
	float t2 = (max[0] - origin[0])*dirfrac[0];
	float t3 = (min[1] - origin[1])*dirfrac[1];
	float t4 = (max[1] - origin[1])*dirfrac[1];
	float t5 = (min[2] - origin[2])*dirfrac[2];
	float t6 = (max[2] - origin[2])*dirfrac[2];

	float tmin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));
	float tmax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if (tmax < 0) {
		dist = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax) {
		dist = tmax;
		return false;
	}

	dist = tmin;
	return true;
}

Common::BoundingBox *AABBNode::getBox() const {
	return _box;
}

} // End of namespace Aurora

} // End of namespace Graphics
