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

/** @file engines/nwn/gui/chargen/charalignment.cpp
 *  The NWN alignment selection for the character generator.
 */

#include <sstream>
#include <iostream>

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/chargen/charalignment.h"
#include "engines/nwn/gui/widgets/editbox.h"

namespace Engines {

namespace NWN {

CharAlignment::CharAlignment(Creature &character) : _character(&character), _goodness(0), _loyalty(0) {
	load("cg_alignment");
	getWidget("Title", true)->setHorCentered();

	///TODO Recommended button for the alignment.
	getButton("RecommendButton", true)->setDisabled(true);

	createAlignmentList();

	// Set default texts for the helpbox.
	getEditBox("HelpEdit", true)->setTitle(TalkMan.getString(111));
	getEditBox("HelpEdit", true)->setMainText(TalkMan.getString(458));

	getButton("OkButton", true)->setDisabled(true);
}

CharAlignment::~CharAlignment() {
}

void CharAlignment::reset() {
	for (uint it = 0; it < _alignmentButtons.size(); ++it)
		_alignmentButtons[it]->setPressed(false);

	getButton("OkButton", true)->setDisabled(true);
}

void CharAlignment::show() {
	setRestriction();

	Engines::GUI::show();
}

void CharAlignment::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		_character->setGoodEvil(_goodness);
		_character->setLawChaos(_loyalty);
		_returnCode = 2;
		return;
	}

	if (widget.getTag().endsWith("EButton") || widget.getTag().endsWith("GButton") || widget.getTag().endsWith("NButton")) {
		setAlignment((WidgetButton *) &widget);
		getButton("OkButton", true)->setDisabled(false);
	}
}

void CharAlignment::createAlignmentList() {
	// Retrieve help texts.
	for (uint it = 448; it < 457; ++it)
		_alignmentTexts.push_back(TalkMan.getString(it));

	// Get alignment buttons.
	_alignmentButtons.push_back(getButton("CEButton", true));
	_alignmentButtons.push_back(getButton("CGButton", true));
	_alignmentButtons.push_back(getButton("CNButton", true));
	_alignmentButtons.push_back(getButton("LEButton", true));
	_alignmentButtons.push_back(getButton("LGButton", true));
	_alignmentButtons.push_back(getButton("LNButton", true));
	_alignmentButtons.push_back(getButton("NEButton", true));
	_alignmentButtons.push_back(getButton("NGButton", true));
	_alignmentButtons.push_back(getButton("TNButton", true));

	// Ask alignment buttons to stay pressed and retrieve alignment names.
	for (std::vector<WidgetButton *>::iterator it = _alignmentButtons.begin(); it != _alignmentButtons.end(); ++it) {
		(*it)->setMode(WidgetButton::kButtonModeStayPressed);
		_alignmentName.push_back((*it)->getCaption());
	}
}

void CharAlignment::setAlignment(WidgetButton *button) {
	for (uint it = 0; it < _alignmentButtons.size(); ++it) {
		if (_alignmentButtons[it] != button)
			_alignmentButtons[it]->setPressed(false);
		else {
			_alignmentButtons[it]->setPressed(true);
			getEditBox("HelpEdit", true)->setTitle(_alignmentName[it]);
			getEditBox("HelpEdit", true)->setMainText(_alignmentTexts[it]);

			float point[3] = { 0, 100, 50 };
			uint  rest     = it % 3;
			_goodness = point[rest];
			_loyalty  = point[(it - rest) / 3];
		}
	}
}

void CharAlignment::setRestriction() {
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow  &row = twodaClasses.getRow(_characterChoices.classID);
	std::stringstream       ss;
	ss << std::hex << row.getString("AlignRestrict").c_str();

	int alignRestrict;
	ss >> alignRestrict;
	bool invertRestrict = row.getInt("InvertRestrict");

	int  axisRestrict[5] = { 0x01, 0x02, 0x04, 0x08, 0x10 };
	uint good[]  = { 1, 4, 7 };
	uint evil[]  = { 0, 3, 6 };
	uint loyal[] = { 3, 4, 5 };
	uint chaos[] = { 0, 1, 2 };
	std::vector<uint> goodAxis(good, good + sizeof(good) / sizeof(uint));
	std::vector<uint> evilAxis(evil, evil + sizeof(evil) / sizeof(uint));
	std::vector<uint> loyalAxis(loyal, loyal + sizeof(loyal) / sizeof(uint));
	std::vector<uint> chaosAxis(chaos, chaos + sizeof(chaos) / sizeof(uint));

	std::vector<uint> neutralAxis;
	neutralAxis.push_back(8);
	ss.clear();
	ss << std::hex << row.getString("AlignRstrctType").c_str();
	int alignRstrctType;
	ss >> alignRstrctType;
	if (alignRstrctType == 0x1) {
		neutralAxis.push_back(6);
		neutralAxis.push_back(7);
	} else if (alignRstrctType == 0x2) {
		neutralAxis.push_back(2);
		neutralAxis.push_back(5);
	} else if (alignRstrctType == 0x3) {
		neutralAxis.push_back(6);
		neutralAxis.push_back(7);
		neutralAxis.push_back(2);
		neutralAxis.push_back(5);
	}
	std::vector<std::vector<uint> > axis;
	axis.push_back(neutralAxis);
	axis.push_back(loyalAxis);
	axis.push_back(chaosAxis);
	axis.push_back(goodAxis);
	axis.push_back(evilAxis);

	for (std::vector<WidgetButton *>::iterator it = _alignmentButtons.begin(); it != _alignmentButtons.end(); ++it)
		(*it)->setDisabled(invertRestrict);

	for (uint it = 0; it < 5; ++it) {
		if (axisRestrict[it] & alignRestrict) {
			for (std::vector<uint>::iterator ax = axis[it].begin(); ax != axis[it].end(); ++ax)
				_alignmentButtons[*ax]->setDisabled(!invertRestrict);
		}
	}
}

} // End of namespace NWN

} // End of namespace Engines
