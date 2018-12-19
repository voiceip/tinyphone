/**
 * Include file for a wide string class with case-insensitive compares.
 *
 * This code adapted from a StackOverflow topic
 * "Case insensitive string comparison in C++"
 *
 * See http://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
 */
//
#pragma once
#include <string>

struct ci_wchar_t_traits : public std::char_traits<wchar_t> {
    static bool eq(wchar_t c1, wchar_t c2) { return toupper(c1) == toupper(c2); }
    static bool ne(wchar_t c1, wchar_t c2) { return toupper(c1) != toupper(c2); }
    static bool lt(wchar_t c1, wchar_t c2) { return toupper(c1) <  toupper(c2); }
    static int compare(const wchar_t* s1, const wchar_t* s2, size_t n) {
        while( n-- != 0 ) {
            if( toupper(*s1) < toupper(*s2) ) return -1;
            if( toupper(*s1) > toupper(*s2) ) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const wchar_t* find(const wchar_t* s, int n, wchar_t a) {
        while( n-- > 0 && toupper(*s) != toupper(a) ) {
            ++s;
        }
        return (n >= 0 ? s : NULL);
    }
};

//* Wide string class with case-insensitive compares.
typedef std::basic_string<wchar_t, ci_wchar_t_traits> ci_wstring;
