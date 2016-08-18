/*
 * xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names can be
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * The Infinity, Aurora, Odyssey, Eclipse and Lycium engines, Copyright (c) BioWare corp.
 * The Electron engine, Copyright (c) Obsidian Entertainment and BioWare corp.
 *
 */

/** @file graphics/aurora/AABBNode.h
 *  An AABB node.
 */

#ifndef GRAPHICS_AURORA_AABBNODE_H
#define GRAPHICS_AURORA_AABBNODE_H

#include <vector>

#include "src/common/boundingbox.h"
#include "src/common/types.h"

namespace Graphics {

namespace Aurora {

class AABBNode {
public:
	AABBNode(std::vector<float> min, std::vector<float> max, int32 id, int32 uid);
	~AABBNode();

	void getMin(std::vector<float> &min);
	void getAbsoluteMin(std::vector<float> &min);
	void getMax(std::vector<float> &max);
	void getAbsoluteMax(std::vector<float> &max);
	void getAbsoluteMinMax(std::vector<float> &min, std::vector<float> &max);

	void move(float x, float y, float z);
	void setPosition(float x, float y, float z);
	void getPosition(float &x, float &y, float &z);
	void setOrientation(uint8 orientation);
	void applyOrientation(std::vector<float> &min, std::vector<float> &max);

	void scale(float scale[3]);
	void setChildren(AABBNode *left, AABBNode *right);

	bool hasChildren() const;
	AABBNode *getLeft() const;
	AABBNode *getRight() const;

	bool isIn(const Common::BoundingBox &otherBox, float shiftX = 0.f, float shiftY = 0.f, float shiftZ = 0.f);
	bool isSelected() const;
	void unselect();
	int32 isWalkable(float x1, float y1, float z1, float x2, float y2, float z2);
	int32 isWalkable(float x, float y);
	bool isIntersecting(std::vector<float> origin, std::vector<float> direction, float dist);
	Common::BoundingBox *getBox() const;

private:
	bool _hasChildren;
	bool _selected;

	AABBNode *_left;
	AABBNode *_right;

	std::vector<float> _min;
	std::vector<float> _max;

	float _x, _y, _z;
	uint8 _orientation;

	int32 _id;
	int32 _uid;

	Common::BoundingBox *_box;
};

} // End of namespace Aurora

} // End of namespace Graphics

#endif // GRAPHICS_AURORA_AABBNODE_H
