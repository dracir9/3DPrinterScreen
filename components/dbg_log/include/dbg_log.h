/**
 * @file   dbg_log.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 28-09-2021
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

#ifndef DBG_LOG_H
#define DBG_LOG_H

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DBG_LOG_NONE    0   /*!< No log output */
#define DBG_LOG_ERROR   1   /*!< Critical errors, software module can not recover on its own */
#define DBG_LOG_WARN    2   /*!< Error conditions from which recovery measures have been taken */
#define DBG_LOG_INFO    3   /*!< Information messages which describe normal flow of events */
#define DBG_LOG_DEBUG   4   /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
#define DBG_LOG_VERBOSE 5   /*!< All information of the process. */

#define CONFIG_DBG_LOG_LEVEL   DBG_LOG_DEBUG
#define CONFIG_DBG_COUNT

/**
 * @brief Ever increasing counter
 * 
 * @return uint32_t counter value
 */
uint32_t dbg_log_cnt();

/**
 * @brief Create new section
 * 
 * @param id Section number ID (0 < id < 64)
 */
void dbg_log_define_section(uint8_t id);

/**
 * @brief Enable/disable debug level sections
 * 
 * @param id    Section ID
 * @param state New section state
 */
void dbg_log_Dlevel_set(uint8_t id, bool state);

/**
 * @brief Enable/disable verbose level sections
 * 
 * @param id    Section ID
 * @param state New section state
 */
void dbg_log_Vlevel_set(uint8_t id, bool state);

/**
 * @brief If section is enabled for the corresponding level print message to UART
 * 
 * @param level     Logging level
 * @param format    Message
 * @param ... 
 */
void dbg_log_write_s(uint8_t level, const char *format, ...);

/**
 * @brief Print message to UART
 * 
 * @param format Message
 * @param ... 
 */
void dbg_log_write(const char *format, ...);

#ifndef __FILENAME__
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifdef CONFIG_DBG_COUNT
#define DBG_FORMAT(letter, format) LOG_COLOR_ ## letter #letter " (%lu) %s:%u: " format LOG_RESET_COLOR "\n", dbg_log_cnt(), __FILENAME__, __LINE__
#else
#if CONFIG_LOG_TIMESTAMP_SOURCE_RTOS
#define DBG_FORMAT(letter, format) LOG_COLOR_ ## letter #letter " (%u) %s:%u: " format LOG_RESET_COLOR "\n", esp_log_timestamp(), __FILENAME__, __LINE__
#elif CONFIG_LOG_TIMESTAMP_SOURCE_SYSTEM
#define DBG_FORMAT(letter, format) LOG_COLOR_ ## letter #letter " (%s) %s:%u: " format LOG_RESET_COLOR "\n", esp_log_system_timestamp(), __FILENAME__, __LINE__
#endif //CONFIG_LOG_TIMESTAMP_SOURCE_xxx
#endif

#define DBG_LOG_EARLY_IMPL(format, level, letter, ...) if (LOG_LOCAL_LEVEL >= level) \
        esp_rom_printf(DBG_FORMAT(letter, format), ##__VA_ARGS__)

/// macro to output logs in startup code, before heap allocator and syscalls have been initialized.
#define DBG_EARLY_LOGE(format, ... ) DBG_LOG_EARLY_IMPL(format, ESP_LOG_ERROR,   E, ##__VA_ARGS__)
#define DBG_EARLY_LOGW(format, ... ) DBG_LOG_EARLY_IMPL(format, ESP_LOG_WARN,    W, ##__VA_ARGS__)
#define DBG_EARLY_LOGI(format, ... ) DBG_LOG_EARLY_IMPL(format, ESP_LOG_INFO,    I, ##__VA_ARGS__)
#define DBG_EARLY_LOGD(format, ... ) DBG_LOG_EARLY_IMPL(format, ESP_LOG_DEBUG,   D, ##__VA_ARGS__)
#define DBG_EARLY_LOGV(format, ... ) DBG_LOG_EARLY_IMPL(format, ESP_LOG_VERBOSE, V, ##__VA_ARGS__)

// Macros
#ifndef BOOTLOADER_BUILD
    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_VERBOSE
        #define DBG_LOGV(format, ...) dbg_log_write_s(DBG_LOG_VERBOSE, DBG_FORMAT(V, format), ##__VA_ARGS__)
    #else
        #define DBG_LOGV(format, ...)
    #endif

    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_DEBUG
        #define DBG_LOGD(format, ...) dbg_log_write_s(DBG_LOG_DEBUG, DBG_FORMAT(D, format), ##__VA_ARGS__)
        #define DBG_LOGD_IF(__e, format, ...) ({if (__e) {DBG_LOGD(format, ##__VA_ARGS__);}})
    #else
        #define DBG_LOGD(format, ...)
        #define DBG_LOGD_IF(__e, format, ...)
    #endif

    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_INFO
        #define DBG_LOGI(format, ...) dbg_log_write(DBG_FORMAT(I, format), ##__VA_ARGS__)
    #else
        #define DBG_LOGI(format, ...)
    #endif

    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_WARN
        #define DBG_LOGW(format, ...) dbg_log_write(DBG_FORMAT(W, format), ##__VA_ARGS__)
        #define DBG_LOGW_IF(__e, format, ...) ({if (__e) {DBG_LOGW(format, ##__VA_ARGS__);}})
    #else
        #define DBG_LOGW(format, ...)
        #define DBG_LOGW_IF(__e, format, ...)
    #endif

    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_ERROR
        #define DBG_LOGE(format, ...) dbg_log_write(DBG_FORMAT(E, format), ##__VA_ARGS__)
        #define DBG_LOGE_IF(__e, format, ...) ({if (__e) {DBG_LOGE(format, ##__VA_ARGS__);}})
        #define DBG_LOGE_AND_RETURN_IF(__e, ret, format, ...) ({    \
                if (__e) {                                          \
                    DBG_LOGE(format, ##__VA_ARGS__);                \
                    return ret;                                     \
                }})
    #else
        #define DBG_LOGE(format, ...)
        #define DBG_LOGE_IF(__e, format, ...)
        #define DBG_LOGE_AND_RETURN_IF(__e, ret, format, ...)
    #endif
#else
#define DBG_LOGV(format, ...)  DBG_EARLY_LOGE(tag, format, ##__VA_ARGS__)
#define DBG_LOGD(format, ...)  DBG_EARLY_LOGW(tag, format, ##__VA_ARGS__)
#define DBG_LOGI(format, ...)  DBG_EARLY_LOGI(tag, format, ##__VA_ARGS__)
#define DBG_LOGW(format, ...)  DBG_EARLY_LOGD(tag, format, ##__VA_ARGS__)
#define DBG_LOGE(format, ...)  DBG_EARLY_LOGV(tag, format, ##__VA_ARGS__)
#endif  // BOOTLOADER_BUILD

#ifdef __cplusplus
}
#endif

#endif // SAFESERIAL_H
