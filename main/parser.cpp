
#include "parser.h"

//**********************************************************************
// Must be declared for allocation and to satisfy the linker           *
char *GcodeParser::command_ptr,  //                                    *
     *GcodeParser::string_arg,   //                                    *
     *GcodeParser::value_ptr;    //                                    *
char GcodeParser::command_letter;//                                    *
int GcodeParser::codenum;        //                                    *
uint32_t GcodeParser::codebits;  // found bits                         *
uint8_t GcodeParser::param[26];  // parameter offsets from command_ptr *
//                                                                     *
//**********************************************************************

// Create a global instance of the GCode parser singleton
GcodeParser parser;

/**
 * Clear all code-seen (and value pointers)
 *
 * Since each param is set/cleared on seen codes,
 * this may be optimized by commenting out ZERO(param)
 */
void GcodeParser::reset() {
    string_arg = nullptr;                 // No whole line argument
    command_letter = '?';                 // No command letter
    codenum = 0;                          // No command code
    codebits = 0;                       // No codes yet
    //ZERO(param);                      // No parameters (should be safe to comment out this line)
}

// Populate all fields by parsing a single line of GCode
// 58 bytes of SRAM are used to speed up seen/value
void GcodeParser::parse(char *p)
{

    reset(); // No codes to report

    auto uppercase = [](char c)
    {
        if (WITHIN(c, 'a', 'z')) c += 'A' - 'a';
        return c;
    };

    // Skip spaces
    while (*p == ' ') ++p;

    // Skip N[-0-9] if included in the command line
    if (uppercase(*p) == 'N' && NUMERIC_SIGNED(p[1]))
    {
        set('N', p + 1);     // (optional) Set the 'N' parameter value

        p += 2;                  // skip N[-0-9]
        while (NUMERIC(*p)) ++p; // skip [0-9]*
        while (*p == ' ')   ++p; // skip [ ]*
    }

    // *p now points to the current command, which should be G, M, or T
    command_ptr = p;

    // Get the command letter, which must be G, M, or T
    const char letter = uppercase(*p++);

    // Nullify asterisk and trailing whitespace
    char *starpos = strchr(p, '*');
    if (starpos)
    {
        --starpos;                          // *
        while (*starpos == ' ') --starpos;  // spaces...
        starpos[1] = '\0';
    }

    // Bail if the letter is not G, M, or T
    // (or a valid parameter for the current motion mode)
    switch (letter)
    {

        case 'G': case 'M': case 'T':
            // Skip spaces to get the numeric part
            while (*p == ' ') p++;

            // Bail if there's no command code number
            if (!NUMERIC(*p)) return;

            // Save the command letter at this point
            // A '?' signifies an unknown command
            command_letter = letter;

            // Get the code number - integer digits only
            codenum = 0;
            do { codenum *= 10, codenum += *p++ - '0'; } while (NUMERIC(*p));

            // Allow for decimal point in command
            #if defined(USE_GCODE_SUBCODES)
                if (*p == '.') {
                    p++;
                    while (NUMERIC(*p)) p++;
                }
            #endif

            // Skip all spaces to get to the first argument, or nul
            while (*p == ' ') p++;

            break;

        default: return;
    }

    // The command parameters (if any) start here, for sure!

    // Only use string_arg for these M codes
    if (letter == 'M') switch (codenum)
    {
        case 810 ... 819:
        case 16:
        case 23: case 28: case 30: case 117: case 118: case 928:
        string_arg = unescape_string(p);
        return;
        default: break;
    }

    /**
     * Find all parameters, set flags and pointers for fast parsing
     *
     * Most codes ignore 'string_arg', but those that want a string will get the right pointer.
     * The following loop assigns the first "parameter" having no numeric value to 'string_arg'.
     * This allows M0/M1 with expire time to work: "M0 S5 You Win!"
     * For 'M118' you must use 'E1' and 'A1' rather than just 'E' or 'A'
     */
    string_arg = nullptr;
    while (const char param = uppercase(*p++)) // Get the next parameter. A NUL ends the loop
    {
        // Special handling for M32 [P] !/path/to/file.g#
        // The path must be the last parameter
        if (param == '!' && letter == 'M' && codenum == 32)
        {
            string_arg = p;                           // Name starts after '!'
            char * const lb = strchr(p, '#');         // Already seen '#' as SD char (to pause buffering)
            if (lb) *lb = '\0';                       // Safe to mark the end of the filename
            return;
        }

        if (WITHIN(param, 'A', 'Z'))
        {
            while (*p == ' ') p++;                    // Skip spaces between parameters & values

            const bool has_val = valid_float(p);
            char * const valptr = has_val ? p : nullptr;

            if (!has_val && !string_arg) // No value? First time, keep as string_arg
            {
                string_arg = p - 1;
            }
            
            set(param, valptr);                     // Set parameter exists and pointer (nullptr for no value)
        }
        else if (!string_arg) // Not A-Z? First time, keep as the string_arg
        {                     
            string_arg = p - 1;
        }

        if (!WITHIN(*p, 'A', 'Z')) // Another parameter right away?
        {
            while (*p && DECIMAL_SIGNED(*p)) p++;     // Skip over the value section of a parameter
            while (*p == ' ') p++;                    // Skip over all spaces
        }
    }
}
