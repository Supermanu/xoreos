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
 */

/** @file engines/nwn/gui/widgets/listbox.h
 *  A NWN listbox widget.
 */

#ifndef ENGINES_NWN_GUI_WIDGETS_LISTBOX_H
#define ENGINES_NWN_GUI_WIDGETS_LISTBOX_H

#include "common/types.h"

#include "graphics/aurora/types.h"

#include "engines/nwn/gui/widgets/modelwidget.h"
#include "engines/nwn/gui/widgets/portrait.h"


namespace Common {
	class UString;
}

namespace Engines {

class GUI;

namespace NWN {

class WidgetButton;
class WidgetScrollbar;

/** An item widget within a NWN listbox widget. */
class WidgetListItem : public NWNWidget {
public:
	WidgetListItem(::Engines::GUI &gui);
	~WidgetListItem();

	void mouseUp  (uint8 state, float x, float y);
	void mouseWheel(uint8 state, int x, int y);

	void mouseDblClick(uint8 state, float x, float y);

	void select();

protected:
	bool getState();

	virtual bool activate();
	virtual bool deactivate();

	void signalGroupMemberActive();
	uint _itemNumber;

private:
	bool _state;

	friend class WidgetListBox;
};

/** A text line widget within a NWN listbox widget. */
class WidgetListItemTextLine : public WidgetListItem {
public:
	WidgetListItemTextLine(::Engines::GUI &gui, const Common::UString &font,
	                       const Common::UString &text, float spacing = 0.0);
	~WidgetListItemTextLine();

	void show();
	void hide();

	void setPosition(float x, float y, float z);

	void setUnselectedColor(float r, float g, float b, float a);
	void setSelectedColor(float r, float g, float b, float a);

	float getWidth () const;
	float getHeight() const;

	void setTag(const Common::UString &tag);

protected:
	bool activate();
	bool deactivate();

private:
	Graphics::Aurora::Text *_text;

	float _uR, _uG, _uB, _uA;
	float _sR, _sG, _sB, _sA;

	float _fontHeight;
	float _spacing;
};

/** A button widget within a NWN listbox widget with a text and an icon. */
class WidgetListItemButton : public WidgetListItem {
public:
	WidgetListItemButton(::Engines::GUI &gui, const Common::UString &model, const Common::UString &font, const Common::UString &text, const Common::UString &icon = "");
	~WidgetListItemButton();

	void mouseUp(uint8 state, float x, float y);

	void show();
	void hide();

	float getWidth() const;
	float getHeight() const;
	void setPosition(float x, float y, float z);
	void setDisabled(bool disable);

	void setTag(const Common::UString &tag);

protected:
	bool activate();
	bool deactivate();

private:
	Graphics::Aurora::Model *_button;
	Graphics::Aurora::Text *_text;
	PortraitWidget *_icon;

	bool _available;
};

/** A NWN listbox widget. */
class WidgetListBox : public ModelWidget {
public:
	enum Mode {
		kModeStatic     = 0,
		kModeSelectable
	};
	
	enum ViewStyle {
		kViewStyleOneColumn	= 0,
		kViewStyleColumns
	};

	WidgetListBox(::Engines::GUI &gui, const Common::UString &tag,
	              const Common::UString &model);
	~WidgetListBox();

	Mode getMode() const;
	void setMode(Mode mode);
	void setViewStyle(ViewStyle viewStyle);
	ViewStyle getViewStyle() const;

	void show();
	void hide();

	void setPosition(float x, float y, float z);

	float getContentWidth () const;
	float getContentHeight() const;

	void lock();
	void clear();
	void reserve(uint n);
	void add(WidgetListItem *item);
	void unlock();

	void setText(const Common::UString &font, const Common::UString &text,
	             float spacing = 0.0);

	void select(uint item);
	uint getSelected() const;

	WidgetListItem * getItem(uint item) const;

	bool wasDblClicked();

	void mouseDown(uint8 state, float x, float y);
	void mouseWheel(uint8 state, int x, int y);

	/** Set the scrollbar position.
	 * @param position The value must be in the interval [0,1]. 
	 */
	void setScrollBarPosition(float position);

	void subActive(Widget &widget);

private:
	Mode _mode;
	ViewStyle _viewStyle;

	float _contentX;
	float _contentY;
	float _contentZ;

	float _contentWidth;
	float _contentHeight;

	bool _hasScrollbar;
	WidgetButton    *_up;
	WidgetButton    *_down;
	WidgetScrollbar *_scrollbar;

	bool _dblClicked;

	std::vector<WidgetListItem *> _items;
	std::vector<WidgetListItem *> _visibleItems;

	uint _startItem;
	uint _selectedItem;

	bool _locked;

	void getProperties();

	void createScrollbar();

	void updateScrollbarLength();
	void updateScrollbarPosition();

	void scrollUp(uint n);
	void scrollDown(uint n);

	void updateVisible();

	void itemDblClicked();

	uint _itemsByRow;

	friend class WidgetListItem;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_WIDGETS_LISTBOX_H
