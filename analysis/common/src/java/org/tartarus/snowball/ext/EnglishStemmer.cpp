using namespace std;

#include "EnglishStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    EnglishStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_0 = {make_shared<org::tartarus::snowball::Among>(
                               L"arsen", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"commun", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"gener", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                               L"'", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"'s'", 0, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"'s", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"ied", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ies", 1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sses", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ss", 1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"us", 1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<
    org::tartarus::snowball::Among>> const EnglishStemmer::a_3 = {
    make_shared<org::tartarus::snowball::Among>(L"", -1, 3, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"bb", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"dd", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"ff", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"gg", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"bl", 0, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"mm", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"nn", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"pp", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"rr", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"at", 0, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"tt", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"iz", 0, 1, L"",
                                                methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"ed", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eed", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ing", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"edly", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eedly", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ingly", -1, 2, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_5 = {
        make_shared<org::tartarus::snowball::Among>(L"anci", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enci", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ogi", -1, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"li", -1, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bli", 3, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abli", 4, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alli", 3, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fulli", 3, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lessli", 3, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ousli", 3, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entli", 3, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aliti", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"biliti", -1, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iviti", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tional", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ational", 14, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alism", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ation", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ization", 17, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izer", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ator", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iveness", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fulness", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ousness", -1, 10, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"icate", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ative", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alize", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iciti", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ical", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tional", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ational", 5, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ful", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ness", -1, 5, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_7 = {
        make_shared<org::tartarus::snowball::Among>(L"ic", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ance", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ence", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"able", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ible", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ate", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ive", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ize", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"al", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ism", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ion", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ous", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ment", 15, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ement", 16, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_8 = {make_shared<org::tartarus::snowball::Among>(
                               L"e", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"l", -1, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_9 = {
        make_shared<org::tartarus::snowball::Among>(L"succeed", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"proceed", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"exceed", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"canning", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inning", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"earring", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"herring", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"outing", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    EnglishStemmer::a_10 = {
        make_shared<org::tartarus::snowball::Among>(L"andes", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atlas", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bias", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cosmos", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dying", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"early", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gently", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"howe", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idly", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lying", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"news", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"only", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"singly", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"skies", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"skis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sky", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tying", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ugly", -1, 8, L"",
                                                    methodObject)};
std::deque<wchar_t> const EnglishStemmer::g_v = {17, 65, 16, 1};
std::deque<wchar_t> const EnglishStemmer::g_v_WXY = {1, 17, 65, 208, 1};
std::deque<wchar_t> const EnglishStemmer::g_valid_LI = {55, 141, 2};

void EnglishStemmer::copy_from(shared_ptr<EnglishStemmer> other)
{
  B_Y_found = other->B_Y_found;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool EnglishStemmer::r_prelude()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 25
  // unset Y_found, line 26
  B_Y_found = false;
  // do, line 27
  v_1 = cursor;
  do {
    // (, line 27
    // [, line 27
    bra = cursor;
    // literal, line 27
    if (!(eq_s(1, L"'"))) {
      goto lab0Break;
    }
    // ], line 27
    ket = cursor;
    // delete, line 27
    slice_del();
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 28
  v_2 = cursor;
  do {
    // (, line 28
    // [, line 28
    bra = cursor;
    // literal, line 28
    if (!(eq_s(1, L"y"))) {
      goto lab1Break;
    }
    // ], line 28
    ket = cursor;
    // <-, line 28
    slice_from(L"Y");
    // set Y_found, line 28
    B_Y_found = true;
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // do, line 29
  v_3 = cursor;
  do {
    // repeat, line 29
    while (true) {
      v_4 = cursor;
      do {
        // (, line 29
        // goto, line 29
        while (true) {
          v_5 = cursor;
          do {
            // (, line 29
            if (!(in_grouping(g_v, 97, 121))) {
              goto lab6Break;
            }
            // [, line 29
            bra = cursor;
            // literal, line 29
            if (!(eq_s(1, L"y"))) {
              goto lab6Break;
            }
            // ], line 29
            ket = cursor;
            cursor = v_5;
            goto golab5Break;
          } while (false);
        lab6Continue:;
        lab6Break:
          cursor = v_5;
          if (cursor >= limit) {
            goto lab4Break;
          }
          cursor++;
        golab5Continue:;
        }
      golab5Break:
        // <-, line 29
        slice_from(L"Y");
        // set Y_found, line 29
        B_Y_found = true;
        goto replab3Continue;
      } while (false);
    lab4Continue:;
    lab4Break:
      cursor = v_4;
      goto replab3Break;
    replab3Continue:;
    }
  replab3Break:;
  } while (false);
lab2Continue:;
lab2Break:
  cursor = v_3;
  return true;
}

bool EnglishStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  // (, line 32
  I_p1 = limit;
  I_p2 = limit;
  // do, line 35
  v_1 = cursor;
  do {
    // (, line 35
    // or, line 41
    do {
      v_2 = cursor;
      do {
        // among, line 36
        if (find_among(a_0, 3) == 0) {
          goto lab2Break;
        }
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = v_2;
      // (, line 41
      // gopast, line 41
      while (true) {
        do {
          if (!(in_grouping(g_v, 97, 121))) {
            goto lab4Break;
          }
          goto golab3Break;
        } while (false);
      lab4Continue:;
      lab4Break:
        if (cursor >= limit) {
          goto lab0Break;
        }
        cursor++;
      golab3Continue:;
      }
    golab3Break:
      // gopast, line 41
      while (true) {
        do {
          if (!(out_grouping(g_v, 97, 121))) {
            goto lab6Break;
          }
          goto golab5Break;
        } while (false);
      lab6Continue:;
      lab6Break:
        if (cursor >= limit) {
          goto lab0Break;
        }
        cursor++;
      golab5Continue:;
      }
    golab5Break:;
    } while (false);
  lab1Continue:;
  lab1Break:
    // setmark p1, line 42
    I_p1 = cursor;
    // gopast, line 43
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 121))) {
          goto lab8Break;
        }
        goto golab7Break;
      } while (false);
    lab8Continue:;
    lab8Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab7Continue:;
    }
  golab7Break:
    // gopast, line 43
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 121))) {
          goto lab10Break;
        }
        goto golab9Break;
      } while (false);
    lab10Continue:;
    lab10Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab9Continue:;
    }
  golab9Break:
    // setmark p2, line 43
    I_p2 = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  return true;
}

