/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2017 Andreas Jonsson
  
   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.
  
   Andreas Jonsson
   andreas@angelcode.com
*/

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <map>
#include <sstream>
using std::map;

#include "dynamic_funcs.h"
#include "ac_string_util.h"
#include "acutil_unicode.h"
#include "fontgen.h"

#include "unicode.h"

// These are the defined character sets from the Unicode 6.2 standard
// http://www.unicode.org/charts/PDF/
const UnicodeSubset_t UnicodeSubsets[] = {
// Plane 0 - Base Multilingual Plane
{"Latin + Latin Supplement"                      , 0x0000  , 0x00FF},
{"Latin Extended A"                              , 0x0100  , 0x017F},
{"Latin Extended B"                              , 0x0180  , 0x024F},
{"IPA Extensions"                                , 0x0250  , 0x02AF},
{"Spacing Modifier Letters"                      , 0x02B0  , 0x02FF},
{"Combining Diacritical Marks"                   , 0x0300  , 0x036F},
{"Greek and Coptic"                              , 0x0370  , 0x03FF},
{"Cyrillic"                                      , 0x0400  , 0x04FF},
{"Cyrillic Supplement"                           , 0x0500  , 0x052F},
{"Armenian"                                      , 0x0530  , 0x058F},
{"Hebrew"                                        , 0x0590  , 0x05FF},
{"Arabic"                                        , 0x0600  , 0x06FF},
{"Syriac"                                        , 0x0700  , 0x074F},
{"Arabic Supplement"                             , 0x0750  , 0x077F},
{"Thaana"                                        , 0x0780  , 0x07BF},
{"N'Ko"                                          , 0x07C0  , 0x07FF}, // 5.0
{"Samaritan"                                     , 0x0800  , 0x083F}, // 5.2
{"Mandaic"                                       , 0x0840  , 0x085F}, // 6.0
{"(0x0860 - 0x089F)"                             , 0x0860  , 0x089F},
{"Arabic Extended A"                             , 0x08A0  , 0x08FF}, // 6.1
{"Devanagari"                                    , 0x0900  , 0x097F},
{"Bengali"                                       , 0x0980  , 0x09FF},
{"Gurmukhi"                                      , 0x0A00  , 0x0A7F},
{"Gujarati"                                      , 0x0A80  , 0x0AFF},
{"Oriya"                                         , 0x0B00  , 0x0B7F},
{"Tamil"                                         , 0x0B80  , 0x0BFF},
{"Telugu"                                        , 0x0C00  , 0x0C7F},
{"Kannada"                                       , 0x0C80  , 0x0CFF},
{"Malayalam"                                     , 0x0D00  , 0x0D7F},
{"Sinhala"                                       , 0x0D80  , 0x0DFF},
{"Thai"                                          , 0x0E00  , 0x0E7F},
{"Lao"                                           , 0x0E80  , 0x0EFF},
{"Tibetan"                                       , 0x0F00  , 0x0FFF},
{"Myanmar"                                       , 0x1000  , 0x109F},
{"Georgian"                                      , 0x10A0  , 0x10FF},
{"Hangul Jamo"                                   , 0x1100  , 0x11FF},
{"Ethiopic"                                      , 0x1200  , 0x12BF},
{"(0x12C0 - 0x137F)"                             , 0x12C0  , 0x137F},
{"Ethiopic Supplement"                           , 0x1380  , 0x139F},
{"Cherokee"                                      , 0x13A0  , 0x13FF},
{"Canadian Aboriginal Syllabics"                 , 0x1400  , 0x167F},
{"Ogham"                                         , 0x1680  , 0x169F},
{"Runic"                                         , 0x16A0  , 0x16FF},
{"Tagalog"                                       , 0x1700  , 0x171F},
{"Hanunoo"                                       , 0x1720  , 0x173F},
{"Buhid"                                         , 0x1740  , 0x175F},
{"Tagbanwa"                                      , 0x1760  , 0x177F},
{"Khmer"                                         , 0x1780  , 0x17FF},
{"Mongolian"                                     , 0x1800  , 0x18AF},
{"Unified Canadian Aboriginal Syllabics Extended", 0x18B0  , 0x18FF}, // 5.2
{"Limbu"                                         , 0x1900  , 0x194F},
{"Tai Le"                                        , 0x1950  , 0x197F},
{"New Tai Lue"                                   , 0x1980  , 0x19DF},
{"Khmer Symbols"                                 , 0x19E0  , 0x19FF},
{"Buginese"                                      , 0x1A00  , 0x1A1F},
{"Tai Tham"                                      , 0x1A20  , 0x1AAF}, // 5.2
{"Combining Diacritical Marks Extended"          , 0x1AB0  , 0x1AFF}, // 7.0
{"Balinese"                                      , 0x1B00  , 0x1B7F}, // 5.0
{"Sundanese"                                     , 0x1B80  , 0x1BBF}, // 5.1
{"Batak"                                         , 0x1BC0  , 0x1BFF}, // 6.0
{"Lepcha"                                        , 0x1C00  , 0x1C4F}, // 5.1
{"Ol Chiki"                                      , 0x1C50  , 0x1C7F}, // 5.1
{"(0x1C80 - 0x1CBF)"                             , 0x1C80  , 0x1CBF},
{"Sundanese Supplement"                          , 0x1CC0  , 0x1CCF}, // 6.1
{"Vedic Extensions"                              , 0x1CD0  , 0x1CFF}, // 5.2
{"Phonetic Extensions"                           , 0x1D00  , 0x1D7F},
{"Phonetic Extensions Supplement"                , 0x1D80  , 0x1DBF},
{"Combining Diacritical Marks Supplement"        , 0x1DC0  , 0x1DFF},
{"Latin Extended Additional"                     , 0x1E00  , 0x1EFF},
{"Greek Extended"                                , 0x1F00  , 0x1FFF},
{"General Punctuation"                           , 0x2000  , 0x206F},
{"Subscripts and Superscripts"                   , 0x2070  , 0x209F},
{"Currency Symbols"                              , 0x20A0  , 0x20CF},
{"Combining Diacritical Marks for Symbols"       , 0x20D0  , 0x20FF},
{"Letterlike Symbols"                            , 0x2100  , 0x214F},
{"Number Forms"                                  , 0x2150  , 0x218F},
{"Arrows"                                        , 0x2190  , 0x21FF},
{"Mathematical Operators"                        , 0x2200  , 0x22FF},
{"Miscellaneous Technical"                       , 0x2300  , 0x23FF},
{"Control Pictures"                              , 0x2400  , 0x243F},
{"Optical Character Recognition"                 , 0x2440  , 0x245F},
{"Enclosed Alphanumerics"                        , 0x2460  , 0x24FF},
{"Box Drawing"                                   , 0x2500  , 0x257F},
{"Block Elements"                                , 0x2580  , 0x259F},
{"Geometric Shapes"                              , 0x25A0  , 0x25FF},
{"Miscellaneous Symbols"                         , 0x2600  , 0x26FF},
{"Dingbats"                                      , 0x2700  , 0x27BF},
{"Miscellaneous Mathematical Symbols A"          , 0x27C0  , 0x27EF},
{"Supplemental Arrows A"                         , 0x27F0  , 0x27FF},
{"Braille"                                       , 0x2800  , 0x28FF},
{"Supplemental Arrows B"                         , 0x2900  , 0x297F},
{"Miscellaneous Mathematical Symbols B"          , 0x2980  , 0x29FF},
{"Supplemental Mathematical Operators"           , 0x2A00  , 0x2AFF},
{"Miscellaneous Symbols and Arrows"              , 0x2B00  , 0x2BFF},
{"Glagolitic"                                    , 0x2C00  , 0x2C5F},
{"Latin Extended C"                              , 0x2C60  , 0x2C7F}, // 5.0
{"Coptic"                                        , 0x2C80  , 0x2CFF},
{"Georgian Supplement"                           , 0x2D00  , 0x2D2F},
{"Tifinagh"                                      , 0x2D30  , 0x2D7F},
{"Ethiopic Extended"                             , 0x2D80  , 0x2DDF},
{"Cyrillic Extended A"                           , 0x2DE0  , 0x2DFF}, // 5.1
{"Supplemental Punctuation"                      , 0x2E00  , 0x2E7F},
{"CJK Radicals Supplement"                       , 0x2E80  , 0x2EFF},
{"KangXi Radicals"                               , 0x2F00  , 0x2FDF},
{"(0x2FE0 - 0x2FEF)"                             , 0x2FE0  , 0x2FEF},
{"Ideographic Description"                       , 0x2FF0  , 0x2FFF},
{"CJK Symbols and Punctuation"                   , 0x3000  , 0x303F},
{"Hiragana"                                      , 0x3040  , 0x309F},
{"Katakana"                                      , 0x30A0  , 0x30FF},
{"Bopomofo"                                      , 0x3100  , 0x312F},
{"Hangul Compatibility Jamo"                     , 0x3130  , 0x318F},
{"Kanbun"                                        , 0x3190  , 0x319F},
{"Extended Bopomofo"                             , 0x31A0  , 0x31BF},
{"CJK Strokes"                                   , 0x31C0  , 0x31EF},
{"Katakana Phonetic Extensions"                  , 0x31F0  , 0x31FF},
{"Enclosed CJK Letters and Months"               , 0x3200  , 0x32FF},
{"CJK Compatibility"                             , 0x3300  , 0x33FF},
{"CJK Unified Ideographs Extension A"            , 0x3400  , 0x4DBF},
{"Yijing Hexagram Symbols"                       , 0x4DC0  , 0x4DFF},
{"CJK Unified Ideographs"                        , 0x4E00  , 0x9FFF},
{"Yi"                                            , 0xA000  , 0xA48F},
{"Yi Radicals"                                   , 0xA490  , 0xA4CF},
{"Lisu"                                          , 0xA4D0  , 0xA4FF}, // 5.2
{"Vai"                                           , 0xA500  , 0xA59F}, // 5.1
{"(0xA600 - 0xA63F)"                             , 0xA600  , 0xA63F},
{"Cyrillic Extended B"                           , 0xA640  , 0xA69F}, // 5.1
{"Bamum"                                         , 0xA6A0  , 0xA6FF}, // 5.2
{"Modifier Tone Letters"                         , 0xA700  , 0xA71F},
{"Latin Extended D"                              , 0xA720  , 0xA7FF}, // 5.0
{"Syloti Nagri"                                  , 0xA800  , 0xA82F},
{"Common Indic Number Forms"                     , 0xA830  , 0xA83F}, // 5.2
{"Phags-Pa"                                      , 0xA840  , 0xA87F}, // 5.0
{"Saurashtra"                                    , 0xA880  , 0xA8DF}, // 5.1
{"Devanagari Extended"                           , 0xA8E0  , 0xA8FF}, // 5.2
{"Kayah Li"                                      , 0xA900  , 0xA92F}, // 5.1
{"Rejang"                                        , 0xA930  , 0xA95F}, // 5.1
{"Hangul Jamo Extended A"                        , 0xA960  , 0xA97F}, // 5.2
{"Javanese"                                      , 0xA980  , 0xA9DF}, // 5.2
{"Myanmar Extended B"                            , 0xA9E0  , 0xA9FF}, // 7.0
{"Cham"                                          , 0xAA00  , 0xAA5F}, // 5.1
{"Myanmar Extended A"                            , 0xAA60  , 0xAA7F}, // 5.2
{"Tai Viet"                                      , 0xAA80  , 0xAADF}, // 5.2
{"Meetei Mayek Extensions"                       , 0xAAE0  , 0xAAFF}, // 6.1
{"Ethiopic Extended A"                           , 0xAB00  , 0xAB2F}, // 6.0
{"Latin Extended E"                              , 0xAB30  , 0xAB6F}, // 7.0
{"Cherokee Supplement"                           , 0xAB70  , 0xABBF}, // 8.0
{"Meetei Mayek"                                  , 0xABC0  , 0xABFF}, // 5.2
{"Hangul"                                        , 0xAC00  , 0xD7AF},
{"Hangul Jamo Extended B"                        , 0xD7B0  , 0xD7FF}, // 5.2
{"(High Surrogates)"                             , 0xD800  , 0xDBFF},
{"(Low Surrogates)"                              , 0xDC00  , 0xDFFF},
{"Private Use Area"                              , 0xE000  , 0xF8FF},
{"CJK Compatibility Ideographs"                  , 0xF900  , 0xFAFF},
{"Alphabetical Presentation Forms"               , 0xFB00  , 0xFB4F},
{"Arabic Presentation Forms A"                   , 0xFB50  , 0xFDFF},
{"Variation Selectors"                           , 0xFE00  , 0xFE0F},
{"Vertical Forms"                                , 0xFE10  , 0xFE1F},
{"Combining Half Marks"                          , 0xFE20  , 0xFE2F},
{"CJK Compatibility Forms"                       , 0xFE30  , 0xFE4F},
{"Small Form Variants"                           , 0xFE50  , 0xFE6F},
{"Arabic Presentation Forms B"                   , 0xFE70  , 0xFEFF},
{"Halfwidth and Fullwidth Forms"                 , 0xFF00  , 0xFFEF},
{"Specials"                                      , 0xFFF0  , 0xFFFD},
{"(Reserved)"                                    , 0xFFFE  , 0xFFFF},

// Plane 1
{"Linear B Syllabary"                       , 0x10000 , 0x1007F},
{"Linear B Ideograms"                       , 0x10080 , 0x100FF},
{"Aegean Numbers"                           , 0x10100 , 0x1013F},
{"Ancient Greek Numbers"                    , 0x10140 , 0x1018F},
{"Ancient Symbols"                          , 0x10190 , 0x101CF}, // 5.1
{"Phaistos Disc"                            , 0x101D0 , 0x101FF}, // 5.1
{"(0x10200 - 0x1027F)"                      , 0x10200 , 0x1027F},
{"Lycian"                                   , 0x10280 , 0x1029F}, // 5.1
{"Carian"                                   , 0x102A0 , 0x102DF}, // 5.1
{"Coptic Epact Numbers"                     , 0x102E0 , 0x102FF}, // 7.0
{"Old Italic"                               , 0x10300 , 0x1032F},
{"Gothic"                                   , 0x10330 , 0x1034F},
{"Old Permic"                               , 0x10350 , 0x1037F}, // 7.0
{"Ugaritic"                                 , 0x10380 , 0x1039F},
{"Old Persian"                              , 0x103A0 , 0x103DF},
{"(0x103E0 - 0x103FF)"                      , 0x103E0 , 0x103FF},
{"Deseret"                                  , 0x10400 , 0x1044F},
{"Shavian"                                  , 0x10450 , 0x1047F},
{"Osmanya"                                  , 0x10480 , 0x104AF},
{"(0x104B0 - 0x104FF)"                      , 0x104B0 , 0x104FF},
{"Elbasan"                                  , 0x10500 , 0x1052F}, // 7.0
{"Caucasian Albanian"                       , 0x10530 , 0x1056F}, // 7.0
{"(0x10570 - 0x105FF)"                      , 0x10570 , 0x105FF},
{"Linear A"                                 , 0x10600 , 0x1077F}, // 7.0
{"(0x10780 - 0x107FF)"                      , 0x10780 , 0x107FF},
{"Cypriot Syllabary"                        , 0x10800 , 0x1083F},
{"Imperial Aramaic"                         , 0x10840 , 0x1085F}, // 5.2
{"Palmyrene"                                , 0x10860 , 0x1087F}, // 7.0
{"Nabataean"                                , 0x10880 , 0x108AF}, // 7.0
{"(0x108B0 - 0x108DF)"                      , 0x108B0 , 0x108DF},
{"Hatran"                                   , 0x108E0 , 0x108FF}, // 8.0
{"Phoenician"                               , 0x10900 , 0x1091F}, // 5.0
{"Lydian"                                   , 0x10920 , 0x1093F}, // 5.1
{"(0x10940 - 0x1097F)"                      , 0x10940 , 0x1097F},
{"Meroitic Hieroglyphs"                     , 0x10980 , 0x1099F}, // 6.1
{"Meroitic Cursive"                         , 0x109A0 , 0x109FF}, // 6.1
{"Kharoshthi"                               , 0x10A00 , 0x10A5F},
{"Old South Arabian"                        , 0x10A60 , 0x10A7F}, // 5.2
{"Old North Arabian"                        , 0x10A80 , 0x10A9F}, // 7.0
{"(0x10AA0 - 0x10ABF)"                      , 0x10AA0 , 0x10ABF},
{"Manichaean"                               , 0x10AC0 , 0x10AFF}, // 7.0
{"Avestan"                                  , 0x10B00 , 0x10B3F}, // 5.2
{"Inscriptional Parthian"                   , 0x10B40 , 0x10B5F}, // 5.2
{"Inscriptional Pahlavi"                    , 0x10B60 , 0x10B7F}, // 5.2
{"Psalter Pahlavi"                          , 0x10B80 , 0x10BAF}, // 7.0
{"(0x10BB0 - 0x10BFF)"                      , 0x10BB0 , 0x10BFF},
{"Old Turkic"                               , 0x10C00 , 0x10C4F}, // 5.2
{"(0x10C50 - 0x10C7F)"                      , 0x10C50 , 0x10C7F},
{"Old Hungarian"                            , 0x10C80 , 0x10CFF}, // 8.0
{"(0x10D00 - 0x10E5F)"                      , 0x10D00 , 0x10E5F}, 
{"Rumi Numeral Symbols"                     , 0x10E60 , 0x10E7F}, // 5.2
{"(0x10E60 - 0x10FFF)"                      , 0x10E80 , 0x10FFF},
{"Brahmi"                                   , 0x11000 , 0x1107F}, // 6.0
{"Kaithi"                                   , 0x11080 , 0x110CF}, // 5.2
{"Sora Sompeng"                             , 0x110D0 , 0x110FF}, // 6.1
{"Chakma"                                   , 0x11100 , 0x1114F}, // 6.1
{"Mahajani"                                 , 0x11150 , 0x1117F}, // 7.0
{"Sharada"                                  , 0x11180 , 0x111DF}, // 6.1
{"Sinhala Archaic Numbers"                  , 0x111E0 , 0x111FF}, // 7.0
{"Khojki"                                   , 0x11200 , 0x1124F}, // 7.0
{"(0x11250 - 0x1127F)"                      , 0x11250 , 0x1127F},
{"Multani"                                  , 0x11280 , 0x112AF}, // 8.0
{"Khudawadi"                                , 0x112B0 , 0x112FF}, // 7.0
{"Grantha"                                  , 0x11300 , 0x1137F}, // 7.0
{"(0x11380 - 0x1147F)"                      , 0x11380 , 0x1147F},
{"Tirhuta"                                  , 0x11480 , 0x114DF}, // 7.0
{"(0x114E0 - 0x1157F)"                      , 0x114E0 , 0x1157F},
{"Siddham"                                  , 0x11580 , 0x115FF}, // 7.0
{"Modi"                                     , 0x11600 , 0x1165F}, // 7.0
{"(0x11660 - 0x1167F)"                      , 0x11660 , 0x1167F},
{"Takri"                                    , 0x11680 , 0x116CF}, // 6.1
{"(0x116D0 - 0x116FF)"                      , 0x116D0 , 0x116FF},
{"Ahom"                                     , 0x11700 , 0x1173F}, // 8.0
{"(0x11740 - 0x1189F)"                      , 0x11740 , 0x1189F},
{"Warang Citi"                              , 0x118A0 , 0x118FF}, // 7.0
{"(0x11900 - 0x11ABF)"                      , 0x11900 , 0x11ABF},
{"Pau Cin Hau"                              , 0x11AC0 , 0x11AFF}, // 7.0
{"(0x11B00 - 0x11FFF)"                      , 0x11B00 , 0x11FFF},
{"Cuneiform"                                , 0x12000 , 0x123FF}, // 5.0
{"Cuneiform Numbers and Punctuation"        , 0x12400 , 0x1247F}, // 5.0
{"Early Dynastic Cuneiform"                 , 0x12480 , 0x1254F}, // 8.0
{"(0x12550 - 0x12FFF)"                      , 0x12550 , 0x12FFF},
{"Egyptian Hieroglyphs"                     , 0x13000 , 0x1342F}, // 5.2
{"(0x13430 - 0x143FF)"                      , 0x13430 , 0x143FF},
{"Anatolian Hieroglyphs"                    , 0x14400 , 0x1467F}, // 8.0
{"(0x14680 - 0x167FF)"                      , 0x14680 , 0x167FF},
{"Bamum Supplement"                         , 0x16800 , 0x16A3D}, // 6.0
{"Mro"                                      , 0x16A40 , 0x16A6F}, // 7.0
{"(0x16A70 - 0x16ACF)"                      , 0x16A70 , 0x16ACF},
{"Bassa Vah"                                , 0x16AD0 , 0x16AFF}, // 7.0
{"Pahawh Hmong"                             , 0x16B00 , 0x16B8F}, // 7.0
{"(0x16B90 - 0x16EFF)"                      , 0x16B90 , 0x16EFF},
{"Miao"                                     , 0x16F00 , 0x16F9F}, // 6.1
{"(0x16FA0 - 0x1AFFF)"                      , 0x16FA0 , 0x1AFFF},
{"Kana Supplement"                          , 0x1B000 , 0x1B0FF}, // 6.0
{"(0x1B100 - 0x1BBFF)"                      , 0x1B100 , 0x1BBFF},
{"Duployan"                                 , 0x1BC00 , 0x1BC9F}, // 7.0
{"Shorthand Format Controls"                , 0x1BCA0 , 0x1BCAF}, // 7.0
{"(0x1BCB0 - 0x1CFFF)"                      , 0x1BCB0 , 0x1CFFF},
{"Byzantine Musical Symbols"                , 0x1D000 , 0x1D0FF},
{"Musical Symbols"                          , 0x1D100 , 0x1D1FF},
{"Ancient Greek Musical Notation"           , 0x1D200 , 0x1D24F},
{"(0x1D250 - 0x1D3FF)"                      , 0x1D250 , 0x1D2FF},
{"Tai Xuan Jing Symbols"                    , 0x1D300 , 0x1D35F},
{"Counting Rod Numerals"                    , 0x1D360 , 0x1D37F}, // 5.0
{"(0x1D380 - 0x1D3FF)"                      , 0x1D380 , 0x1D3FF},
{"Mathematical Alphanumeric Symbols"        , 0x1D400 , 0x1D7FF},
{"Sutton SignWriting"                       , 0x1D800 , 0x1DAAF}, // 8.0
{"(0x1DAB0 - 0x1E7FF)"                      , 0x1DAB0 , 0x1E7FF},
{"Mende Kikakui"                            , 0x1E800 , 0x1E8DF}, // 7.0
{"(0x1E8E0 - 0x1EDFF)"                      , 0x1E8E0 , 0x1EDFF},
{"Arabic Mathematical Alphabetic Symbols"   , 0x1EE00 , 0x1EEFF}, // 6.1
{"(0x1EF00 - 0x1EFFF)"                      , 0x1EF00 , 0x1EFFF},
{"Mahjong Tiles"                            , 0x1F000 , 0x1F02F}, // 5.1
{"Domino Tiles"                             , 0x1F030 , 0x1F09F}, // 5.1
{"Playing Cards"                            , 0x1F0A0 , 0x1F0FF}, // 6.0
{"Enclosed Alphanumeric Supplement"         , 0x1F100 , 0x1F1FF}, // 5.2
{"Enclosed Ideographic Supplement"          , 0x1F200 , 0x1F2FF}, // 5.2
{"Miscellaneous Symbols and Pictographs"    , 0x1F300 , 0x1F5FF}, // 6.0
{"Emoticons"                                , 0x1F600 , 0x1F64F}, // 6.0
{"Ornamental Dingbats"                      , 0x1F650 , 0x1F67F}, // 7.0
{"Transport and Map Symbols"                , 0x1F680 , 0x1F6FF}, // 6.0
{"Alchemical Symbols"                       , 0x1F700 , 0x1F77F}, // 6.0
{"Geometric Shapes Extended"                , 0x1F780 , 0x1F7FF}, // 7.0
{"Supplemental Arrows C"                    , 0x1F800 , 0x1F8FF}, // 7.0
{"Supplemental Symbols and Pictographs"     , 0x1F900 , 0x1F9FF}, // 8.0
{"(0x1FA00 - 0x1FF7F)"                      , 0x1FA00 , 0x1FF7F},
{"Unassigned"                               , 0x1FF80 , 0x1FFFD},
{"(Reserved)"                               , 0x1FFFE , 0x1FFFF},

// Plane 2
{"CJK Unified Ideographs Extension B"       , 0x20000 , 0x2A6DF},
{"(0x2A6E0 - 0x2A6FF)"                      , 0x2A6E0 , 0x2A6FF},
{"CJK Unified Ideographs Extension C"       , 0x2A700 , 0x2B73F}, // 5.2
{"CJK Unified Ideographs Extension D"       , 0x2B740 , 0x2B81F}, // 6.0
{"CJK Unified Ideographs Extension E"       , 0x2B820 , 0x2CEA1}, // 8.0
{"(0x2CEA2 - 0x2F7FF)"                      , 0x2CEA2 , 0x2F7FF},
{"CJK Compatibility Ideographs Supplement"  , 0x2F800 , 0x2FA1F}, 
{"(0x2FA20 - 0x2FF7F)"                      , 0x2FA20 , 0x2FF7F},
{"Unassigned"                               , 0x2FF80 , 0x2FFFD},
{"(Reserved)"                               , 0x2FFFE , 0x2FFFF},

// Plane 3
{"(0x30000 - 0x3FFFD)"                      , 0x30000 , 0x3FFFD},
{"(Reserved)"                               , 0x3FFFE , 0x3FFFF},

// Plane 4
{"(0x40000 - 0x4FFFD)"                      , 0x40000 , 0x4FFFD},
{"(Reserved)"                               , 0x4FFFE , 0x4FFFF},

// Plane 5
{"(0x50000 - 0x5FFFD)"                      , 0x50000 , 0x5FFFD},
{"(Reserved)"                               , 0x5FFFE , 0x5FFFF},

// Plane 6
{"(0x60000 - 0x6FFFD)"                      , 0x60000 , 0x6FFFD},
{"(Reserved)"                               , 0x6FFFE , 0x6FFFF},

// Plane 7
{"(0x70000 - 0x7FFFD)"                      , 0x70000 , 0x7FFFD},
{"(Reserved)"                               , 0x7FFFE , 0x7FFFF},

// Plane 8
{"(0x80000 - 0x8FFFD)"                      , 0x80000 , 0x8FFFD},
{"(Reserved)"                               , 0x8FFFE , 0x8FFFF},

// Plane 9
{"(0x90000 - 0x9FFFD)"                      , 0x90000 , 0x9FFFD},
{"(Reserved)"                               , 0x9FFFE , 0x9FFFF},

// Plane 10
{"(0xA0000 - 0xAFFFD)"                      , 0xA0000 , 0xAFFFD},
{"(Reserved)"                               , 0xAFFFE , 0xAFFFF},

// Plane 11
{"(0xB0000 - 0xBFFFD)"                      , 0xB0000 , 0xBFFFD},
{"(Reserved)"                               , 0xBFFFE , 0xBFFFF},

// Plane 12
{"(0xC0000 - 0xCFFFD)"                      , 0xC0000 , 0xCFFFD},
{"(Reserved)"                               , 0xCFFFE , 0xCFFFF},

// Plane 13
{"(0xD0000 - 0xDFFFD)"                      , 0xD0000 , 0xDFFFD},
{"(Reserved)"                               , 0xDFFFE , 0xDFFFF},

// Plane 14
{"Tags"                                     , 0xE0000 , 0xE007F},
{"(0xE0080 - 0xE00FF)"                      , 0xE0080 , 0xE00FF},
{"Variation Selectors Supplement"           , 0xE0100 , 0xE01EF},
{"(0xE01F0 - 0xEFFFD)"                      , 0xE01F0 , 0xEFFFD},
{"(Reserved)"                               , 0xEFFFE , 0xEFFFF},

// Plane 15
{"Supplementary Private Use Area A"         , 0xF0000 , 0xFFFFD}, 
{"(Reserved)"                               , 0xFFFFE , 0xFFFFF},

// Plane 16
{"Supplementary Private Use Area B"         , 0x100000, 0x10FFFD},
{"(Reserved)"                               , 0x10FFFE, 0x10FFFF},
};

