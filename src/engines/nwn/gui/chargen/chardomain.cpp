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

/** @file engines/nwn/gui/chargen/chardomain.cpp
 *  The NWN domains selection for the character generator.
 */

#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "graphics/aurora/model.h"

#include "engines/nwn/gui/widgets/listitembutton.h"
#include "engines/nwn/gui/widgets/button.h"

#include "engines/nwn/gui/chargen/chardomain.h"


namespace Engines {

namespace NWN {

WidgetListItemDomain::WidgetListItemDomain(Engines::NWN::GUI &gui, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, const Common::UString &description, bool left, uint16 domainIndex) : WidgetListItemExchange(gui, "ctl_cg_btn_dom", text, icon, tag, description, left, domainIndex) {
	_helpButton->hide();
}

WidgetListItemDomain::~WidgetListItemDomain() {
}

bool WidgetListItemDomain::activate() {
	bool isDomainChoosen = false;

	WidgetListItemDomain *firstDomain = ((CharDomain *) _gui)->_firstDomain;
	WidgetListItemDomain *secondDomain = ((CharDomain *) _gui)->_secondDomain;

	if (firstDomain == this) {
		if (secondDomain) {
			secondDomain->deactivate();
			secondDomain->_button->setState("up");
		}

		isDomainChoosen = true;
	}

	if (secondDomain == this) {
		firstDomain->deactivate();
		firstDomain->_button->setState("up");
		isDomainChoosen = true;
	}

	if (isDomainChoosen) {
		_button->setState("down");
		((CharDomain *) _gui)->callbackActive(*this);
	}

	return Engines::NWN::WidgetListItemButton::activate();
}

CharDomain::CharDomain() : _firstDomain(0), _secondDomain(0){
	load("cg_domain");
	getWidget("Title", true)->setHorCentered();

	_domainListBox = getListBox("DomainListBox", true);
	_helpBox = getEditBox("DomainDesc", true);

	getButton("OkButton", true)->setDisabled(true);
	getButton("RecommendButton", true)->setDisabled(true);
}

CharDomain::~CharDomain() {
}

void CharDomain::reset() {
	if (_secondDomain)
		moveDomain(_secondDomain, true, 0);

	if (_firstDomain)
		moveDomain(_firstDomain, true, 0);
}

void CharDomain::show() {
	createDomainList();

	Engines::GUI::show();
}


void CharDomain::fixWidgetType(const Common::UString &tag, NWN::GUI::WidgetType &type) {
	if (tag == "DomainListBox")
		type = NWN::GUI::kWidgetTypeListBox;
}

void CharDomain::createDomainList() {
	_domainListBox->lock();
	_domainListBox->clear();
	_domainListBox->setMode(WidgetListBox::kModeSelectable);

	const Aurora::TwoDAFile &twodaDomains = TwoDAReg.get("domains");
	for (uint dom = 0; dom < twodaDomains.getRowCount(); ++dom) {
		const Aurora::TwoDARow &row = twodaDomains.getRow(dom);
		if (row.isEmpty("Name"))
			continue;

		Common::UString name = TalkMan.getString(row.getInt("Name"));
		Common::UString descript = TalkMan.getString(row.getInt("Description"));
		Common::UString icon = row.getString("Icon");

		WidgetListItemDomain *domItem = new WidgetListItemDomain(*this, name, icon, name, descript, false, dom);
		_domainListBox->add(domItem);
	}
	
	_domainListBox->unlock();
}

void CharDomain::moveDomain(WidgetListItemDomain *item, bool left, uint itemNumber) {
	if (_secondDomain && !left)
		return;

	removeFocus();
	_domainListBox->lock();

	item->changeOrientation();
	if (left) {
		// If the first domain is removed, move the second one to the first one position.
		item->hide();
		if (item == _firstDomain && _secondDomain) {
			float pX, pY, pZ;
			_firstDomain->getPosition(pX, pY, pZ);
			_secondDomain->setPosition(pX, pY, pZ);
			_firstDomain = _secondDomain;
			_secondDomain = 0;
		}

		if (item == _firstDomain)
			_firstDomain = 0;

		if (item == _secondDomain)
			_secondDomain = 0;

		_domainListBox->add(item);
		_domainListBox->sortByTag();
		getButton("OkButton", true)->setDisabled(true);
	} else {
		_domainListBox->removeItem(itemNumber);
		_domainListBox->deselect();

		float pX, pY, pZ;
		if (_firstDomain) {
			_secondDomain = item;
			_helpBox->getNodePosition("domain2", pX, pY, pZ);
			_secondDomain->setPosition(pX, pY, -150);
			getButton("OkButton")->setDisabled(false);
		} else {
			_firstDomain = item;
			_helpBox->getNodePosition("domain1", pX, pY, pZ);
			_firstDomain->setPosition(pX, pY, -150);
		}

		item->show();
	}

	_domainListBox->unlock();
}

void CharDomain::setHelp(WidgetListItemDomain &item) {
	Common::UString domName, domDescript, icon;
	item.getProperties(domName, domDescript, icon);
	_helpBox->setTitle(domName);
	_helpBox->setMainText(domDescript);
}

void CharDomain::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}
	
	if (widget.getTag() == "OkButton") {
		_characterChoices.domain[0] = _firstDomain->getAbstractIndex();
		_characterChoices.domain[1] = _secondDomain->getAbstractIndex();
		_returnCode = 2;
		return;
	}

	if (widget.getTag() == "DomainListBox") {
		if (_firstDomain)
			_firstDomain->_button->setState("up");
		
		if (_secondDomain)
			_secondDomain->_button->setState("up");

		uint itemNumber = _domainListBox->getSelected();
		setHelp(*((WidgetListItemDomain *) _domainListBox->getItem(itemNumber)));
		return;
	}

	if (_firstDomain) {
		WidgetListItemDomain *choosenDomain = 0;
		if (widget.getTag() == _firstDomain->getTag()) {
			choosenDomain = _firstDomain;
		}

		if (_secondDomain) {
			if (widget.getTag() == _secondDomain->getTag()) {
				choosenDomain = _secondDomain;
			}
		}

		if (choosenDomain) {
			_domainListBox->deselect();
			setHelp(*choosenDomain);
			return;
		}
	}

	
}

} // End of namespace NWN

} // End of namespace Engines
