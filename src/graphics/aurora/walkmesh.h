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

#ifndef GRAPHICS_AURORA_WALKMESH_H
#define GRAPHICS_AURORA_WALKMESH_H

namespace Common {
class AABBTree;
class AABBNode;
class UString;
}

namespace Graphics {

namespace Aurora {

class Walkmesh {

public:
	Walkmesh(const Common::UString &nodeName, Common::AABBNode *aabb, std::vector<uint32> &faceProperty);
	~Walkmesh();

	void setWalkMap(std::vector<bool> &walkMap);

	void setPosition(float x, float y, float z);
	void setOrientation(uint8 orientation);

	bool isIn(float x, float y) const;
	bool isIn(float x1, float y1, float z1, float x2, float y2, float z2) const;

	/** Wether or not a point in the XY plan is walkable. */
	bool isWalkable(float x, float y) const;
	/** Check walkability from the intersection between a ray and the walkmesh. */
	bool isWalkable(float x1, float y1, float z1, float x2, float y2, float z2, float &dist) const;

	void drawAABBs();

private:
	void drawAABB(Common::AABBNode *aabbNode);

	Common::UString _nodeName;
	Common::AABBNode *_aabbTree;
	std::vector<uint32> _facePropertyMap;
	std::vector<bool>   _walkMap;
};

}

}

#endif // GRAPHICS_AURORA_ALKMESH_H
