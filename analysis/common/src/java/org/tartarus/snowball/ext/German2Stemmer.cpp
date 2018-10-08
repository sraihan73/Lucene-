using namespace std;

#include "German2Stemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    German2Stemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    German2Stemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ae", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oe", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qu", 0, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ue", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00DF", 0, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    German2Stemmer::a_1 = {
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
    German2Stemmer::a_2 = {
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
    German2Stemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                               L"en", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"er", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"st", -1, 2, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"est", 2, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    German2Stemmer::a_4 = {make_shared<org::tartarus::snowball::Among>(
                               L"ig", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"lich", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    German2Stemmer::a_5 = {
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
std::deque<wchar_t> const German2Stemmer::g_v = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32, 8};
std::deque<wchar_t> const German2Stemmer::g_s_ending = {117, 30, 5};
std::deque<wchar_t> const German2Stemmer::g_st_ending = {117, 30, 4};

void German2Stemmer::copy_from(shared_ptr<German2Stemmer> other)
{
  I_x = other->I_x;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool German2Stemmer::r_prelude()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 28
  // test, line 30
  v_1 = cursor;
  // repeat, line 30
  while (true) {
    v_2 = cursor;
    do {
      // goto, line 30
      while (true) {
        v_3 = cursor;
        do {
          // (, line 30
          if (!(in_grouping(g_v, 97, 252))) {
            goto lab3Break;
          }
          // [, line 31
          bra = cursor;
          // or, line 31
          do {
            v_4 = cursor;
            do {
              // (, line 31
              // literal, line 31
              if (!(eq_s(1, L"u"))) {
                goto lab5Break;
              }
              // ], line 31
              ket = cursor;
              if (!(in_grouping(g_v, 97, 252))) {
                goto lab5Break;
              }
              // <-, line 31
              slice_from(L"U");
              goto lab4Break;
            } while (false);
          lab5Continue:;
          lab5Break:
            cursor = v_4;
            // (, line 32
            // literal, line 32
            if (!(eq_s(1, L"y"))) {
              goto lab3Break;
            }
            // ], line 32
            ket = cursor;
            if (!(in_grouping(g_v, 97, 252))) {
              goto lab3Break;
            }
            // <-, line 32
            slice_from(L"Y");
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = v_3;
          goto golab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = v_3;
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
      golab2Continue:;
      }
    golab2Break:
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
  // repeat, line 35
  while (true) {
    v_5 = cursor;
    do {
      // (, line 35
      // [, line 36
      bra = cursor;
      // substring, line 36
      among_var = find_among(a_0, 6);
      if (among_var == 0) {
        goto lab7Break;
      }
      // ], line 36
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab7Break;
      case 1:
        // (, line 37
        // <-, line 37
        slice_from(L"ss");
        break;
      case 2:
        // (, line 38
        // <-, line 38
        slice_from(L"\u00E4");
        break;
      case 3:
        // (, line 39
        // <-, line 39
        slice_from(L"\u00F6");
        break;
      case 4:
        // (, line 40
        // <-, line 40
        slice_from(L"\u00FC");
        break;
      case 5:
        // (, line 41
        // hop, line 41
        {
          int c = cursor + 2;
          if (0 > c || c > limit) {
            goto lab7Break;
          }
          cursor = c;
        }
        break;
      case 6:
        // (, line 42
        // next, line 42
        if (cursor >= limit) {
          goto lab7Break;
        }
        cursor++;
        break;
      }
      goto replab6Continue;
    } while (false);
  lab7Continue:;
  lab7Break:
    cursor = v_5;
    goto replab6Break;
  replab6Continue:;
  }
replab6Break:
  return true;
}

bool German2Stemmer::r_mark_regions()
{
  int v_1;
  // (, line 48
  I_p1 = limit;
  I_p2 = limit;
  // test, line 53
  v_1 = cursor;
  // (, line 53
  // hop, line 53
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // setmark x, line 53
  I_x = cursor;
  cursor = v_1;
  // gopast, line 55
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
  // gopast, line 55
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
  // setmark p1, line 55
  I_p1 = cursor;
  // try, line 56
  do {
    // (, line 56
    if (!(I_p1 < I_x)) {
      goto lab4Break;
    }
    I_p1 = I_x;
  } while (false);
lab4Continue:;
lab4Break:
  // gopast, line 57
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
  // gopast, line 57
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
  // setmark p2, line 57
  I_p2 = cursor;
  return true;
}

bool German2Stemmer::r_postlude()
{
  int among_var;
  int v_1;
  // repeat, line 61
  while (true) {
    v_1 = cursor;
    do {
      // (, line 61
      // [, line 63
      bra = cursor;
      // substring, line 63
      among_var = find_among(a_1, 6);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 63
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 64
        // <-, line 64
        slice_from(L"y");
        break;
      case 2:
        // (, line 65
        // <-, line 65
        slice_from(L"u");
        break;
      case 3:
        // (, line 66
        // <-, line 66
        slice_from(L"a");
        break;
      case 4:
        // (, line 67
        // <-, line 67
        slice_from(L"o");
        break;
      case 5:
        // (, line 68
        // <-, line 68
        slice_from(L"u");
        break;
      case 6:
        // (, line 69
        // next, line 69
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

bool German2Stemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool German2Stemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool German2Stemmer::r_standard_suffix()
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
  // (, line 79
  // do, line 80
  v_1 = limit - cursor;
  do {
    // (, line 80
    // [, line 81
    ket = cursor;
    // substring, line 81
    among_var = find_among_b(a_2, 7);
    if (among_var == 0) {
      goto lab0Break;
    }
    // ], line 81
    bra = cursor;
    // call R1, line 81
    if (!r_R1()) {
      goto lab0Break;
    }
    switch (among_var) {
    case 0:
      goto lab0Break;
    case 1:
      // (, line 83
      // delete, line 83
      slice_del();
      break;
    case 2:
      // (, line 86
      if (!(in_grouping_b(g_s_ending, 98, 116))) {
        goto lab0Break;
      }
      // delete, line 86
      slice_del();
      break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  // do, line 90
  v_2 = limit - cursor;
  do {
    // (, line 90
    // [, line 91
    ket = cursor;
    // substring, line 91
    among_var = find_among_b(a_3, 4);
    if (among_var == 0) {
      goto lab1Break;
    }
    // ], line 91
    bra = cursor;
    // call R1, line 91
    if (!r_R1()) {
      goto lab1Break;
    }
    switch (among_var) {
    case 0:
      goto lab1Break;
    case 1:
      // (, line 93
      // delete, line 93
      slice_del();
      break;
    case 2:
      // (, line 96
      if (!(in_grouping_b(g_st_ending, 98, 116))) {
        goto lab1Break;
      }
      {
        // hop, line 96
        int c = cursor - 3;
        if (limit_backward > c || c > limit) {
          goto lab1Break;
        }
        cursor = c;
      }
      // delete, line 96
      slice_del();
      break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 100
  v_3 = limit - cursor;
  do {
    // (, line 100
    // [, line 101
    ket = cursor;
    // substring, line 101
    among_var = find_among_b(a_5, 8);
    if (among_var == 0) {
      goto lab2Break;
    }
    // ], line 101
    bra = cursor;
    // call R2, line 101
    if (!r_R2()) {
      goto lab2Break;
    }
    switch (among_var) {
    case 0:
      goto lab2Break;
    case 1:
      // (, line 103
      // delete, line 103
      slice_del();
      // try, line 104
      v_4 = limit - cursor;
      do {
        // (, line 104
        // [, line 104
        ket = cursor;
        // literal, line 104
        if (!(eq_s_b(2, L"ig"))) {
          cursor = limit - v_4;
          goto lab3Break;
        }
        // ], line 104
        bra = cursor;
        // not, line 104
        {
          v_5 = limit - cursor;
          do {
            // literal, line 104
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
        // call R2, line 104
        if (!r_R2()) {
          cursor = limit - v_4;
          goto lab3Break;
        }
        // delete, line 104
        slice_del();
      } while (false);
    lab3Continue:;
    lab3Break:
      break;
    case 2:
      // (, line 107
      // not, line 107
      {
        v_6 = limit - cursor;
        do {
          // literal, line 107
          if (!(eq_s_b(1, L"e"))) {
            goto lab5Break;
          }
          goto lab2Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_6;
      }
      // delete, line 107
      slice_del();
      break;
    case 3:
      // (, line 110
      // delete, line 110
      slice_del();
      // try, line 111
      v_7 = limit - cursor;
      do {
        // (, line 111
        // [, line 112
        ket = cursor;
        // or, line 112
        do {
          v_8 = limit - cursor;
          do {
            // literal, line 112
            if (!(eq_s_b(2, L"er"))) {
              goto lab8Break;
            }
            goto lab7Break;
          } while (false);
        lab8Continue:;
        lab8Break:
          cursor = limit - v_8;
          // literal, line 112
          if (!(eq_s_b(2, L"en"))) {
            cursor = limit - v_7;
            goto lab6Break;
          }
        } while (false);
      lab7Continue:;
      lab7Break:
        // ], line 112
        bra = cursor;
        // call R1, line 112
        if (!r_R1()) {
          cursor = limit - v_7;
          goto lab6Break;
        }
        // delete, line 112
        slice_del();
      } while (false);
    lab6Continue:;
    lab6Break:
      break;
    case 4:
      // (, line 116
      // delete, line 116
      slice_del();
      // try, line 117
      v_9 = limit - cursor;
      do {
        // (, line 117
        // [, line 118
        ket = cursor;
        // substring, line 118
        among_var = find_among_b(a_4, 2);
        if (among_var == 0) {
          cursor = limit - v_9;
          goto lab9Break;
        }
        // ], line 118
        bra = cursor;
        // call R2, line 118
        if (!r_R2()) {
          cursor = limit - v_9;
          goto lab9Break;
        }
        switch (among_var) {
        case 0:
          cursor = limit - v_9;
          goto lab9Break;
        case 1:
          // (, line 120
          // delete, line 120
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

bool German2Stemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 130
  // do, line 131
  v_1 = cursor;
  do {
    // call prelude, line 131
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 132
  v_2 = cursor;
  do {
    // call mark_regions, line 132
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 133
  limit_backward = cursor;
  cursor = limit;
  // do, line 134
  v_3 = limit - cursor;
  do {
    // call standard_suffix, line 134
    if (!r_standard_suffix()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  cursor = limit_backward; // do, line 135
  v_4 = cursor;
  do {
    // call postlude, line 135
    if (!r_postlude()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = v_4;
  return true;
}

bool German2Stemmer::equals(any o)
{
  return std::dynamic_pointer_cast<German2Stemmer>(o) != nullptr;
}

int German2Stemmer::hashCode()
{
  return German2Stemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext