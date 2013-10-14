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

#include "engines/nwn/gui/chargen/charsex.h"



namespace Engines {

namespace NWN {


CharSex::CharSex(Module &module, Creature &character) : _module(&module), _character(&character) {
	load("cg_sex");

	getWidget("Title"		, true)->setHorCentered();

	_genderWidgets = new  std::map<Common::UString, WidgetButton *>;
	_genderWidgets->insert(std::pair<Common::UString, WidgetButton *>("MaleButton", ((WidgetButton *) getWidget("MaleButton"	, true))));
	_genderWidgets->insert(std::pair<Common::UString, WidgetButton *>("FemaleButton", ((WidgetButton *) getWidget("FemaleButton"	, true))));


	_genderWidgets->at("MaleButton")->setStayPressed();
	_genderWidgets->at("FemaleButton")->setStayPressed();
	_gender = _character->getGender();
	if (_gender == Aurora::kGenderFemale) {
		_genderWidgets->at("FemaleButton")->setPressed();
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
		if (_gender != _character->getGender())
			swapGender();
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		_character->setGender(_gender);
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "FemaleButton") {
		if (_gender == Aurora::kGenderFemale) {
			return;
		}
		swapGender();
		return;
	}

	if (widget.getTag() == "MaleButton") {
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
	} else if (_gender == Aurora::kGenderFemale) {
		_gender = Aurora::kGenderMale;
		_genderWidgets->at("MaleButton")->setPressed(true);
		_genderWidgets->at("FemaleButton")->setPressed(false);
	}
}



} // End of namespace NWN

} // End of namespace Engines
