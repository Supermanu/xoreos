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

/** @file engines/nwn/gui/chargen/charschool.h
 *  The NWN magic schools selection for the character generator.
 */

#include "aurora/talkman.h"

#include "engines/nwn/gui/widgets/editbox.h"

#include "engines/nwn/gui/chargen/charschool.h"


namespace Engines {

namespace NWN {

CharSchool::CharSchool() {
	load("cg_school");
	getWidget("Title", true)->setHorCentered();

	_helpBox = getEditBox("HelpBox", true);

	initButtons();

	// Set defaults.
	_helpBox->setMainText(TalkMan.getString(381));
	_helpBox->setTitle(TalkMan.getString(381));
}

CharSchool::~CharSchool() {
}

void CharSchool::reset() {
	_characterChoices.school = 0;
}

void CharSchool::initButtons() {
	// Get school buttons.
	_buttonList.push_back(getButton("General", true));
	_buttonList.push_back(getButton("Abjuration", true));
	_buttonList.push_back(getButton("Conjuration", true));
	_buttonList.push_back(getButton("Divination", true));
	_buttonList.push_back(getButton("Enchantment", true));
	_buttonList.push_back(getButton("Evocation", true));
	_buttonList.push_back(getButton("Illusion", true));
	_buttonList.push_back(getButton("Necromancy", true));
	_buttonList.push_back(getButton("Transmutation", true));

	for (std::vector<WidgetButton *>::iterator it = _buttonList.begin(); it != _buttonList.end(); ++it)
		(*it)->setMode(WidgetButton::kButtonModeStayPressed);

	_buttonList[0]->setPressed(true);

	// Get help texts.
	for (uint it = 0; it < 9; ++it)
		_helpTexts.push_back(TalkMan.getString(10320 + it));
}

void CharSchool::callbackActive(Widget &widget) {
	if (widget.getTag() == "OkButton") {
		for (uint it = 0; it < _buttonList.size(); ++it) {
			if (_buttonList[it]->isPressed()){
				_characterChoices.school = it;
				break;
			}
		}

		_returnCode = 2;
		return;
	}
	
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
	}

	for (uint in = 0; in < _buttonList.size(); ++in) {
		if (_buttonList[in]->getTag() == widget.getTag()) {
			_helpBox->setMainText(_helpTexts[in]);
			_helpBox->setTitle(_buttonList[in]->getCaption());
			for (std::vector<WidgetButton *>::iterator it = _buttonList.begin(); it != _buttonList.end(); ++it)
				(*it)->setPressed(false);

			_buttonList[in]->setPressed(true);
			break;
		}
	}
}

} // End of namespace NWN

} // End of namespace Engines
