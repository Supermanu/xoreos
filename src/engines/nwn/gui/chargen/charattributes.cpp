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

/** @file engines/nwn/gui/chargen/charattributes.h
 *  The NWN attributes selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/widgets/textwidget.h"
#include "engines/nwn/gui/widgets/label.h"
#include "engines/nwn/gui/widgets/editbox.h"

#include "engines/nwn/gui/chargen/charattributes.h"

namespace Engines {

namespace NWN {

CharAttributes::CharAttributes(Creature &character) : _character(&character), _pointLeft(30) {
	load("cg_attributes");
	getWidget("Title", true)->setHorCentered();

	getButton("OkButton", true)->setDisabled(true);

	init();
}

CharAttributes::~CharAttributes() {}

void CharAttributes::show() {
	// Check attribute adjustment from racial type.
	const Aurora::TwoDAFile &twoda = TwoDAReg.get("racialtypes");
	const Aurora::TwoDARow  &row   = twoda.getRow(_character->getRace());
	_attrAdjust.clear();
	_attrAdjust.push_back(row.getInt(8));
	_attrAdjust.push_back(row.getInt(9));
	_attrAdjust.push_back(row.getInt(13));
	_attrAdjust.push_back(row.getInt(12));
	_attrAdjust.push_back(row.getInt(10));
	_attrAdjust.push_back(row.getInt(11));

	for (uint8 it = 0; it < 6; ++it) {
		genTextAttributes(it);
		updateText(it);
	}

	Engines::GUI::show();
}

void CharAttributes::reset() {
	_pointLeft = 30;
	for (uint8 it = 0; it < _attrAdjust.size(); ++it) {
		_attributes[it] = 8;
		updateText(it);
	}

	getButton("CostEdit", true)->setCaption("1");
}

void CharAttributes::init() {
	// Init attribute values.
	_attributes.assign(6, 8);

	// Init point left.
	getButton("PointsEdit", true)->setCaption("30");
	getButton("PointsEdit", true)->moveCaptionPosition(-7, -7, 0);
	getButton("CostEdit",   true)->setCaption("1");
	getButton("CostEdit",   true)->moveCaptionPosition(-7, -7, 0);

	// Init buttons.
	_attrButtons.push_back(getButton("StrLabel", true));
	_attrButtons.push_back(getButton("DexLabel", true));
	_attrButtons.push_back(getButton("ConLabel", true));
	_attrButtons.push_back(getButton("WisLabel", true));
	_attrButtons.push_back(getButton("IntLabel", true));
	_attrButtons.push_back(getButton("ChaLabel", true));

	for (uint it = 0; it < 6; ++it) {
		_attrButtons[it]->setMode(WidgetButton::kButtonModeUnchanged);
		_attrButtons[it]->setCaptionLeft();
		_attrButtons[it]->moveCaptionPosition(30, 0, 0);

		TextWidget *attrValue = new TextWidget(*this, _attrButtons[it]->getTag() + "#AttrValue", "fnt_galahad14", "");
		_attrButtons[it]->addChild(*attrValue);
		attrValue->setVerCentered();
		attrValue->movePosition(250, -7, -80);
	}

	// Init help texts.
	getEditBox("HelpEdit", true)->setTitle(TalkMan.getString(261));
	getEditBox("HelpEdit", true)->setMainText(TalkMan.getString(457));
	for (uint it = 459; it < 464; ++it)
		_helpTexts.push_back(TalkMan.getString(it));

	_helpTexts.push_back(TalkMan.getString(478));
}

const Common::UString CharAttributes::genTextAttributes(uint8 attribute) {
	uint realValue = _attributes[attribute] + _attrAdjust.at(attribute);
	realValue -= 6;
	int modifier = (realValue - realValue % 2) / 2;
	modifier -= 2;

	Common::UString    sign = (modifier < 0) ? "" : "+";
	std::ostringstream convertAttr;
	convertAttr << _attributes[attribute] + _attrAdjust.at(attribute);
	std::ostringstream convertModif;
	convertModif << modifier;
	const Common::UString result(convertAttr.str() + " (" + sign + convertModif.str() + ")");
	return result;
}

void CharAttributes::updateText(uint8 attribute) {
	TextWidget *attrValue = (TextWidget *) _attrButtons[attribute]->getChild(_attrButtons[attribute]->getTag() + "#AttrValue");
	attrValue->setText(genTextAttributes(attribute));

	std::ostringstream convert;
	convert << _pointLeft;
	getButton("PointsEdit", true)->setCaption(convert.str());
}

void CharAttributes::changeHelp(uint8 attribute) {
	getEditBox("HelpEdit", true)->setTitle(_attrButtons[attribute]->getCaption());
	getEditBox("HelpEdit", true)->setMainText(_helpTexts[attribute]);
}

uint CharAttributes::pointCost(uint8 attributeValue) {
	if (attributeValue < 15) {
		getButton("CostEdit", true)->setCaption("1");
		return 1;
	}

	attributeValue -= 15;
	uint cost = ((attributeValue - attributeValue % 2) / 2) + 2;

	if (cost > 3)
		return cost;

	std::ostringstream convert;
	convert << cost;
	    getButton("CostEdit", true)->setCaption(convert.str());

	return cost;
}

void CharAttributes::setRecommend() {
	_pointLeft = 0;
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow  &row = twodaClasses.getRow(_characterChoices.classID);
	for (uint it = 0; it < 6; ++it) {
		_attributes.at(it) = row.getInt(17 + it);
		updateText(it);
	}

	getButton("OkButton", true)->setDisabled(false);
}

void CharAttributes::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		for (int8 it = 0; it < kAbilityMAX; ++it)
			_character->setAbility(it, _attributes[it]);

		_returnCode = 2;
		return;
	}

	if (widget.getTag() == "RecommendButton") {
		setRecommend();
		return;
	}

	for (uint8 it = 0; it < 6; ++it) {
		if (widget.getTag() == _attrButtons[it]->getTag()) {
			changeHelp(it);
			pointCost(_attributes[it]);
		}
	}

	for (uint8 it = 0; it < 6; ++it) {
		if (_attrButtons[it]->getTag() == widget.getParent()->getTag()) {
			changeHelp(it);
			if (widget.getTag().contains("Up")) {
				uint cost = pointCost(_attributes[it] + 1);
				if ((_attributes[it] == 18) || (cost > _pointLeft))
					break;

				_pointLeft -= cost;
				_attributes[it]++;
			} else if (widget.getTag().contains("Down")) {
				if (_attributes[it] == 8)
					break;

				_pointLeft += pointCost(_attributes[it]);
				_attributes[it]--;
			}

			updateText(it);
			if (_pointLeft == 0)
				getButton("OkButton", true)->setDisabled(false);
			else
				getButton("OkButton", true)->setDisabled(true);
		}
	}
}

} // End of namespace NWN

} // End of namespace Engines
