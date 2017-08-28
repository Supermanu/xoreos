
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

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/algorithms/intersects.hpp>

#include "src/common/vec3util.h"
#include "src/common/util.h"
#include "src/common/aabbnode.h"


typedef boost::geometry::model::d2::point_xy<float> boostPoint2d;

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

void AABBNode::adjustChildrenProperty(int32 adjust) {
	if (!hasChildren()) {
		_property += adjust;
		return;
	}

	_leftChild->adjustChildrenProperty(adjust);
	_rightChild->adjustChildrenProperty(adjust);
}

void AABBNode::getNodes(float x, float y, std::vector<AABBNode *> &nodes) {
	if (!isIn(x, y))
		return;

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodes(x, y, nodes);
	_rightChild->getNodes(x, y, nodes);
}

void AABBNode::getNodes(float x1, float y1, float x2, float y2, std::vector<AABBNode *> &nodes) {
	if (!isIn(x1, y1, _min[2], x2, y2, _min[2]))
		return;

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodes(x1, y1, x2, y2, nodes);
	_rightChild->getNodes(x1, y1, x2, y2, nodes);
}

void AABBNode::getNodesInCircle(Common::Vector3 center, float radius, std::vector<AABBNode *> &nodes) {
	Common::Vector3 rightTop(_max[0], _max[1], center[2]);
	Common::Vector3 rightBottom(_max[0], _min[1], center[2]);
	Common::Vector3 leftTop(_min[0], _max[1], center[2]);
	Common::Vector3 leftBottom(_min[0], _min[1], center[2]);

	// Check if the circle is inside the node
	bool insideNode = triangleArea2(leftBottom, leftTop, center) <= 0
	                  && triangleArea2(leftTop, rightTop, center) <= 0
	                  && triangleArea2(rightTop, rightBottom, center) <= 0
	                  && triangleArea2(rightBottom, leftBottom, center) <= 0;

	if (!insideNode
	    && !inCircle(center, radius, rightTop, rightBottom)
	    && !inCircle(center, radius, rightTop, leftTop)
	    && !inCircle(center, radius, leftTop, leftBottom)
	    && !inCircle(center, radius, leftBottom, rightBottom))
		return;
/*
	if ((center - vec1).length() >= radius
		&& (center - vec2).length() >= radius
		&& (center - vec3).length() >= radius
		&& (center - vec4).length() >= radius
	)
		return;*/

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodesInCircle(center, radius, nodes);
	_rightChild->getNodesInCircle(center, radius, nodes);
}

void AABBNode::getNodesInAABox2D(Common::Vector3 min, Common::Vector3 max, std::vector<AABBNode *> &nodes) {
	boost::geometry::model::box<boostPoint2d> box(boostPoint2d(min[0], min[1]), boostPoint2d(max[0], max[1]));
	boost::geometry::model::box<boostPoint2d> currentNode(boostPoint2d(_min[0], _min[1]), boostPoint2d(_max[0], _max[1]));

	if (!boost::geometry::intersects(box, currentNode))
		return;

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodesInAABox2D(min, max, nodes);
	_rightChild->getNodesInAABox2D(min, max, nodes);

	return;
}

void AABBNode::getNodesInPolygon(Common::Vector3 vertices[], uint32 vertexCount, std::vector<AABBNode *> &nodes) {
	boost::geometry::model::polygon<boostPoint2d> polygon;
	for (uint32 v = 0; v < vertexCount; ++v) {
		boostPoint2d vert(vertices[v][0], vertices[v][1]);
		boost::geometry::append(polygon.outer(), vert);
	}

	boost::geometry::model::box<boostPoint2d> currentNode(boostPoint2d(_min[0], _min[1]), boostPoint2d(_max[0], _max[1]));

	if (!boost::geometry::intersects(polygon, currentNode))
		return;

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodesInPolygon(vertices, vertexCount, nodes);
	_rightChild->getNodesInPolygon(vertices, vertexCount, nodes);

	return;
}

void AABBNode::getNodesInSegment(Common::Vector3 start, Common::Vector3 end, std::vector<AABBNode *> &nodes) {
	boost::geometry::model::segment<boostPoint2d> line(boostPoint2d(start[0], start[1]), boostPoint2d(end[0], end[1]));
	boost::geometry::model::box<boostPoint2d> currentNode(boostPoint2d(_min[0], _min[1]), boostPoint2d(_max[0], _max[1]));

	if (!boost::geometry::intersects(line, currentNode))
		return;

	if (!hasChildren()) {
		nodes.push_back(this);
		return;
	}

	_leftChild->getNodesInSegment(start, end, nodes);
	_rightChild->getNodesInSegment(start, end, nodes);

	return;
}

// bool AABBNode::isIn(float x1, float y1, float x2, float y2) const {
// 	if (_empty)
// 		return false;
//
// 	float minX, minY, minZ;
// 	getMin(minX, minY, minZ);
//
// 	float maxX, maxY, maxZ;
// 	getMax(maxX, maxY, maxZ);
//
// 	if ((x2 < minX) && (x1 < minX)) return false;
// 	if ((x2 > maxX) && (x1 > maxX)) return false;
// 	if ((y2 < minY) && (y1 < minY)) return false;
// 	if ((y2 > maxY) && (y1 > maxY)) return false;
//
// 	if ((x1 > minX) && (x1 < maxX) &&
// 		(y1 > minY) && (y1 < maxY))
// 		return true;
//
// 	float x, y, z;
//
// 	if (getIntersection(x1 - minX, x2 - minX, x1, y1, z1, x2, y2, z2, x, y, z) &&
// 		inBox(x, y, z, minX, minY, minZ, maxX, maxY, maxZ, 1))
// 		return true;
// 	if (getIntersection(y1 - minY, y2 - minY, x1, y1, z1, x2, y2, z2, x, y, z) &&
// 		inBox(x, y, z, minX, minY, minZ, maxX, maxY, maxZ, 2))
// 		return true;
// 	if (getIntersection(z1 - minZ, z2 - minZ, x1, y1, z1, x2, y2, z2, x, y, z) &&
// 		inBox(x, y, z, minX, minY, minZ, maxX, maxY, maxZ, 3))
// 		return true;
// 	if (getIntersection(x1 - maxX, x2 - maxX, x1, y1, z1, x2, y2, z2, x, y, z) &&
// 		inBox(x, y, z, minX, minY, minZ, maxX, maxY, maxZ, 1))
// 		return true;
// 	if (getIntersection(y1 - maxY, y2 - maxY, x1, y1, z1, x2, y2, z2, x, y, z) &&
// 		inBox(x, y, z, minX, minY, minZ, maxX, maxY, maxZ, 2))
// 		return true;
// 	if (getIntersection(z1 - maxZ, z2 - maxZ, x1, y1, z1, x2, y2, z2, x, y, z) &&
// 		inBox(x, y, z, minX, minY, minZ, maxX, maxY, maxZ, 3))
// 		return true;
//
// 	return false;
// }

} // End of namespace Common
