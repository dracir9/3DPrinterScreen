/**
 * @file   dbg_log.c
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 04-12-2021
 * -----
 * Last Modified: 06-06-2022
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2021 Ricard Bitriá Ribes
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

#include <stdio.h>
#include "esp_bit_defs.h"
#include "dbg_log.h"

static uint8_t activeSection = 0;
static uint64_t sectionD_state = UINT64_MAX;
static uint64_t sectionV_state = 0;
static uint32_t cnt = 0;

uint32_t dbg_log_cnt()
{
    return cnt++;
}

void dbg_log_define_section(uint8_t id)
{
    if (id >= 64) return;
    
    activeSection = id;
}

void dbg_log_Dlevel_set(uint8_t id, bool state)
{
    if (id >= 64) return;
    if (state)
    {
        sectionD_state |= BIT64(id);
    }
    else
    {
        sectionD_state &= ~BIT64(id);
    }
}

void dbg_log_Vlevel_set(uint8_t id, bool state)
{
    if (id >= 64) return;
    if (state)
    {
        sectionV_state |= BIT64(id);
    }
    else
    {
        sectionV_state &= ~BIT64(id);
    }
}

static bool should_output(uint8_t level, uint8_t section)
{
    if (level == DBG_LOG_DEBUG)
    {
        return (bool)(sectionD_state & BIT64(section));
    }
    else if (level == DBG_LOG_VERBOSE)
    {
        return (bool)(sectionV_state & BIT64(section));
    }
    
    return true;
}

void dbg_log_write_s(uint8_t level, const char *format, ...)
{
    va_list list;
    va_start(list, format);
    if (should_output(level, activeSection))
    {
        vprintf(format, list);
    }
    va_end(list);
}

void dbg_log_write(const char *format, ...)
{
    va_list list;
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
}