const int numUnicodeSubsets = sizeof(UnicodeSubsets)/sizeof(UnicodeSubset_t);

int GetSubsetFromChar(unsigned int chr)
{
	for( int n = 0; n < numUnicodeSubsets; n++ )
	{
		if( chr >= (unsigned)UnicodeSubsets[n].beginChar && chr <= (unsigned)UnicodeSubsets[n].endChar )
			return n;
	}

	return -1;
}

string GetCharSetName(int charSet)
{
	string str;

	switch( charSet )
	{
	case ANSI_CHARSET:
		str = "ANSI";
		break;
	case DEFAULT_CHARSET:
		str = "DEFAULT";
		break;
	case SYMBOL_CHARSET:
		str = "SYMBOL";
		break;
	case SHIFTJIS_CHARSET:
		str = "SHIFTJIS";
		break;
	case HANGUL_CHARSET:
		str = "HANGUL";
		break;
	case GB2312_CHARSET:
		str = "GB2312";
		break;
	case CHINESEBIG5_CHARSET:
		str = "CHINESEBIG5";
		break;
	case OEM_CHARSET:
		str = "OEM";
		break;
	case 130: // JOHAB_CHARSET
		str = "JOHAB";
		break;
	case 177: // HEBREW_CHARSET
		str = "HEBREW";
		break;
	case 178: // ARABIC_CHARSET
		str = "ARABIC";
		break;
	case 161: // GREEK_CHARSET
		str = "GREEK";
		break;
	case 162: // TURKISH_CHARSET
		str = "TURKISH";
		break;
	case 163: // VIETNAMESE_CHARSET
		str = "VIETNAMESE";
		break;
	case 222: // THAI_CHARSET
		str = "THAI";
		break;
	case 238: // EASTEUROPE_CHARSET
		str = "EASTEUROPE";
		break;
	case 204: // RUSSIAN_CHARSET
		str = "RUSSIAN";
		break;
	case 77:  // MAC_CHARSET
		str = "MAC";
		break;
	case 186: // BALTIC_CHARSET
		str = "BALTIC";
		break;

	default:
		str = acStringFormat("%d", charSet);
	}
	
	return str;
}

int GetCharSet(const char *charSetName)
{
	string str = charSetName;
	int set = 0;

	if( str == "ANSI" )             set = 0;
	else if( str == "DEFAULT" )     set = 1;
	else if( str == "SYMBOL" )      set = 2;
	else if( str == "SHIFTJIS" )    set = 128;
	else if( str == "HANGUL" )      set = 129;
	else if( str == "GB2312" )      set = 134;
	else if( str == "CHINESEBIG5" ) set = 136;
	else if( str == "OEM" )         set = 255;
	else if( str == "JOHAB" )       set = 130;
	else if( str == "HEBREW" )      set = 177;
	else if( str == "ARABIC" )      set = 178;
	else if( str == "GREEK" )       set = 161;
	else if( str == "TURKISH" )     set = 162;
	else if( str == "VIETNAMESE" )  set = 163;
	else if( str == "THAI" )        set = 222;
	else if( str == "EASTEUROPE" )  set = 238;
	else if( str == "RUSSIAN" )     set = 204;
	else if( str == "MAC" )         set = 77;
	else if( str == "BALTIC" )      set = 186;
	else set = acStringScanInt(charSetName, 10, 0);

	return set;
}

int EnumUnicodeGlyphs(HDC dc, map<unsigned int, unsigned int> &unicodeToGlyphMap)
{
	unicodeToGlyphMap.clear();

	// This function is only called as fallback in case EnumTrueTypeCMAP didn't
	// work. so we can assume that the font doesn't have 32bit Unicode support
	for (unsigned int ch = 0; ch < 0xFFFF; ch++)
	{
		// Skip the range for surrogate pairs
		if (ch == 0xD800)
		{
			ch = 0xDFFF;
			continue;
		}

		WCHAR buf[] = { WCHAR(ch) };
		WORD idx; 
		int r = fGetGlyphIndicesW(dc, buf, 1, &idx, GGI_MARK_NONEXISTING_GLYPHS);
		if (r != GDI_ERROR && idx != 0xFFFF && idx != 0)
			unicodeToGlyphMap[ch] = idx;
	}

	return 0;
}

