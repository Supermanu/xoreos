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

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/widgets/editbox.h"
#include "engines/nwn/gui/widgets/label.h"

#include "engines/nwn/gui/chargen/charhelpinfo.h"

namespace Engines {

namespace NWN {

CharHelpInfo::CharHelpInfo(const Common::UString &guiName) : _icon(0) {
	load(guiName);

	setPosition(0, 0, -120);
	_icon = new PortraitWidget(*this, "FeatIcon", "", Portrait::kSizeIcon);
	getEditBox("EditBox", true)->addChild(*_icon);
	float pX, pY, pZ;
	getEditBox("EditBox", true)->getParent()->getPosition(pX, pY, pZ);
	
	// The position is hardcoded, there is no icon node in the featInfo model.
	// From what I saw, the aurora engine use another panel which has a positional node.

	_icon->setPosition(-200, 145, pZ - 10);

	if (guiName == "cg_featinfo")
		getWidget("OkButton", true)->setInvisible(true);

	
}

CharHelpInfo::~CharHelpInfo() {
}

void CharHelpInfo::setInfo(const Common::UString &feat, const Common::UString &featDescription, const Common::UString &icon) {
	// Title.
	getLabel("Title", true)->setText(feat);
	// Description.
	getEditBox("EditBox", true)->setMainText(featDescription);
	// Icon.
	_icon->setPortrait(icon);
}

void CharHelpInfo::callbackActive(Widget &widget) {
	if (widget.getTag() == "CloseButton")
		_returnCode = 1;

}

} // End of namespace NWN

} // End of namespace Engines
