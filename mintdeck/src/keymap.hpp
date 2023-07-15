/// @file keymap.hpp
#ifndef KEYMAP_HPP
#define KEYMAP_HPP

#include <map>
#include <string>
#include <vector>

namespace decklibrary {

    enum keymap {
        ESCAPE = 65307, F1 = 65470, F2 = 65471, F3 = 65472, F4 = 65473, F5 = 65474, F6 = 65475, F7 = 65476, F8 = 65477, F9 = 65478, F10 = 65479, F11 = 65480, F12 = 65481, PRINT_SCREEN = 65377, SCROLL_LOCK = 65300, PAUSE = 65299, BACKTICK = 96, TILDE = 126, ONE = 49, EXCLAMATION = 33, TWO = 50, AT_SIGN = 64, THREE = 51, HASH = 35, FOUR = 52, DOLLAR_SIGN = 36, FIVE = 53, PERCENT = 37, SIX = 54, CARET = 94, SEVEN = 55, AMPERSAND = 38, EIGHT = 56, ASTERISK = 42, NINE = 57, LEFT_PARENTHESIS = 40, ZERO = 48, RIGHT_PARENTHESIS = 41, DASH = 45, UNDERSCORE = 95, EQUALS = 61, PLUS = 43, BACKSPACE = 65288, INSERT = 65379, HOME = 65360, PAGEUP = 65365, TAB = 65289, q = 113, Q = 81, w = 119, W = 87, e = 101, E = 69, r = 114, R = 82, t = 116, T = 84, y = 121, Y = 89, u = 117, U = 85, i = 105, I = 73, o = 111, O = 79, p = 112, P = 80, LEFT_BRACKET = 91, LEFT_BRACE = 123, RIGHT_BRACKET = 93, RIGHT_BRACE = 125, BACKSLASH = 92, PIPE = 124, DELETE = 65535, END = 65367, PAGE_DOWN = 65366, CAPS_LOCK = 65509, a = 97, A = 65, s = 115, S = 83, d = 100, D = 68, f = 102, F = 70, g = 103, G = 71, h = 104, H = 72, j = 106, J = 74, k = 107, K = 75, l = 108, L = 76, SEMICOLON = 59, COLON = 58, APOSTROPHE = 39, QUOTE = 34, ENTER = 65293, SHIFT_L = 65505, z = 122, Z = 90, x = 120, X = 88, c = 99, C = 67, v = 118, V = 86, b = 98, B = 66, n = 110, N = 78, m = 109, M = 77, COMMA = 44, LESS_THAN = 60, PERIOD = 46, GREATER_THAN = 62, SLASH = 47, QUESTION_MARK = 63, SHIFT_R = 65506, CTRL_L = 65507, META_L = 65511, ALT_L = 65513, SPACE = 32, ALT_R = 65514, META_R = 65512, MENU = 65383, CTRL_R = 65508, LEFT_ARROW = 65361, UP_ARROW = 65362, DOWN_ARROW = 65364, RIGHT_ARROW = 65363, NUM_LOCK = 65407, KP_DIVIDE = 65455, KP_MULTIPLY = 65450, KP_SUBTRACT = 65453, KP_7 =  65429, KP_UP =  65429, KP_8 =  65431, KP_HOME =  65431, KP_9 =  65434, KP_PRIOR =  65434, KP_ADD =  65451, KP_4 =  65430, KP_LEFT =  65430, KP_5 =  65437, KP_BEGIN =  65437, KP_6 =  65432, KP_RIGHT =  65432, KP_1 =  65436, KP_END =  65436, KP_2 =  65433, KP_DOWN =  65433, KP_3 =  65435, KP_NEXT =  65435, KP_0 =  65438, KP_INSERT =  65438, KP_DECIMAL = 65439, KP_DELETE = 65439, KP_ENTER = 65421
    };
    enum keystates { KSHIFT = 0, KCTRL = 1, KALT = 2, KMETA = 3 };
    enum updown { PRESS, RELEASE };
    enum modestrings { RECORD, EDIT, LOADIMAGE, PUSH, POP, DEMO };
    enum keytech { EVEMU, X11 };

    extern std::map<keymap, std::string> keystrings;
    extern std::map<std::string, keymap> keycodes;
    extern std::map<keymap, std::string> evemucode;
    extern std::map<keymap, std::string> xdotoolcode;
    extern std::map<keymap, keymap> lowkeys;
    extern std::initializer_list<keymap> simplestrings;
    extern std::initializer_list<keymap> nocasekeys;
    extern std::initializer_list<keymap> shiftkeys;
    extern std::initializer_list<keymap> modkeys;
    extern std::vector<keymap> testkeys;
    extern std::map<std::string, modestrings> modestringmap;

}

#endif // KEYMAP_HPP
