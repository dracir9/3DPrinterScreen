/**
 * @file   draw_Scr.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 24-06-2022
 * -----
 * Last Modified: 24-06-2022
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2022 Ricard Bitriá Ribes
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

#ifndef DRAW_SCR_H
#define DRAW_SCR_H

#include "../lcdUI.h"


class Draw_Scr : public Screen
{
public:
    Draw_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);
    void update(const uint32_t deltaTime, TchScr_Drv& ts) override;
    void render(tftLCD& tft) override;
    void handleTouch(const TchEvent& event) override;

private:
    bool touched = false;
    Vec2h point;
};

#endif // DRAW_SCR_H