/*
// I've retired this code. Instead the EnumTrueTypeCMAP or EnumUnicodeGlyphs shall be used
// to map all the existing glyphs in one pass, and then the lookup is done using that map.
// The ScriptShape wasn't very reliable anyway, and returned a lot of characters as existing
// even though they didn't.
int GetUnicodeGlyphIndex(HDC dc, SCRIPT_CACHE *sc, UINT ch)
{
	SCRIPT_CACHE mySc = 0;
	if( sc == 0 ) sc = &mySc;

	if( ch < 0x10000 )
	{
		// GetGlyphIndices seems to work better than ScriptShape in 
		// the base plane. It reports less missing characters as existing
		WCHAR buf[] = {WCHAR(ch)};
		WORD idx;
		int r = fGetGlyphIndicesW(dc, buf, 1, &idx, GGI_MARK_NONEXISTING_GLYPHS);
		if( r == GDI_ERROR || idx == 0xFFFF )
			return -1;

		return idx;
	}

	// Convert the unicode character to a UTF16 encoded 
	// buffer that Uniscribe understands
	WCHAR buf[2];
	int length = acUtility::EncodeUTF16(ch, (unsigned char*)buf, 0);

	// Call ScriptItemize to analyze the unicode string 
	// to find it's logical pieces
	SCRIPT_ITEM items[2];
	int nitems;
	HRESULT hr;
	hr = ScriptItemize(buf, length/2, 2, 0, 0, items, &nitems);
	if( FAILED(hr) )
		return -1;

	// Call ScriptShape to determine the glyphs that will 
	// be used to render each character
	WORD glyphs[10];
	WORD cluster[10];
	int nglyphs;
	SCRIPT_VISATTR attr[10];
	hr = ScriptShape(dc, sc, &buf[0], length/2, 10, &items[0].a, glyphs, cluster, attr, &nglyphs);
	if( FAILED(hr) )
		return -1;

	if( mySc ) 
		ScriptFreeCache(&mySc);

	// Was the glyph found?
	return glyphs[0];
}
*/
/*
int DoesUnicodeCharExist(HDC dc, SCRIPT_CACHE *sc, UINT ch)
{
	int idx = GetUnicodeGlyphIndex(dc, sc, ch);
	
	if( idx < 0 ) 
		return 0;

	SCRIPT_FONTPROPERTIES props;
	props.cBytes = sizeof(props);
	ScriptGetFontProperties(dc, sc, &props);	
	
	if( idx != props.wgDefault )
		return 1;

	return 0;
}
*/

