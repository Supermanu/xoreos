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
#include "src/graphics/detour/DetourNavMesh.h"
#include "src/graphics/detour/DebugDraw.h"
#include "src/graphics/detour/DetourDebugDraw.h"


class GLCheckerTextureB
{
	unsigned int m_texId;
public:
	GLCheckerTextureB() : m_texId(0)
	{
	}
	
	~GLCheckerTextureB()
	{
		if (m_texId != 0)
			glDeleteTextures(1, &m_texId);
	}
	void bind()
	{
		if (m_texId == 0)
		{
			// Create checker pattern.
			const unsigned int col0 = duRGBA(215,215,215,255);
			const unsigned int col1 = duRGBA(255,255,255,255);
			static const int TSIZE = 64;
			unsigned int data[TSIZE*TSIZE];
			
			glGenTextures(1, &m_texId);
			glBindTexture(GL_TEXTURE_2D, m_texId);

			int level = 0;
			int size = TSIZE;
			while (size > 0)
			{
				for (int y = 0; y < size; ++y)
					for (int x = 0; x < size; ++x)
						data[x+y*size] = (x==0 || y==0) ? col0 : col1;
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size,size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				size /= 2;
				level++;
			}
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_texId);
		}
	}
};
GLCheckerTextureB g_texa;

void DebugDrawGLB::depthMask(bool state)
{
	glDepthMask(state ? GL_TRUE : GL_FALSE);
}

void DebugDrawGLB::texture(bool state)
{
	if (state)
	{
		glEnable(GL_TEXTURE_2D);
		g_texa.bind();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}

void DebugDrawGLB::begin(duDebugDrawPrimitives prim, float size)
{
	switch (prim)
	{
		case DU_DRAW_POINTS:
			glPointSize(size);
			glBegin(GL_POINTS);
			break;
		case DU_DRAW_LINES:
			glLineWidth(size);
			glBegin(GL_LINES);
			break;
		case DU_DRAW_TRIS:
			glBegin(GL_TRIANGLES);
			break;
		case DU_DRAW_QUADS:
			glBegin(GL_QUADS);
			break;
	};
}

void DebugDrawGLB::vertex(const float* pos, unsigned int color)
{
//	warning("vertex: %f, %f, %f", pos[0], pos[1], pos[2]);
	glColor4ubv((GLubyte*)&color);
	glVertex3fv(pos);
}

void DebugDrawGLB::vertex(const float x, const float y, const float z, unsigned int color)
{
	glColor4ubv((GLubyte*)&color);
	glVertex3f(x,y,z);
}

void DebugDrawGLB::vertex(const float* pos, unsigned int color, const float* uv)
{
	glColor4ubv((GLubyte*)&color);
	glTexCoord2fv(uv);
	glVertex3fv(pos);
}

void DebugDrawGLB::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	glColor4ubv((GLubyte*)&color);
	glTexCoord2f(u,v);
	glVertex3f(x,y,z);
}

void DebugDrawGLB::end()
{
	glEnd();
	glLineWidth(1.0f);
	glPointSize(1.0f);
}

namespace Graphics {

namespace Aurora {

Walkmesh::Walkmesh(const Common::UString &nodeName, Common::AABBNode *aabb, std::vector<uint32> &faceProperty, dtNavMesh *navMesh) :
    _nodeName(nodeName), _aabbTree(aabb), _facePropertyMap(faceProperty), _navMesh(navMesh) {
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

void Walkmesh::drawNavMesh() {
	if (!_navMesh)
		return;

	DebugDrawGLB dd;
//	glScalef(0.01, 0.01, 0.01);
	duDebugDrawNavMesh(&dd, *_navMesh, 0);
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
