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
 *
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/chargen/charfeatspopup.h"

namespace Engines {
	
namespace NWN {

CharFeatsPopUp::CharFeatsPopUp(): _feat(-1){
	load("cg_feats_popup");

	setPosition(0, 0, -120);
	_masterFeat = 0;
}

CharFeatsPopUp::~CharFeatsPopUp(){
}

void CharFeatsPopUp::reset() {
}

int16 CharFeatsPopUp::getFeats() const {
	return _feat;
}

void CharFeatsPopUp::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "FeatsList")
		type = NWN::GUI::kWidgetTypeListBox;
}

void CharFeatsPopUp::callbackActive(Widget &widget) {
	if (widget.getTag() == "OkButton") {
		_feat = _feats[getListBox("FeatsList", true)->getSelected()];
		_returnCode = 2;
	}

	if (widget.getTag() == "CancelButton")
		_returnCode = 1;
}

void CharFeatsPopUp::buildList(uint16 masterFeat) {
	const Aurora::TwoDAFile &twodaFeats = TwoDAReg.get("feat");
	
	Engines::NWN::WidgetListBox* featList = getListBox("FeatsList", true);
	featList->lock();
	featList->clear();
	featList->setMode(WidgetListBox::kModeSelectable);

	_feats.clear();
	for (uint it = 0; it < twodaFeats.getRowCount(); ++it) {
		if (!_characterChoices.hasPrereqFeat(it))
			continue;

		const Aurora::TwoDARow &row = twodaFeats.getRow(it);

		if (row.isEmpty("MASTERFEAT"))
			continue;

		if (row.getInt("MASTERFEAT") != masterFeat)
			continue;

		WidgetListItemTextLine *featItem = new WidgetListItemTextLine(*this, "fnt_galahad14", TalkMan.getString(row.getInt("FEAT")));
		featList->add(featItem);
		_feats.push_back(it);
	}

	featList->unlock();
}

} // End of namespace NWN

} // End of namespace Engines
