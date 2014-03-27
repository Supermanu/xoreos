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

/** @file engines/nwn/gui/widgets/editbox.h
 *  A NWN editbox widget.
 */

#ifndef ENGINES_NWN_GUI_WIDGETS_EDITBOX_H
#define ENGINES_NWN_GUI_WIDGETS_EDITBOX_H

#include "graphics/font.h"

#include "engines/nwn/gui/widgets/modelwidget.h"

#include "graphics/aurora/fontman.h"

namespace Common {
class UString;
}

namespace Engines {

namespace NWN {

class WidgetButton;
class WidgetScrollbar;

/** A NWN editbox widget. */
class WidgetEditBox : public ModelWidget {
public:
	WidgetEditBox(::Engines::GUI &gui, const Common::UString &tag,
	              const Common::UString &model, const Common::UString &font);
	~WidgetEditBox();

	void show();
	void hide();

	void setTitle(Common::UString title);
	void setMainText(Common::UString &mainText);
	/** Set position's widget adjustement. Default : x = +9, y = -199. **/
	void setAdjustement(int x, int y);
	void subActive(Widget &widget);
	void mouseDown(uint8 state, float x, float y);
	void mouseWheel(uint8 state, int x, int y);

private:
	void createScrollbar();
	void createTitle();
	void scrollUp(uint n);
	void scrollDown(uint n);
	void getProperties();
	void updateScrollbarLength();
	void updateScrollbarPosition();

	Graphics::Aurora::FontHandle _fontHandle;
	Graphics::Aurora::Text       *_title;
	std::vector<Graphics::Aurora::Text *> _mainText;
	WidgetButton    *_up;
	WidgetButton    *_down;
	WidgetScrollbar *_scrollbar;
	bool _hasScrollbar;
	bool _hasTitle;
	int  _xAdjust;
	int  _yAdjust;
	// There is 18 lines to show
	Uint8 _firstLineToShow;
	bool  _stillPressed;

};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_WIDGETS_EDITBOX_H
