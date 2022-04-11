/**
 * @file   display_conf_Scr.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 11-04-2022
 * -----
 * Last Modified: 11-04-2022
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

#ifndef DISPLAY_CONF_SCR_H
#define DISPLAY_CONF_SCR_H

#include "../lcdUI.h"

class DisplayConf_Scr final: public Screen
{
public:
    DisplayConf_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);

    void update(uint32_t deltaTime, TchScr_Drv& ts) override;
    void render(tftLCD& tft) override;
    void handleTouch(const TchEvent& event) override;

private:
    bool engaged = false;
    int16_t sliderX = 0;
};

#endif // DISPLAY_CONF_SCR_H
