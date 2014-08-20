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

/** @file engines/nwn/gui/chargen/charpackage.cpp
 *  The NWN package selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/widgets/editbox.h"
#include "engines/nwn/gui/widgets/button.h"
#include "engines/nwn/gui/chargen/charpackage.h"

namespace Engines {

namespace NWN {

CharPackage::CharPackage(Creature &character): _character(&character), _hasTakenPackage(false), _charSkills(0) {
	load("cg_package");
	getWidget("Title", true)->setHorCentered();

	_helpBox = getEditBox("HelpBox", true);

	///TODO Recommend button for CharPackage.
	getButton("RecommendButton", true)->setDisabled(true);
}

CharPackage::~CharPackage() {
	delete _charSkills;
}

void CharPackage::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "ClassListBox")
		type = NWN::GUI::kWidgetTypeListBox;
}

void CharPackage::show() {
	// Create package list.
	_helpTexts.clear();
	_packageNames.clear();
	_packageList = getListBox("ClassListBox", true);
	uint16 package = 0;
	if (_hasTakenPackage)
		package = _packageList->getSelected();

	_packageList->lock();
	_packageList->clear();
	_packageList->setMode(WidgetListBox::kModeSelectable);

	const Aurora::TwoDAFile &twoda = TwoDAReg.get("packages");
	for (uint16 it = 0; it < twoda.getRowCount(); ++it) {
		const Aurora::TwoDARow &row = twoda.getRow(it);
		if (row.getInt("PlayerClass") == 0 || row.getInt("ClassID") != (int) _characterChoices.classID || row.getInt("Name") == 0)
			continue;

		_helpTexts.push_back(TalkMan.getString(row.getInt("Description")));
		_packageNames.push_back(TalkMan.getString(row.getInt("Name")));
		WidgetListItemButton *packagetItem = new WidgetListItemButton(*this, "ctl_cg_btn_class", "fnt_galahad14", _packageNames.back(), "");
		_packageList->add(packagetItem);
	}
	_packageList->unlock();
	_packageList->select(package);
	if (_hasTakenPackage) {
		_helpBox->setTitle(TalkMan.getString(483));
		_helpBox->setMainText(TalkMan.getString(487));
	} else {
		_helpBox->setTitle(_packageNames[package]);
		_helpBox->setMainText(_helpTexts[package]);
	}

	Engines::GUI::show();
}

void CharPackage::reset() {
	_hasTakenPackage = false;
}

void CharPackage::applyPackage() {
	uint16 package = _packageList->getSelected();
	// We need to find back the appropiate row in the 2da file.
	const Aurora::TwoDAFile &twodaPck = TwoDAReg.get("packages");
	uint count = 0;
	Common::UString packageFiles[3];
	for (uint16 it = 0; it < twodaPck.getRowCount(); ++it) {
		const Aurora::TwoDARow &row = twodaPck.getRow(it);
		if (row.getInt("PlayerClass") == 0 || row.getInt("ClassID") != (int) _characterChoices.classID || row.getInt("Name") == 0)
			continue;

		if (count == package) {
			packageFiles[0] = row.getString("FeatPref2DA");
			packageFiles[0].tolower();
			packageFiles[1] = row.getString("SkillPref2DA");
			packageFiles[1].tolower();
			packageFiles[2] = row.getString("Equip2DA");
			packageFiles[2].tolower();
			_character->setPackage(it);
			break;
		}

		count++;
	}

	// We now have to apply the package.

	//Racial feats:
	const Aurora::TwoDAFile &twodaRace = TwoDAReg.get("racialtypes");
	const Aurora::TwoDAFile &twodaFeatRace = TwoDAReg.get(twodaRace.getRow(_character->getRace()).getString("FeatsTable"));
	for (uint8 it = 0; it < twodaFeatRace.getRowCount(); ++it)
		_characterChoices.setFeat(twodaFeatRace.getRow(it).getInt("FeatIndex"));

	// Normal feats:
	const Aurora::TwoDAFile &twodaFeatPackage = TwoDAReg.get(packageFiles[0]);
	_characterChoices.setFeat(twodaFeatPackage.getRow(0).getInt("FEATINDEX"));

	// If the character have the feat QuickMaster(i.e. human), he can have a supplementary feat.
	if (_character->hasFeat(258))
		_characterChoices.setFeat(twodaFeatPackage.getRow(1).getInt("FEATINDEX"));

	//Skills
	_characterChoices.setPackageSkill(package);

	///TODO Load equipments package.
}

bool CharPackage::hasTakenPackage() {
	return _hasTakenPackage;
}

void CharPackage::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		_hasTakenPackage = true;
		_returnCode = 2;
		return;
	}

	if (widget.getTag() == "ConfigurePckg") {
		_charSkills = new CharSkills(*_character, _packageList->getSelected());
		if (sub(*_charSkills, 0, false) == 2);
			_returnCode = 2;

		hide();
		return;
	}

	if (widget.getTag() == "ClassListBox") {
		_helpBox->setTitle(_packageNames[_packageList->getSelected()]);
		_helpBox->setMainText(_helpTexts[_packageList->getSelected()]);
	}
}

} // End of namespace NWN

} // End of namespace Engines
