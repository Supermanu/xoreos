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

/** @file engines/nwn/gui/chargen/charappearance.cpp
 *  The NWN appearance selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/widgets/button.h"

#include "engines/nwn/gui/chargen/charappearance.h"

namespace Engines {

namespace NWN {

CharAppearance::CharAppearance(Creature &character): _character(&character) {
	load("cg_appearance");

	getWidget("Title", true)->setHorCentered();

	init();
}

CharAppearance::~CharAppearance() {
}

void CharAppearance::show() {
	_character->setAppearance(_character->getRace());

	Engines::GUI::show();
}

void CharAppearance::hide() {
	_character->hide();

	Engines::GUI::hide();
}

void CharAppearance::reset() {
// 	_cloth = 0;
}

void CharAppearance::init() {
	setArrowButton("HeadButton");
	setArrowButton("TattooButton");
	setArrowButton("BodyButton");
	setArrowButton("ClothingButton");

	getButton("SkinButton", true)->setCaptionLeft();
	getButton("HairButton", true)->setCaptionLeft();
	getButton("TattooColButton", true)->setCaptionLeft();

	// Retrieve available clothes.
	const Aurora::TwoDAFile &twodaClothes = TwoDAReg.get("chargenclothes");
	for (uint it = 0; it < twodaClothes.getRowCount(); ++it)
		_clothes.push_back(twodaClothes.getRow(it).getString("ItemResRef"));

	_cloth = 0;
}

void CharAppearance::setArrowButton(const Common::UString &buttonName) {
	WidgetButton *button = getButton(buttonName, true);

	button->setCaptionLeft();

	float pX, pY, pZ;
	WidgetButton *leftArrow = new WidgetButton(*this, buttonName + "#Left", "ctl_cg_btn_left");
	button->getNodePosition("button1", pX, pY, pZ);
	leftArrow->setPosition(pX, pY, -101);

	WidgetButton *rightArrow = new WidgetButton(*this, buttonName + "#Right", "ctl_cg_btn_right");
	button->getNodePosition("button2", pX, pY, pZ);
	rightArrow->setPosition(pX, pY, -101);

	addWidget(rightArrow);
	addWidget(leftArrow);
}

void CharAppearance::changeCloth(uint8 cloth) {
	if (cloth == - 1)
		cloth = _clothes.size() - 1;

	_cloth = cloth % _clothes.size();
}

void CharAppearance::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		_charInfo = new CharInfo(*_character);
		if (sub(*_charInfo) == 2)
			_returnCode = 1;

		return;
	}

	if (widget.getTag().beginsWith("ClothingButton#")) {
		if (widget.getTag() == "ClothingButton#Right")
			changeCloth(_cloth + 1);
		else
			changeCloth(_cloth - 1);
	}
}

} // End of namespace NWN

} // End of namespace Engines

