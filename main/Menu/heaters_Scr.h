/**
 * @file   heaters_Scr.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 12-08-2023
 * -----
 * Last Modified: 12-08-2023
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

#ifndef HEATERS_SCR_H
#define HEATERS_SCR_H

#include "../lcdUI.h"
#include "../printer.h"

class Heaters_Scr final: public Screen
{
public:
    Heaters_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);

    void update(uint32_t deltaTime, TchScr_Drv& ts) override;
    void render(tftLCD& tft) override;
    void handleTouch(const TchEvent& event) override;

private:
    Printer* _printer = Printer::instance();

    static constexpr float buttSize = 80.0f;

    uint8_t numTools = 2;
    float stepSize = 5.0f;
    std::vector<float> pos_E;
};

#endif