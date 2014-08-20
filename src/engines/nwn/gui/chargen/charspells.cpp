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

/** @file engines/nwn/gui/chargen/charspells.cpp
 *  The NWN spells selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/widgets/label.h"

#include "engines/nwn/gui/chargen/charspells.h"


namespace Engines {

namespace NWN {

WidgetListItemSpell::WidgetListItemSpell(Engines::NWN::GUI &gui, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, const Common::UString &description, bool left, uint16 spellIndex) : WidgetListItemExchange(gui, "ctl_cg_btn_feat", text, icon, tag, description,  left, spellIndex) {
}

WidgetListItemSpell::~WidgetListItemSpell() {
}

void WidgetListItemSpell::helpActivated() {
	((CharSpells *) _gui)->openSpellInfo(_text->get(), _description, _icon->getPortrait());
}

CharSpells::CharSpells() : _remainingSpells(0), _prepareSpell(false), _currentLevel(11), _maxLevel(0) {
	load("cg_spells");
	getWidget("Title", true)->setHorCentered();

	getWidget("OkButton", true)->setDisabled(true);

	///TODO Recommend button in the spell selection for the character generator.
	getWidget("RecommendButton", true)->setDisabled(true);
	///TODO Reset button in the spell selection for the character generator.
	getWidget("ResetButton", true)->setDisabled(true);

	_availableWidgetList = getListBox("AvailBox", true);
	_knownWidgetList = getListBox("KnownBox", true);
	_spellInfo = new CharHelpInfo("cg_spellinfo");
}

CharSpells::~CharSpells() {
	delete _spellInfo;
}

void CharSpells::reset() {
	for (std::vector<std::vector<uint32> >::iterator sl = _characterChoices.spells.begin(); sl != _characterChoices.spells.end(); ++sl) {
		sl->clear();
	}

	_characterChoices.spellsPerDay.clear();
}

void CharSpells::show() {
	buildSpellList();

	Engines::GUI::show();
}

void CharSpells::openSpellInfo(const Common::UString &spellName, const Common::UString &description, const Common::UString &icon) {
	_spellInfo->setInfo(spellName, description, icon);
	_spellInfo->show();
	_spellInfo->run();
	_spellInfo->hide();
}

void CharSpells::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "AvailBox" || tag == "KnownBox")
		type = NWN::GUI::kWidgetTypeListBox;
}

bool CharSpells::chooseSpells() {
	reset();

	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow &rowClass = twodaClasses.getRow(_characterChoices.classID);

	_prepareSpell = rowClass.isEmpty("SpellKnownTable");

	Common::UString gainTable = rowClass.getString("SpellGainTable");

	// We need to know the spellCaster type. We can get it from the spellGain table.
	if (gainTable == "CLS_SPGN_BARD")
		_casterName = "Bard";
	else if (gainTable == "CLS_SPGN_CLER")
		_casterName = "Cleric";
	else if (gainTable == "CLS _SPGN_DRUI")
		_casterName = "Druid";
	else if (gainTable == "CLS _SPGN_PAL")
		_casterName = "Paladin";
	else if (gainTable == "CLS _SPGN_RANG")
		_casterName = "Ranger";
	else
		_casterName = "Wiz_Sorc";


	const Aurora::TwoDAFile &twodaSpellGain = TwoDAReg.get(gainTable);
	_maxLevel = _characterChoices.character->getMaxSpellLevel(_characterChoices.classID, true);

	if (!_prepareSpell || _casterName.equals("Wiz_Sorc"))
		return true;

	// Compute if new spell level has been reached.
	uint oldSpellLvl = _characterChoices.character->getMaxSpellLevel(_characterChoices.classID);

	if (oldSpellLvl == _maxLevel)
		return false;

	// Add spells for all divine casters.
	const Aurora::TwoDAFile &twodaSpells = TwoDAReg.get("spells");
	for (uint it = 0; it < twodaSpells.getRowCount(); ++it) {
		const Aurora::TwoDARow &rowSpell = twodaSpells.getRow(it);

		if (rowSpell.isEmpty(_casterName))
			continue;

		uint16 spellLevel = rowSpell.getInt(_casterName);
		if (spellLevel > _maxLevel)
			continue;

		if (spellLevel < _maxLevel && _characterChoices.character->getClassLevel(_characterChoices.classID) > 0)
			continue;

		_characterChoices.spells[spellLevel].push_back(it);
	}

	return false;
}

void CharSpells::buildSpellList() {
	_spellAvailable.clear();
	_spellAvailable.resize(_maxLevel + 1);
	_spellKnown.clear();
	_spellKnown.resize(_maxLevel + 1);

	_availableWidgetList->setMode(WidgetListBox::kModeSelectable);

	// Construct available and known list.
	const Aurora::TwoDAFile &twodaSpells = TwoDAReg.get("spells");
	const Aurora::TwoDAFile &twodaSpellSchool = TwoDAReg.get("spellschools");

	for (uint it = 0; it < twodaSpells.getRowCount(); ++it) {
		const Aurora::TwoDARow &rowSpell = twodaSpells.getRow(it);

		if (rowSpell.isEmpty(_casterName))
			continue;

		uint16 spellLevel = rowSpell.getInt(_casterName);
		if (spellLevel > _maxLevel)
			continue;

		// Check spell school for wizard.
		if (_characterChoices.school) {
			const Aurora::TwoDARow &rowSchool = twodaSpellSchool.getRow(_characterChoices.school);
			const Aurora::TwoDARow &rowOppSchool = twodaSpellSchool.getRow(rowSchool.getInt("Opposition"));
			if (rowSpell.getString("School") == rowOppSchool.getString("Letter"))
				continue;
		}

		// The wizard know all the level 0 spells.
		if (_prepareSpell && spellLevel == 0) {
			_spellKnown[spellLevel].push_back(it);
			continue;
		}

		if (_characterChoices.character->hasSpell(it, 0x03, _characterChoices.classID)) {
			if (!_prepareSpell)
				_spellKnown[spellLevel].push_back(it);

			continue;
		}

		///TODO Check if the character ability score's is high enough.

		_spellAvailable[spellLevel].push_back(it);
	}

	// Compute how many new spell slots and bonus spells are available at this level.
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow &rowClass = twodaClasses.getRow(_characterChoices.classID);

	// Compute bonus spells.
	std::vector<uint> bonus;
	// Translate string label ability to the enum ability.
		Common::UString primAbString = rowClass.getString("PrimaryAbil");
		Ability primAb = kAbilityMAX;
		if (primAbString.equals("STR"))
			primAb = kAbilityStrength;
		else if (primAbString.equals("DEX"))
			primAb = kAbilityDexterity;
		else if (primAbString.equals("CON"))
			primAb = kAbilityConstitution;
		else if (primAbString.equals("INT"))
			primAb = kAbilityIntelligence;	
		else if (primAbString.equals("WIS"))
			primAb = kAbilityWisdom;
		else if (primAbString.equals("CHA"))
			primAb = kAbilityCharisma;

		for (uint lvl = 0; lvl <= _maxLevel; ++lvl) {
			uint lvlBonus = floor((_characterChoices.character->getAbilityModifier(primAb) - lvl) / 4 + 1);
			if (lvlBonus < 0)
				lvlBonus = 0;

			bonus.push_back(lvlBonus);
		}

	_newSpellSlot.clear();

	const Aurora::TwoDAFile &twodaSpellGain = TwoDAReg.get(rowClass.getString("SpellGainTable"));

	uint currentLevel = _characterChoices.character->getClassLevel(_characterChoices.classID);

	const Aurora::TwoDARow &rowSpGnNew = twodaSpellGain.getRow(currentLevel);
	const Aurora::TwoDARow &newLevel = (_prepareSpell) ? rowSpGnNew : TwoDAReg.get(rowClass.getString("SpellKnownTable")).getRow(currentLevel);
	const Aurora::TwoDARow &oldLevel = (_prepareSpell) ? twodaSpellGain.getRow(currentLevel - 1) : TwoDAReg.get(rowClass.getString("SpellKnownTable")).getRow(currentLevel - 1);

	uint startColumn = 0;

	if (!_prepareSpell) {
		startColumn = 1;

		// Compute spells per day per level.
		_characterChoices.spellsPerDay.clear();

		for (uint lvl = 0; lvl <= _maxLevel; ++lvl)
			_characterChoices.spellsPerDay.push_back(rowSpGnNew.getInt(2 + lvl) + bonus[lvl]);
	} else {
		startColumn = 2;
	}

	// Compute spell slots.
	if ( currentLevel == 0) {
		for (uint sL = 0; sL <= _maxLevel; ++sL)
			_newSpellSlot.push_back(newLevel.getInt(sL + startColumn));
	} else {
		for (uint sL = 0; sL <= _maxLevel; ++sL) {
			uint newSlots = newLevel.getInt(sL + 1) - oldLevel.getInt(sL + startColumn);
			_newSpellSlot.push_back(newSlots);
		}
	}

	// Compute new known spell for wizard.
	if (_prepareSpell) {
		uint firstLevel = 0;
		if (_characterChoices.character->getClassLevel(_characterChoices.classID) == 0)
			firstLevel += _characterChoices.character->getAbilityModifier(kAbilityIntelligence) + 3;

		_remainingSpells = 2 + firstLevel;
		
	}

	// We show the first level which has new slot(s) or for wizard the maximum spell level.
	uint firstLvlToShow = 0;
	if (_prepareSpell) {
		firstLvlToShow = _maxLevel;
	} else {
		for (uint lvl = 0; lvl <= _maxLevel; ++lvl) {
			if (_newSpellSlot[lvl] > 0)
				break;

			++firstLvlToShow;
		}

		_remainingSpells = _newSpellSlot[firstLvlToShow];

		if (firstLvlToShow == _maxLevel && _newSpellSlot[_maxLevel] == 0)
			getWidget("OkButton", true)->setDisabled(false);
	}

	showSpellLevel(firstLvlToShow);

	

	// Show icon spell level limited to max level.
// 	for (uint16 spLvl = 0; spLvl < _maxLevel + 1; ++spLvl) {
// 		std::ostringstream convert;
// 		convert << spLvl;
// 
		///TODO Set the spell level icon.
// 		getButton("SpellLevel" + convert.str());
// 	}
}

void CharSpells::showSpellLevel(uint16 spellLevel) {
	if (spellLevel == _currentLevel)
		return;

	// Save the previous available and known list.
	if (_currentLevel != 11) {
		_spellAvailable[_currentLevel].clear();
		for (uint sp = 0; sp < _availableWidgetList->getSize(); ++sp)
			_spellAvailable[_currentLevel].push_back(((WidgetListItemSpell *) _availableWidgetList->getItem(sp))->getAbstractIndex());

		_spellKnown[_currentLevel].clear();
		for (uint sp = 0; sp < _knownWidgetList->getSize(); ++sp)
			_spellKnown[_currentLevel].push_back(((WidgetListItemSpell *) _knownWidgetList->getItem(sp))->getAbstractIndex());
	}

	_currentLevel = spellLevel;
	const Aurora::TwoDAFile &twodaSpells = TwoDAReg.get("spells");

	if (!_prepareSpell)
		_remainingSpells = _newSpellSlot[spellLevel];

	updateRemaining();

	// Build available spells.
	_availableWidgetList->lock();
	_availableWidgetList->clear();

	for (uint it = 0; it < _spellAvailable[spellLevel].size(); ++it) {
		const Aurora::TwoDARow &rowSpell = twodaSpells.getRow(_spellAvailable[spellLevel][it]);

		const Common::UString &name = TalkMan.getString(rowSpell.getInt("Name"));
		WidgetListItemSpell *spellItem = new WidgetListItemSpell(*this, name, rowSpell.getString("IconResRef"), name, TalkMan.getString(rowSpell.getInt("SpellDesc")), false, _spellAvailable[spellLevel][it]);
		_availableWidgetList->add(spellItem);
	}

	if (_spellAvailable[spellLevel].size() > 0)
		_availableWidgetList->getChild("EmptySpellListLabel")->setInvisible(true);
	else
		_availableWidgetList->getChild("EmptySpellListLabel")->setInvisible(false);

	_availableWidgetList->sortByTag();
	_availableWidgetList->unlock();

	// Build known spells.
	///TODO Is the character able to have spells at this level ?
	if (_knownWidgetList->getChild("CannotLearn"))
		_knownWidgetList->getChild("CannotLearn")->setInvisible(true);

	if (_spellKnown.empty())
		return;

	_knownWidgetList->lock();
	_knownWidgetList->clear();

	for (uint it = 0; it < _spellKnown[spellLevel].size(); ++it) {
		const Aurora::TwoDARow &rowSpell = twodaSpells.getRow(_spellKnown[spellLevel][it]);
		
		WidgetListItemSpell *spellItem = new WidgetListItemSpell(*this, TalkMan.getString(rowSpell.getInt("Name")), rowSpell.getString("IconResRef"), rowSpell.getString("Label"), TalkMan.getString(rowSpell.getInt("SpellDesc")), true, _spellKnown[spellLevel][it]);
		_knownWidgetList->add(spellItem);
		if (spellLevel == 0 && _prepareSpell)
			spellItem->setDisabled(true);
	}

	_knownWidgetList->unlock();
}

void CharSpells::updateRemaining() {
	if (!_prepareSpell)
		_newSpellSlot[_currentLevel] = _remainingSpells;

	std::ostringstream convert;
	convert << _remainingSpells;
	getLabel("RemainLabel", true)->setText(convert.str());
}

void CharSpells::moveSpell(WidgetListItemSpell *item, bool left, uint itemNumber) {
	if ((!left && _remainingSpells == 0) && item->isAvailable())
		return;

	removeFocus();
	item->hide();

	_availableWidgetList->lock();
	_knownWidgetList->lock();

	item->changeOrientation();
	if (left) {
		_knownWidgetList->removeItem(itemNumber);
		_availableWidgetList->add(item);
		_remainingSpells++;
	} else  {
		_availableWidgetList->removeItem(itemNumber);
		_knownWidgetList->add(item);
		_remainingSpells--;
	}

	_availableWidgetList->sortByTag();
	_availableWidgetList->unlock();
	_knownWidgetList->unlock();
	
	updateRemaining();

	if (_availableWidgetList->getSize() == 0)
		_availableWidgetList->getChild("EmptySpellListLabel")->show();

	bool remains = false;
	for (uint lvl = 0; lvl <= _maxLevel; ++lvl) {
		if (_prepareSpell && _remainingSpells > 0) {
			remains = true;
			break;
		}

		if (!_prepareSpell && _newSpellSlot[lvl] > 0) {
			remains = true;
			break;
		}
	}

	if (remains) {
		// Go to the next spell level if any.
		if (_remainingSpells == 0 && _currentLevel < _maxLevel && !_prepareSpell)
			showSpellLevel(_currentLevel + 1);

		getWidget("OkButton", true)->setDisabled(true);
		return;
	} else {
		getWidget("OkButton", true)->setDisabled(false);
	}
}

void CharSpells::callbackActive(Widget &widget) {
	if (widget.getTag() == "OkButton") {
		_spellKnown[_currentLevel].clear();
		for (uint sp = 0; sp < _knownWidgetList->getSize(); ++sp)
			_spellKnown[_currentLevel].push_back(((WidgetListItemSpell *) _knownWidgetList->getItem(sp))->getAbstractIndex());

			_characterChoices.spells = _spellKnown;
	
		_returnCode = 2;
	}

	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
	}

	for (uint16 spLvl = 0; spLvl < _maxLevel + 1; ++spLvl) {
		std::ostringstream convert;
		convert << spLvl;

		if (widget.getTag() == "SpellLevel" + convert.str()) {
			showSpellLevel(spLvl);
			break;
		}
	}
}

} // End of namespace NWN

} // End of namespace Engines
