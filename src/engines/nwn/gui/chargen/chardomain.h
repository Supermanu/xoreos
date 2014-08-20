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

/** @file engines/nwn/gui/chargen/chardomain.h
 *  The NWN domains selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARDOMAIN_H
#define ENGINES_NWN_GUI_CHARGEN_CHARDOMAIN_H

#include "engines/nwn/gui/widgets/listbox.h"
#include "engines/nwn/gui/widgets/editbox.h"
#include "engines/nwn/gui/widgets/listitembutton.h"

#include "engines/nwn/gui/chargen/chargenabstract.h"

namespace Engines {

namespace NWN {

class CharDomain;

class WidgetListItemDomain : public WidgetListItemExchange {
public:
	WidgetListItemDomain(Engines::NWN::GUI &gui, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, const Common::UString &description, bool left, uint16 domainIndex);
	~WidgetListItemDomain();

	virtual bool activate();

	friend class CharDomain;
};

class CharDomain : public CharGenAbstract {
public:
	CharDomain();
	~CharDomain();

	void reset();
	void show();
	void fixWidgetType(const Common::UString &tag, WidgetType &type);
	void moveDomain(WidgetListItemDomain *item, bool left, uint itemNumber);

private:
	void callbackActive(Widget &widget);
	void createDomainList();
	void setHelp(WidgetListItemDomain &item);

	WidgetListBox *_domainListBox;
	WidgetEditBox *_helpBox;
	WidgetListItemDomain *_firstDomain;
	WidgetListItemDomain *_secondDomain;

	friend class WidgetListItemDomain;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARDOMAIN_H