bool EnglishStemmer::r_shortv()
{
  int v_1;
  // (, line 49
  // or, line 51
  do {
    v_1 = limit - cursor;
    do {
      // (, line 50
      if (!(out_grouping_b(g_v_WXY, 89, 121))) {
        goto lab1Break;
      }
      if (!(in_grouping_b(g_v, 97, 121))) {
        goto lab1Break;
      }
      if (!(out_grouping_b(g_v, 97, 121))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 52
    if (!(out_grouping_b(g_v, 97, 121))) {
      return false;
    }
    if (!(in_grouping_b(g_v, 97, 121))) {
      return false;
    }
    // atlimit, line 52
    if (cursor > limit_backward) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool EnglishStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool EnglishStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool EnglishStemmer::r_Step_1a()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 58
  // try, line 59
  v_1 = limit - cursor;
  do {
    // (, line 59
    // [, line 60
    ket = cursor;
    // substring, line 60
    among_var = find_among_b(a_1, 3);
    if (among_var == 0) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    // ], line 60
    bra = cursor;
    switch (among_var) {
    case 0:
      cursor = limit - v_1;
      goto lab0Break;
    case 1:
      // (, line 62
      // delete, line 62
      slice_del();
      break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  // [, line 65
  ket = cursor;
  // substring, line 65
  among_var = find_among_b(a_2, 6);
  if (among_var == 0) {
    return false;
  }
  // ], line 65
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 66
    // <-, line 66
    slice_from(L"ss");
    break;
  case 2:
    // (, line 68
    // or, line 68
    do {
      v_2 = limit - cursor;
      do {
        // (, line 68
        // hop, line 68
        {
          int c = cursor - 2;
          if (limit_backward > c || c > limit) {
            goto lab2Break;
          }
          cursor = c;
        }
        // <-, line 68
        slice_from(L"i");
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_2;
      // <-, line 68
      slice_from(L"ie");
    } while (false);
  lab1Continue:;
  lab1Break:
    break;
  case 3:
    // (, line 69
    // next, line 69
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
    // gopast, line 69
    while (true) {
      do {
        if (!(in_grouping_b(g_v, 97, 121))) {
          goto lab4Break;
        }
        goto golab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      if (cursor <= limit_backward) {
        return false;
      }
      cursor--;
    golab3Continue:;
    }
  golab3Break:
    // delete, line 69
    slice_del();
    break;
  }
  return true;
}

bool EnglishStemmer::r_Step_1b()
{
  int among_var;
  int v_1;
  int v_3;
  int v_4;
  // (, line 74
  // [, line 75
  ket = cursor;
  // substring, line 75
  among_var = find_among_b(a_4, 6);
  if (among_var == 0) {
    return false;
  }
  // ], line 75
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 77
    // call R1, line 77
    if (!r_R1()) {
      return false;
    }
    // <-, line 77
    slice_from(L"ee");
    break;
  case 2:
    // (, line 79
    // test, line 80
    v_1 = limit - cursor;
    // gopast, line 80
    while (true) {
      do {
        if (!(in_grouping_b(g_v, 97, 121))) {
          goto lab1Break;
        }
        goto golab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      if (cursor <= limit_backward) {
        return false;
      }
      cursor--;
    golab0Continue:;
    }
  golab0Break:
    cursor = limit - v_1;
    // delete, line 80
    slice_del();
    // test, line 81
    v_3 = limit - cursor;
    // substring, line 81
    among_var = find_among_b(a_3, 13);
    if (among_var == 0) {
      return false;
    }
    cursor = limit - v_3;
    switch (among_var) {
    case 0:
      return false;
    case 1:
      // (, line 83
      // <+, line 83
      {
        int c = cursor;
        insert(cursor, cursor, L"e");
        cursor = c;
      }
      break;
    case 2:
      // (, line 86
      // [, line 86
      ket = cursor;
      // next, line 86
      if (cursor <= limit_backward) {
        return false;
      }
      cursor--;
      // ], line 86
      bra = cursor;
      // delete, line 86
      slice_del();
      break;
    case 3:
      // (, line 87
      // atmark, line 87
      if (cursor != I_p1) {
        return false;
      }
      // test, line 87
      v_4 = limit - cursor;
      // call shortv, line 87
      if (!r_shortv()) {
        return false;
      }
      cursor = limit - v_4;
      // <+, line 87
      {
        int c = cursor;
        insert(cursor, cursor, L"e");
        cursor = c;
      }
      break;
    }
    break;
  }
  return true;
}

bool EnglishStemmer::r_Step_1c()
{
  int v_1;
  int v_2;
  // (, line 93
  // [, line 94
  ket = cursor;
  // or, line 94
  do {
    v_1 = limit - cursor;
    do {
      // literal, line 94
      if (!(eq_s_b(1, L"y"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // literal, line 94
    if (!(eq_s_b(1, L"Y"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  // ], line 94
  bra = cursor;
  if (!(out_grouping_b(g_v, 97, 121))) {
    return false;
  }
  {
    // not, line 95
    v_2 = limit - cursor;
    do {
      // atlimit, line 95
      if (cursor > limit_backward) {
        goto lab2Break;
      }
      return false;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_2;
  }
  // <-, line 96
  slice_from(L"i");
  return true;
}

bool EnglishStemmer::r_Step_2()
{
  int among_var;
  // (, line 99
  // [, line 100
  ket = cursor;
  // substring, line 100
  among_var = find_among_b(a_5, 24);
  if (among_var == 0) {
    return false;
  }
  // ], line 100
  bra = cursor;
  // call R1, line 100
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 101
    // <-, line 101
    slice_from(L"tion");
    break;
  case 2:
    // (, line 102
    // <-, line 102
    slice_from(L"ence");
    break;
  case 3:
    // (, line 103
    // <-, line 103
    slice_from(L"ance");
    break;
  case 4:
    // (, line 104
    // <-, line 104
    slice_from(L"able");
    break;
  case 5:
    // (, line 105
    // <-, line 105
    slice_from(L"ent");
    break;
  case 6:
    // (, line 107
    // <-, line 107
    slice_from(L"ize");
    break;
  case 7:
    // (, line 109
    // <-, line 109
    slice_from(L"ate");
    break;
  case 8:
    // (, line 111
    // <-, line 111
    slice_from(L"al");
    break;
  case 9:
    // (, line 112
    // <-, line 112
    slice_from(L"ful");
    break;
  case 10:
    // (, line 114
    // <-, line 114
    slice_from(L"ous");
    break;
  case 11:
    // (, line 116
    // <-, line 116
    slice_from(L"ive");
    break;
  case 12:
    // (, line 118
    // <-, line 118
    slice_from(L"ble");
    break;
  case 13:
    // (, line 119
    // literal, line 119
    if (!(eq_s_b(1, L"l"))) {
      return false;
    }
    // <-, line 119
    slice_from(L"og");
    break;
  case 14:
    // (, line 120
    // <-, line 120
    slice_from(L"ful");
    break;
  case 15:
    // (, line 121
    // <-, line 121
    slice_from(L"less");
    break;
  case 16:
    // (, line 122
    if (!(in_grouping_b(g_valid_LI, 99, 116))) {
      return false;
    }
    // delete, line 122
    slice_del();
    break;
  }
  return true;
}

bool EnglishStemmer::r_Step_3()
{
  int among_var;
  // (, line 126
  // [, line 127
  ket = cursor;
  // substring, line 127
  among_var = find_among_b(a_6, 9);
  if (among_var == 0) {
    return false;
  }
  // ], line 127
  bra = cursor;
  // call R1, line 127
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 128
    // <-, line 128
    slice_from(L"tion");
    break;
  case 2:
    // (, line 129
    // <-, line 129
    slice_from(L"ate");
    break;
  case 3:
    // (, line 130
    // <-, line 130
    slice_from(L"al");
    break;
  case 4:
    // (, line 132
    // <-, line 132
    slice_from(L"ic");
    break;
  case 5:
    // (, line 134
    // delete, line 134
    slice_del();
    break;
  case 6:
    // (, line 136
    // call R2, line 136
    if (!r_R2()) {
      return false;
    }
    // delete, line 136
    slice_del();
    break;
  }
  return true;
}

bool EnglishStemmer::r_Step_4()
{
  int among_var;
  int v_1;
  // (, line 140
  // [, line 141
  ket = cursor;
  // substring, line 141
  among_var = find_among_b(a_7, 18);
  if (among_var == 0) {
    return false;
  }
  // ], line 141
  bra = cursor;
  // call R2, line 141
  if (!r_R2()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 144
    // delete, line 144
    slice_del();
    break;
  case 2:
    // (, line 145
    // or, line 145
    do {
      v_1 = limit - cursor;
      do {
        // literal, line 145
        if (!(eq_s_b(1, L"s"))) {
          goto lab1Break;
        }
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_1;
      // literal, line 145
      if (!(eq_s_b(1, L"t"))) {
        return false;
      }
    } while (false);
  lab0Continue:;
  lab0Break:
    // delete, line 145
    slice_del();
    break;
  }
  return true;
}

bool EnglishStemmer::r_Step_5()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 149
  // [, line 150
  ket = cursor;
  // substring, line 150
  among_var = find_among_b(a_8, 2);
  if (among_var == 0) {
    return false;
  }
  // ], line 150
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 151
    // or, line 151
    do {
      v_1 = limit - cursor;
      do {
        // call R2, line 151
        if (!r_R2()) {
          goto lab1Break;
        }
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_1;
      // (, line 151
      // call R1, line 151
      if (!r_R1()) {
        return false;
      }
      {
        // not, line 151
        v_2 = limit - cursor;
        do {
          // call shortv, line 151
          if (!r_shortv()) {
            goto lab2Break;
          }
          return false;
        } while (false);
      lab2Continue:;
      lab2Break:
        cursor = limit - v_2;
      }
    } while (false);
  lab0Continue:;
  lab0Break:
    // delete, line 151
    slice_del();
    break;
  case 2:
    // (, line 152
    // call R2, line 152
    if (!r_R2()) {
      return false;
    }
    // literal, line 152
    if (!(eq_s_b(1, L"l"))) {
      return false;
    }
    // delete, line 152
    slice_del();
    break;
  }
  return true;
}

bool EnglishStemmer::r_exception2()
{
  // (, line 156
  // [, line 158
  ket = cursor;
  // substring, line 158
  if (find_among_b(a_9, 8) == 0) {
    return false;
  }
  // ], line 158
  bra = cursor;
  // atlimit, line 158
  if (cursor > limit_backward) {
    return false;
  }
  return true;
}

bool EnglishStemmer::r_exception1()
{
  int among_var;
  // (, line 168
  // [, line 170
  bra = cursor;
  // substring, line 170
  among_var = find_among(a_10, 18);
  if (among_var == 0) {
    return false;
  }
  // ], line 170
  ket = cursor;
  // atlimit, line 170
  if (cursor < limit) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 174
    // <-, line 174
    slice_from(L"ski");
    break;
  case 2:
    // (, line 175
    // <-, line 175
    slice_from(L"sky");
    break;
  case 3:
    // (, line 176
    // <-, line 176
    slice_from(L"die");
    break;
  case 4:
    // (, line 177
    // <-, line 177
    slice_from(L"lie");
    break;
  case 5:
    // (, line 178
    // <-, line 178
    slice_from(L"tie");
    break;
  case 6:
    // (, line 182
    // <-, line 182
    slice_from(L"idl");
    break;
  case 7:
    // (, line 183
    // <-, line 183
    slice_from(L"gentl");
    break;
  case 8:
    // (, line 184
    // <-, line 184
    slice_from(L"ugli");
    break;
  case 9:
    // (, line 185
    // <-, line 185
    slice_from(L"earli");
    break;
  case 10:
    // (, line 186
    // <-, line 186
    slice_from(L"onli");
    break;
  case 11:
    // (, line 187
    // <-, line 187
    slice_from(L"singl");
    break;
  }
  return true;
}

bool EnglishStemmer::r_postlude()
{
  int v_1;
  int v_2;
  // (, line 203
  // Boolean test Y_found, line 203
  if (!(B_Y_found)) {
    return false;
  }
  // repeat, line 203
  while (true) {
    v_1 = cursor;
    do {
      // (, line 203
      // goto, line 203
      while (true) {
        v_2 = cursor;
        do {
          // (, line 203
          // [, line 203
          bra = cursor;
          // literal, line 203
          if (!(eq_s(1, L"Y"))) {
            goto lab3Break;
          }
          // ], line 203
          ket = cursor;
          cursor = v_2;
          goto golab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = v_2;
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
      golab2Continue:;
      }
    golab2Break:
      // <-, line 203
      slice_from(L"y");
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool EnglishStemmer::stem()
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
  // (, line 205
  // or, line 207
  do {
    v_1 = cursor;
    do {
      // call exception1, line 207
      if (!r_exception1()) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    do {
      {
        // not, line 208
        v_2 = cursor;
        do {
          {
            // hop, line 208
            int c = cursor + 3;
            if (0 > c || c > limit) {
              goto lab3Break;
            }
            cursor = c;
          }
          goto lab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = v_2;
      }
      goto lab0Break;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = v_1;
    // (, line 208
    // do, line 209
    v_3 = cursor;
    do {
      // call prelude, line 209
      if (!r_prelude()) {
        goto lab4Break;
      }
    } while (false);
  lab4Continue:;
  lab4Break:
    cursor = v_3;
    // do, line 210
    v_4 = cursor;
    do {
      // call mark_regions, line 210
      if (!r_mark_regions()) {
        goto lab5Break;
      }
    } while (false);
  lab5Continue:;
  lab5Break:
    cursor = v_4;
    // backwards, line 211
    limit_backward = cursor;
    cursor = limit;
    // (, line 211
    // do, line 213
    v_5 = limit - cursor;
    do {
      // call Step_1a, line 213
      if (!r_Step_1a()) {
        goto lab6Break;
      }
    } while (false);
  lab6Continue:;
  lab6Break:
    cursor = limit - v_5;
    // or, line 215
    do {
      v_6 = limit - cursor;
      do {
        // call exception2, line 215
        if (!r_exception2()) {
          goto lab8Break;
        }
        goto lab7Break;
      } while (false);
    lab8Continue:;
    lab8Break:
      cursor = limit - v_6;
      // (, line 215
      // do, line 217
      v_7 = limit - cursor;
      do {
        // call Step_1b, line 217
        if (!r_Step_1b()) {
          goto lab9Break;
        }
      } while (false);
    lab9Continue:;
    lab9Break:
      cursor = limit - v_7;
      // do, line 218
      v_8 = limit - cursor;
      do {
        // call Step_1c, line 218
        if (!r_Step_1c()) {
          goto lab10Break;
        }
      } while (false);
    lab10Continue:;
    lab10Break:
      cursor = limit - v_8;
      // do, line 220
      v_9 = limit - cursor;
      do {
        // call Step_2, line 220
        if (!r_Step_2()) {
          goto lab11Break;
        }
      } while (false);
    lab11Continue:;
    lab11Break:
      cursor = limit - v_9;
      // do, line 221
      v_10 = limit - cursor;
      do {
        // call Step_3, line 221
        if (!r_Step_3()) {
          goto lab12Break;
        }
      } while (false);
    lab12Continue:;
    lab12Break:
      cursor = limit - v_10;
      // do, line 222
      v_11 = limit - cursor;
      do {
        // call Step_4, line 222
        if (!r_Step_4()) {
          goto lab13Break;
        }
      } while (false);
    lab13Continue:;
    lab13Break:
      cursor = limit - v_11;
      // do, line 224
      v_12 = limit - cursor;
      do {
        // call Step_5, line 224
        if (!r_Step_5()) {
          goto lab14Break;
        }
      } while (false);
    lab14Continue:;
    lab14Break:
      cursor = limit - v_12;
    } while (false);
  lab7Continue:;
  lab7Break:
    cursor = limit_backward; // do, line 227
    v_13 = cursor;
    do {
      // call postlude, line 227
      if (!r_postlude()) {
        goto lab15Break;
      }
    } while (false);
  lab15Continue:;
  lab15Break:
    cursor = v_13;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool EnglishStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<EnglishStemmer>(o) != nullptr;
}

int EnglishStemmer::hashCode()
{
  return EnglishStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext