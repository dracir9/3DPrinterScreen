/**
 * @file   Screen.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 25-02-2023
 * -----
 * Last Modified: 10-04-2023
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2023 Ricard Bitriá Ribes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef SCREEN_H
#define SCREEN_H

#include "tftLCD.h"
#include "TchScr_Drv.h"


// Forward declaration
class lcdUI;

/**************************************************************************
    Base frame of the UI
**************************************************************************/
class Screen
{
protected:
    lcdUI *_UI;

public:
    Screen(lcdUI* UI);
    virtual ~Screen();

    virtual void render(tftLCD& tft) = 0;
    virtual void update(const uint32_t deltaTime, TchScr_Drv& ts) = 0;
    virtual void handleTouch(const TchEvent& event);
};

#endif
