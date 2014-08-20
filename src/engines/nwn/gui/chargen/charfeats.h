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

/** @file engines/nwn/gui/chargen/charfeats.h
 *  The NWN feats selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARFEATS_H
#define ENGINES_NWN_GUI_CHARGEN_CHARFEATS_H


#include "graphics/aurora/model.h"
#include "graphics/aurora/modelnode.h"

#include "engines/nwn/gui/widgets/listbox.h"
#include "engines/nwn/gui/widgets/listitembutton.h"
#include "engines/nwn/gui/widgets/button.h"

#include "engines/nwn/gui/chargen/charhelpinfo.h"
#include "engines/nwn/gui/chargen/charfeatspopup.h"

#include "engines/nwn/gui/chargen/chargenabstract.h"

namespace Engines {

namespace NWN {

class WidgetListItemFeat : public WidgetListItemExchange {
public:
	WidgetListItemFeat(Engines::NWN::GUI & gui, const Common::UString & text, const Common::UString & icon, const Common::UString &tag, const Common::UString &featDescription, bool left, uint16 feat, bool masterFeat);
	virtual ~WidgetListItemFeat();

	void helpActivated();
	void subActive(Widget &widget);

	bool isMasterFeat() const;

private:
	bool _isMasterFeat;
};

class CharFeats : public CharGenAbstract {
public:
	CharFeats();
	~CharFeats();

	void show();
	void reset();

	uint availableFeats();

	void fixWidgetType(const Common::UString &tag, WidgetType &type);
	void moveFeat(Engines::NWN::WidgetListItemFeat *item, bool left, uint itemNumber);
	void openFeatInfo(const Common::UString &feat, const Common::UString &featDescription, const Common::UString &icon);
	void callbackActive(Widget &widget);
	void callbackRun();

private:
	void makeAvailList();
	bool makeBonusList();
	void makeGrantedList();
	void updateFeatsRemainLabel();
	void addFeatToList(uint32 featIndex, bool isChosenFeat, bool isMovable);

	WidgetListBox *_availableWidgetList;
	WidgetListBox *_chosenWidgetList;

	CharHelpInfo *_featInfo;
	CharFeatsPopUp *_featsPopUp;

	uint _availableFeats;
	std::vector<bool> _isMasterFeatDone;
	bool _hasBonusFeat;
	int32 _bonusFeat;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARFEATS_H
