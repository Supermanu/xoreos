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

/** @file engines/nwn/gui/chargen/charskills.cpp
 *  The NWN skills selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "graphics/aurora/modelnode.h"
#include "graphics/aurora/model.h"

#include "engines/nwn/gui/widgets/editbox.h"

#include "engines/nwn/gui/chargen/charskills.h"

#include "engines/nwn/gui/chargen/charfeats.h"
#include "engines/nwn/gui/chargen/charschool.h"
#include "engines/nwn/gui/chargen/chardomain.h"

namespace Engines {

namespace NWN {

WidgetListItemSkill::WidgetListItemSkill(Engines::GUI &gui, const Common::UString &model, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, uint32 skillIndex, uint16 initialSkill, bool classSkill) : WidgetListItemButton(gui, model, "fnt_galahad14", text, icon), _skillRank(initialSkill), _initialSkillValue(initialSkill), _skillIndex(skillIndex), _classSkill(classSkill) {
	setTag(tag);

	// Up and down buttons.
	_downButton = new WidgetButton(gui, _tag + "#Down", "ctl_cg_btn_skdn", "");
	_upButton = new WidgetButton(gui, _tag + "#Up", "ctl_cg_btn_skup", "");

	float pX, pY, pZ;
	_buttonItem->getNode("skillup")->getPosition(pX, pY, pZ);
	_upButton->setPosition(pX, pY, -90);
	_buttonItem->getNode("skilldown")->getPosition(pX, pY, pZ);
	_downButton->setPosition(pX, pY, -90);

	addChild(*_upButton);
	addChild(*_downButton);
	addSub(*_upButton);
	addSub(*_downButton);

	// Skill value panel and text.
	_skillPointPanel = new WidgetPanel(gui, _tag + "#SkillPointPanel", "ctl_cg_numbox3");
	_buttonItem->getNode("skillrank")->getPosition(pX, pY, pZ);
	_skillPointPanel->setPosition(pX, pY, -90);

	addChild(*_skillPointPanel);

	_skillPoint = new WidgetLabel(gui, _tag + "#SkillPointValue", "fnt_galahad14", "0");
	_skillPointPanel->addChild(*_skillPoint);
	_skillPoint->setVerCentered();
	_skillPoint->setHorCentered();
	_skillPoint->movePosition(0, 0, -91);

	
}

NWN::WidgetListItemSkill::~WidgetListItemSkill() {}

void WidgetListItemSkill::reset() {
	_skillRank = 0;
	updateText();
}

void WidgetListItemSkill::subActive(Widget &widget) {
	select();


	if (widget.getTag().contains("#Up"))
		changeSkill(1);

	if (widget.getTag().contains("#Down"))
		changeSkill(-1);

	((CharSkills *) _gui)->setHelp(_itemNumber);
}

void WidgetListItemSkill::changeSkill(int8 amount) {
	if (_skillRank + amount < 0)
		return;


	if (!((CharSkills *) _gui)->changeSkillPoints(amount, _skillRank,_classSkill)) {
		return;
	}

	if (!_classSkill)
		amount *= 2;

	_skillRank += amount;

	updateText();
}

uint8 WidgetListItemSkill::getSkillRank() const {
	return _skillRank;
}

void WidgetListItemSkill::setSkillRank(uint8 skillRank) {
	_skillRank = skillRank;
	updateText();
}

uint32 WidgetListItemSkill::getSkillIndex() const {
	return _skillIndex;
}

void WidgetListItemSkill::updateText() {
	std::ostringstream convert;
	convert << (uint16) _skillRank;
	_skillPoint->setText(convert.str());
}

CharSkills::CharSkills(Engines::NWN::Creature &character, uint8 package): _character(&character) {
	load("cg_skills");
	getWidget("Title", true)->setHorCentered();

	getEditBox("HelpBox", true)->setTitle(TalkMan.getString(58250));
	getEditBox("HelpBox", true)->setMainText(TalkMan.getString(58251));

	_package = package;
	if (package == 200)
		_package = _character->getPackage();
}

CharSkills::~CharSkills() {}

void CharSkills::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "SkillsButtonBox")
		type = NWN::GUI::kWidgetTypeListBox;
}

void CharSkills::reset() {
	_characterChoices.skills.clear();
	_characterChoices.skills.resize(28, 0);
	for (uint it = 0; it < _skillNames.size(); ++it)
		((WidgetListItemSkill *) _skillList->getItem(it))->reset();
}

void CharSkills::show() {
	// Construct the listbox widget from available character's class skills and retrieve helptexts by the way.
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow &row = twodaClasses.getRow(_characterChoices.classID);
	_skillPointsLeft = (_character->getAbilityModifier(kAbilityIntelligence) + row.getInt("SkillPointBase"));
	if (_character->getRace() == kRaceHuman)
		_skillPointsLeft++;

	if (_character->getLevel() == 0)
		_skillPointsLeft *= 4;

	std::ostringstream convert;
	convert << _skillPointsLeft;
	getLabel("PtsRemainingBox", true)->setText(convert.str());

	_skillList = getListBox("SkillsButtonBox", true);

	_skillList->lock();
	_skillList->clear();
	_skillList->setMode(WidgetListBox::kModeSelectable);

	const Aurora::TwoDAFile &twodaSkills = TwoDAReg.get("skills");
	const Aurora::TwoDAFile &twodaSkillsCls = TwoDAReg.get(row.getString("SkillsTable"));

	_skillNames.clear();
	_helpTexts.clear();

	for (uint16 it = 0; it < twodaSkillsCls.getRowCount(); ++it) {
		const Aurora::TwoDARow &rowSkillsCls = twodaSkillsCls.getRow(it);
		uint32 skillIndex = rowSkillsCls.getInt("SkillIndex");
		const Aurora::TwoDARow &rowSkills = twodaSkills.getRow(skillIndex);

		_skillNames.push_back(TalkMan.getString(rowSkills.getInt("Name")));
		_helpTexts.push_back(TalkMan.getString(rowSkills.getInt("Description")));

		bool classSkill = (bool) rowSkillsCls.getInt("ClassSkill");
		Common::UString skillName = _skillNames.back();
		if (classSkill)
			skillName += " " + TalkMan.getString(52951);

		WidgetListItemSkill *skillButton = new WidgetListItemSkill(*this, "ctl_cg_btn_skill", skillName, rowSkills.getString("Icon"), skillName, skillIndex, _character->getSkillRank(skillIndex), classSkill);
		_skillList->add(skillButton);
	}

	_skillList->sortByTag();
	_skillList->unlock();

	Engines::GUI::show();
}

bool CharSkills::changeSkillPoints(int8 amount, uint initialRank, bool isClassSkill) {
	if (!isClassSkill)
		amount *= 2;

	uint8 upperLimit = _character->getLevel() + 4;
	if (!isClassSkill)
		upperLimit = round(upperLimit / 2 - 0.5);

	if ((int8) _skillPointsLeft - amount < 0 || initialRank + amount > upperLimit)
		return false;

	_skillPointsLeft -= amount;
	std::ostringstream convert;
	convert << _skillPointsLeft;
	getLabel("PtsRemainingBox", true)->setText(convert.str());

	return true;
}

void CharSkills::setHelp(uint8 skillItem) {
	getEditBox("HelpBox", true)->setTitle(_skillNames[skillItem]);
	getEditBox("HelpBox", true)->setMainText(_helpTexts[skillItem]);
}

void CharSkills::callbackActive(Widget &widget) {
	if (widget.getTag() == "OkButton") {
		for (uint it = 0; it < _skillList->getSize(); ++it) {
			WidgetListItemSkill *skillItem = (WidgetListItemSkill *) _skillList->getItem(it);
			_characterChoices.skills[skillItem->getSkillIndex()] = skillItem->getSkillRank();
		}

		CharFeats *charFeats = new CharFeats();
		if (charFeats->availableFeats() > 0) {
			if (sub(*charFeats, 0, false) == 1)
				_returnCode = 1;

			hide();
		}
		delete charFeats;

// 		const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
// 		const Aurora::TwoDARow &row = twodaClasses.getRow(_characterChoices.classID);
// 		if (row.getInt("SpellCaster")) {
// 			CharSpells *charSpells = new CharSpells();
// 
// 			// Check if spells need to be chosen. If not (for divine casters) adds new spells if needed and doesn't show the gui.
// 			if (charSpells->chooseSpells()) {
// 				// Only for wizard at the first level.
// 				if (_characterChoices.classID == 10 && _character->getClassLevel(_characterChoices.classID) == 0) {
// 					CharSchool *charSchool = new CharSchool();
// 					if (sub(*charSchool, 0, false) == 1)
// 						_returnCode = 1;
// 
// 					delete charSchool;
// 				}
// 
// 				if (sub(*charSpells, 0, false) == 1)
// 					_returnCode = 1;
// 
// 			} else if (_characterChoices.classID == 2 && _character->getClassLevel(_characterChoices.classID) == 0){
// 				// Only for the cleric at the first level
// 				CharDomain *charDomain = new CharDomain();
// 				if (sub(*charDomain, 0, false) == 1)
// 					_returnCode = 1;
// 
// 				delete charDomain;
// 			}
// 			delete charSpells;
// 		}

		_returnCode = 2;

		return;
	}

	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "RecommendButton") {
		int8 pointsLeft =_characterChoices.setPackageSkill(_package);
		for (uint sk = 0; sk < _skillList->getSize(); ++sk) {
			WidgetListItemSkill * skill = (WidgetListItemSkill *) _skillList->getItem(sk);
			skill->setSkillRank(_characterChoices.skills[skill->getSkillIndex()]);
		}

		_skillPointsLeft = pointsLeft;
		changeSkillPoints(0, 0, true);
	}

	if (widget.getTag() == "SkillsButtonBox") {
		setHelp(_skillList->getSelected());
	}
 
}

} // End of namespace NWN

} // End of namespace Engines
