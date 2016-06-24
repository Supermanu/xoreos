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
 *  The walkmesh related to a model.
 */

#include "src/graphics/graphics.h"

#include "src/common/aabb.h"
#include "src/common/ustring.h"
#include "src/common/util.h"

#include "src/graphics/aurora/walkmesh.h"

namespace Graphics {

namespace Aurora {

Walkmesh::Walkmesh(const Common::UString &nodeName, Common::AABBNode *aabb, std::vector<uint32> &faceProperty) :
    _nodeName(nodeName), _aabbTree(aabb), _facePropertyMap(faceProperty) {
}

Walkmesh::~Walkmesh() {
}

void Walkmesh::setWalkMap(std::vector<bool> &walkMap) {
	_walkMap = walkMap;
}

void Walkmesh::setPosition(float x, float y, float z) {
	_aabbTree->translate(x, y, z);
}

void Walkmesh::setOrientation(uint8 orientation) {
	_aabbTree->setOrientation(orientation);
}

bool Walkmesh::isIn(float x, float y) const {
	return _aabbTree->isIn(x, y);
}

bool Walkmesh::isIn(float x1, float y1, float z1, float x2, float y2, float z2) const {
	return _aabbTree->isIn(x1, y1, z1, x2, y2, z2);
}

bool Walkmesh::isWalkable(float x, float y) const {
	Common::AABBNode *leafNode = _aabbTree->getNode(x, y);

	if (!leafNode)
		return false;

	return _walkMap[leafNode->getProperty()];
}

bool Walkmesh::isWalkable(float x1, float y1, float z1, float x2, float y2, float z2, float &dist) const {
	Common::AABBNode *leafNode = _aabbTree->getNode(x1, y1, z1, x2, y2, z2);

	_aabbTree->select(false);
	if (!leafNode)
		return false;

	leafNode->select(true);

	// Compute squarred distance.
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	leafNode->getMin(minX, minY, minZ);
	leafNode->getMax(maxX, maxY, maxZ);

	float xP = (minX + maxX) / 2;
	float yP = (minY + maxY) / 2;
	float zP = (minZ + maxZ) / 2;

	dist = (xP - x1)*(xP - x1) + (yP - y1)*(yP - y1) + (zP - z1)*(zP - z1);

	return _walkMap[_facePropertyMap[leafNode->getProperty()]];
}

void Walkmesh::drawAABBs() {
	drawAABB(_aabbTree);
}

void Walkmesh::drawAABB(Common::AABBNode *aabbNode) {
	if (aabbNode->hasChildren()) {
		drawAABB(aabbNode->getLeft());
		drawAABB(aabbNode->getRight());
		return;
	}

	if (aabbNode->isSelected()) {
		glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
		glLineWidth(3.0f);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);
	}
	

	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	
	aabbNode->getRelativeMin(minX, minY, minZ);
	aabbNode->getRelativeMax(maxX, maxY, maxZ);

	glBegin(GL_LINE_LOOP);
	glVertex3f(minX, minY, minZ);
	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, maxY, minZ);
	glVertex3f(minX, maxY, minZ);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(minX, minY, maxZ);
	glVertex3f(maxX, minY, maxZ);
	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(minX, maxY, maxZ);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(minX, minY, minZ);
	glVertex3f(minX, maxY, minZ);
	glVertex3f(minX, maxY, maxZ);
	glVertex3f(minX, minY, maxZ);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, maxY, minZ);
	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(maxX, minY, maxZ);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(minX, minY, minZ);
	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, minY, maxZ);
	glVertex3f(minX, minY, maxZ);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(minX, maxY, minZ);
	glVertex3f(maxX, maxY, minZ);
	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(minX, maxY, maxZ);
	glEnd();
}

} // End of namespace Aurora

} // End of namespace Graphics
