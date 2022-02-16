/**
 * @file   gcodePreview_Scr.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 16-02-2022
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

#ifndef GCODEPREVIEW_SCR_H
#define GCODEPREVIEW_SCR_H

#include "../lcdUI.h"
#include <bitset>
#include "../parser.h"
#include "../utility.h"
#include "GCodeRenderer.h"

class GcodePreview_Scr final : public Screen
{
public:
    GcodePreview_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);

    void update(const uint32_t deltaTime, TchScr_Drv& ts) override;

    void render(tftLCD& tft) override;

    void handleTouch(const TchEvent& event) override;

private:
    void drawInfo(tftLCD& tft);

    // Status and info
    std::bitset<2> displayed;
    int32_t printTime = 0;
    float filament = 0.0f;
    GCodeRenderer* renderEngine;
    bool started = false;
};

#endif