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

/** @file engines/nwn/gui/chargen/charclass.cpp
 *  The NWN class selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "common/ustring.h"

#include "graphics/aurora/model.h"
#include "graphics/aurora/text.h"

#include "engines/aurora/model.h"

#include "engines/nwn/gui/widgets/button.h"
#include "engines/nwn/gui/chargen/charclass.h"

namespace Engines {

namespace NWN {

CharClass::CharClass(Creature &character) : _character(&character), _alreadyChosen(false) {
	load("cg_class");
	getWidget("Title", true)->setHorCentered();

	_helpBox       = getEditBox("HelpBox", true);
	_classesWidget = getListBox("ClassListBox", true);

	createClassList();

	///TODO Recommend button.
	getButton("RecommendButton", true)->setDisabled(true);
}

CharClass::~CharClass() {}

void CharClass::reset() {
	_alreadyChosen = false;
	_classesWidget->select(0);
	_characterChoices.classFeats.clear();
}


void CharClass::fixWidgetType(const Common::UString &tag, WidgetType &type) {
	if (tag == "ClassListBox")
		type = NWN::GUI::kWidgetTypeListBox;
}

void CharClass::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		uint chosenClass = _classesWidget->getSelected();
		// Fill the gap between normal class and prestige class in the 2da file.
		if (chosenClass > 10)
			chosenClass += 16;

		_characterChoices.setClass(chosenClass);

		const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
		const Aurora::TwoDAFile &twodaBaB     = TwoDAReg.get(twodaClasses.getRow(_characterChoices.classID).getString("AttackBonusTable"));
		uint16 newClassLevel = _characterChoices.character->getClassLevel(_characterChoices.classID);
		uint8  bABChange     = twodaBaB.getRow(newClassLevel).getInt("BAB");
		if (newClassLevel)
			bABChange -= twodaBaB.getRow(newClassLevel - 1).getInt("BAB");

		_characterChoices.baseAttackBonusChange = bABChange;

		_returnCode = 2;
		return;
	}

	if (widget.getTag() == "ClassListBox") {
		_helpBox->setMainText(_helpTexts[_classesWidget->getSelected()]);
		_helpBox->setTitle(_className[_classesWidget->getSelected()]);
		if (!((WidgetListItemButton *) _classesWidget->getItem(_classesWidget->getSelected()))->isAvailable())
			getButton("OkButton", true)->setDisabled(true);
		else
			getButton("OkButton", true)->setDisabled(false);
	}
}

void CharClass::createClassList() {
	_classesWidget->lock();
	_classesWidget->clear();
	_classesWidget->setMode(WidgetListBox::kModeSelectable);

	const Aurora::TwoDAFile &twoda = TwoDAReg.get("classes");
	for (uint it = 0; it < twoda.getRowCount(); it++) {
		const Aurora::TwoDARow &row = twoda.getRow(it);
		if (row.getInt("PlayerClass") == 0)
			continue;

		_className.push_back(TalkMan.getString(row.getInt("Name")));
		_helpTexts.push_back(TalkMan.getString(row.getInt("Description")));

		WidgetListItemButton *itemClass = new WidgetListItemButton(*this, "ctl_cg_btn_class", "fnt_galahad14", _className.back(), row.getString("Icon"));

		_classesWidget->add(itemClass);

		if (row.getInt("EpicLevel") > 0)
			itemClass->setDisabled(!checkPrestigeCondition());

	}
	_classesWidget->unlock();
	_classesWidget->select(0);
	_helpBox->setMainText(_helpTexts[0]);
	_helpBox->setTitle(_className[0]);
}

bool CharClass::checkPrestigeCondition() {
	///TODO Implement a real check.
	if (_character->getLevel() == 0)
		return false;
	else
		return true;
}

} // End of namespace NWN

} // End of namespace Engines
