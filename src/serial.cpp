
/**
 * -----------------
 * G-Codes in Marlin
 * -----------------
 *
 * Helpful G-code references:
 *  - http://linuxcnc.org/handbook/gcode/g-code.html
 *  - http://objects.reprap.org/wiki/Mendel_User_Manual:_RepRapGCodes
 *
 * Help to document Marlin's G-codes online:
 *  - http://reprap.org/wiki/G-code
 *  - https://github.com/MarlinFirmware/MarlinDocumentation
 *
 * -----------------
 *
 * "G" Codes
 *
 * G0   -> G1
 * G1   - Coordinated Movement X Y Z E
 * G2   - CW ARC
 * G3   - CCW ARC
 * G4   - Dwell S<seconds> or P<milliseconds>
 * G5   - Cubic B-spline with XYZE destination and IJPQ offsets
 * G6   - Direct stepper move (Requires UNREGISTERED_MOVE_SUPPORT). Hangprinter defaults to relative moves. Others default to absolute moves.
 * G10  - Retract filament according to settings of M207 (Requires FWRETRACT)
 * G11  - Retract recover filament according to settings of M208 (Requires FWRETRACT)
 * G12  - Clean tool (Requires NOZZLE_CLEAN_FEATURE)
 * G17  - Select Plane XY (Requires CNC_WORKSPACE_PLANES)
 * G18  - Select Plane ZX (Requires CNC_WORKSPACE_PLANES)
 * G19  - Select Plane YZ (Requires CNC_WORKSPACE_PLANES)
 * G20  - Set input units to inches (Requires INCH_MODE_SUPPORT)
 * G21  - Set input units to millimeters (Requires INCH_MODE_SUPPORT)
 * G26  - Mesh Validation Pattern (Requires G26_MESH_VALIDATION)
 * G27  - Park Nozzle (Requires NOZZLE_PARK_FEATURE)
 * G28  - Home one or more axes
 * G29  - Start or continue the bed leveling probe procedure (Requires bed leveling)
 * G30  - Single Z probe, probes bed at X Y location (defaults to current XY location)
 * G31  - Dock sled (Z_PROBE_SLED only)
 * G32  - Undock sled (Z_PROBE_SLED only)
 * G33  - Delta Auto-Calibration (Requires DELTA_AUTO_CALIBRATION)
 * G38  - Probe in any direction using the Z_MIN_PROBE (Requires G38_PROBE_TARGET)
 * G42  - Coordinated move to a mesh point (Requires MESH_BED_LEVELING, AUTO_BED_LEVELING_BLINEAR, or AUTO_BED_LEVELING_UBL)
 * G90  - Use Absolute Coordinates
 * G91  - Use Relative Coordinates
 * G92  - Set current position to coordinates given
 * G95  - Set torque mode (Requires MECHADUINO_I2C_COMMANDS enabled)
 * G96  - Set encoder reference point (Requires MECHADUINO_I2C_COMMANDS enabled)
 *
 * "M" Codes
 *
 * M0   - Unconditional stop - Wait for user to press a button on the LCD (Only if ULTRA_LCD is enabled)
 * M1   -> M0
 * M3   - Turn laser/spindle on, set spindle/laser speed/power, set rotation to clockwise
 * M4   - Turn laser/spindle on, set spindle/laser speed/power, set rotation to counter-clockwise
 * M5   - Turn laser/spindle off
 * M17  - Enable/Power all stepper motors
 * M18  - Disable all stepper motors; same as M84
 * M20  - List SD card. (Requires SDSUPPORT)
 * M21  - Init SD card. (Requires SDSUPPORT)
 * M22  - Release SD card. (Requires SDSUPPORT)
 * M23  - Select SD file: "M23 /path/file.gco". (Requires SDSUPPORT)
 * M24  - Start/resume SD print. (Requires SDSUPPORT)
 * M25  - Pause SD print. (Requires SDSUPPORT)
 * M26  - Set SD position in bytes: "M26 S12345". (Requires SDSUPPORT)
 * M27  - Report SD print status. (Requires SDSUPPORT)
 *        OR, with 'S<seconds>' set the SD status auto-report interval. (Requires AUTO_REPORT_SD_STATUS)
 *        OR, with 'C' get the current filename.
 * M28  - Start SD write: "M28 /path/file.gco". (Requires SDSUPPORT)
 * M29  - Stop SD write. (Requires SDSUPPORT)
 * M30  - Delete file from SD: "M30 /path/file.gco"
 * M31  - Report time since last M109 or SD card start to serial.
 * M32  - Select file and start SD print: "M32 [S<bytepos>] !/path/file.gco#". (Requires SDSUPPORT)
 *        Use P to run other files as sub-programs: "M32 P !filename#"
 *        The '#' is necessary when calling from within sd files, as it stops buffer prereading
 * M33  - Get the longname version of a path. (Requires LONG_FILENAME_HOST_SUPPORT)
 * M34  - Set SD Card sorting options. (Requires SDCARD_SORT_ALPHA)
 * M42  - Change pin status via gcode: M42 P<pin> S<value>. LED pin assumed if P is omitted.
 * M43  - Display pin status, watch pins for changes, watch endstops & toggle LED, Z servo probe test, toggle pins
 * M48  - Measure Z Probe repeatability: M48 P<points> X<pos> Y<pos> V<level> E<engage> L<legs> S<chizoid>. (Requires Z_MIN_PROBE_REPEATABILITY_TEST)
 * M75  - Start the print job timer.
 * M76  - Pause the print job timer.
 * M77  - Stop the print job timer.
 * M78  - Show statistical information about the print jobs. (Requires PRINTCOUNTER)
 * M80  - Turn on Power Supply. (Requires POWER_SUPPLY > 0)
 * M81  - Turn off Power Supply. (Requires POWER_SUPPLY > 0)
 * M82  - Set E codes absolute (default).
 * M83  - Set E codes relative while in Absolute (G90) mode.
 * M84  - Disable steppers until next move, or use S<seconds> to specify an idle
 *        duration after which steppers should turn off. S0 disables the timeout.
 * M85  - Set inactivity shutdown timer with parameter S<seconds>. To disable set zero (default)
 * M92  - Set planner.axis_steps_per_mm for one or more axes.
 * M100 - Watch Free Memory (for debugging) (Requires M100_FREE_MEMORY_WATCHER)
 * M104 - Set extruder target temp.
 * M105 - Report current temperatures.
 * M106 - Set print fan speed.
 * M107 - Print fan off.
 * M108 - Break out of heating loops (M109, M190, M303). With no controller, breaks out of M0/M1. (Requires EMERGENCY_PARSER)
 * M109 - Sxxx Wait for extruder current temp to reach target temp. Waits only when heating
 *        Rxxx Wait for extruder current temp to reach target temp. Waits when heating and cooling
 *        If AUTOTEMP is enabled, S<mintemp> B<maxtemp> F<factor>. Exit autotemp by any M109 without F
 * M110 - Set the current line number. (Used by host printing)
 * M111 - Set debug flags: "M111 S<flagbits>". See flag bits defined in enum.h.
 * M112 - Emergency stop.
 * M113 - Get or set the timeout interval for Host Keepalive "busy" messages. (Requires HOST_KEEPALIVE_FEATURE)
 * M114 - Report current position.
 *      - S1 Compute length traveled since last G96 using encoder position data (Requires MECHADUINO_I2C_COMMANDS, only kinematic axes)
 * M115 - Report capabilities. (Extended capabilities requires EXTENDED_CAPABILITIES_REPORT)
 * M117 - Display a message on the controller screen. (Requires an LCD)
 * M118 - Display a message in the host console.
 * M119 - Report endstops status.
 * M120 - Enable endstops detection.
 * M121 - Disable endstops detection.
 * M122 - Debug stepper (Requires at least one _DRIVER_TYPE defined as TMC2130/TMC2208/TMC2660)
 * M125 - Save current position and move to filament change position. (Requires PARK_HEAD_ON_PAUSE)
 * M126 - Solenoid Air Valve Open. (Requires BARICUDA)
 * M127 - Solenoid Air Valve Closed. (Requires BARICUDA)
 * M128 - EtoP Open. (Requires BARICUDA)
 * M129 - EtoP Closed. (Requires BARICUDA)
 * M140 - Set bed target temp. S<temp>
 * M145 - Set heatup values for materials on the LCD. H<hotend> B<bed> F<fan speed> for S<material> (0=PLA, 1=ABS)
 * M149 - Set temperature units. (Requires TEMPERATURE_UNITS_SUPPORT)
 * M150 - Set Status LED Color as R<red> U<green> B<blue> P<bright>. Values 0-255. (Requires BLINKM, RGB_LED, RGBW_LED, NEOPIXEL_LED, or PCA9632).
 * M155 - Auto-report temperatures with interval of S<seconds>. (Requires AUTO_REPORT_TEMPERATURES)
 * M163 - Set a single proportion for a mixing extruder. (Requires MIXING_EXTRUDER)
 * M164 - Commit the mix (Req. MIXING_EXTRUDER) and optionally save as a virtual tool (Req. MIXING_VIRTUAL_TOOLS > 1)
 * M165 - Set the mix for a mixing extruder wuth parameters ABCDHI. (Requires MIXING_EXTRUDER and DIRECT_MIXING_IN_G1)
 * M190 - Sxxx Wait for bed current temp to reach target temp. ** Waits only when heating! **
 *        Rxxx Wait for bed current temp to reach target temp. ** Waits for heating or cooling. **
 * M200 - Set filament diameter, D<diameter>, setting E axis units to cubic. (Use S0 to revert to linear units.)
 * M201 - Set max acceleration in units/s^2 for print moves: "M201 X<accel> Y<accel> Z<accel> E<accel>"
 * M202 - Set max acceleration in units/s^2 for travel moves: "M202 X<accel> Y<accel> Z<accel> E<accel>" ** UNUSED IN MARLIN! **
 * M203 - Set maximum feedrate: "M203 X<fr> Y<fr> Z<fr> E<fr>" in units/sec.
 * M204 - Set default acceleration in units/sec^2: P<printing> R<extruder_only> T<travel>
 * M205 - Set advanced settings. Current units apply:
            S<print> T<travel> minimum speeds
            Q<minimum segment time>
            X<max X jerk>, Y<max Y jerk>, Z<max Z jerk>, E<max E jerk>
 * M206 - Set additional homing offset. (Disabled by NO_WORKSPACE_OFFSETS or DELTA)
 * M207 - Set Retract Length: S<length>, Feedrate: F<units/min>, and Z lift: Z<distance>. (Requires FWRETRACT)
 * M208 - Set Recover (unretract) Additional (!) Length: S<length> and Feedrate: F<units/min>. (Requires FWRETRACT)
 * M209 - Turn Automatic Retract Detection on/off: S<0|1> (For slicers that don't support G10/11). (Requires FWRETRACT)
          Every normal extrude-only move will be classified as retract depending on the direction.
 * M211 - Enable, Disable, and/or Report software endstops: S<0|1> (Requires MIN_SOFTWARE_ENDSTOPS or MAX_SOFTWARE_ENDSTOPS)
 * M218 - Set/get a tool offset: "M218 T<index> X<offset> Y<offset>". (Requires 2 or more extruders)
 * M220 - Set Feedrate Percen__FILE__e: "M220 S<percent>" (i.e., "FR" on the LCD)
 * M221 - Set Flow Percentage: "M221 S<percent>"
 * M226 - Wait until a pin is in a given state: "M226 P<pin> S<state>"
 * M240 - Trigger a camera to take a photograph. (Requires CHDK or PHOTOGRAPH_PIN)
 * M250 - Set LCD contrast: "M250 C<contrast>" (0-63). (Requires LCD support)
 * M260 - i2c Send Data (Requires EXPERIMENTAL_I2CBUS)
 * M261 - i2c Request Data (Requires EXPERIMENTAL_I2CBUS)
 * M280 - Set servo position absolute: "M280 P<index> S<angle|µs>". (Requires servos)
 * M290 - Babystepping (Requires BABYSTEPPING)
 * M300 - Play beep sound S<frequency Hz> P<duration ms>
 * M301 - Set PID parameters P I and D. (Requires PIDTEMP)
 * M302 - Allow cold extrudes, or set the minimum extrude S<temperature>. (Requires PREVENT_COLD_EXTRUSION)
 * M303 - PID relay autotune S<temperature> sets the target temperature. Default 150C. (Requires PIDTEMP)
 * M304 - Set bed PID parameters P I and D. (Requires PIDTEMPBED)
 * M350 - Set microstepping mode. (Requires digital microstepping pins.)
 * M351 - Toggle MS1 MS2 pins directly. (Requires digital microstepping pins.)
 * M355 - Set Case Light on/off and set brightness. (Requires CASE_LIGHT_PIN)
 * M380 - Activate solenoid on active extruder. (Requires EXT_SOLENOID)
 * M381 - Disable all solenoids. (Requires EXT_SOLENOID)
 * M400 - Finish all moves.
 * M401 - Deploy and activate Z probe. (Requires a probe)
 * M402 - Deactivate and stow Z probe. (Requires a probe)
 * M404 - Display or set the Nominal Filament Width: "W<diameter>". (Requires FILAMENT_WIDTH_SENSOR)
 * M405 - Enable Filament Sensor flow control. "M405 D<delay_cm>". (Requires FILAMENT_WIDTH_SENSOR)
 * M406 - Disable Filament Sensor flow control. (Requires FILAMENT_WIDTH_SENSOR)
 * M407 - Display measured filament diameter in millimeters. (Requires FILAMENT_WIDTH_SENSOR)
 * M410 - Quickstop. Abort all planned moves.
 * M420 - Enable/Disable Leveling (with current values) S1=enable S0=disable (Requires MESH_BED_LEVELING or ABL)
 * M421 - Set a single Z coordinate in the Mesh Leveling grid. X<units> Y<units> Z<units> (Requires MESH_BED_LEVELING, AUTO_BED_LEVELING_BILINEAR, or AUTO_BED_LEVELING_UBL)
 * M428 - Set the home_offset based on the current_position. Nearest edge applies. (Disabled by NO_WORKSPACE_OFFSETS or DELTA)
 * M500 - Store parameters in EEPROM. (Requires EEPROM_SETTINGS)
 * M501 - Restore parameters from EEPROM. (Requires EEPROM_SETTINGS)
 * M502 - Revert to the default "factory settings". ** Does not write them to EEPROM! **
 * M503 - Print the current settings (in memory): "M503 S<verbose>". S0 specifies compact output.
 * M540 - Enable/disable SD card abort on endstop hit: "M540 S<state>". (Requires ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED)
 * M600 - Pause for filament change: "M600 X<pos> Y<pos> Z<raise> E<first_retract> L<later_retract>". (Requires ADVANCED_PAUSE_FEATURE)
 * M603 - Configure filament change: "M603 T<tool> U<unload_length> L<load_length>". (Requires ADVANCED_PAUSE_FEATURE)
 * M605 - Set Dual X-Carriage movement mode: "M605 S<mode> [X<x_offset>] [R<temp_offset>]". (Requires DUAL_X_CARRIAGE)
 * M665 - Set Delta configurations: "M665 H<delta height> L<diagonal rod> R<delta radius> S<segments/s> B<calibration radius> X<Alpha angle trim> Y<Beta angle trim> Z<Gamma angle trim> (Requires DELTA)
 * M665 - Set Hangprinter configurations: "M665 W<Ay> E<Az> R<Bx> T<By> Y<Bz> U<Cx> I<Cy> O<Cz> P<Dz> S<segments/s>" (Requires HANGPRINTER)
 * M666 - Set/get endstop offsets for delta (Requires DELTA) or dual endstops (Requires [XYZ]_DUAL_ENDSTOPS).
 * M701 - Load filament (requires FILAMENT_LOAD_UNLOAD_GCODES)
 * M702 - Unload filament (requires FILAMENT_LOAD_UNLOAD_GCODES)
 * M851 - Set Z probe's Z offset in current units. (Negative = below the nozzle.)
 * M852 - Set skew factors: "M852 [I<xy>] [J<xz>] [K<yz>]". (Requires SKEW_CORRECTION_GCODE, and SKEW_CORRECTION_FOR_Z for IJ)
 * M860 - Report the position of position encoder modules.
 * M861 - Report the status of position encoder modules.
 * M862 - Perform an axis continuity test for position encoder modules.
 * M863 - Perform steps-per-mm calibration for position encoder modules.
 * M864 - Change position encoder module I2C address.
 * M865 - Check position encoder module firmware version.
 * M866 - Report or reset position encoder module error count.
 * M867 - Enable/disable or toggle error correction for position encoder modules.
 * M868 - Report or set position encoder module error correction threshold.
 * M869 - Report position encoder module error.
 * M900 - Get or Set Linear Advance K-factor. (Requires LIN_ADVANCE)
 * M906 - Set or get motor current in milliamps using axis codes X, Y, Z, E. Report values if no axis codes given. (Requires at least one _DRIVER_TYPE defined as TMC2130/TMC2208/TMC2660)
 * M907 - Set digital trimpot motor current using axis codes. (Requires a board with digital trimpots)
 * M908 - Control digital trimpot directly. (Requires DAC_STEPPER_CURRENT or DIGIPOTSS_PIN)
 * M909 - Print digipot/DAC current value. (Requires DAC_STEPPER_CURRENT)
 * M910 - Commit digipot/DAC value to external EEPROM via I2C. (Requires DAC_STEPPER_CURRENT)
 * M911 - Report stepper driver overtemperature pre-warn condition. (Requires at least one _DRIVER_TYPE defined as TMC2130/TMC2208/TMC2660)
 * M912 - Clear stepper driver overtemperature pre-warn condition flag. (Requires at least one _DRIVER_TYPE defined as TMC2130/TMC2208/TMC2660)
 * M913 - Set HYBRID_THRESHOLD speed. (Requires HYBRID_THRESHOLD)
 * M914 - Set SENSORLESS_HOMING sensitivity. (Requires SENSORLESS_HOMING)
 *
 * M360 - SCARA calibration: Move to cal-position ThetaA (0 deg calibration)
 * M361 - SCARA calibration: Move to cal-position ThetaB (90 deg calibration - steps per degree)
 * M362 - SCARA calibration: Move to cal-position PsiA (0 deg calibration)
 * M363 - SCARA calibration: Move to cal-position PsiB (90 deg calibration - steps per degree)
 * M364 - SCARA calibration: Move to cal-position PSIC (90 deg to Theta calibration position)
 *
 * ************ Custom codes - This can change to suit future G-code regulations
 * M928 - Start SD logging: "M928 filename.gco". Stop with M29. (Requires SDSUPPORT)
 * M999 - Restart after being stopped by error
*
 * "T" Codes
 *
 * T0-T3 - Select an extruder (tool) by index: "T<n> F<units/min>"
 *
 */

#include "serial.h"