int GetGlyphABCWidths(HDC dc, SCRIPT_CACHE *sc, UINT glyph, ABC *abc)
{
	SCRIPT_CACHE mySc = 0;
	if( sc == 0 ) sc = &mySc;

	HRESULT hr = ScriptGetGlyphABCWidth(dc, sc, glyph, abc);
	if( FAILED(hr) )
	{
		if( mySc ) ScriptFreeCache(&mySc);
		return -1;
	}

	if( mySc ) ScriptFreeCache(&mySc);
	return 0;
}

//=================================================================================
// The functions below are all for extracting kerning data from the GPOS table
//
// Reference: http://www.microsoft.com/typography/otspec/gpos.htm
//            http://www.microsoft.com/typography/otspec/otff.htm
//            http://partners.adobe.com/public/developer/opentype/index_table_formats2.html
//

UINT GetClassFromClassDef(BYTE *classDef, WORD glyphId)
{
	// Go through the class def to determine in which class the glyph belongs
	WORD classFormat = GETUSHORT(classDef);
	if( classFormat == 1 )
	{
		WORD startGlyph = GETUSHORT(classDef+2);
		WORD glyphCount = GETUSHORT(classDef+4);

		if( startGlyph <= glyphId && glyphId - startGlyph < glyphCount )
			return GETUSHORT(classDef+6+2*(glyphId - startGlyph));
	}
	else if( classFormat == 2 )
	{
		WORD rangeCount = GETUSHORT(classDef+2);
		for( UINT n = 0; n < rangeCount; n++ )
		{
			WORD start = GETUSHORT(classDef+4+6*n);
			WORD end   = GETUSHORT(classDef+6+6*n);
			if( start <= glyphId && end >= glyphId )
				return GETUSHORT(classDef+8+6*n);
		}
	}

	return 0;
}

vector<UINT> GetGlyphsFromClassDef(BYTE *classDef, UINT classId)
{
	// Find the class, and return all the glyphs that are part of it
	vector<UINT> glyphs;

	WORD classFormat = GETUSHORT(classDef);
	if( classFormat == 1 )
	{
		WORD startGlyph = GETUSHORT(classDef+2);
		WORD glyphCount = GETUSHORT(classDef+4);

		for( UINT n = 0; n < glyphCount; n++ )
		{
			if( GETUSHORT(classDef+6+2*n) == classId )
				glyphs.push_back(startGlyph + n);
		}
	}
	else if( classFormat == 2 )
	{
		WORD rangeCount = GETUSHORT(classDef+2);
		for( UINT n = 0; n < rangeCount; n++ )
		{
			WORD start = GETUSHORT(classDef+4+6*n);
			WORD end   = GETUSHORT(classDef+6+6*n);
			if( GETUSHORT(classDef+8+6*n) == classId )
			{
				for( UINT g = start; g <= end; g++ )
					glyphs.push_back(g);
			}
		}
	}

	return glyphs;
}

float DetermineDesignUnitToFontUnitFactor(HDC dc)
{
	OUTLINETEXTMETRIC tm;
	GetOutlineTextMetrics(dc, sizeof(tm), &tm);

	DWORD head = TAG('h','e','a','d');
	UINT size = GetFontData(dc, head, 0, 0, 0);
	if( size != GDI_ERROR )
	{
		vector<BYTE> buffer;
		buffer.resize(size);
		GetFontData(dc, head, 0, &buffer[0], size);

		SHORT xMin = GETUSHORT(&buffer[36]);
		SHORT yMin = GETUSHORT(&buffer[38]);
		SHORT xMax = GETUSHORT(&buffer[40]);
		SHORT yMax = GETUSHORT(&buffer[42]);

		float factor = float(tm.otmrcFontBox.top-tm.otmrcFontBox.bottom)/float(yMax-yMin);
		return factor;
	}

	return 1;
}

void AddKerningPairToList(HDC dc, UINT glyphId1, UINT glyphId2, int kerning, vector<KERNINGPAIR> &pairs, float scaleFactor, map<UINT,vector<UINT>> &glyphIdToChar)
{
	assert(kerning != 0);

	for( UINT a = 0; a < glyphIdToChar[glyphId1].size(); a++ )
	{
		for( UINT b = 0; b < glyphIdToChar[glyphId2].size(); b++ )
		{
			// Add the kerning pair to the list
			KERNINGPAIR pair;
			pair.wFirst      = glyphIdToChar[glyphId1][a];
			pair.wSecond     = glyphIdToChar[glyphId2][b];
			if( pair.wFirst == 0 || pair.wSecond == 0 )
				return;

			// Convert from design units to the selected font size
			float kern = kerning*scaleFactor;
			if( kern < 0 )
				pair.iKernAmount = int(kern-0.5f);
			else
				pair.iKernAmount = int(kern+0.5f);

			// Skip 0 kernings
			if( pair.iKernAmount == 0 )
				return;

			if( pairs.capacity() == pairs.size() && pairs.size() > 1 )
				pairs.reserve(pairs.size() * 2);
			pairs.push_back(pair);
		}
	}
}

UINT GetSizeOfValueType(WORD valueType)
{
	UINT size = 0;
	// TODO: Are these the only flags?
	if( valueType & 1 ) // x placement
		size += 2;
	if( valueType & 2 ) // y placement
		size += 2;
	if( valueType & 4 ) // x advance
		size += 2;
	if( valueType & 8 ) // y advance
		size += 2;
	if( valueType & 16 ) // offset to device x placement
		size += 2;
	if( valueType & 32 ) // offset to device y placement
		size += 2;
	if( valueType & 64 ) // offset to device x advance
		size += 2;
	if( valueType & 128 ) // offset to device y advance
		size += 2;
	return size;
}

short GetXAdvance(BYTE *value, WORD valueType)
{
	if( !(valueType & 4) ) 
		return 0;

	UINT offset = 0;
	if( valueType & 1 ) offset += 2;
	if( valueType & 2 ) offset += 2;

	return GETSHORT(value+offset);
}

void ProcessPairAdjustmentFormat1(HDC dc, BYTE *subTable, vector<KERNINGPAIR> &pairs, map<UINT,vector<UINT>> &glyphIdToChar, float scaleFactor)
{
	// Defines kerning between two individual glyphs

	WORD coverageOffset = GETUSHORT(subTable+2);
	WORD valueFormat1   = GETUSHORT(subTable+4);
	assert( valueFormat1 == 4 ); // TODO: Must support others
	WORD valueFormat2   = GETUSHORT(subTable+6);
	assert( valueFormat2 == 0 ); // TODO: Must support others
	WORD pairSetCount   = GETUSHORT(subTable+8);

	UINT valuePairSize = GetSizeOfValueType(valueFormat1) + GetSizeOfValueType(valueFormat2);

	// The first glyph id in the pair is found in the coverage table
	// The second glyph id in the pair is found in the PairSet records
	
	BYTE *coverage = subTable + coverageOffset;
	WORD coverageFormat = GETUSHORT(coverage);
	if( coverageFormat == 1 )
	{
		WORD glyphCount = GETUSHORT(coverage+2);
		assert(glyphCount == pairSetCount);
		for( DWORD g = 0; g < glyphCount; g++ )
		{
			WORD glyphId1 = GETUSHORT(coverage+4+2*g);
			if( glyphIdToChar[glyphId1].size() == 0 )
				continue;

			// For each of the glyph ids we need to search the 
			// PairSets for the matching kerning pairs
			WORD pairSetOffset = GETUSHORT(subTable+10+g*2);
			BYTE *pairSet = subTable + pairSetOffset;
			WORD pairValueCount = GETUSHORT(pairSet);
			for( UINT p = 0; p < pairValueCount; p++ )
			{
				BYTE *pairValue = pairSet + 2 + p*(2+valuePairSize);

				WORD glyphId2 = GETUSHORT(pairValue);
				short xAdv1 = GetXAdvance(pairValue+2, valueFormat1);

				if( xAdv1 != 0 )
				{
					AddKerningPairToList(dc, glyphId1, glyphId2, xAdv1, pairs, scaleFactor, glyphIdToChar);
				}
			}
		}
	}
	else
	{
		WORD rangeCount = GETUSHORT(coverage+2);

		// TODO: Implement this
		assert( false );
	}
}

void ProcessPairAdjustmentFormat2(HDC dc, BYTE *subTable, vector<KERNINGPAIR> &pairs, map<UINT,vector<UINT>> &glyphIdToChar, float scaleFactor)
{
	// Defines kerning between two classes of glyphs

	WORD coverageOffset  = GETUSHORT(subTable+2);
	WORD valueFormat1    = GETUSHORT(subTable+4);
	assert( valueFormat1 == 4 ); // TODO: must support others
	WORD valueFormat2    = GETUSHORT(subTable+6);
	assert( valueFormat2 == 0 ); // TODO: must support others
	WORD classDefOffset1 = GETUSHORT(subTable+8);
	WORD classDefOffset2 = GETUSHORT(subTable+10);
	WORD classCount1     = GETUSHORT(subTable+12);
	WORD classCount2     = GETUSHORT(subTable+14);

	WORD valuePairSize = GetSizeOfValueType(valueFormat1) + GetSizeOfValueType(valueFormat2);

	// The first glyph id in the pair is found in the coverage table
	// The second glyph id is determined from the class definitions

	vector<WORD> glyph1;

	BYTE *coverage = subTable + coverageOffset;
	WORD coverageFormat = GETUSHORT(coverage);
	if( coverageFormat == 1 )
	{
		WORD glyphCount = GETUSHORT(coverage+2);
		glyph1.reserve(glyphCount);

		for( DWORD g = 0; g < glyphCount; g++ )
		{
			WORD glyphId = GETUSHORT(coverage+4+2*g);
			if( glyphIdToChar[glyphId].size() )
				glyph1.push_back(glyphId);
		}
	}
	else
	{
		WORD rangeCount = SWAP16(*(WORD*)(coverage+2));

		// Expand the ranges into the glyph1 array
		for( UINT n = 0; n < rangeCount; n++ )
		{
			WORD start = GETUSHORT(coverage+4+n*6);
			WORD end   = GETUSHORT(coverage+6+n*6);
			WORD startCoverageIndex = GETUSHORT(coverage+8+n*6);

			// TODO: Reserve space for all the glyph ids to reduce number of individual allocations

			for( UINT g = start; g <= end; g++ )
			{
				if( glyphIdToChar[g].size() )
					glyph1.push_back(g);
			}
		}
	}

	for( UINT g = 0; g < glyph1.size(); g++ )
	{
		// What class is this glyph?
		// Need a function for obtaining the class from a ClassDef
		UINT c1 = GetClassFromClassDef(subTable + classDefOffset1, glyph1[g]);

		assert( c1 < classCount1 );
	
		if( c1 < classCount1 )
		{
			BYTE *c1List = subTable + 16 + c1*classCount2*valuePairSize;

			// For each of the classes 
			for( UINT c2 = 0; c2 < classCount2; c2++ )
			{
				// Enumerate the glyphs that are part of the classes
				vector<UINT> glyph2 = GetGlyphsFromClassDef(subTable + classDefOffset2, c2);

				BYTE *valuePair = c1List + valuePairSize*c2;

				short xAdv1 = GetXAdvance(valuePair, valueFormat1);
				if( xAdv1 != 0 )
				{
					// Add a kerning pair for each combination of glyphs in each of the classes
					for( UINT n = 0; n < glyph2.size(); n++ )
					{
						AddKerningPairToList(dc, glyph1[g], glyph2[n], xAdv1, pairs, scaleFactor, glyphIdToChar);
					}
				}
			}
		}
	}
}

void ProcessKernFeature(HDC dc, BYTE *featureRecord, BYTE *featureList, BYTE *lookupList, vector<KERNINGPAIR> &pairs, map<UINT,vector<UINT>> &glyphIdToChar, float scaleFactor)
{
	WORD offset = GETUSHORT(featureRecord+4);

	BYTE *feature = featureList + offset;
	WORD featureParams = GETUSHORT(feature);
	WORD lookupCount = GETUSHORT(feature+2);
	WORD allLookupCount = GETUSHORT(lookupList);
	
	for( DWORD i = 0; i < lookupCount; i++ )
	{
		WORD lookupIndex = GETUSHORT(feature+4+i*2);

		// Determine the features that apply (look for kerning pairs)

		// Find the adjustments in the lookup table
		if( lookupIndex < allLookupCount )
		{
			WORD lookupOffset = GETUSHORT(lookupList + 2 + lookupIndex*2);

			BYTE *lookup = lookupList + lookupOffset;

			WORD lookupType = GETUSHORT(lookup);
			WORD lookupFlag = GETUSHORT(lookup+2);
			WORD subTableCount = GETUSHORT(lookup+4);

			for( UINT s = 0; s < subTableCount; s++ )
			{
				WORD offset = GETUSHORT(lookup + 6 + s*2);
				BYTE *subTable = lookup + offset;

				WORD realLookupType = lookupType;

				if( lookupType == 9 ) // extension positioning
				{
					WORD posFormat = GETUSHORT(subTable);
					assert( posFormat == 1 ); // reserved
					WORD extensionLookupType = GETUSHORT(subTable+2);
					DWORD extensionOffset = GETUINT(subTable+4);

					realLookupType = extensionLookupType;
					subTable = subTable + extensionOffset;
				}

				if( realLookupType == 2 ) // pair adjustment
				{
					WORD posFormat    = GETUSHORT(subTable);
					if( posFormat == 1 )
						ProcessPairAdjustmentFormat1(dc, subTable, pairs, glyphIdToChar, scaleFactor);
					else if( posFormat == 2 )
						ProcessPairAdjustmentFormat2(dc, subTable, pairs, glyphIdToChar, scaleFactor);
					else
						assert(false);
				}
			}
		}
	}
}

void GetKerningPairsFromGPOS(HDC dc, vector<KERNINGPAIR> &pairs, vector<UINT> &chars, const CFontGen *gen)
{
	// Determine the factor for scaling down the values from the design units to the font size
	float scaleFactor = DetermineDesignUnitToFontUnitFactor(dc);

	// TODO: support non unicode as well
	// Build a glyphId to char map. Multiple characters may use  
	// the same glyph, e.g. space, 32, and hard space, 160.
	map<UINT,vector<UINT>> glyphIdToChar;
	for( UINT n = 0; n < chars.size(); n++ )
	{
		int glyphId = gen->GetUnicodeGlyph(chars[n]);
		if( glyphId >= 0 )
			glyphIdToChar[glyphId].push_back(chars[n]);
	}

	// Load the GPOS table from the TrueType font file
	vector<BYTE> buffer;
	DWORD GPOS = TAG('G','P','O','S');
	DWORD size = GetFontData(dc, GPOS, 0, 0, 0);
	if( size != GDI_ERROR )
	{
		buffer.resize(size);
		size = GetFontData(dc, GPOS, 0, &buffer[0], size);
	}
	if( size == GDI_ERROR || size == 0 )
		return;

	// Get the GPOS header info
	DWORD version          = GETUINT(&buffer[0]);
	assert( version == 0x00010000 );
	WORD scriptListOffset  = GETUSHORT(&buffer[4]);
	WORD featureListOffset = GETUSHORT(&buffer[6]);
	WORD lookupListOffset  = GETUSHORT(&buffer[8]);

	BYTE *scriptList  = &buffer[0] + scriptListOffset;
	BYTE *featureList = &buffer[0] + featureListOffset;
	BYTE *lookupList  = &buffer[0] + lookupListOffset;

	// Locate the default script in the script list table
	WORD scriptCount = GETUSHORT(scriptList);
	WORD offset = 0;
	for( UINT c = 0; c < scriptCount; c++ )
	{
		BYTE *scriptRecord = scriptList + 2 + c*6;
		DWORD tag = *(DWORD*)scriptRecord;
		if( tag == TAG('D','F','L','T') )
		{
			offset = GETUSHORT(scriptRecord+4);
			break;
		}
	}

	if( offset == 0 )
		return;

	// Use the default language
	BYTE *script = scriptList + offset;
	WORD defaultLangSysOffset = GETUSHORT(script);
	WORD langSysCount = GETUSHORT(script+2);
	if( defaultLangSysOffset == 0 )
		return;

	BYTE *langSys = script + defaultLangSysOffset;
		
	WORD lookupOrder = GETUSHORT(langSys);
	assert( lookupOrder == 0 ); // reserved for future use
	WORD reqFeatureIndex = GETUSHORT(langSys+2); // Can be 0xFFFF if not used
	WORD featureCount    = GETUSHORT(langSys+4);

	// Find all kerning pairs from all the features that apply
	WORD allFeatureCount = GETUSHORT(featureList);
	for( UINT c = 0; c < featureCount; c++ )
	{
		WORD featureIndex = GETUSHORT(langSys+6+c*2);

		if( featureIndex < allFeatureCount )
		{
			BYTE *featureRecord = featureList + 2 + 6*featureIndex;

			DWORD tag = *(DWORD*)(featureRecord);
			if( tag == TAG('k','e','r','n') )
			{
				ProcessKernFeature(dc, featureRecord, featureList, lookupList, pairs, glyphIdToChar, scaleFactor);
			}
		}
	}
}

//=================================================================================
// The functions below are all for extracting kerning data from the KERN table
//
// Reference: http://www.microsoft.com/typography/otspec/kern.htm
//            http://www.microsoft.com/typography/otspec/otff.htm
//            http://partners.adobe.com/public/developer/opentype/index_table_formats2.html
//


void GetKerningPairsFromKERN(HDC dc, vector<KERNINGPAIR> &pairs, vector<UINT> &chars, const CFontGen *gen)
{
	// Determine the factor for scaling down the values from the design units to the font size
	float scaleFactor = DetermineDesignUnitToFontUnitFactor(dc);

	// TODO: support non unicode as well
	// Build a glyphId to char map. Multiple characters may use  
	// the same glyph, e.g. space, 32, and hard space, 160.
	map<UINT,vector<UINT>> glyphIdToChar;
	for( UINT n = 0; n < chars.size(); n++ )
	{
		int glyphId = gen->GetUnicodeGlyph(chars[n]);
		if( glyphId >= 0 )
			glyphIdToChar[glyphId].push_back(chars[n]);
	}

	// Load the KERN table from the TrueType font file
	vector<BYTE> buffer;
	DWORD KERN = TAG('k','e','r','n');
	DWORD size = GetFontData(dc, KERN, 0, 0, 0);
	if( size != GDI_ERROR )
	{
		buffer.resize(size);
		size = GetFontData(dc, KERN, 0, &buffer[0], size);
	}
	if( size == GDI_ERROR || size == 0 )
		return;

	// Get the KERN header info
	WORD version = GETUSHORT(&buffer[0]);
	assert( version == 0x0000 );
	WORD nTables = GETUSHORT(&buffer[2]);

	UINT pos = 4;
	for( unsigned int n = 0; n < nTables; n++ )
	{
		WORD version  = GETUSHORT(&buffer[pos+0]);
		assert( version == 0x0000 );
		WORD length   = GETUSHORT(&buffer[pos+2]);
		WORD coverage = GETUSHORT(&buffer[pos+4]);

		// We currently only support horizontal text, and don't care about vertical adjustments 
		if( (coverage & 1) == 1 && (coverage & 4) == 0 )
		{
			// Check if this table holds kerning values
			if( (coverage & 2) == 0 )
			{
				BYTE format = (coverage>>8);
				if( format == 0 )
				{
					WORD nPairs        = GETUSHORT(&buffer[pos+6]);
					WORD searchRange   = GETUSHORT(&buffer[pos+8]);
					WORD entrySelector = GETUSHORT(&buffer[pos+10]);
					WORD rangeShift    = GETUSHORT(&buffer[pos+12]);

					// Read each pair
					for( int c = 0; c < nPairs; c++ )
					{
						WORD  left  = GETUSHORT(&buffer[pos+14+c*6]);
						WORD  right = GETUSHORT(&buffer[pos+16+c*6]);
						short value = GETSHORT(&buffer[pos+18+c*6]);

						if( value )
							AddKerningPairToList(dc, left, right, value, pairs, scaleFactor, glyphIdToChar);
					}
				}
				else if( format == 2 )
				{
					// TODO: This format is not properly supported by Windows according to 
					//       above references, so I'll only implement it when I find a need
					//       for it.
					assert( false );
				}
				else
				{
					// These other formats are undefined
					assert( false );
				}
			}
			else
			{
				// TODO: Tables with minimum should be used to limit the accumulated adjustment
			}
		}

		pos += length;
	}
}

//=================================================================================
//
// ref: http://www.microsoft.com/typography/otspec/otff.htm
// ref: https://www.microsoft.com/typography/otspec/cmap.htm

int EnumTrueTypeCMAP(HDC dc, map<unsigned int, unsigned int> &unicodeToGlyphMap)
{
	// Remove old mappings
	unicodeToGlyphMap.clear();

	// Load the CMAP table from the TrueType font file
	vector<BYTE> buffer;
	DWORD CMAP = TAG('c', 'm', 'a', 'p');
	DWORD size = GetFontData(dc, CMAP, 0, 0, 0);
	if (size != GDI_ERROR)
	{
		buffer.resize(size);
		size = GetFontData(dc, CMAP, 0, &buffer[0], size);
	}
	if (size == GDI_ERROR || size == 0)
		return -1;

	// Get the CMAP header info
	WORD version = GETUSHORT(&buffer[0]);
	assert(version == 0x0000);
	WORD nTables = GETUSHORT(&buffer[2]);

	UINT pos = 4;
	for (unsigned int n = 0; n < nTables; n++)
	{
		// Look for the Windows platform CMAP subtable
		WORD platformID = GETUSHORT(&buffer[pos + 0]);
		WORD encodingID = GETUSHORT(&buffer[pos + 2]);
		ULONG offset = GETUINT(&buffer[pos + 4]);
		pos += 8;

		// platformID 1 is for Mac OS
		// platformID 3 is for Windows
		if (platformID == 3)
		{
			// For now this code only supports the following encodings:
			// 1 for Unicode UCS-2 (16bit)
			// 10 for Unicode UCS-4 (32bit)
			if (encodingID == 1 || encodingID == 10)
			{
				// Jump to the start of the subtable
				WORD format = GETUSHORT(&buffer[offset]);

				if (format == 0) // byte encoding table
					assert(false);
				else if (format == 2) // high-byte mapping through table
					assert(false);
				else if (format == 4) // segment mapping to delta values
				{
					// This is the standard format used by Microsoft
					// This format is for example used in the 'Arial' true type font

					// This format supports all Unicode characters below 0xFFFF.
					// The range U+D800 - U+DFFF is reserved for surrogates and cannot have any characters
					WORD length = GETUSHORT(&buffer[offset + 2]);
					WORD language = GETUSHORT(&buffer[offset + 4]);
					assert(language == 0);
					WORD segCountX2 = GETUSHORT(&buffer[offset + 6]);
					WORD searchRange = GETUSHORT(&buffer[offset + 8]);
					WORD entrySelector = GETUSHORT(&buffer[offset + 10]);
					WORD rangeShift = GETUSHORT(&buffer[offset + 12]);

					WORD segCount = segCountX2 / 2;
					WORD *endCount = (WORD*)&buffer[offset + 14];
					WORD *startCount = (WORD*)&buffer[offset + 16 + segCountX2];
					SHORT *idDelta = (SHORT*)&buffer[offset + 16 + segCountX2 * 2];
					WORD *idRangeOffset = (WORD*)&buffer[offset + 16 + segCountX2 * 3];
					// glyphIdArray is used indirectly so we don't need to take its address
					//WORD *glyphIdArray = (WORD*)&buffer[offset + 16 + segCountX2 * 4];
					
					// Iterate over each segment to identify the characters to glyph id mappings
					for (unsigned int s = 0; s < segCount; s++)
					{
						WORD start = SWAP16(startCount[s]);
						WORD end = SWAP16(endCount[s]);

						if (start == 0xFFFF && end == 0xFFFF)
						{
							// We've reached the end
							assert(s == segCount - 1);
							break;
						}

						WORD rangeOffset = SWAP16(idRangeOffset[s]);
						SHORT delta = SWAP16(idDelta[s]);
						for (unsigned int ch = start; ch <= end; ch++)
						{
							WORD glyphId = 0;
							if (idRangeOffset[s] != 0)
							{
								glyphId = SWAP16(*(&idRangeOffset[s] + rangeOffset / 2 + (ch - start)));
								if (glyphId != 0)
									glyphId += delta;
							}
							else
							{
								glyphId = ch + delta;
							}
							if (glyphId != 0)
								unicodeToGlyphMap[ch] = glyphId;
						}
					}
				}
				else if (format == 6) // trimmed table mapping
					assert(false);
				else if (format == 8) // mixed 16-bit and 32-bit coverage
					assert(false);
				else if (format == 10) // trimmed array
					assert(false);
				else if (format == 12) // segmented coverage
				{
					// This is the standard format used by Microsoft
					// This format is for example used in the 'Cambria Math' and 'DejaVu Sans' true type fonts

					ULONG length = GETUINT(&buffer[offset + 4]);
					ULONG language = GETUINT(&buffer[offset + 8]);
					assert(language == 0);
					ULONG nGroups = GETUINT(&buffer[offset + 12]);
					
					// Iterate over each group to identify the characters to glyph id mappings
					offset += 16;
					for (unsigned int s = 0; s < nGroups; s++)
					{
						ULONG start = GETUINT(&buffer[offset + 0]);
						ULONG end = GETUINT(&buffer[offset + 4]);
						ULONG startGlyphId = GETUINT(&buffer[offset + 8]);

						for (unsigned int ch = start; ch <= end; ch++)
						{
							ULONG glyphId = ch - start + startGlyphId;
							unicodeToGlyphMap[ch] = glyphId;
						}

						offset += 12;
					}
				}
				else if (format == 13) // many-to-one range mappings
					assert(false);
				else if (format == 14) // unicode variation sequences
					assert(false);
				else
					assert(false);
			}
		}
	}

	return 0;
}

void ConvertWCharToUtf8(const WCHAR *buf, std::string &utf8)
{
	char bufUTF8[1024];
	WideCharToMultiByte(CP_UTF8, 0, buf, -1, bufUTF8, 1024, NULL, NULL);
	utf8 = bufUTF8;
}

void ConvertUtf8ToWChar(const std::string &utf8, WCHAR *buf, size_t bufSize)
{
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buf, int(bufSize));
}

// ref: https://stackoverflow.com/questions/11387564/get-a-font-filepath-from-name-and-style-in-c-windows
// ref: https://stackoverflow.com/questions/16769758/get-a-font-filename-based-on-the-font-handle-hfont
std::string GetFontFileName(const std::string &faceName, bool bold, bool italic)
{
	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

	HKEY hKey;
	LONG result;
	WCHAR buf[512];
	ConvertUtf8ToWChar(faceName, buf, 512);
	std::wstring wsFaceName(buf);

	// Open Windows font registry key
	result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) 
		return "";

	DWORD maxValueNameSize, maxValueDataSize;
	result = RegQueryInfoKeyW(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
	if (result != ERROR_SUCCESS)
		return "";

	DWORD valueIndex = 0;
	LPWSTR valueName = new WCHAR[maxValueNameSize];
	LPBYTE valueData = new BYTE[maxValueDataSize];
	DWORD valueNameSize, valueDataSize, valueType;
	std::wstring wsFontFile;

	// Look for a matching font name
	do 
	{
		wsFontFile.clear();
		valueDataSize = maxValueDataSize;
		valueNameSize = maxValueNameSize;

		result = RegEnumValueW(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

		valueIndex++;

		if (result != ERROR_SUCCESS || valueType != REG_SZ)
			continue;

		std::wstring wsValueName(valueName, valueNameSize);

		// Found a match
		// TODO: The face name is not always the first part of the regkey
		//       e.g. The font "Microsoft JhengHei UI" matches the regkey "Microsoft JhengHei & Microsoft JhengHei UI (TrueType)"
		// TODO: It is not enough that the face name is part of the regkey name
		//       e.g. The font "DejaVu Sans" shouldn't match "DejaVu Sans Condensed (TrueType)"
		if (wcsstr(wsValueName.c_str(), wsFaceName.c_str()) != 0)
		{
			// TODO: Make sure the font data actually match that of the file
			wsFontFile.assign((LPWSTR)valueData, valueDataSize);
			break;
		}
	} while (result != ERROR_NO_MORE_ITEMS);

	delete[] valueName;
	delete[] valueData;

	RegCloseKey(hKey);

	if (wsFontFile.empty())
		return "";

	// Build full font file path
	WCHAR winDir[MAX_PATH];
	GetWindowsDirectoryW(winDir, MAX_PATH);

	std::wstringstream ss;
	ss << winDir << "\\Fonts\\" << wsFontFile;
	wsFontFile = ss.str();

	std::string out;
	ConvertWCharToUtf8(wsFontFile.c_str(), out);

	return out;
}

