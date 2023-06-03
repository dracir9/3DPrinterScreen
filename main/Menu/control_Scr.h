/**
 * @file   control_Scr.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 10-04-2023
 * -----
 * Last Modified: 19-04-2023
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

#ifndef CONTROL_SCR_H
#define CONTROL_SCR_H

#include "../lcdUI.h"
#include "Vector.h"
#include "../printer.h"

class Control_Scr final: public Screen
{
public:
    Control_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);

    void update(uint32_t deltaTime, TchScr_Drv& ts) override;
    void render(tftLCD& tft) override;
    void handleTouch(const TchEvent& event) override;

private:
    Vec2h _tchPos;
    Vec2h _lastTchPos;

    Printer* _printer = Printer::instance();

    float stepSize = 5.0f;
    float zStepSize = 5.0f;

    bool xpPressed = false;
    bool xnPressed = false;
    bool ypPressed = false;
    bool ynPressed = false;
    bool zpPressed = false;
    bool znPressed = false;
};

#endif