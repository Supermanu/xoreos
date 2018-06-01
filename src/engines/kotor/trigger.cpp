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
 *  Trigger in a Star Wars: Knights of the Old Republic area.
 */

#include "src/aurora/resman.h"
#include "src/aurora/gff3file.h"

#include "src/engines/kotor/trigger.h"

namespace Engines {

namespace KotOR {

Trigger::Trigger(const Aurora::GFF3Struct &gff)
		: ::Engines::Trigger() {
	load(gff);
}

void Trigger::show() {
	Renderable::show();
}

void Trigger::hide() {
	Renderable::hide();
}

void Trigger::hideSoft() {
}

bool Trigger::isVisible() const {
	return Renderable::isVisible();
}

void Trigger::load(const Aurora::GFF3Struct &gff) {
	Common::UString resRef = gff.getString("TemplateResRef");
	if (!resRef.empty())
		loadBlueprint(resRef);

	float x, y, z;
	x = (float)gff.getDouble("XPosition");
	y = (float)gff.getDouble("YPosition");
	z = (float)gff.getDouble("ZPosition");
	glm::vec3 position(x, y, z);

	x = (float)gff.getDouble("XOrientation");
	y = (float)gff.getDouble("YOrientation");
	z = (float)gff.getDouble("ZOrientation");
	glm::vec3 orientation(x, y, z);

	const Aurora::GFF3List &geometry = gff.getList("Geometry");
	for (Aurora::GFF3List::const_iterator p = geometry.begin();
			p != geometry.end();
			++p) {
		x = (float)(*p)->getDouble("PointX");
		y = (float)(*p)->getDouble("PointY");
		z = (float)(*p)->getDouble("PointZ");
		_geometry.push_back(position + glm::vec3(x, y, z));
	}
}

void Trigger::loadBlueprint(const Common::UString &resRef) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(ResMan.getResource(resRef, Aurora::kFileTypeUTT));
	if (stream) {
		Aurora::GFF3File gff(stream.release());
		readScripts(gff.getTopLevel());
	}
}

} // End of namespace KotOR

} // End of namespace Engines
