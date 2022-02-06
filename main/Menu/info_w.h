/**
 * @file   info_w.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 06-02-2022
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

#ifndef INFO_W_H
#define INFO_W_H

#include "../lcdUI.h"

class Info_W final: public Screen
{
public:
    Info_W(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);

    void update(uint32_t deltaTime, TchScr_Drv& ts) override;
    void render(tftLCD& tft) override;
    void handleTouch(const TchEvent& event) override;

private:
    const uint8_t heatbed = 1;//random(0,2);
    const uint8_t tools = 2;//random(1,6);
    const uint8_t fans = 2;//random(0, min(8-tools-heatbed, 4));
    const uint8_t items = heatbed + tools + fans;
    const float cellAdv;
    const uint16_t cellW;
    int nextP = 0;
};

#endif
