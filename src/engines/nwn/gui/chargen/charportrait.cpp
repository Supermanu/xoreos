/* xoreos - A reimplementation of BioWare's Aurora engine
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

#include "aurora/2dareg.h"
#include "aurora/2dafile.h"
#include "aurora/resman.h"
#include "aurora/gfffile.h"

#include "engines/nwn/gui/chargen/charportrait.h"

#include "engines/nwn/gui/widgets/button.h"

// #include "engines/nwn/types.h"


namespace Engines {

namespace NWN {

WidgetListItemPortrait::WidgetListItemPortrait(::Engines::GUI &gui, const Common::UString &portrait, float vSpacing, float hSpacing) : WidgetListItem(gui), _hSpacing(hSpacing), _vSpacing(vSpacing), _border(0) {
	_portrait = new PortraitWidget(*_gui, portrait, portrait + "m", Portrait::kSizeMedium);
	setTag(portrait);
}

WidgetListItemPortrait::~WidgetListItemPortrait() {
	delete _portrait;
}

void WidgetListItemPortrait::show() {
	_portrait->show();
}

void WidgetListItemPortrait::hide() {
	_portrait->hide();
}

void WidgetListItemPortrait::mouseDown(uint8 state, float x, float y) {
	_owner->mouseDown(state, x, y);
	WidgetListItem::mouseDown(state, x, y);
}

bool WidgetListItemPortrait::activate() {
	if (!WidgetListItem::activate())
		return false;

	((CharPortrait *) _gui)->setMainPortrait(getTag() + "h");
	_portrait->setBorder(1.0);
	return true;
}

bool WidgetListItemPortrait::deactivate() {
	if (!WidgetListItem::deactivate())
		return false;

	_portrait->setBorder(0.0);
	return true;

}

float WidgetListItemPortrait::getHeight() const {
	return _portrait->getHeight() + _vSpacing ;
}

float WidgetListItemPortrait::getWidth() const {
	return _portrait->getWidth() + _hSpacing;
}

void WidgetListItemPortrait::setPosition(float x, float y, float z) {
	NWNWidget::setPosition(x, y, z);

	getPosition(x, y, z);

	_portrait->setPosition(x, y, z);
}


CharPortrait::CharPortrait(Module &module, Creature &character) : _module(&module), _character(&character), _selected("") {
	load("cg_portrait");

	getWidget("Title"	, true)->setHorCentered();
	_bigPortraitPanel = getPanel("Portrait", true);
	_mainPortrait = new PortraitWidget(*this, _bigPortraitPanel->getTag() + "#PortraitWidget", "mainPortrait" , Portrait::kSizeHuge);

	float pX, pY, pZ;
	_bigPortraitPanel->getPosition(pX, pY, pZ);
	_mainPortrait->setPosition(pX + 3, pY + 3, pZ - 1);
	_mainPortrait->setPortrait(_character->getPortrait() + "h");
	_bigPortraitPanel->addChild(*_mainPortrait);

	_portraitListBox = new WidgetListBox(*this, "ListBox", "ctl_cg_portraits");
	_portraitListBox->setPosition(pX + _mainPortrait->getWidth() + 9, pY, pZ);
	_portraitListBox->setViewStyle(WidgetListBox::kViewStyleColumns);
	addWidget(_portraitListBox);

	if (_mainPortrait->getPortrait() == "gui_po_nwnlogo_h")
		getWidget("OkButton", true)->setDisabled(true);

	initPortraitList();
}

CharPortrait::~CharPortrait() {
	delete _mainPortrait;
	delete _bigPortraitPanel;
	delete _portraitListBox;
}

void CharPortrait::reset() const {
	_character->setPortrait("gui_po_nwnlogo_");
}

void CharPortrait::show() {
	buildPortraitList();
	_portraitListBox->show();
	_mainPortrait->setPortrait(_character->getPortrait() + "h");
	Engines::GUI::show();
}

void CharPortrait::hide() {
// 	Engines::GUI::hide();
	_portraitListBox->hide();
	Engines::GUI::hide();
}

void CharPortrait::setMainPortrait(Common::UString portrait) {
	_mainPortrait->setPortrait(portrait);
}

void CharPortrait::initPortraitList() {
	const Aurora::TwoDAFile &twoda = TwoDAReg.get("portraits");
	for (unsigned int it = 1; it < 140; ++it) {
		const Aurora::TwoDARow row = twoda.getRow(it);
		if (row.getInt("Race") == kRaceDwarf) {
			if (row.getInt("Sex") == Aurora::kGenderMale) {
				_dwarfMalePortraits.push_back("po_" + row.getString("BaseResRef"));
			} else if (row.getInt("Sex") == Aurora::kGenderFemale) {
				_dwarfFemalePortraits.push_back("po_" + row.getString("BaseResRef"));
			}
		}

		if (row.getInt("Race") == kRaceHuman) {
			if (row.getInt("Sex") == Aurora::kGenderMale) {
				_humanMalePortraits.push_back("po_" + row.getString("BaseResRef"));
			} else if (row.getInt("Sex") == Aurora::kGenderFemale) {
				_humanFemalePortraits.push_back("po_" + row.getString("BaseResRef"));
			}
		}

		if (row.getInt("Race") == kRaceElf) {
			if (row.getInt("Sex") == Aurora::kGenderMale) {
				_elfMalePortraits.push_back("po_" + row.getString("BaseResRef"));
			} else if (row.getInt("Sex") == Aurora::kGenderFemale) {
				_elfFemalePortraits.push_back("po_" + row.getString("BaseResRef"));
			}
		}

		if (row.getInt("Race") == kRaceHalfling) {
			if (row.getInt("Sex") == Aurora::kGenderMale) {
				_halflingMalePortraits.push_back("po_" + row.getString("BaseResRef"));
			} else if (row.getInt("Sex") == Aurora::kGenderFemale) {
				_halflingFemalePortraits.push_back("po_" + row.getString("BaseResRef"));
			}
		}

		if (row.getInt("Race") == kRaceHalfOrc) {
			if (row.getInt("Sex") == Aurora::kGenderMale) {
				_halforcMalePortraits.push_back("po_" + row.getString("BaseResRef"));
			} else if (row.getInt("Sex") == Aurora::kGenderFemale) {
				_halforcFemalePortraits.push_back("po_" + row.getString("BaseResRef"));
			}
		}

		if (row.getInt("Race") == kRaceGnome) {
			if (row.getInt("Sex") == Aurora::kGenderMale) {
				_gnomeMalePortraits.push_back("po_" + row.getString("BaseResRef"));
			} else if (row.getInt("Sex") == Aurora::kGenderFemale) {
				_gnomeFemalePortraits.push_back("po_" + row.getString("BaseResRef"));
			}
		}
	}

	_portraitsMale.push_back(_dwarfMalePortraits);
	_portraitsMale.push_back(_elfMalePortraits);
	_portraitsMale.push_back(_gnomeMalePortraits);
	_portraitsMale.push_back(_halflingMalePortraits);
	_portraitsMale.push_back(_humanMalePortraits);
	_portraitsMale.push_back(_halforcMalePortraits);
	_portraitsMale.push_back(_humanMalePortraits);

	_portraitsFemale.push_back(_dwarfFemalePortraits);
	_portraitsFemale.push_back(_elfFemalePortraits);
	_portraitsFemale.push_back(_gnomeFemalePortraits);
	_portraitsFemale.push_back(_halflingFemalePortraits);
	_portraitsFemale.push_back(_humanFemalePortraits);
	_portraitsFemale.push_back(_halforcFemalePortraits);
	_portraitsFemale.push_back(_humanFemalePortraits);
}

void CharPortrait::buildPortraitList() {
	_portraitListBox->lock();
	_portraitListBox->clear();
	_portraitListBox->setMode(WidgetListBox::kModeSelectable);

	if (_character->getGender() == Aurora::kGenderMale) {
		for (std::vector<Common::UString>::iterator it = _portraitsMale.at(_character->getRace()).begin(); it != _portraitsMale.at(_character->getRace()).end(); ++it) {
			_portraitListBox->add(new WidgetListItemPortrait(*this, (*it), 10.0, 6.0));
		}
		for (Uint32 it = 0; it < 7; ++it) {
			//The portraits for human and halfelf are the same.
			if (it == kRaceHalfElf || it == _character->getRace())
				continue;

			for (std::vector<Common::UString>::iterator subIt = _portraitsMale.at(it).begin(); subIt != _portraitsMale.at(it).end(); ++subIt) {
				WidgetListItemPortrait *itemPortrait = new WidgetListItemPortrait(*this, (*subIt), 10.0, 6.0);
				_portraitListBox->add(itemPortrait);
			}
		}
	} else if (_character->getGender() == Aurora::kGenderFemale) {
		for (std::vector<Common::UString>::iterator it = _portraitsFemale.at(_character->getRace()).begin(); it != _portraitsFemale.at(_character->getRace()).end(); ++it) {
			_portraitListBox->add(new WidgetListItemPortrait(*this, (*it), 10.0, 6.0));
		}
		for (Uint32 it = 0; it < 7; ++it) {
			//The portraits for human and halfelf are the same.
			if (it == kRaceHalfElf || it == _character->getRace())
				continue;

			for (std::vector<Common::UString>::iterator subIt = _portraitsFemale.at(it).begin(); subIt != _portraitsFemale.at(it).end(); ++subIt) {
				WidgetListItemPortrait *itemPortrait = new WidgetListItemPortrait(*this, (*subIt), 10.0, 6.0);
				_portraitListBox->add(itemPortrait);
			}
		}
	}
	_portraitListBox->unlock();
}

void CharPortrait::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		if (_portraitListBox->getSelected() != 0xFFFFFFFF) {
			// The last letter indicate only the size of the portrait, we do not need it.
			_selected = _mainPortrait->getPortrait();
			_selected.erase(--(_selected.end()));
			_character->setPortrait(_selected);
			_returnCode = 1;
		}
		return;
	}

	if (widget.getTag() == "ListBox")
		getButton("OkButton")->setDisabled(false);
}

} // End of namespace NWN

} // End of namespace Engines
