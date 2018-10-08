using namespace std;

#include "GermanStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    GermanStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    GermanStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"U", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"Y", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E4", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FC", 0, 5, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    GermanStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"em", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ern", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", 5, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    GermanStemmer::a_2 = {make_shared<org::tartarus::snowball::Among>(
                              L"en", -1, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"er", -1, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"st", -1, 2, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"est", 2, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    GermanStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                              L"ig", -1, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"lich", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    GermanStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"end", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ig", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ung", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lich", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isch", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ik", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heit", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"keit", -1, 4, L"",
                                                    methodObject)};
std::deque<wchar_t> const GermanStemmer::g_v = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32, 8};
std::deque<wchar_t> const GermanStemmer::g_s_ending = {117, 30, 5};
std::deque<wchar_t> const GermanStemmer::g_st_ending = {117, 30, 4};

void GermanStemmer::copy_from(shared_ptr<GermanStemmer> other)
{
  I_x = other->I_x;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool GermanStemmer::r_prelude()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  // (, line 28
  // test, line 30
  v_1 = cursor;
  // repeat, line 30
  while (true) {
    v_2 = cursor;
    do {
      // (, line 30
      // or, line 33
      do {
        v_3 = cursor;
        do {
          // (, line 31
          // [, line 32
          bra = cursor;
          // literal, line 32
          if (!(eq_s(1, L"\u00DF"))) {
            goto lab3Break;
          }
          // ], line 32
          ket = cursor;
          // <-, line 32
          slice_from(L"ss");
          goto lab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = v_3;
        // next, line 33
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
      } while (false);
    lab2Continue:;
    lab2Break:
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_2;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  cursor = v_1;
  // repeat, line 36
  while (true) {
    v_4 = cursor;
    do {
      // goto, line 36
      while (true) {
        v_5 = cursor;
        do {
          // (, line 36
          if (!(in_grouping(g_v, 97, 252))) {
            goto lab7Break;
          }
          // [, line 37
          bra = cursor;
          // or, line 37
          do {
            v_6 = cursor;
            do {
              // (, line 37
              // literal, line 37
              if (!(eq_s(1, L"u"))) {
                goto lab9Break;
              }
              // ], line 37
              ket = cursor;
              if (!(in_grouping(g_v, 97, 252))) {
                goto lab9Break;
              }
              // <-, line 37
              slice_from(L"U");
              goto lab8Break;
            } while (false);
          lab9Continue:;
          lab9Break:
            cursor = v_6;
            // (, line 38
            // literal, line 38
            if (!(eq_s(1, L"y"))) {
              goto lab7Break;
            }
            // ], line 38
            ket = cursor;
            if (!(in_grouping(g_v, 97, 252))) {
              goto lab7Break;
            }
            // <-, line 38
            slice_from(L"Y");
          } while (false);
        lab8Continue:;
        lab8Break:
          cursor = v_5;
          goto golab6Break;
        } while (false);
      lab7Continue:;
      lab7Break:
        cursor = v_5;
        if (cursor >= limit) {
          goto lab5Break;
        }
        cursor++;
      golab6Continue:;
      }
    golab6Break:
      goto replab4Continue;
    } while (false);
  lab5Continue:;
  lab5Break:
    cursor = v_4;
    goto replab4Break;
  replab4Continue:;
  }
replab4Break:
  return true;
}

bool GermanStemmer::r_mark_regions()
{
  int v_1;
  // (, line 42
  I_p1 = limit;
  I_p2 = limit;
  // test, line 47
  v_1 = cursor;
  // (, line 47
  // hop, line 47
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // setmark x, line 47
  I_x = cursor;
  cursor = v_1;
  // gopast, line 49
  while (true) {
    do {
      if (!(in_grouping(g_v, 97, 252))) {
        goto lab1Break;
      }
      goto golab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab0Continue:;
  }
golab0Break:
  // gopast, line 49
  while (true) {
    do {
      if (!(out_grouping(g_v, 97, 252))) {
        goto lab3Break;
      }
      goto golab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab2Continue:;
  }
golab2Break:
  // setmark p1, line 49
  I_p1 = cursor;
  // try, line 50
  do {
    // (, line 50
    if (!(I_p1 < I_x)) {
      goto lab4Break;
    }
    I_p1 = I_x;
  } while (false);
lab4Continue:;
lab4Break:
  // gopast, line 51
  while (true) {
    do {
      if (!(in_grouping(g_v, 97, 252))) {
        goto lab6Break;
      }
      goto golab5Break;
    } while (false);
  lab6Continue:;
  lab6Break:
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab5Continue:;
  }
golab5Break:
  // gopast, line 51
  while (true) {
    do {
      if (!(out_grouping(g_v, 97, 252))) {
        goto lab8Break;
      }
      goto golab7Break;
    } while (false);
  lab8Continue:;
  lab8Break:
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab7Continue:;
  }
golab7Break:
  // setmark p2, line 51
  I_p2 = cursor;
  return true;
}

bool GermanStemmer::r_postlude()
{
  int among_var;
  int v_1;
  // repeat, line 55
  while (true) {
    v_1 = cursor;
    do {
      // (, line 55
      // [, line 57
      bra = cursor;
      // substring, line 57
      among_var = find_among(a_0, 6);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 57
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 58
        // <-, line 58
        slice_from(L"y");
        break;
      case 2:
        // (, line 59
        // <-, line 59
        slice_from(L"u");
        break;
      case 3:
        // (, line 60
        // <-, line 60
        slice_from(L"a");
        break;
      case 4:
        // (, line 61
        // <-, line 61
        slice_from(L"o");
        break;
      case 5:
        // (, line 62
        // <-, line 62
        slice_from(L"u");
        break;
      case 6:
        // (, line 63
        // next, line 63
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
        break;
      }
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

bool GermanStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool GermanStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool GermanStemmer::r_standard_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  // (, line 73
  // do, line 74
  v_1 = limit - cursor;
  do {
    // (, line 74
    // [, line 75
    ket = cursor;
    // substring, line 75
    among_var = find_among_b(a_1, 7);
    if (among_var == 0) {
      goto lab0Break;
    }
    // ], line 75
    bra = cursor;
    // call R1, line 75
    if (!r_R1()) {
      goto lab0Break;
    }
    switch (among_var) {
    case 0:
      goto lab0Break;
    case 1:
      // (, line 77
      // delete, line 77
      slice_del();
      break;
    case 2:
      // (, line 80
      if (!(in_grouping_b(g_s_ending, 98, 116))) {
        goto lab0Break;
      }
      // delete, line 80
      slice_del();
      break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  // do, line 84
  v_2 = limit - cursor;
  do {
    // (, line 84
    // [, line 85
    ket = cursor;
    // substring, line 85
    among_var = find_among_b(a_2, 4);
    if (among_var == 0) {
      goto lab1Break;
    }
    // ], line 85
    bra = cursor;
    // call R1, line 85
    if (!r_R1()) {
      goto lab1Break;
    }
    switch (among_var) {
    case 0:
      goto lab1Break;
    case 1:
      // (, line 87
      // delete, line 87
      slice_del();
      break;
    case 2:
      // (, line 90
      if (!(in_grouping_b(g_st_ending, 98, 116))) {
        goto lab1Break;
      }
      {
        // hop, line 90
        int c = cursor - 3;
        if (limit_backward > c || c > limit) {
          goto lab1Break;
        }
        cursor = c;
      }
      // delete, line 90
      slice_del();
      break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 94
  v_3 = limit - cursor;
  do {
    // (, line 94
    // [, line 95
    ket = cursor;
    // substring, line 95
    among_var = find_among_b(a_4, 8);
    if (among_var == 0) {
      goto lab2Break;
    }
    // ], line 95
    bra = cursor;
    // call R2, line 95
    if (!r_R2()) {
      goto lab2Break;
    }
    switch (among_var) {
    case 0:
      goto lab2Break;
    case 1:
      // (, line 97
      // delete, line 97
      slice_del();
      // try, line 98
      v_4 = limit - cursor;
      do {
        // (, line 98
        // [, line 98
        ket = cursor;
        // literal, line 98
        if (!(eq_s_b(2, L"ig"))) {
          cursor = limit - v_4;
          goto lab3Break;
        }
        // ], line 98
        bra = cursor;
        // not, line 98
        {
          v_5 = limit - cursor;
          do {
            // literal, line 98
            if (!(eq_s_b(1, L"e"))) {
              goto lab4Break;
            }
            cursor = limit - v_4;
            goto lab3Break;
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = limit - v_5;
        }
        // call R2, line 98
        if (!r_R2()) {
          cursor = limit - v_4;
          goto lab3Break;
        }
        // delete, line 98
        slice_del();
      } while (false);
    lab3Continue:;
    lab3Break:
      break;
    case 2:
      // (, line 101
      // not, line 101
      {
        v_6 = limit - cursor;
        do {
          // literal, line 101
          if (!(eq_s_b(1, L"e"))) {
            goto lab5Break;
          }
          goto lab2Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_6;
      }
      // delete, line 101
      slice_del();
      break;
    case 3:
      // (, line 104
      // delete, line 104
      slice_del();
      // try, line 105
      v_7 = limit - cursor;
      do {
        // (, line 105
        // [, line 106
        ket = cursor;
        // or, line 106
        do {
          v_8 = limit - cursor;
          do {
            // literal, line 106
            if (!(eq_s_b(2, L"er"))) {
              goto lab8Break;
            }
            goto lab7Break;
          } while (false);
        lab8Continue:;
        lab8Break:
          cursor = limit - v_8;
          // literal, line 106
          if (!(eq_s_b(2, L"en"))) {
            cursor = limit - v_7;
            goto lab6Break;
          }
        } while (false);
      lab7Continue:;
      lab7Break:
        // ], line 106
        bra = cursor;
        // call R1, line 106
        if (!r_R1()) {
          cursor = limit - v_7;
          goto lab6Break;
        }
        // delete, line 106
        slice_del();
      } while (false);
    lab6Continue:;
    lab6Break:
      break;
    case 4:
      // (, line 110
      // delete, line 110
      slice_del();
      // try, line 111
      v_9 = limit - cursor;
      do {
        // (, line 111
        // [, line 112
        ket = cursor;
        // substring, line 112
        among_var = find_among_b(a_3, 2);
        if (among_var == 0) {
          cursor = limit - v_9;
          goto lab9Break;
        }
        // ], line 112
        bra = cursor;
        // call R2, line 112
        if (!r_R2()) {
          cursor = limit - v_9;
          goto lab9Break;
        }
        switch (among_var) {
        case 0:
          cursor = limit - v_9;
          goto lab9Break;
        case 1:
          // (, line 114
          // delete, line 114
          slice_del();
          break;
        }
      } while (false);
    lab9Continue:;
    lab9Break:
      break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  return true;
}

bool GermanStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 124
  // do, line 125
  v_1 = cursor;
  do {
    // call prelude, line 125
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 126
  v_2 = cursor;
  do {
    // call mark_regions, line 126
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 127
  limit_backward = cursor;
  cursor = limit;
  // do, line 128
  v_3 = limit - cursor;
  do {
    // call standard_suffix, line 128
    if (!r_standard_suffix()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  cursor = limit_backward; // do, line 129
  v_4 = cursor;
  do {
    // call postlude, line 129
    if (!r_postlude()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = v_4;
  return true;
}

bool GermanStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<GermanStemmer>(o) != nullptr;
}

int GermanStemmer::hashCode()
{
  return GermanStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext