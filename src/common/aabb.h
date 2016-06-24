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


#ifndef COMMON_AABBTREE_H
#define COMMON_AABBTREE_H

#include "src/common/boundingbox.h"
#include "src/common/types.h"

namespace Common {

class AABBNode : public BoundingBox {
public:
	AABBNode(float min[], float max[]);
	~AABBNode();

	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void setOrientation(uint8 orientation);
	void absolutize();

	void setProperty(int32 property);
	int32 getProperty() const;
	void setChildren(AABBNode *left, AABBNode *right);

	/** Get the intersecting leaf node in the XY plan. */
	AABBNode *getNode(float x, float y);
	/** Get the intersecting leaf node from this node and its children and a ray. */
	AABBNode *getNode(float x1, float y1, float z1, float x2, float y2, float z2);

	bool hasChildren() const;
	AABBNode *getLeft() const;
	AABBNode *getRight() const;

	// Helper methods
	bool isSelected() const;
	void select(bool select);

private:
	bool _hasChildren;

	AABBNode *_left;
	AABBNode *_right;

	int32 _property;
	bool _selected;


	friend class Walkmesh;
};

} // End of namespace Common

#endif // COMMON_AABBTREE_H
