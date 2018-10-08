using namespace std;

#include "TurkishStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    TurkishStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"m", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"miz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"niz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"muz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nuz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"m\u00FCz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u00FCz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"m\u0131z", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u0131z", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                               L"leri", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"lar\u0131", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_2 = {make_shared<org::tartarus::snowball::Among>(
                               L"ni", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"nu", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"n\u00FC", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"n\u0131", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                               L"in", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"un", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"\u00FCn", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"\u0131n", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_4 = {make_shared<org::tartarus::snowball::Among>(
                               L"a", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"e", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_5 = {make_shared<org::tartarus::snowball::Among>(
                               L"na", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"ne", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_6 = {make_shared<org::tartarus::snowball::Among>(
                               L"da", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"ta", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"de", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"te", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_7 = {make_shared<org::tartarus::snowball::Among>(
                               L"nda", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"nde", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_8 = {make_shared<org::tartarus::snowball::Among>(
                               L"dan", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"tan", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"den", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"ten", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_9 = {make_shared<org::tartarus::snowball::Among>(
                               L"ndan", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"nden", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_10 = {make_shared<org::tartarus::snowball::Among>(
                                L"la", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"le", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_11 = {make_shared<org::tartarus::snowball::Among>(
                                L"ca", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"ce", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_12 = {make_shared<org::tartarus::snowball::Among>(
                                L"im", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"um", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"\u00FCm", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"\u0131m", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_13 = {make_shared<org::tartarus::snowball::Among>(
                                L"sin", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"sun", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"s\u00FCn", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"s\u0131n", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_14 = {make_shared<org::tartarus::snowball::Among>(
                                L"iz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"uz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"\u00FCz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"\u0131z", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_15 = {make_shared<org::tartarus::snowball::Among>(
                                L"siniz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"sunuz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"s\u00FCn\u00FCz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"s\u0131n\u0131z", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_16 = {make_shared<org::tartarus::snowball::Among>(
                                L"lar", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"ler", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_17 = {make_shared<org::tartarus::snowball::Among>(
                                L"niz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"nuz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"n\u00FCz", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"n\u0131z", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_18 = {
        make_shared<org::tartarus::snowball::Among>(L"dir", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tir", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dur", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tur", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u00FCr", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00FCr", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u0131r", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0131r", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_19 = {make_shared<org::tartarus::snowball::Among>(
                                L"cas\u0131na", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"cesine", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_20 = {
        make_shared<org::tartarus::snowball::Among>(L"di", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ti", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dik", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tik", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"duk", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tuk", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u00FCk", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00FCk", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u0131k", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0131k", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dim", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tim", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dum", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u00FCm", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00FCm", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u0131m", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0131m", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"din", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tin", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dun", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tun", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u00FCn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00FCn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u0131n", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0131n", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"du", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tu", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u00FC", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00FC", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u0131", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0131", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_21 = {
        make_shared<org::tartarus::snowball::Among>(L"sa", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"se", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sak", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sek", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sam", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sem", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"san", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sen", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_22 = {make_shared<org::tartarus::snowball::Among>(
                                L"mi\u015F", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"mu\u015F", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"m\u00FC\u015F", -1, -1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"m\u0131\u015F", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    TurkishStemmer::a_23 = {make_shared<org::tartarus::snowball::Among>(
                                L"b", -1, 1, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"c", -1, 2, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"d", -1, 3, L"", methodObject),
                            make_shared<org::tartarus::snowball::Among>(
                                L"\u011F", -1, 4, L"", methodObject)};
std::deque<wchar_t> const TurkishStemmer::g_vowel = {
    17, 65, 16, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0, 32, 8, 0, 0, 0, 0, 0, 0, 1};
std::deque<wchar_t> const TurkishStemmer::g_U = {1, 16, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0,  0, 0, 0, 0, 0, 0, 0,
                                                  8, 0,  0, 0, 0, 0, 0, 1};
std::deque<wchar_t> const TurkishStemmer::g_vowel1 = {
    1, 64, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
std::deque<wchar_t> const TurkishStemmer::g_vowel2 = {
    17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 130};
std::deque<wchar_t> const TurkishStemmer::g_vowel3 = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
std::deque<wchar_t> const TurkishStemmer::g_vowel4 = {17};
std::deque<wchar_t> const TurkishStemmer::g_vowel5 = {65};
std::deque<wchar_t> const TurkishStemmer::g_vowel6 = {65};

void TurkishStemmer::copy_from(shared_ptr<TurkishStemmer> other)
{
  B_continue_stemming_noun_suffixes = other->B_continue_stemming_noun_suffixes;
  I_strlen = other->I_strlen;
  SnowballProgram::copy_from(other);
}

bool TurkishStemmer::r_check_vowel_harmony()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  int v_10;
  int v_11;
  // (, line 111
  // test, line 112
  v_1 = limit - cursor;
  // (, line 113
  // (, line 114
  // goto, line 114
  while (true) {
    v_2 = limit - cursor;
    do {
      if (!(in_grouping_b(g_vowel, 97, 305))) {
        goto lab1Break;
      }
      cursor = limit - v_2;
      goto golab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
  golab0Continue:;
  }
golab0Break:
  // (, line 115
  // or, line 116
  do {
    v_3 = limit - cursor;
    do {
      // (, line 116
      // literal, line 116
      if (!(eq_s_b(1, L"a"))) {
        goto lab3Break;
      }
      // goto, line 116
      while (true) {
        v_4 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel1, 97, 305))) {
            goto lab5Break;
          }
          cursor = limit - v_4;
          goto golab4Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_4;
        if (cursor <= limit_backward) {
          goto lab3Break;
        }
        cursor--;
      golab4Continue:;
      }
    golab4Break:
      goto lab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = limit - v_3;
    do {
      // (, line 117
      // literal, line 117
      if (!(eq_s_b(1, L"e"))) {
        goto lab6Break;
      }
      // goto, line 117
      while (true) {
        v_5 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel2, 101, 252))) {
            goto lab8Break;
          }
          cursor = limit - v_5;
          goto golab7Break;
        } while (false);
      lab8Continue:;
      lab8Break:
        cursor = limit - v_5;
        if (cursor <= limit_backward) {
          goto lab6Break;
        }
        cursor--;
      golab7Continue:;
      }
    golab7Break:
      goto lab2Break;
    } while (false);
  lab6Continue:;
  lab6Break:
    cursor = limit - v_3;
    do {
      // (, line 118
      // literal, line 118
      if (!(eq_s_b(1, L"\u0131"))) {
        goto lab9Break;
      }
      // goto, line 118
      while (true) {
        v_6 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel3, 97, 305))) {
            goto lab11Break;
          }
          cursor = limit - v_6;
          goto golab10Break;
        } while (false);
      lab11Continue:;
      lab11Break:
        cursor = limit - v_6;
        if (cursor <= limit_backward) {
          goto lab9Break;
        }
        cursor--;
      golab10Continue:;
      }
    golab10Break:
      goto lab2Break;
    } while (false);
  lab9Continue:;
  lab9Break:
    cursor = limit - v_3;
    do {
      // (, line 119
      // literal, line 119
      if (!(eq_s_b(1, L"i"))) {
        goto lab12Break;
      }
      // goto, line 119
      while (true) {
        v_7 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel4, 101, 105))) {
            goto lab14Break;
          }
          cursor = limit - v_7;
          goto golab13Break;
        } while (false);
      lab14Continue:;
      lab14Break:
        cursor = limit - v_7;
        if (cursor <= limit_backward) {
          goto lab12Break;
        }
        cursor--;
      golab13Continue:;
      }
    golab13Break:
      goto lab2Break;
    } while (false);
  lab12Continue:;
  lab12Break:
    cursor = limit - v_3;
    do {
      // (, line 120
      // literal, line 120
      if (!(eq_s_b(1, L"o"))) {
        goto lab15Break;
      }
      // goto, line 120
      while (true) {
        v_8 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel5, 111, 117))) {
            goto lab17Break;
          }
          cursor = limit - v_8;
          goto golab16Break;
        } while (false);
      lab17Continue:;
      lab17Break:
        cursor = limit - v_8;
        if (cursor <= limit_backward) {
          goto lab15Break;
        }
        cursor--;
      golab16Continue:;
      }
    golab16Break:
      goto lab2Break;
    } while (false);
  lab15Continue:;
  lab15Break:
    cursor = limit - v_3;
    do {
      // (, line 121
      // literal, line 121
      if (!(eq_s_b(1, L"\u00F6"))) {
        goto lab18Break;
      }
      // goto, line 121
      while (true) {
        v_9 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel6, 246, 252))) {
            goto lab20Break;
          }
          cursor = limit - v_9;
          goto golab19Break;
        } while (false);
      lab20Continue:;
      lab20Break:
        cursor = limit - v_9;
        if (cursor <= limit_backward) {
          goto lab18Break;
        }
        cursor--;
      golab19Continue:;
      }
    golab19Break:
      goto lab2Break;
    } while (false);
  lab18Continue:;
  lab18Break:
    cursor = limit - v_3;
    do {
      // (, line 122
      // literal, line 122
      if (!(eq_s_b(1, L"u"))) {
        goto lab21Break;
      }
      // goto, line 122
      while (true) {
        v_10 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel5, 111, 117))) {
            goto lab23Break;
          }
          cursor = limit - v_10;
          goto golab22Break;
        } while (false);
      lab23Continue:;
      lab23Break:
        cursor = limit - v_10;
        if (cursor <= limit_backward) {
          goto lab21Break;
        }
        cursor--;
      golab22Continue:;
      }
    golab22Break:
      goto lab2Break;
    } while (false);
  lab21Continue:;
  lab21Break:
    cursor = limit - v_3;
    // (, line 123
    // literal, line 123
    if (!(eq_s_b(1, L"\u00FC"))) {
      return false;
    }
    // goto, line 123
    while (true) {
      v_11 = limit - cursor;
      do {
        if (!(in_grouping_b(g_vowel6, 246, 252))) {
          goto lab25Break;
        }
        cursor = limit - v_11;
        goto golab24Break;
      } while (false);
    lab25Continue:;
    lab25Break:
      cursor = limit - v_11;
      if (cursor <= limit_backward) {
        return false;
      }
      cursor--;
    golab24Continue:;
    }
  golab24Break:;
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_1;
  return true;
}

bool TurkishStemmer::r_mark_suffix_with_optional_n_consonant()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  // (, line 132
  // or, line 134
  do {
    v_1 = limit - cursor;
    do {
      // (, line 133
      // (, line 133
      // test, line 133
      v_2 = limit - cursor;
      // literal, line 133
      if (!(eq_s_b(1, L"n"))) {
        goto lab1Break;
      }
      cursor = limit - v_2;
      // next, line 133
      if (cursor <= limit_backward) {
        goto lab1Break;
      }
      cursor--;
      // (, line 133
      // test, line 133
      v_3 = limit - cursor;
      if (!(in_grouping_b(g_vowel, 97, 305))) {
        goto lab1Break;
      }
      cursor = limit - v_3;
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 135
    // (, line 135
    // not, line 135
    {
      v_4 = limit - cursor;
      do {
        // (, line 135
        // test, line 135
        v_5 = limit - cursor;
        // literal, line 135
        if (!(eq_s_b(1, L"n"))) {
          goto lab2Break;
        }
        cursor = limit - v_5;
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_4;
    }
    // test, line 135
    v_6 = limit - cursor;
    // (, line 135
    // next, line 135
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // (, line 135
    // test, line 135
    v_7 = limit - cursor;
    if (!(in_grouping_b(g_vowel, 97, 305))) {
      return false;
    }
    cursor = limit - v_7;
    cursor = limit - v_6;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_mark_suffix_with_optional_s_consonant()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  // (, line 143
  // or, line 145
  do {
    v_1 = limit - cursor;
    do {
      // (, line 144
      // (, line 144
      // test, line 144
      v_2 = limit - cursor;
      // literal, line 144
      if (!(eq_s_b(1, L"s"))) {
        goto lab1Break;
      }
      cursor = limit - v_2;
      // next, line 144
      if (cursor <= limit_backward) {
        goto lab1Break;
      }
      cursor--;
      // (, line 144
      // test, line 144
      v_3 = limit - cursor;
      if (!(in_grouping_b(g_vowel, 97, 305))) {
        goto lab1Break;
      }
      cursor = limit - v_3;
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 146
    // (, line 146
    // not, line 146
    {
      v_4 = limit - cursor;
      do {
        // (, line 146
        // test, line 146
        v_5 = limit - cursor;
        // literal, line 146
        if (!(eq_s_b(1, L"s"))) {
          goto lab2Break;
        }
        cursor = limit - v_5;
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_4;
    }
    // test, line 146
    v_6 = limit - cursor;
    // (, line 146
    // next, line 146
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // (, line 146
    // test, line 146
    v_7 = limit - cursor;
    if (!(in_grouping_b(g_vowel, 97, 305))) {
      return false;
    }
    cursor = limit - v_7;
    cursor = limit - v_6;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_mark_suffix_with_optional_y_consonant()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  // (, line 153
  // or, line 155
  do {
    v_1 = limit - cursor;
    do {
      // (, line 154
      // (, line 154
      // test, line 154
      v_2 = limit - cursor;
      // literal, line 154
      if (!(eq_s_b(1, L"y"))) {
        goto lab1Break;
      }
      cursor = limit - v_2;
      // next, line 154
      if (cursor <= limit_backward) {
        goto lab1Break;
      }
      cursor--;
      // (, line 154
      // test, line 154
      v_3 = limit - cursor;
      if (!(in_grouping_b(g_vowel, 97, 305))) {
        goto lab1Break;
      }
      cursor = limit - v_3;
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 156
    // (, line 156
    // not, line 156
    {
      v_4 = limit - cursor;
      do {
        // (, line 156
        // test, line 156
        v_5 = limit - cursor;
        // literal, line 156
        if (!(eq_s_b(1, L"y"))) {
          goto lab2Break;
        }
        cursor = limit - v_5;
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_4;
    }
    // test, line 156
    v_6 = limit - cursor;
    // (, line 156
    // next, line 156
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // (, line 156
    // test, line 156
    v_7 = limit - cursor;
    if (!(in_grouping_b(g_vowel, 97, 305))) {
      return false;
    }
    cursor = limit - v_7;
    cursor = limit - v_6;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_mark_suffix_with_optional_U_vowel()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  // (, line 159
  // or, line 161
  do {
    v_1 = limit - cursor;
    do {
      // (, line 160
      // (, line 160
      // test, line 160
      v_2 = limit - cursor;
      if (!(in_grouping_b(g_U, 105, 305))) {
        goto lab1Break;
      }
      cursor = limit - v_2;
      // next, line 160
      if (cursor <= limit_backward) {
        goto lab1Break;
      }
      cursor--;
      // (, line 160
      // test, line 160
      v_3 = limit - cursor;
      if (!(out_grouping_b(g_vowel, 97, 305))) {
        goto lab1Break;
      }
      cursor = limit - v_3;
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 162
    // (, line 162
    // not, line 162
    {
      v_4 = limit - cursor;
      do {
        // (, line 162
        // test, line 162
        v_5 = limit - cursor;
        if (!(in_grouping_b(g_U, 105, 305))) {
          goto lab2Break;
        }
        cursor = limit - v_5;
        return false;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_4;
    }
    // test, line 162
    v_6 = limit - cursor;
    // (, line 162
    // next, line 162
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // (, line 162
    // test, line 162
    v_7 = limit - cursor;
    if (!(out_grouping_b(g_vowel, 97, 305))) {
      return false;
    }
    cursor = limit - v_7;
    cursor = limit - v_6;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_mark_possessives()
{
  // (, line 166
  // among, line 167
  if (find_among_b(a_0, 10) == 0) {
    return false;
  }
  // (, line 169
  // call mark_suffix_with_optional_U_vowel, line 169
  if (!r_mark_suffix_with_optional_U_vowel()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_sU()
{
  // (, line 172
  // call check_vowel_harmony, line 173
  if (!r_check_vowel_harmony()) {
    return false;
  }
  if (!(in_grouping_b(g_U, 105, 305))) {
    return false;
  }
  // (, line 175
  // call mark_suffix_with_optional_s_consonant, line 175
  if (!r_mark_suffix_with_optional_s_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_lArI()
{
  // (, line 178
  // among, line 179
  if (find_among_b(a_1, 2) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_yU()
{
  // (, line 182
  // call check_vowel_harmony, line 183
  if (!r_check_vowel_harmony()) {
    return false;
  }
  if (!(in_grouping_b(g_U, 105, 305))) {
    return false;
  }
  // (, line 185
  // call mark_suffix_with_optional_y_consonant, line 185
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_nU()
{
  // (, line 188
  // call check_vowel_harmony, line 189
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 190
  if (find_among_b(a_2, 4) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_nUn()
{
  // (, line 193
  // call check_vowel_harmony, line 194
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 195
  if (find_among_b(a_3, 4) == 0) {
    return false;
  }
  // (, line 196
  // call mark_suffix_with_optional_n_consonant, line 196
  if (!r_mark_suffix_with_optional_n_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_yA()
{
  // (, line 199
  // call check_vowel_harmony, line 200
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 201
  if (find_among_b(a_4, 2) == 0) {
    return false;
  }
  // (, line 202
  // call mark_suffix_with_optional_y_consonant, line 202
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_nA()
{
  // (, line 205
  // call check_vowel_harmony, line 206
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 207
  if (find_among_b(a_5, 2) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_DA()
{
  // (, line 210
  // call check_vowel_harmony, line 211
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 212
  if (find_among_b(a_6, 4) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ndA()
{
  // (, line 215
  // call check_vowel_harmony, line 216
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 217
  if (find_among_b(a_7, 2) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_DAn()
{
  // (, line 220
  // call check_vowel_harmony, line 221
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 222
  if (find_among_b(a_8, 4) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ndAn()
{
  // (, line 225
  // call check_vowel_harmony, line 226
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 227
  if (find_among_b(a_9, 2) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ylA()
{
  // (, line 230
  // call check_vowel_harmony, line 231
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 232
  if (find_among_b(a_10, 2) == 0) {
    return false;
  }
  // (, line 233
  // call mark_suffix_with_optional_y_consonant, line 233
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ki()
{
  // (, line 236
  // literal, line 237
  if (!(eq_s_b(2, L"ki"))) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ncA()
{
  // (, line 240
  // call check_vowel_harmony, line 241
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 242
  if (find_among_b(a_11, 2) == 0) {
    return false;
  }
  // (, line 243
  // call mark_suffix_with_optional_n_consonant, line 243
  if (!r_mark_suffix_with_optional_n_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_yUm()
{
  // (, line 246
  // call check_vowel_harmony, line 247
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 248
  if (find_among_b(a_12, 4) == 0) {
    return false;
  }
  // (, line 249
  // call mark_suffix_with_optional_y_consonant, line 249
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_sUn()
{
  // (, line 252
  // call check_vowel_harmony, line 253
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 254
  if (find_among_b(a_13, 4) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_yUz()
{
  // (, line 257
  // call check_vowel_harmony, line 258
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 259
  if (find_among_b(a_14, 4) == 0) {
    return false;
  }
  // (, line 260
  // call mark_suffix_with_optional_y_consonant, line 260
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_sUnUz()
{
  // (, line 263
  // among, line 264
  if (find_among_b(a_15, 4) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_lAr()
{
  // (, line 267
  // call check_vowel_harmony, line 268
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 269
  if (find_among_b(a_16, 2) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_nUz()
{
  // (, line 272
  // call check_vowel_harmony, line 273
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 274
  if (find_among_b(a_17, 4) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_DUr()
{
  // (, line 277
  // call check_vowel_harmony, line 278
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 279
  if (find_among_b(a_18, 8) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_cAsInA()
{
  // (, line 282
  // among, line 283
  if (find_among_b(a_19, 2) == 0) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_yDU()
{
  // (, line 286
  // call check_vowel_harmony, line 287
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 288
  if (find_among_b(a_20, 32) == 0) {
    return false;
  }
  // (, line 292
  // call mark_suffix_with_optional_y_consonant, line 292
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ysA()
{
  // (, line 296
  // among, line 297
  if (find_among_b(a_21, 8) == 0) {
    return false;
  }
  // (, line 298
  // call mark_suffix_with_optional_y_consonant, line 298
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_ymUs_()
{
  // (, line 301
  // call check_vowel_harmony, line 302
  if (!r_check_vowel_harmony()) {
    return false;
  }
  // among, line 303
  if (find_among_b(a_22, 4) == 0) {
    return false;
  }
  // (, line 304
  // call mark_suffix_with_optional_y_consonant, line 304
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_mark_yken()
{
  // (, line 307
  // literal, line 308
  if (!(eq_s_b(3, L"ken"))) {
    return false;
  }
  // (, line 308
  // call mark_suffix_with_optional_y_consonant, line 308
  if (!r_mark_suffix_with_optional_y_consonant()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::r_stem_nominal_verb_suffixes()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  int v_10;
  // (, line 311
  // [, line 312
  ket = cursor;
  // set continue_stemming_noun_suffixes, line 313
  B_continue_stemming_noun_suffixes = true;
  // or, line 315
  do {
    v_1 = limit - cursor;
    do {
      // (, line 314
      // or, line 314
      do {
        v_2 = limit - cursor;
        do {
          // call mark_ymUs_, line 314
          if (!r_mark_ymUs_()) {
            goto lab3Break;
          }
          goto lab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = limit - v_2;
        do {
          // call mark_yDU, line 314
          if (!r_mark_yDU()) {
            goto lab4Break;
          }
          goto lab2Break;
        } while (false);
      lab4Continue:;
      lab4Break:
        cursor = limit - v_2;
        do {
          // call mark_ysA, line 314
          if (!r_mark_ysA()) {
            goto lab5Break;
          }
          goto lab2Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_2;
        // call mark_yken, line 314
        if (!r_mark_yken()) {
          goto lab1Break;
        }
      } while (false);
    lab2Continue:;
    lab2Break:
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    do {
      // (, line 316
      // call mark_cAsInA, line 316
      if (!r_mark_cAsInA()) {
        goto lab6Break;
      }
      // (, line 316
      // or, line 316
      do {
        v_3 = limit - cursor;
        do {
          // call mark_sUnUz, line 316
          if (!r_mark_sUnUz()) {
            goto lab8Break;
          }
          goto lab7Break;
        } while (false);
      lab8Continue:;
      lab8Break:
        cursor = limit - v_3;
        do {
          // call mark_lAr, line 316
          if (!r_mark_lAr()) {
            goto lab9Break;
          }
          goto lab7Break;
        } while (false);
      lab9Continue:;
      lab9Break:
        cursor = limit - v_3;
        do {
          // call mark_yUm, line 316
          if (!r_mark_yUm()) {
            goto lab10Break;
          }
          goto lab7Break;
        } while (false);
      lab10Continue:;
      lab10Break:
        cursor = limit - v_3;
        do {
          // call mark_sUn, line 316
          if (!r_mark_sUn()) {
            goto lab11Break;
          }
          goto lab7Break;
        } while (false);
      lab11Continue:;
      lab11Break:
        cursor = limit - v_3;
        do {
          // call mark_yUz, line 316
          if (!r_mark_yUz()) {
            goto lab12Break;
          }
          goto lab7Break;
        } while (false);
      lab12Continue:;
      lab12Break:
        cursor = limit - v_3;
      } while (false);
    lab7Continue:;
    lab7Break:
      // call mark_ymUs_, line 316
      if (!r_mark_ymUs_()) {
        goto lab6Break;
      }
      goto lab0Break;
    } while (false);
  lab6Continue:;
  lab6Break:
    cursor = limit - v_1;
    do {
      // (, line 318
      // call mark_lAr, line 319
      if (!r_mark_lAr()) {
        goto lab13Break;
      }
      // ], line 319
      bra = cursor;
      // delete, line 319
      slice_del();
      // try, line 319
      v_4 = limit - cursor;
      do {
        // (, line 319
        // [, line 319
        ket = cursor;
        // (, line 319
        // or, line 319
        do {
          v_5 = limit - cursor;
          do {
            // call mark_DUr, line 319
            if (!r_mark_DUr()) {
              goto lab16Break;
            }
            goto lab15Break;
          } while (false);
        lab16Continue:;
        lab16Break:
          cursor = limit - v_5;
          do {
            // call mark_yDU, line 319
            if (!r_mark_yDU()) {
              goto lab17Break;
            }
            goto lab15Break;
          } while (false);
        lab17Continue:;
        lab17Break:
          cursor = limit - v_5;
          do {
            // call mark_ysA, line 319
            if (!r_mark_ysA()) {
              goto lab18Break;
            }
            goto lab15Break;
          } while (false);
        lab18Continue:;
        lab18Break:
          cursor = limit - v_5;
          // call mark_ymUs_, line 319
          if (!r_mark_ymUs_()) {
            cursor = limit - v_4;
            goto lab14Break;
          }
        } while (false);
      lab15Continue:;
      lab15Break:;
      } while (false);
    lab14Continue:;
    lab14Break:
      // unset continue_stemming_noun_suffixes, line 320
      B_continue_stemming_noun_suffixes = false;
      goto lab0Break;
    } while (false);
  lab13Continue:;
  lab13Break:
    cursor = limit - v_1;
    do {
      // (, line 323
      // call mark_nUz, line 323
      if (!r_mark_nUz()) {
        goto lab19Break;
      }
      // (, line 323
      // or, line 323
      do {
        v_6 = limit - cursor;
        do {
          // call mark_yDU, line 323
          if (!r_mark_yDU()) {
            goto lab21Break;
          }
          goto lab20Break;
        } while (false);
      lab21Continue:;
      lab21Break:
        cursor = limit - v_6;
        // call mark_ysA, line 323
        if (!r_mark_ysA()) {
          goto lab19Break;
        }
      } while (false);
    lab20Continue:;
    lab20Break:
      goto lab0Break;
    } while (false);
  lab19Continue:;
  lab19Break:
    cursor = limit - v_1;
    do {
      // (, line 325
      // (, line 325
      // or, line 325
      do {
        v_7 = limit - cursor;
        do {
          // call mark_sUnUz, line 325
          if (!r_mark_sUnUz()) {
            goto lab24Break;
          }
          goto lab23Break;
        } while (false);
      lab24Continue:;
      lab24Break:
        cursor = limit - v_7;
        do {
          // call mark_yUz, line 325
          if (!r_mark_yUz()) {
            goto lab25Break;
          }
          goto lab23Break;
        } while (false);
      lab25Continue:;
      lab25Break:
        cursor = limit - v_7;
        do {
          // call mark_sUn, line 325
          if (!r_mark_sUn()) {
            goto lab26Break;
          }
          goto lab23Break;
        } while (false);
      lab26Continue:;
      lab26Break:
        cursor = limit - v_7;
        // call mark_yUm, line 325
        if (!r_mark_yUm()) {
          goto lab22Break;
        }
      } while (false);
    lab23Continue:;
    lab23Break:
      // ], line 325
      bra = cursor;
      // delete, line 325
      slice_del();
      // try, line 325
      v_8 = limit - cursor;
      do {
        // (, line 325
        // [, line 325
        ket = cursor;
        // call mark_ymUs_, line 325
        if (!r_mark_ymUs_()) {
          cursor = limit - v_8;
          goto lab27Break;
        }
      } while (false);
    lab27Continue:;
    lab27Break:
      goto lab0Break;
    } while (false);
  lab22Continue:;
  lab22Break:
    cursor = limit - v_1;
    // (, line 327
    // call mark_DUr, line 327
    if (!r_mark_DUr()) {
      return false;
    }
    // ], line 327
    bra = cursor;
    // delete, line 327
    slice_del();
    // try, line 327
    v_9 = limit - cursor;
    do {
      // (, line 327
      // [, line 327
      ket = cursor;
      // (, line 327
      // or, line 327
      do {
        v_10 = limit - cursor;
        do {
          // call mark_sUnUz, line 327
          if (!r_mark_sUnUz()) {
            goto lab30Break;
          }
          goto lab29Break;
        } while (false);
      lab30Continue:;
      lab30Break:
        cursor = limit - v_10;
        do {
          // call mark_lAr, line 327
          if (!r_mark_lAr()) {
            goto lab31Break;
          }
          goto lab29Break;
        } while (false);
      lab31Continue:;
      lab31Break:
        cursor = limit - v_10;
        do {
          // call mark_yUm, line 327
          if (!r_mark_yUm()) {
            goto lab32Break;
          }
          goto lab29Break;
        } while (false);
      lab32Continue:;
      lab32Break:
        cursor = limit - v_10;
        do {
          // call mark_sUn, line 327
          if (!r_mark_sUn()) {
            goto lab33Break;
          }
          goto lab29Break;
        } while (false);
      lab33Continue:;
      lab33Break:
        cursor = limit - v_10;
        do {
          // call mark_yUz, line 327
          if (!r_mark_yUz()) {
            goto lab34Break;
          }
          goto lab29Break;
        } while (false);
      lab34Continue:;
      lab34Break:
        cursor = limit - v_10;
      } while (false);
    lab29Continue:;
    lab29Break:
      // call mark_ymUs_, line 327
      if (!r_mark_ymUs_()) {
        cursor = limit - v_9;
        goto lab28Break;
      }
    } while (false);
  lab28Continue:;
  lab28Break:;
  } while (false);
lab0Continue:;
lab0Break:
  // ], line 328
  bra = cursor;
  // delete, line 328
  slice_del();
  return true;
}

bool TurkishStemmer::r_stem_suffix_chain_before_ki()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  int v_10;
  int v_11;
  // (, line 332
  // [, line 333
  ket = cursor;
  // call mark_ki, line 334
  if (!r_mark_ki()) {
    return false;
  }
  // (, line 335
  // or, line 342
  do {
    v_1 = limit - cursor;
    do {
      // (, line 336
      // call mark_DA, line 336
      if (!r_mark_DA()) {
        goto lab1Break;
      }
      // ], line 336
      bra = cursor;
      // delete, line 336
      slice_del();
      // try, line 336
      v_2 = limit - cursor;
      do {
        // (, line 336
        // [, line 336
        ket = cursor;
        // or, line 338
        do {
          v_3 = limit - cursor;
          do {
            // (, line 337
            // call mark_lAr, line 337
            if (!r_mark_lAr()) {
              goto lab4Break;
            }
            // ], line 337
            bra = cursor;
            // delete, line 337
            slice_del();
            // try, line 337
            v_4 = limit - cursor;
            do {
              // (, line 337
              // call stem_suffix_chain_before_ki, line 337
              if (!r_stem_suffix_chain_before_ki()) {
                cursor = limit - v_4;
                goto lab5Break;
              }
            } while (false);
          lab5Continue:;
          lab5Break:
            goto lab3Break;
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = limit - v_3;
          // (, line 339
          // call mark_possessives, line 339
          if (!r_mark_possessives()) {
            cursor = limit - v_2;
            goto lab2Break;
          }
          // ], line 339
          bra = cursor;
          // delete, line 339
          slice_del();
          // try, line 339
          v_5 = limit - cursor;
          do {
            // (, line 339
            // [, line 339
            ket = cursor;
            // call mark_lAr, line 339
            if (!r_mark_lAr()) {
              cursor = limit - v_5;
              goto lab6Break;
            }
            // ], line 339
            bra = cursor;
            // delete, line 339
            slice_del();
            // call stem_suffix_chain_before_ki, line 339
            if (!r_stem_suffix_chain_before_ki()) {
              cursor = limit - v_5;
              goto lab6Break;
            }
          } while (false);
        lab6Continue:;
        lab6Break:;
        } while (false);
      lab3Continue:;
      lab3Break:;
      } while (false);
    lab2Continue:;
    lab2Break:
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    do {
      // (, line 343
      // call mark_nUn, line 343
      if (!r_mark_nUn()) {
        goto lab7Break;
      }
      // ], line 343
      bra = cursor;
      // delete, line 343
      slice_del();
      // try, line 343
      v_6 = limit - cursor;
      do {
        // (, line 343
        // [, line 343
        ket = cursor;
        // or, line 345
        do {
          v_7 = limit - cursor;
          do {
            // (, line 344
            // call mark_lArI, line 344
            if (!r_mark_lArI()) {
              goto lab10Break;
            }
            // ], line 344
            bra = cursor;
            // delete, line 344
            slice_del();
            goto lab9Break;
          } while (false);
        lab10Continue:;
        lab10Break:
          cursor = limit - v_7;
          do {
            // (, line 346
            // [, line 346
            ket = cursor;
            // or, line 346
            do {
              v_8 = limit - cursor;
              do {
                // call mark_possessives, line 346
                if (!r_mark_possessives()) {
                  goto lab13Break;
                }
                goto lab12Break;
              } while (false);
            lab13Continue:;
            lab13Break:
              cursor = limit - v_8;
              // call mark_sU, line 346
              if (!r_mark_sU()) {
                goto lab11Break;
              }
            } while (false);
          lab12Continue:;
          lab12Break:
            // ], line 346
            bra = cursor;
            // delete, line 346
            slice_del();
            // try, line 346
            v_9 = limit - cursor;
            do {
              // (, line 346
              // [, line 346
              ket = cursor;
              // call mark_lAr, line 346
              if (!r_mark_lAr()) {
                cursor = limit - v_9;
                goto lab14Break;
              }
              // ], line 346
              bra = cursor;
              // delete, line 346
              slice_del();
              // call stem_suffix_chain_before_ki, line 346
              if (!r_stem_suffix_chain_before_ki()) {
                cursor = limit - v_9;
                goto lab14Break;
              }
            } while (false);
          lab14Continue:;
          lab14Break:
            goto lab9Break;
          } while (false);
        lab11Continue:;
        lab11Break:
          cursor = limit - v_7;
          // (, line 348
          // call stem_suffix_chain_before_ki, line 348
          if (!r_stem_suffix_chain_before_ki()) {
            cursor = limit - v_6;
            goto lab8Break;
          }
        } while (false);
      lab9Continue:;
      lab9Break:;
      } while (false);
    lab8Continue:;
    lab8Break:
      goto lab0Break;
    } while (false);
  lab7Continue:;
  lab7Break:
    cursor = limit - v_1;
    // (, line 351
    // call mark_ndA, line 351
    if (!r_mark_ndA()) {
      return false;
    }
    // (, line 351
    // or, line 353
    do {
      v_10 = limit - cursor;
      do {
        // (, line 352
        // call mark_lArI, line 352
        if (!r_mark_lArI()) {
          goto lab16Break;
        }
        // ], line 352
        bra = cursor;
        // delete, line 352
        slice_del();
        goto lab15Break;
      } while (false);
    lab16Continue:;
    lab16Break:
      cursor = limit - v_10;
      do {
        // (, line 354
        // (, line 354
        // call mark_sU, line 354
        if (!r_mark_sU()) {
          goto lab17Break;
        }
        // ], line 354
        bra = cursor;
        // delete, line 354
        slice_del();
        // try, line 354
        v_11 = limit - cursor;
        do {
          // (, line 354
          // [, line 354
          ket = cursor;
          // call mark_lAr, line 354
          if (!r_mark_lAr()) {
            cursor = limit - v_11;
            goto lab18Break;
          }
          // ], line 354
          bra = cursor;
          // delete, line 354
          slice_del();
          // call stem_suffix_chain_before_ki, line 354
          if (!r_stem_suffix_chain_before_ki()) {
            cursor = limit - v_11;
            goto lab18Break;
          }
        } while (false);
      lab18Continue:;
      lab18Break:
        goto lab15Break;
      } while (false);
    lab17Continue:;
    lab17Break:
      cursor = limit - v_10;
      // (, line 356
      // call stem_suffix_chain_before_ki, line 356
      if (!r_stem_suffix_chain_before_ki()) {
        return false;
      }
    } while (false);
  lab15Continue:;
  lab15Break:;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_stem_noun_suffixes()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  int v_10;
  int v_11;
  int v_12;
  int v_13;
  int v_14;
  int v_15;
  int v_16;
  int v_17;
  int v_18;
  int v_19;
  int v_20;
  int v_21;
  int v_22;
  int v_23;
  int v_24;
  int v_25;
  int v_26;
  int v_27;
  // (, line 361
  // or, line 363
  do {
    v_1 = limit - cursor;
    do {
      // (, line 362
      // [, line 362
      ket = cursor;
      // call mark_lAr, line 362
      if (!r_mark_lAr()) {
        goto lab1Break;
      }
      // ], line 362
      bra = cursor;
      // delete, line 362
      slice_del();
      // try, line 362
      v_2 = limit - cursor;
      do {
        // (, line 362
        // call stem_suffix_chain_before_ki, line 362
        if (!r_stem_suffix_chain_before_ki()) {
          cursor = limit - v_2;
          goto lab2Break;
        }
      } while (false);
    lab2Continue:;
    lab2Break:
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    do {
      // (, line 364
      // [, line 364
      ket = cursor;
      // call mark_ncA, line 364
      if (!r_mark_ncA()) {
        goto lab3Break;
      }
      // ], line 364
      bra = cursor;
      // delete, line 364
      slice_del();
      // try, line 365
      v_3 = limit - cursor;
      do {
        // (, line 365
        // or, line 367
        do {
          v_4 = limit - cursor;
          do {
            // (, line 366
            // [, line 366
            ket = cursor;
            // call mark_lArI, line 366
            if (!r_mark_lArI()) {
              goto lab6Break;
            }
            // ], line 366
            bra = cursor;
            // delete, line 366
            slice_del();
            goto lab5Break;
          } while (false);
        lab6Continue:;
        lab6Break:
          cursor = limit - v_4;
          do {
            // (, line 368
            // [, line 368
            ket = cursor;
            // or, line 368
            do {
              v_5 = limit - cursor;
              do {
                // call mark_possessives, line 368
                if (!r_mark_possessives()) {
                  goto lab9Break;
                }
                goto lab8Break;
              } while (false);
            lab9Continue:;
            lab9Break:
              cursor = limit - v_5;
              // call mark_sU, line 368
              if (!r_mark_sU()) {
                goto lab7Break;
              }
            } while (false);
          lab8Continue:;
          lab8Break:
            // ], line 368
            bra = cursor;
            // delete, line 368
            slice_del();
            // try, line 368
            v_6 = limit - cursor;
            do {
              // (, line 368
              // [, line 368
              ket = cursor;
              // call mark_lAr, line 368
              if (!r_mark_lAr()) {
                cursor = limit - v_6;
                goto lab10Break;
              }
              // ], line 368
              bra = cursor;
              // delete, line 368
              slice_del();
              // call stem_suffix_chain_before_ki, line 368
              if (!r_stem_suffix_chain_before_ki()) {
                cursor = limit - v_6;
                goto lab10Break;
              }
            } while (false);
          lab10Continue:;
          lab10Break:
            goto lab5Break;
          } while (false);
        lab7Continue:;
        lab7Break:
          cursor = limit - v_4;
          // (, line 370
          // [, line 370
          ket = cursor;
          // call mark_lAr, line 370
          if (!r_mark_lAr()) {
            cursor = limit - v_3;
            goto lab4Break;
          }
          // ], line 370
          bra = cursor;
          // delete, line 370
          slice_del();
          // call stem_suffix_chain_before_ki, line 370
          if (!r_stem_suffix_chain_before_ki()) {
            cursor = limit - v_3;
            goto lab4Break;
          }
        } while (false);
      lab5Continue:;
      lab5Break:;
      } while (false);
    lab4Continue:;
    lab4Break:
      goto lab0Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = limit - v_1;
    do {
      // (, line 374
      // [, line 374
      ket = cursor;
      // (, line 374
      // or, line 374
      do {
        v_7 = limit - cursor;
        do {
          // call mark_ndA, line 374
          if (!r_mark_ndA()) {
            goto lab13Break;
          }
          goto lab12Break;
        } while (false);
      lab13Continue:;
      lab13Break:
        cursor = limit - v_7;
        // call mark_nA, line 374
        if (!r_mark_nA()) {
          goto lab11Break;
        }
      } while (false);
    lab12Continue:;
    lab12Break:
      // (, line 375
      // or, line 377
      do {
        v_8 = limit - cursor;
        do {
          // (, line 376
          // call mark_lArI, line 376
          if (!r_mark_lArI()) {
            goto lab15Break;
          }
          // ], line 376
          bra = cursor;
          // delete, line 376
          slice_del();
          goto lab14Break;
        } while (false);
      lab15Continue:;
      lab15Break:
        cursor = limit - v_8;
        do {
          // (, line 378
          // call mark_sU, line 378
          if (!r_mark_sU()) {
            goto lab16Break;
          }
          // ], line 378
          bra = cursor;
          // delete, line 378
          slice_del();
          // try, line 378
          v_9 = limit - cursor;
          do {
            // (, line 378
            // [, line 378
            ket = cursor;
            // call mark_lAr, line 378
            if (!r_mark_lAr()) {
              cursor = limit - v_9;
              goto lab17Break;
            }
            // ], line 378
            bra = cursor;
            // delete, line 378
            slice_del();
            // call stem_suffix_chain_before_ki, line 378
            if (!r_stem_suffix_chain_before_ki()) {
              cursor = limit - v_9;
              goto lab17Break;
            }
          } while (false);
        lab17Continue:;
        lab17Break:
          goto lab14Break;
        } while (false);
      lab16Continue:;
      lab16Break:
        cursor = limit - v_8;
        // (, line 380
        // call stem_suffix_chain_before_ki, line 380
        if (!r_stem_suffix_chain_before_ki()) {
          goto lab11Break;
        }
      } while (false);
    lab14Continue:;
    lab14Break:
      goto lab0Break;
    } while (false);
  lab11Continue:;
  lab11Break:
    cursor = limit - v_1;
    do {
      // (, line 384
      // [, line 384
      ket = cursor;
      // (, line 384
      // or, line 384
      do {
        v_10 = limit - cursor;
        do {
          // call mark_ndAn, line 384
          if (!r_mark_ndAn()) {
            goto lab20Break;
          }
          goto lab19Break;
        } while (false);
      lab20Continue:;
      lab20Break:
        cursor = limit - v_10;
        // call mark_nU, line 384
        if (!r_mark_nU()) {
          goto lab18Break;
        }
      } while (false);
    lab19Continue:;
    lab19Break:
      // (, line 384
      // or, line 384
      do {
        v_11 = limit - cursor;
        do {
          // (, line 384
          // call mark_sU, line 384
          if (!r_mark_sU()) {
            goto lab22Break;
          }
          // ], line 384
          bra = cursor;
          // delete, line 384
          slice_del();
          // try, line 384
          v_12 = limit - cursor;
          do {
            // (, line 384
            // [, line 384
            ket = cursor;
            // call mark_lAr, line 384
            if (!r_mark_lAr()) {
              cursor = limit - v_12;
              goto lab23Break;
            }
            // ], line 384
            bra = cursor;
            // delete, line 384
            slice_del();
            // call stem_suffix_chain_before_ki, line 384
            if (!r_stem_suffix_chain_before_ki()) {
              cursor = limit - v_12;
              goto lab23Break;
            }
          } while (false);
        lab23Continue:;
        lab23Break:
          goto lab21Break;
        } while (false);
      lab22Continue:;
      lab22Break:
        cursor = limit - v_11;
        // (, line 384
        // call mark_lArI, line 384
        if (!r_mark_lArI()) {
          goto lab18Break;
        }
      } while (false);
    lab21Continue:;
    lab21Break:
      goto lab0Break;
    } while (false);
  lab18Continue:;
  lab18Break:
    cursor = limit - v_1;
    do {
      // (, line 386
      // [, line 386
      ket = cursor;
      // call mark_DAn, line 386
      if (!r_mark_DAn()) {
        goto lab24Break;
      }
      // ], line 386
      bra = cursor;
      // delete, line 386
      slice_del();
      // try, line 386
      v_13 = limit - cursor;
      do {
        // (, line 386
        // [, line 386
        ket = cursor;
        // (, line 387
        // or, line 389
        do {
          v_14 = limit - cursor;
          do {
            // (, line 388
            // call mark_possessives, line 388
            if (!r_mark_possessives()) {
              goto lab27Break;
            }
            // ], line 388
            bra = cursor;
            // delete, line 388
            slice_del();
            // try, line 388
            v_15 = limit - cursor;
            do {
              // (, line 388
              // [, line 388
              ket = cursor;
              // call mark_lAr, line 388
              if (!r_mark_lAr()) {
                cursor = limit - v_15;
                goto lab28Break;
              }
              // ], line 388
              bra = cursor;
              // delete, line 388
              slice_del();
              // call stem_suffix_chain_before_ki, line 388
              if (!r_stem_suffix_chain_before_ki()) {
                cursor = limit - v_15;
                goto lab28Break;
              }
            } while (false);
          lab28Continue:;
          lab28Break:
            goto lab26Break;
          } while (false);
        lab27Continue:;
        lab27Break:
          cursor = limit - v_14;
          do {
            // (, line 390
            // call mark_lAr, line 390
            if (!r_mark_lAr()) {
              goto lab29Break;
            }
            // ], line 390
            bra = cursor;
            // delete, line 390
            slice_del();
            // try, line 390
            v_16 = limit - cursor;
            do {
              // (, line 390
              // call stem_suffix_chain_before_ki, line 390
              if (!r_stem_suffix_chain_before_ki()) {
                cursor = limit - v_16;
                goto lab30Break;
              }
            } while (false);
          lab30Continue:;
          lab30Break:
            goto lab26Break;
          } while (false);
        lab29Continue:;
        lab29Break:
          cursor = limit - v_14;
          // (, line 392
          // call stem_suffix_chain_before_ki, line 392
          if (!r_stem_suffix_chain_before_ki()) {
            cursor = limit - v_13;
            goto lab25Break;
          }
        } while (false);
      lab26Continue:;
      lab26Break:;
      } while (false);
    lab25Continue:;
    lab25Break:
      goto lab0Break;
    } while (false);
  lab24Continue:;
  lab24Break:
    cursor = limit - v_1;
    do {
      // (, line 396
      // [, line 396
      ket = cursor;
      // or, line 396
      do {
        v_17 = limit - cursor;
        do {
          // call mark_nUn, line 396
          if (!r_mark_nUn()) {
            goto lab33Break;
          }
          goto lab32Break;
        } while (false);
      lab33Continue:;
      lab33Break:
        cursor = limit - v_17;
        // call mark_ylA, line 396
        if (!r_mark_ylA()) {
          goto lab31Break;
        }
      } while (false);
    lab32Continue:;
    lab32Break:
      // ], line 396
      bra = cursor;
      // delete, line 396
      slice_del();
      // try, line 397
      v_18 = limit - cursor;
      do {
        // (, line 397
        // or, line 399
        do {
          v_19 = limit - cursor;
          do {
            // (, line 398
            // [, line 398
            ket = cursor;
            // call mark_lAr, line 398
            if (!r_mark_lAr()) {
              goto lab36Break;
            }
            // ], line 398
            bra = cursor;
            // delete, line 398
            slice_del();
            // call stem_suffix_chain_before_ki, line 398
            if (!r_stem_suffix_chain_before_ki()) {
              goto lab36Break;
            }
            goto lab35Break;
          } while (false);
        lab36Continue:;
        lab36Break:
          cursor = limit - v_19;
          do {
            // (, line 400
            // [, line 400
            ket = cursor;
            // or, line 400
            do {
              v_20 = limit - cursor;
              do {
                // call mark_possessives, line 400
                if (!r_mark_possessives()) {
                  goto lab39Break;
                }
                goto lab38Break;
              } while (false);
            lab39Continue:;
            lab39Break:
              cursor = limit - v_20;
              // call mark_sU, line 400
              if (!r_mark_sU()) {
                goto lab37Break;
              }
            } while (false);
          lab38Continue:;
          lab38Break:
            // ], line 400
            bra = cursor;
            // delete, line 400
            slice_del();
            // try, line 400
            v_21 = limit - cursor;
            do {
              // (, line 400
              // [, line 400
              ket = cursor;
              // call mark_lAr, line 400
              if (!r_mark_lAr()) {
                cursor = limit - v_21;
                goto lab40Break;
              }
              // ], line 400
              bra = cursor;
              // delete, line 400
              slice_del();
              // call stem_suffix_chain_before_ki, line 400
              if (!r_stem_suffix_chain_before_ki()) {
                cursor = limit - v_21;
                goto lab40Break;
              }
            } while (false);
          lab40Continue:;
          lab40Break:
            goto lab35Break;
          } while (false);
        lab37Continue:;
        lab37Break:
          cursor = limit - v_19;
          // call stem_suffix_chain_before_ki, line 402
          if (!r_stem_suffix_chain_before_ki()) {
            cursor = limit - v_18;
            goto lab34Break;
          }
        } while (false);
      lab35Continue:;
      lab35Break:;
      } while (false);
    lab34Continue:;
    lab34Break:
      goto lab0Break;
    } while (false);
  lab31Continue:;
  lab31Break:
    cursor = limit - v_1;
    do {
      // (, line 406
      // [, line 406
      ket = cursor;
      // call mark_lArI, line 406
      if (!r_mark_lArI()) {
        goto lab41Break;
      }
      // ], line 406
      bra = cursor;
      // delete, line 406
      slice_del();
      goto lab0Break;
    } while (false);
  lab41Continue:;
  lab41Break:
    cursor = limit - v_1;
    do {
      // (, line 408
      // call stem_suffix_chain_before_ki, line 408
      if (!r_stem_suffix_chain_before_ki()) {
        goto lab42Break;
      }
      goto lab0Break;
    } while (false);
  lab42Continue:;
  lab42Break:
    cursor = limit - v_1;
    do {
      // (, line 410
      // [, line 410
      ket = cursor;
      // or, line 410
      do {
        v_22 = limit - cursor;
        do {
          // call mark_DA, line 410
          if (!r_mark_DA()) {
            goto lab45Break;
          }
          goto lab44Break;
        } while (false);
      lab45Continue:;
      lab45Break:
        cursor = limit - v_22;
        do {
          // call mark_yU, line 410
          if (!r_mark_yU()) {
            goto lab46Break;
          }
          goto lab44Break;
        } while (false);
      lab46Continue:;
      lab46Break:
        cursor = limit - v_22;
        // call mark_yA, line 410
        if (!r_mark_yA()) {
          goto lab43Break;
        }
      } while (false);
    lab44Continue:;
    lab44Break:
      // ], line 410
      bra = cursor;
      // delete, line 410
      slice_del();
      // try, line 410
      v_23 = limit - cursor;
      do {
        // (, line 410
        // [, line 410
        ket = cursor;
        // (, line 410
        // or, line 410
        do {
          v_24 = limit - cursor;
          do {
            // (, line 410
            // call mark_possessives, line 410
            if (!r_mark_possessives()) {
              goto lab49Break;
            }
            // ], line 410
            bra = cursor;
            // delete, line 410
            slice_del();
            // try, line 410
            v_25 = limit - cursor;
            do {
              // (, line 410
              // [, line 410
              ket = cursor;
              // call mark_lAr, line 410
              if (!r_mark_lAr()) {
                cursor = limit - v_25;
                goto lab50Break;
              }
            } while (false);
          lab50Continue:;
          lab50Break:
            goto lab48Break;
          } while (false);
        lab49Continue:;
        lab49Break:
          cursor = limit - v_24;
          // call mark_lAr, line 410
          if (!r_mark_lAr()) {
            cursor = limit - v_23;
            goto lab47Break;
          }
        } while (false);
      lab48Continue:;
      lab48Break:
        // ], line 410
        bra = cursor;
        // delete, line 410
        slice_del();
        // [, line 410
        ket = cursor;
        // call stem_suffix_chain_before_ki, line 410
        if (!r_stem_suffix_chain_before_ki()) {
          cursor = limit - v_23;
          goto lab47Break;
        }
      } while (false);
    lab47Continue:;
    lab47Break:
      goto lab0Break;
    } while (false);
  lab43Continue:;
  lab43Break:
    cursor = limit - v_1;
    // (, line 412
    // [, line 412
    ket = cursor;
    // or, line 412
    do {
      v_26 = limit - cursor;
      do {
        // call mark_possessives, line 412
        if (!r_mark_possessives()) {
          goto lab52Break;
        }
        goto lab51Break;
      } while (false);
    lab52Continue:;
    lab52Break:
      cursor = limit - v_26;
      // call mark_sU, line 412
      if (!r_mark_sU()) {
        return false;
      }
    } while (false);
  lab51Continue:;
  lab51Break:
    // ], line 412
    bra = cursor;
    // delete, line 412
    slice_del();
    // try, line 412
    v_27 = limit - cursor;
    do {
      // (, line 412
      // [, line 412
      ket = cursor;
      // call mark_lAr, line 412
      if (!r_mark_lAr()) {
        cursor = limit - v_27;
        goto lab53Break;
      }
      // ], line 412
      bra = cursor;
      // delete, line 412
      slice_del();
      // call stem_suffix_chain_before_ki, line 412
      if (!r_stem_suffix_chain_before_ki()) {
        cursor = limit - v_27;
        goto lab53Break;
      }
    } while (false);
  lab53Continue:;
  lab53Break:;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_post_process_last_consonants()
{
  int among_var;
  // (, line 415
  // [, line 416
  ket = cursor;
  // substring, line 416
  among_var = find_among_b(a_23, 4);
  if (among_var == 0) {
    return false;
  }
  // ], line 416
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 417
    // <-, line 417
    slice_from(L"p");
    break;
  case 2:
    // (, line 418
    // <-, line 418
    slice_from(L"\u00E7");
    break;
  case 3:
    // (, line 419
    // <-, line 419
    slice_from(L"t");
    break;
  case 4:
    // (, line 420
    // <-, line 420
    slice_from(L"k");
    break;
  }
  return true;
}

bool TurkishStemmer::r_append_U_to_stems_ending_with_d_or_g()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  int v_10;
  int v_11;
  int v_12;
  int v_13;
  int v_14;
  int v_15;
  // (, line 430
  // test, line 431
  v_1 = limit - cursor;
  // (, line 431
  // or, line 431
  do {
    v_2 = limit - cursor;
    do {
      // literal, line 431
      if (!(eq_s_b(1, L"d"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // literal, line 431
    if (!(eq_s_b(1, L"g"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  // or, line 433
  do {
    v_3 = limit - cursor;
    do {
      // (, line 432
      // test, line 432
      v_4 = limit - cursor;
      // (, line 432
      // (, line 432
      // goto, line 432
      while (true) {
        v_5 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel, 97, 305))) {
            goto lab5Break;
          }
          cursor = limit - v_5;
          goto golab4Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_5;
        if (cursor <= limit_backward) {
          goto lab3Break;
        }
        cursor--;
      golab4Continue:;
      }
    golab4Break:
      // or, line 432
      do {
        v_6 = limit - cursor;
        do {
          // literal, line 432
          if (!(eq_s_b(1, L"a"))) {
            goto lab7Break;
          }
          goto lab6Break;
        } while (false);
      lab7Continue:;
      lab7Break:
        cursor = limit - v_6;
        // literal, line 432
        if (!(eq_s_b(1, L"\u0131"))) {
          goto lab3Break;
        }
      } while (false);
    lab6Continue:;
    lab6Break:
      cursor = limit - v_4;
      // <+, line 432
      {
        int c = cursor;
        insert(cursor, cursor, L"\u0131");
        cursor = c;
      }
      goto lab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = limit - v_3;
    do {
      // (, line 434
      // test, line 434
      v_7 = limit - cursor;
      // (, line 434
      // (, line 434
      // goto, line 434
      while (true) {
        v_8 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel, 97, 305))) {
            goto lab10Break;
          }
          cursor = limit - v_8;
          goto golab9Break;
        } while (false);
      lab10Continue:;
      lab10Break:
        cursor = limit - v_8;
        if (cursor <= limit_backward) {
          goto lab8Break;
        }
        cursor--;
      golab9Continue:;
      }
    golab9Break:
      // or, line 434
      do {
        v_9 = limit - cursor;
        do {
          // literal, line 434
          if (!(eq_s_b(1, L"e"))) {
            goto lab12Break;
          }
          goto lab11Break;
        } while (false);
      lab12Continue:;
      lab12Break:
        cursor = limit - v_9;
        // literal, line 434
        if (!(eq_s_b(1, L"i"))) {
          goto lab8Break;
        }
      } while (false);
    lab11Continue:;
    lab11Break:
      cursor = limit - v_7;
      // <+, line 434
      {
        int c = cursor;
        insert(cursor, cursor, L"i");
        cursor = c;
      }
      goto lab2Break;
    } while (false);
  lab8Continue:;
  lab8Break:
    cursor = limit - v_3;
    do {
      // (, line 436
      // test, line 436
      v_10 = limit - cursor;
      // (, line 436
      // (, line 436
      // goto, line 436
      while (true) {
        v_11 = limit - cursor;
        do {
          if (!(in_grouping_b(g_vowel, 97, 305))) {
            goto lab15Break;
          }
          cursor = limit - v_11;
          goto golab14Break;
        } while (false);
      lab15Continue:;
      lab15Break:
        cursor = limit - v_11;
        if (cursor <= limit_backward) {
          goto lab13Break;
        }
        cursor--;
      golab14Continue:;
      }
    golab14Break:
      // or, line 436
      do {
        v_12 = limit - cursor;
        do {
          // literal, line 436
          if (!(eq_s_b(1, L"o"))) {
            goto lab17Break;
          }
          goto lab16Break;
        } while (false);
      lab17Continue:;
      lab17Break:
        cursor = limit - v_12;
        // literal, line 436
        if (!(eq_s_b(1, L"u"))) {
          goto lab13Break;
        }
      } while (false);
    lab16Continue:;
    lab16Break:
      cursor = limit - v_10;
      // <+, line 436
      {
        int c = cursor;
        insert(cursor, cursor, L"u");
        cursor = c;
      }
      goto lab2Break;
    } while (false);
  lab13Continue:;
  lab13Break:
    cursor = limit - v_3;
    // (, line 438
    // test, line 438
    v_13 = limit - cursor;
    // (, line 438
    // (, line 438
    // goto, line 438
    while (true) {
      v_14 = limit - cursor;
      do {
        if (!(in_grouping_b(g_vowel, 97, 305))) {
          goto lab19Break;
        }
        cursor = limit - v_14;
        goto golab18Break;
      } while (false);
    lab19Continue:;
    lab19Break:
      cursor = limit - v_14;
      if (cursor <= limit_backward) {
        return false;
      }
      cursor--;
    golab18Continue:;
    }
  golab18Break:
    // or, line 438
    do {
      v_15 = limit - cursor;
      do {
        // literal, line 438
        if (!(eq_s_b(1, L"\u00F6"))) {
          goto lab21Break;
        }
        goto lab20Break;
      } while (false);
    lab21Continue:;
    lab21Break:
      cursor = limit - v_15;
      // literal, line 438
      if (!(eq_s_b(1, L"\u00FC"))) {
        return false;
      }
    } while (false);
  lab20Continue:;
  lab20Break:
    cursor = limit - v_13;
    // <+, line 438
    {
      int c = cursor;
      insert(cursor, cursor, L"\u00FC");
      cursor = c;
    }
  } while (false);
lab2Continue:;
lab2Break:
  return true;
}

bool TurkishStemmer::r_more_than_one_syllable_word()
{
  int v_1;
  int v_3;
  // (, line 445
  // test, line 446
  v_1 = cursor;
  // (, line 446
  // atleast, line 446
  {
    int v_2 = 2;
    // atleast, line 446
    while (true) {
      v_3 = cursor;
      do {
        // (, line 446
        // gopast, line 446
        while (true) {
          do {
            if (!(in_grouping(g_vowel, 97, 305))) {
              goto lab3Break;
            }
            goto golab2Break;
          } while (false);
        lab3Continue:;
        lab3Break:
          if (cursor >= limit) {
            goto lab1Break;
          }
          cursor++;
        golab2Continue:;
        }
      golab2Break:
        v_2--;
        goto replab0Continue;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = v_3;
      goto replab0Break;
    replab0Continue:;
    }
  replab0Break:
    if (v_2 > 0) {
      return false;
    }
  }
  cursor = v_1;
  return true;
}

bool TurkishStemmer::r_is_reserved_word()
{
  int v_1;
  int v_2;
  int v_4;
  // (, line 449
  // or, line 451
  do {
    v_1 = cursor;
    do {
      // test, line 450
      v_2 = cursor;
      // (, line 450
      // gopast, line 450
      while (true) {
        do {
          // literal, line 450
          if (!(eq_s(2, L"ad"))) {
            goto lab3Break;
          }
          goto golab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
      golab2Continue:;
      }
    golab2Break:
      // (, line 450
      I_strlen = 2;
      // (, line 450
      if (!(I_strlen == limit)) {
        goto lab1Break;
      }
      cursor = v_2;
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    // test, line 452
    v_4 = cursor;
    // (, line 452
    // gopast, line 452
    while (true) {
      do {
        // literal, line 452
        if (!(eq_s(5, L"soyad"))) {
          goto lab5Break;
        }
        goto golab4Break;
      } while (false);
    lab5Continue:;
    lab5Break:
      if (cursor >= limit) {
        return false;
      }
      cursor++;
    golab4Continue:;
    }
  golab4Break:
    // (, line 452
    I_strlen = 5;
    // (, line 452
    if (!(I_strlen == limit)) {
      return false;
    }
    cursor = v_4;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool TurkishStemmer::r_postlude()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 455
  // not, line 456
  {
    v_1 = cursor;
    do {
      // (, line 456
      // call is_reserved_word, line 456
      if (!r_is_reserved_word()) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = v_1;
  }
  // backwards, line 457
  limit_backward = cursor;
  cursor = limit;
  // (, line 457
  // do, line 458
  v_2 = limit - cursor;
  do {
    // call append_U_to_stems_ending_with_d_or_g, line 458
    if (!r_append_U_to_stems_ending_with_d_or_g()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 459
  v_3 = limit - cursor;
  do {
    // call post_process_last_consonants, line 459
    if (!r_post_process_last_consonants()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  cursor = limit_backward;
  return true;
}

bool TurkishStemmer::stem()
{
  int v_1;
  int v_2;
  // (, line 464
  // (, line 465
  // call more_than_one_syllable_word, line 465
  if (!r_more_than_one_syllable_word()) {
    return false;
  }
  // (, line 466
  // backwards, line 467
  limit_backward = cursor;
  cursor = limit;
  // (, line 467
  // do, line 468
  v_1 = limit - cursor;
  do {
    // call stem_nominal_verb_suffixes, line 468
    if (!r_stem_nominal_verb_suffixes()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  // Boolean test continue_stemming_noun_suffixes, line 469
  if (!(B_continue_stemming_noun_suffixes)) {
    return false;
  }
  // do, line 470
  v_2 = limit - cursor;
  do {
    // call stem_noun_suffixes, line 470
    if (!r_stem_noun_suffixes()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  cursor = limit_backward; // call postlude, line 473
  if (!r_postlude()) {
    return false;
  }
  return true;
}

bool TurkishStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<TurkishStemmer>(o) != nullptr;
}

int TurkishStemmer::hashCode()
{
  return TurkishStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext