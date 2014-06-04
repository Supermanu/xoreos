/* xoreos - A reimplementation of BioWare's Aurora engine
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

/** @file engines/nwn/gui/chargen/charsex.cpp
 *  The NWN gender selection for the character generator.
 */

#include "engines/nwn/gui/chargen/charsex.h"

#include "aurora/talkman.h"



namespace Engines {

namespace NWN {


CharSex::CharSex(Creature &character) : _character(&character) {
	load("cg_sex");

	getWidget("Title"		, true)->setHorCentered();

	_helpTexts.push_back(TalkMan.getString(199));
	_helpTexts.push_back(TalkMan.getString(200));
	_helpTexts.push_back(TalkMan.getString(447));
	
	_helpBox = getEditBox("HelpBox", true);
	
	_helpBox->setTitle(TalkMan.getString(203));
	_helpBox->setMainText(_helpTexts[2]);
	
	_genderWidgets = new  std::map<Common::UString, WidgetButton *>;
	_genderWidgets->insert(std::pair<Common::UString, WidgetButton *>("MaleButton", ((WidgetButton *) getWidget("MaleButton"	, true))));
	_genderWidgets->insert(std::pair<Common::UString, WidgetButton *>("FemaleButton", ((WidgetButton *) getWidget("FemaleButton"	, true))));

	_genderWidgets->at("MaleButton")->setMode(WidgetButton::kButtonModeStayPressed);
	_genderWidgets->at("FemaleButton")->setMode(WidgetButton::kButtonModeStayPressed);
	_gender = _character->getGender();
	if (_gender == Aurora::kGenderFemale) {
		_genderWidgets->at("FemaleButton")->setPressed();
		_helpBox->setMainText(_helpTexts[1]);
	} else if (_gender == Aurora::kGenderMale){
		_genderWidgets->at("MaleButton")->setPressed();
		_gender = Aurora::kGenderMale;
		_helpBox->setMainText(_helpTexts[0]);
	} else {
		_genderWidgets->at("MaleButton")->setPressed();
		_gender = Aurora::kGenderMale;
	}
}

CharSex::~CharSex() {
	delete _genderWidgets;
}

void CharSex::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_helpBox->setMainText(_helpTexts[2]);
		if (_gender != _character->getGender())
			swapGender();
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		_helpBox->setMainText(_helpTexts[2]);
		_character->setGender(_gender);
		_returnCode = 2;
		return;
	}

	if (widget.getTag() == "FemaleButton") {
		_helpBox->setMainText(_helpTexts[1]);
		if (_gender == Aurora::kGenderFemale) {
			return;
		}
		swapGender();
		return;
	}

	if (widget.getTag() == "MaleButton") {
		_helpBox->setMainText(_helpTexts[0]);
		if (_gender == Aurora::kGenderMale)
			return;

		swapGender();
		return;
	}
}

void CharSex::swapGender() {
	if (_gender == Aurora::kGenderMale) {
		_gender = Aurora::kGenderFemale;
		_genderWidgets->at("MaleButton")->setPressed(false);
		_genderWidgets->at("FemaleButton")->setPressed(true);
	} else {
		_gender = Aurora::kGenderMale;
		_genderWidgets->at("MaleButton")->setPressed(true);
		_genderWidgets->at("FemaleButton")->setPressed(false);
	}
}

void CharSex::reset() {
	_genderWidgets->at("MaleButton")->setPressed();
	_gender = Aurora::kGenderMale;
}



} // End of namespace NWN

} // End of namespace Engines
