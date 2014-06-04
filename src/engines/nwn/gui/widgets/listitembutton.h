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

/** @file engines/nwn/gui/widgets/listitembutton.h
 *  Button widgets within a NWN listbox widget with at least a text and an icon.
 */

#ifndef WIDGETLISTITEMBUTTON_H
#define WIDGETLISTITEMBUTTON_H

#include "graphics/aurora/text.h"

#include "engines/nwn/gui/widgets/listbox.h"

namespace Engines {

class GUI;

namespace NWN {

class WidgetListItemButton : public WidgetListItem {
public:
	WidgetListItemButton(::Engines::GUI &gui, const Common::UString &model, const Common::UString &font, const Common::UString &text, const Common::UString &icon = "");
	virtual ~WidgetListItemButton();
	
	void mouseUp(uint8 state, float x, float y);
	void mouseDown(uint8 state, float x, float y);
	
	void show();
	void hide();
	
	float getWidth() const;
	float getHeight() const;
	void setPosition(float x, float y, float z);
	void setDisabled(bool disable);
	/** Check if the button have been disabled. Return false if it's the case. **/
	bool isAvailable();
	
	void setTag(const Common::UString &tag);
	
protected:
	bool activate();
	bool deactivate();
	Graphics::Aurora::Model *_button;
	Graphics::Aurora::Text *_text;
	PortraitWidget *_icon;
	
private:
	bool _available;
};

class WidgetListItemButtonWithHelp : public WidgetListItemButton {
public:
	WidgetListItemButtonWithHelp(Engines::GUI & gui, const Common::UString & model, const Common::UString & text, const Common::UString & icon, const Common::UString & tag);
	virtual ~WidgetListItemButtonWithHelp();
	
	void enter();
	void leave();
	
	void mouseDown(uint8 state, float x, float y);
	void mouseUp(uint8 state, float x, float y);
	
	virtual void setPosition(float x, float y, float z);
	
protected:
	virtual void helpActivated();
	Graphics::Aurora::Model *_helpButton;
};

class WidgetListItemExchange : public WidgetListItemButtonWithHelp {
public:
	WidgetListItemExchange(Engines::GUI &gui, const Common::UString &model, const Common::UString &text, const Common::UString & icon, const Common::UString &tag, const Common::UString &description, bool left, uint16 abstractIndex);
	virtual ~WidgetListItemExchange();

	void setPosition(float x, float y, float z);
	void subActive(Widget &widget);
	
	void getProperties(Common::UString &title, Common::UString &description, Common::UString &icon);
	void setProperties(const Common::UString &title, const Common::UString &description);
	void setUnmovable();
	
	uint16 getAbstractIndex() const;
	void setAbstractIndex(uint16 feat);
	
protected:
	WidgetButton *_addRemoveButton;
	
	Common::UString _description;
	bool _isMovable;
	bool _left;
	uint16 _abstractIndex;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // WIDGETLISTITEMBUTTON_H
