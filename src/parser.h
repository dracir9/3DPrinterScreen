
#ifndef PARSER_H
#define PARSER_H

#include "Arduino.h"

#define MAX_PARAM 26 // Letters from 'A' to 'Z'

#define WITHIN(N,L,H)       ((N) >= (L) && (N) <= (H))
#define NUMERIC(a)          WITHIN(a, '0', '9')
#define DECIMAL(a)          (NUMERIC(a) || a == '.')
#define NUMERIC_SIGNED(a)   (NUMERIC(a) || (a) == '-' || (a) == '+')
#define DECIMAL_SIGNED(a)   (DECIMAL(a) || (a) == '-' || (a) == '+')
#define COUNT(a)            (sizeof(a)/sizeof(*a))
#define ZERO(a)             memset(a,0,sizeof(a))

class GcodeParser
{
private:
    static char *value_ptr;
    static uint32_t codebits;       // Parameters pre-scanned
    static uint8_t param[MAX_PARAM];

public:    
    static char *command_ptr,               // The command, so it can be echoed
            *string_arg,                // string of command line
            command_letter;             // G, M, or T
    static int codenum;

    static void reset();

    #define LETTER_BIT(N) ((N) - 'A')

    inline static bool valid_signless(const char * const p)
    {
        return NUMERIC(p[0]) || (p[0] == '.' && NUMERIC(p[1])); // .?[0-9]
    }

    inline static bool valid_float(const char * const p)
    {
        return valid_signless(p) || ((p[0] == '-' || p[0] == '+') && valid_signless(&p[1])); // [-+]?.?[0-9]
    }

    inline static bool valid_int(const char * const p)
    {
        return NUMERIC(p[0]) || ((p[0] == '-' || p[0] == '+') && NUMERIC(p[1])); // [-+]?[0-9]
    }

    // Set the flag and pointer for a parameter
    static inline void set(const char c, char * const ptr)
    {
        const uint8_t ind = LETTER_BIT(c);
        if (ind >= MAX_PARAM) return;                      // Only A-Z
        codebits |= 1UL << ind;                     // parameter exists
        param[ind] = ptr ? ptr - command_ptr : 0;   // parameter offset or 0
    }

    // Code seen bit was set. If not found, value_ptr is unchanged.
    // This allows "if (seen('A')||seen('B'))" to use the last-found value.
    static inline bool seen(const char c)
    {
        const uint8_t ind = LETTER_BIT(c);
        if (ind >= MAX_PARAM) return false;                // Only A-Z
        const bool b = !!(codebits & (1UL << ind));
        if (b) {
            char * const ptr = command_ptr + param[ind];
            value_ptr = param[ind] && valid_float(ptr) ? ptr : nullptr;
        }
        return b;
    }

    // Line has any parameter value?
    static inline bool seen_any() { return !!codebits; }

    // Test if parameter exist without changing value_ptr
    inline static bool seen_test(const char c) { return !!(codebits & (1UL << LETTER_BIT(c))); }

    // Seen any axis parameter
    static inline bool seen_axis()
    {
        return seen_test('X') || seen_test('Y') || seen_test('Z') || seen_test('E');
    }

    inline static char* unescape_string(char* &src) { return src; }

    // Populate all fields by parsing a single line of GCode
    static void parse(char * p);

    // The code value pointer was set
    inline static bool has_value() { return value_ptr != nullptr; }

    // Seen a parameter with a value
    static inline bool seenval(const char c) { return seen(c) && has_value(); }

    // The value as a string
    static inline char* value_string() { return value_ptr; }

    // Float removes 'E' to prevent scientific notation interpretation
    static inline float value_float()
    {
        if (value_ptr) 
        {
            char *e = value_ptr;
            for (;;)
            {
                const char c = *e;
                if (c == '\0' || c == ' ') break;
                if (c == 'E' || c == 'e') 
                {
                    *e = '\0';
                    const float ret = strtof(value_ptr, nullptr);
                    *e = c;
                    return ret;
                }
                ++e;
            }
            return strtof(value_ptr, nullptr);
        }
        return 0;
    }

    // Code value as a long or ulong
    static inline int32_t value_long() { return value_ptr ? strtol(value_ptr, nullptr, 10) : 0L; }
    static inline uint32_t value_ulong() { return value_ptr ? strtoul(value_ptr, nullptr, 10) : 0UL; }

    // Reduce to fewer bits
    static inline int16_t value_int() { return (int16_t)value_long(); }
    static inline uint16_t value_ushort() { return (uint16_t)value_long(); }
    static inline uint8_t value_byte() { return (uint8_t)constrain(value_long(), 0, 255); }

    // Bool is true with no value or non-zero
    static inline bool value_bool() { return !has_value() || !!value_byte(); }
};

#endif