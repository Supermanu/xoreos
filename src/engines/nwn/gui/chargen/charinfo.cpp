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
 */

/** @file engines/nwn/gui/chargen/charinfo.cpp
 *  The NWN character's informations for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/aurora/gui.h"

#include "engines/nwn/gui/widgets/listbox.h"
#include "engines/nwn/gui/widgets/editbox.h"

#include "engines/nwn/gui/chargen/charinfo.h"

namespace Engines {

namespace NWN {

CharInfo::CharInfo(Creature &character): _character(&character) {
	load("cg_char_info");
	getWidget("Title", true)->setHorCentered();

	///TODO Deity and random button.
	getWidget("RandomButton", true)->setDisabled(true);
	getWidget("DeityButton", true)->setDisabled(true);
}

CharInfo::~CharInfo() {}

void CharInfo::show() {
	///TODO Fill default text.
	getEditBox("DescEdit", true)->setMainText("Once upon a time...");
	getEditBox("DescEdit", true)->setMode(WidgetEditBox::kModeEditable);
	getEditBox("FirstNameEdit", true)->setMainText("John");
	getEditBox("FirstNameEdit", true)->setMode(WidgetEditBox::kModeEditable);
	getEditBox("LastNameEdit", true)->setMainText("Doe");
	getEditBox("LastNameEdit", true)->setMode(WidgetEditBox::kModeEditable);

	Engines::GUI::show();
}

void CharInfo::reset() {

}

void CharInfo::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "DescEdit")
		type = NWN::GUI::kWidgetTypeEditBox;
}

void CharInfo::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {

		_returnCode = 2;
		return;
	}
}

} // End of namespace NWN

} // End of namespace Engines
