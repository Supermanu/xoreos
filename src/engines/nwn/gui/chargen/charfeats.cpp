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

/** @file engines/nwn/gui/chargen/charfeats.cpp
 *  The NWN feats selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "events/events.h"

#include "graphics/graphics.h"

#include "engines/aurora/model.h"

#include "engines/nwn/gui/widgets/editbox.h"
#include "engines/nwn/gui/widgets/label.h"

#include "engines/nwn/gui/chargen/charfeats.h"

namespace Engines {

namespace NWN {

WidgetListItemFeat::WidgetListItemFeat(Engines::NWN::GUI &gui, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, const Common::UString &featDescription, bool left, uint16 feat, bool masterFeat) : WidgetListItemExchange(gui, "ctl_cg_btn_feat", text, icon, tag, featDescription, left, feat) {
	_isMasterFeat = masterFeat;
}

WidgetListItemFeat::~WidgetListItemFeat() {
}

void WidgetListItemFeat::helpActivated() {
	((CharFeats *) _gui)->openFeatInfo(_text->get(), _description, _icon->getPortrait());
}

bool WidgetListItemFeat::isMasterFeat() const {
	return _isMasterFeat;
}

void WidgetListItemFeat::subActive(Widget &widget) {
	Engines::NWN::WidgetListItemExchange::subActive(widget);


	if (widget.getTag().endsWith("#AddRemove"))
		_needToMove = true;
}

CharFeats::CharFeats() : _availableFeats(0), _bonusFeat(-1) {
	load("cg_feats");
	getWidget("Title", true)->setHorCentered();

	_availableWidgetList = getListBox("AvailBox", true);
	_chosenWidgetList = getListBox("KnownBox", true);
	_featInfo = new CharHelpInfo("cg_featinfo");
	_featsPopUp = new CharFeatsPopUp();

	getWidget("OkButton", true)->setDisabled(true);

	///TODO Recommend Button (from package).
	///TODO reset button.
}

CharFeats::~CharFeats() {
	_chosenWidgetList->lock();
	_chosenWidgetList->clear();
	_chosenWidgetList->unlock();
	delete _featInfo;
	delete _featsPopUp;
}

void CharFeats::reset() {
	for (uint f = 0; f < _chosenWidgetList->getSize(); ++f) {
		WidgetListItemFeat *feat = (WidgetListItemFeat *) _chosenWidgetList->getItem(f);
		if (feat->isAvailable())
			_availableFeats++;
	}

	makeGrantedList();
	makeAvailList();
}

void CharFeats::show() {
	_characterChoices.normalFeats.clear();
	makeGrantedList();
	makeAvailList();
	updateFeatsRemainLabel();

	Engines::GUI::show();
}

uint CharFeats::availableFeats() {
	_availableFeats = 0;
	// Get an additional feat each new level divisible by 3.
	if ((_characterChoices.character->getLevel() + 1) % 3)
		_availableFeats++;

	// Get an additional feat at level 1.
	if (!_characterChoices.character->getLevel())
		_availableFeats++;

	// Get an additional level if the character is human (Quick Master feat).
	if (_characterChoices.character->getRace() == kRaceHuman)
		_availableFeats++;

	///TODO The cls_bfeat_*.2da only exist in the last extension (HotU). Bonus feats  seems to be hardcoded for the others.
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow &rowClass = twodaClasses.getRow(_characterChoices.classID);
	if (!rowClass.isEmpty("BonusFeatsTable")) {
		const Aurora::TwoDAFile &twodaClassBFeats = TwoDAReg.get(rowClass.getString("BonusFeatsTable"));
		const Aurora::TwoDARow &bonusFeatRow = twodaClassBFeats.getRow(_characterChoices.character->getClassLevel(_characterChoices.classID));
		
		if (bonusFeatRow.getInt("Bonus") > 0) {
			_hasBonusFeat = true;
			_availableFeats++;
		}
	}

	return _availableFeats;
}

void CharFeats::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "AvailBox" || tag == "KnownBox")
		type = NWN::GUI::kWidgetTypeListBox;
}

void CharFeats::openFeatInfo(const Common::UString &feat, const Common::UString &featDescription, const Common::UString &icon) {
	_featInfo->setInfo(feat, featDescription, icon);
	_featInfo->show();
	_featInfo->run();
	_featInfo->hide();
}

void CharFeats::callbackActive(Widget &widget) {
	if (widget.getTag() == "OkButton") {
		_characterChoices.normalFeats.clear();
		for (uint it = 0; it < _chosenWidgetList->getSize(); ++it)
			_characterChoices.setFeat(((WidgetListItemFeat *)_chosenWidgetList->getItem(it))->getAbstractIndex());

		_returnCode = 2;
		removeFocus();
		return;
	}

	if (widget.getTag() == "CancelButton") {
		_characterChoices.normalFeats.clear();
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "ResetButton")
		reset();
}

void CharFeats::callbackRun() {
	bool featChanged = false;
	// Check if some feats need to move to the other list.
	for (uint it = 0; it < _availableWidgetList->getSize(); ++it) {
		WidgetListItemFeat *item = (WidgetListItemFeat *) _availableWidgetList->getItem(it);
		if (item->hasToMove()) {
			moveFeat(item, item->isLeft(), it);
			item->setNeedToMove(false);
			featChanged = true;
		}
	}

	for (uint it = 0; it < _chosenWidgetList->getSize(); ++it) {
		WidgetListItemFeat *item = (WidgetListItemFeat *) _chosenWidgetList->getItem(it);
		if (item->hasToMove()) {
			moveFeat(item, item->isLeft(), it);
			item->setNeedToMove(false);
			featChanged = true;
		}
	}

	if (!featChanged)
		return;

	// Clear the available list or show only bonuses feats.
	if (!makeBonusList() && _availableFeats == 1) {
		makeAvailList();
	}

	if (_availableFeats == 0) {
		_availableWidgetList->lock();
		_availableWidgetList->clear();
		_availableWidgetList->unlock();

		getButton("OkButton", true)->setDisabled(false);
	} else {
		getButton("OkButton", true)->setDisabled(true);
	}

	GUI::callbackRun();
}

void CharFeats::updateFeatsRemainLabel() {
	std::ostringstream convert;
	convert << _availableFeats;
	getLabel("RemainLabel", true)->setText(convert.str());
}

void CharFeats::makeAvailList() {
	// If there are only bonus feats.
	if (makeBonusList())
		return;

	// Build the list of available feats.
	_availableWidgetList->lock();
	_availableWidgetList->clear();
	_availableWidgetList->setMode(WidgetListBox::kModeSelectable);

	// Build list from all available feats.
	const Aurora::TwoDAFile &twodaFeats = TwoDAReg.get("feat");
	for (uint16 it = 0; it < twodaFeats.getRowCount(); ++it) {
		if (!_characterChoices.hasPrereqFeat(it))
			continue;

		addFeatToList(it, false, true);
	}

	// General class feats.
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDARow &rowClass = twodaClasses.getRow(_characterChoices.classID);
	const Aurora::TwoDAFile &twodaClassFeats = TwoDAReg.get(rowClass.getString("FeatsTable"));
	for (uint f = 0; f < twodaClassFeats.getRowCount(); ++f){
		const Aurora::TwoDARow &rowClassFeats = twodaClassFeats.getRow(f);

		if (rowClassFeats.getInt("List") > 1 )
			continue;

		uint32 featIndex = rowClassFeats.getInt("FeatIndex");
		if (!_characterChoices.hasPrereqFeat(featIndex, true))
			continue;

		addFeatToList(featIndex, false, true);
	}	

	_availableWidgetList->sortByTag();
	_availableWidgetList->unlock();
}

void CharFeats::makeGrantedList() {
	_chosenWidgetList->lock();
	_chosenWidgetList->clear();
	_chosenWidgetList->setMode(WidgetListBox::kModeSelectable);

	// Build the list of chosen feats from race and class.
	// Race feats.
	if (_characterChoices.character->getLevel() == 0) {
		for (std::vector<uint32 >::iterator it = _characterChoices.racialFeats.begin(); it != _characterChoices.racialFeats.end(); ++it) {
			addFeatToList(*it, true, false);
		}
	}
	
	// Class feats.
	for (std::vector<uint32 >::iterator it = _characterChoices.classFeats.begin(); it != _characterChoices.classFeats.end(); ++it) {
		addFeatToList(*it, true, false);
	}

	///TODO Cleric domain feats.

	_chosenWidgetList->sortByTag();
	_chosenWidgetList->unlock();
}

bool CharFeats::makeBonusList() {
	if (_hasBonusFeat && _bonusFeat < 0 && _availableFeats == 1) {
		std::cout << "Making bonus feat" << std::endl;
		_availableWidgetList->lock();
		_availableWidgetList->clear();
		_availableWidgetList->setMode(WidgetListBox::kModeSelectable);

		const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
		const Aurora::TwoDARow &rowClass = twodaClasses.getRow(_characterChoices.classID);
		const Aurora::TwoDAFile &twodaClassFeats = TwoDAReg.get(rowClass.getString("FeatsTable"));
		
		for (uint f = 0; f < twodaClassFeats.getRowCount(); ++f){
			const Aurora::TwoDARow &rowClassFeats = twodaClassFeats.getRow(f);
			
			if (rowClassFeats.getInt("List") != 1 &&  rowClassFeats.getInt("List") != 2)
				continue;

			uint32 featIndex = rowClassFeats.getInt("FeatIndex");
			if (!_characterChoices.hasPrereqFeat(featIndex, true))
				continue;

			addFeatToList(featIndex, false, true);
		}
		_availableWidgetList->sortByTag();
		_availableWidgetList->unlock();
		return true;
	}

	return false;
}

void CharFeats::addFeatToList(uint32 featIndex, bool isChosenFeat, bool isMovable) {
	// This function does not include locking mechanism of the listBox neither prerequisite check.
	const Aurora::TwoDAFile &twodaFeats = TwoDAReg.get("feat");
	const Aurora::TwoDAFile &twodaMasterFeats = TwoDAReg.get("masterfeats");

	if (_isMasterFeatDone.empty())
		_isMasterFeatDone.resize(twodaMasterFeats.getRowCount(), false);

	// Check if the feat is already in one of the lists.
	for (uint cf = 0; cf < _chosenWidgetList->getSize(); ++cf) {
		WidgetListItemFeat *feat = (WidgetListItemFeat *) _chosenWidgetList->getItem(cf);
		if (feat->getAbstractIndex() == featIndex)
			return;
	}

	for (uint cf = 0; cf < _availableWidgetList->getSize(); ++cf) {
		WidgetListItemFeat *feat = (WidgetListItemFeat *) _availableWidgetList->getItem(cf);
		if (feat->getAbstractIndex() == featIndex)
			return;
	}

	WidgetListBox *listBox = 0;
	if (isChosenFeat)
		listBox = _chosenWidgetList;
	else
		listBox = _availableWidgetList;

	const Aurora::TwoDARow &row = twodaFeats.getRow(featIndex);
	if (!row.isEmpty("MASTERFEAT")) {
		uint16 masterFeat = row.getInt("MASTERFEAT");
		if (_isMasterFeatDone[masterFeat])
			return;
		
		const Aurora::TwoDARow &rowMasterFeat = twodaMasterFeats.getRow(masterFeat);
		_isMasterFeatDone[masterFeat] = true;
		const Common::UString name = TalkMan.getString(rowMasterFeat.getInt("STRREF"));
		WidgetListItemFeat *masterFeatItem = new WidgetListItemFeat(*this, name, rowMasterFeat.getString("ICON"), name, TalkMan.getString(rowMasterFeat.getInt("DESCRIPTION")), isChosenFeat, masterFeat, true);
		listBox->add(masterFeatItem);

		if (!isMovable)
			masterFeatItem->setUnmovable();

		return;
	}

	const Common::UString name = TalkMan.getString(row.getInt("FEAT"));
	WidgetListItemFeat *featItem = new WidgetListItemFeat(*this, name, row.getString("ICON"), name, TalkMan.getString(row.getInt("DESCRIPTION")), isChosenFeat, featIndex, false);
	listBox->add(featItem);

	if (!isMovable)
		featItem->setUnmovable();
}

void CharFeats::moveFeat(Engines::NWN::WidgetListItemFeat *item, bool left, uint itemNumber) {
	if ((!left && _availableFeats == 0) || !item->isAvailable())
		return;

	item->hide();
	std::cout << "moving feat" << std::endl;
	const Aurora::TwoDAFile &twodaFeats = TwoDAReg.get("feat");
	const Aurora::TwoDAFile &twodaMasterFeats = TwoDAReg.get("masterfeats");
	if (item->isMasterFeat()) {
		Common::UString featName;
		Common::UString featDescription;
		if (!left) {
			_featsPopUp->buildList(item->getAbstractIndex());
			_featsPopUp->show();
			_featsPopUp->run();
			_featsPopUp->hide();

			int16 feat = _featsPopUp->getFeats(); 
			if (feat < 0)
				return;

			item->setAbstractIndex(feat);
			featName = TalkMan.getString(twodaFeats.getRow(feat).getInt("FEAT"));
			featDescription = TalkMan.getString(twodaFeats.getRow(feat).getInt("DESCRIPTION"));
		} else {
			uint16 masterFeat = twodaFeats.getRow(item->getAbstractIndex()).getInt("MASTERFEAT");
			const Aurora::TwoDARow &rowMaster = twodaMasterFeats.getRow(masterFeat);
			featName = TalkMan.getString(rowMaster.getInt("STRREF"));
			featDescription = TalkMan.getString(rowMaster.getInt("DESCRIPTION"));
			item->setAbstractIndex(masterFeat);
		}
		item->setProperties(featName, featDescription);
	}

	_availableWidgetList->lock();
	_chosenWidgetList->lock();

// 	item->changeOrientation();
	Common::UString title, desc, icon;
	item->getProperties(title, desc, icon);
	std::cout << "removing old feat" << std::endl;
	if (left) {
		std::cout << "remove from list" << std::endl;
		_chosenWidgetList->removeItem(itemNumber);
		std::cout << "remove item" << std::endl;
		item->remove();
		WidgetListItemFeat *newItem = new WidgetListItemFeat(*this, title, icon, item->getTag(),desc, !left, item->getAbstractIndex(), item->isMasterFeat());
		_availableWidgetList->add(newItem);
		_availableFeats++;
// 		_currentWidget = _availableWidgetList;
	} else  {
		std::cout << "remove from list" << std::endl;
// 		_availableWidgetList->removeItem(itemNumber);
		std::cout << "remove item" << std::endl;
		
		item->remove();
		WidgetListItemFeat *newItem = new WidgetListItemFeat(*this, title, icon, item->getTag(),desc, !left, item->getAbstractIndex(), item->isMasterFeat());
		_chosenWidgetList->add(newItem);
		_availableFeats--;
// 		_currentWidget = _chosenWidgetList;
	}

	_availableWidgetList->sortByTag();

	_availableWidgetList->unlock();
	_chosenWidgetList->unlock();

	updateFeatsRemainLabel();

	// Deal with bonus feats.
	if (!_hasBonusFeat)
		return;

	if (_bonusFeat == item->getAbstractIndex() && left)
		_bonusFeat = -1;

	// At first, the bonus feat is always the last chosen feat.
	if (_availableFeats == 0 && _bonusFeat < 0) {
		_bonusFeat = item->getAbstractIndex();
		return;
	}
}

} // End of namespace NWN

} // End of namespace Engines
