using namespace std;

#include "DutchStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    DutchStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DutchStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E4", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EB", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ED", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EF", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FA", 0, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FC", 0, 5, L"",
                                                    methodObject)};
std::deque<
    std::shared_ptr<org::tartarus::snowball::Among>> const DutchStemmer::a_1 = {
    make_shared<org::tartarus::snowball::Among>(L"", -1, 3, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"I", 0, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"Y", 0, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DutchStemmer::a_2 = {make_shared<org::tartarus::snowball::Among>(
                             L"dd", -1, -1, L"", methodObject),
                         make_shared<org::tartarus::snowball::Among>(
                             L"kk", -1, -1, L"", methodObject),
                         make_shared<org::tartarus::snowball::Among>(
                             L"tt", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DutchStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"ene", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"se", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heden", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 3, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DutchStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"end", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ig", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ing", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lijk", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"baar", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bar", -1, 5, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DutchStemmer::a_5 = {make_shared<org::tartarus::snowball::Among>(
                             L"aa", -1, -1, L"", methodObject),
                         make_shared<org::tartarus::snowball::Among>(
                             L"ee", -1, -1, L"", methodObject),
                         make_shared<org::tartarus::snowball::Among>(
                             L"oo", -1, -1, L"", methodObject),
                         make_shared<org::tartarus::snowball::Among>(
                             L"uu", -1, -1, L"", methodObject)};
std::deque<wchar_t> const DutchStemmer::g_v = {17, 65, 16, 1, 0, 0, 0, 0,  0,
                                                0,  0,  0,  0, 0, 0, 0, 128};
std::deque<wchar_t> const DutchStemmer::g_v_I = {
    1, 0, 0, 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128};
std::deque<wchar_t> const DutchStemmer::g_v_j = {17, 67, 16, 1, 0, 0, 0, 0,  0,
                                                  0,  0,  0,  0, 0, 0, 0, 128};

void DutchStemmer::copy_from(shared_ptr<DutchStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  B_e_found = other->B_e_found;
  SnowballProgram::copy_from(other);
}

bool DutchStemmer::r_prelude()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  // (, line 41
  // test, line 42
  v_1 = cursor;
  // repeat, line 42
  while (true) {
    v_2 = cursor;
    do {
      // (, line 42
      // [, line 43
      bra = cursor;
      // substring, line 43
      among_var = find_among(a_0, 11);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 43
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 45
        // <-, line 45
        slice_from(L"a");
        break;
      case 2:
        // (, line 47
        // <-, line 47
        slice_from(L"e");
        break;
      case 3:
        // (, line 49
        // <-, line 49
        slice_from(L"i");
        break;
      case 4:
        // (, line 51
        // <-, line 51
        slice_from(L"o");
        break;
      case 5:
        // (, line 53
        // <-, line 53
        slice_from(L"u");
        break;
      case 6:
        // (, line 54
        // next, line 54
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
    cursor = v_2;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  cursor = v_1;
  // try, line 57
  v_3 = cursor;
  do {
    // (, line 57
    // [, line 57
    bra = cursor;
    // literal, line 57
    if (!(eq_s(1, L"y"))) {
      cursor = v_3;
      goto lab2Break;
    }
    // ], line 57
    ket = cursor;
    // <-, line 57
    slice_from(L"Y");
  } while (false);
lab2Continue:;
lab2Break:
  // repeat, line 58
  while (true) {
    v_4 = cursor;
    do {
      // goto, line 58
      while (true) {
        v_5 = cursor;
        do {
          // (, line 58
          if (!(in_grouping(g_v, 97, 232))) {
            goto lab6Break;
          }
          // [, line 59
          bra = cursor;
          // or, line 59
          do {
            v_6 = cursor;
            do {
              // (, line 59
              // literal, line 59
              if (!(eq_s(1, L"i"))) {
                goto lab8Break;
              }
              // ], line 59
              ket = cursor;
              if (!(in_grouping(g_v, 97, 232))) {
                goto lab8Break;
              }
              // <-, line 59
              slice_from(L"I");
              goto lab7Break;
            } while (false);
          lab8Continue:;
          lab8Break:
            cursor = v_6;
            // (, line 60
            // literal, line 60
            if (!(eq_s(1, L"y"))) {
              goto lab6Break;
            }
            // ], line 60
            ket = cursor;
            // <-, line 60
            slice_from(L"Y");
          } while (false);
        lab7Continue:;
        lab7Break:
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
      goto replab3Continue;
    } while (false);
  lab4Continue:;
  lab4Break:
    cursor = v_4;
    goto replab3Break;
  replab3Continue:;
  }
replab3Break:
  return true;
}

bool DutchStemmer::r_mark_regions()
{
  // (, line 64
  I_p1 = limit;
  I_p2 = limit;
  // gopast, line 69
  while (true) {
    do {
      if (!(in_grouping(g_v, 97, 232))) {
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
  // gopast, line 69
  while (true) {
    do {
      if (!(out_grouping(g_v, 97, 232))) {
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
  // setmark p1, line 69
  I_p1 = cursor;
  // try, line 70
  do {
    // (, line 70
    if (!(I_p1 < 3)) {
      goto lab4Break;
    }
    I_p1 = 3;
  } while (false);
lab4Continue:;
lab4Break:
  // gopast, line 71
  while (true) {
    do {
      if (!(in_grouping(g_v, 97, 232))) {
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
  // gopast, line 71
  while (true) {
    do {
      if (!(out_grouping(g_v, 97, 232))) {
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
  // setmark p2, line 71
  I_p2 = cursor;
  return true;
}

bool DutchStemmer::r_postlude()
{
  int among_var;
  int v_1;
  // repeat, line 75
  while (true) {
    v_1 = cursor;
    do {
      // (, line 75
      // [, line 77
      bra = cursor;
      // substring, line 77
      among_var = find_among(a_1, 3);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 77
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 78
        // <-, line 78
        slice_from(L"y");
        break;
      case 2:
        // (, line 79
        // <-, line 79
        slice_from(L"i");
        break;
      case 3:
        // (, line 80
        // next, line 80
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

bool DutchStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool DutchStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool DutchStemmer::r_undouble()
{
  int v_1;
  // (, line 90
  // test, line 91
  v_1 = limit - cursor;
  // among, line 91
  if (find_among_b(a_2, 3) == 0) {
    return false;
  }
  cursor = limit - v_1;
  // [, line 91
  ket = cursor;
  // next, line 91
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // ], line 91
  bra = cursor;
  // delete, line 91
  slice_del();
  return true;
}

bool DutchStemmer::r_e_ending()
{
  int v_1;
  // (, line 94
  // unset e_found, line 95
  B_e_found = false;
  // [, line 96
  ket = cursor;
  // literal, line 96
  if (!(eq_s_b(1, L"e"))) {
    return false;
  }
  // ], line 96
  bra = cursor;
  // call R1, line 96
  if (!r_R1()) {
    return false;
  }
  // test, line 96
  v_1 = limit - cursor;
  if (!(out_grouping_b(g_v, 97, 232))) {
    return false;
  }
  cursor = limit - v_1;
  // delete, line 96
  slice_del();
  // set e_found, line 97
  B_e_found = true;
  // call undouble, line 98
  if (!r_undouble()) {
    return false;
  }
  return true;
}

bool DutchStemmer::r_en_ending()
{
  int v_1;
  int v_2;
  // (, line 101
  // call R1, line 102
  if (!r_R1()) {
    return false;
  }
  // and, line 102
  v_1 = limit - cursor;
  if (!(out_grouping_b(g_v, 97, 232))) {
    return false;
  }
  cursor = limit - v_1;
  // not, line 102
  {
    v_2 = limit - cursor;
    do {
      // literal, line 102
      if (!(eq_s_b(3, L"gem"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  // delete, line 102
  slice_del();
  // call undouble, line 103
  if (!r_undouble()) {
    return false;
  }
  return true;
}

bool DutchStemmer::r_standard_suffix()
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
  int v_10;
  // (, line 106
  // do, line 107
  v_1 = limit - cursor;
  do {
    // (, line 107
    // [, line 108
    ket = cursor;
    // substring, line 108
    among_var = find_among_b(a_3, 5);
    if (among_var == 0) {
      goto lab0Break;
    }
    // ], line 108
    bra = cursor;
    switch (among_var) {
    case 0:
      goto lab0Break;
    case 1:
      // (, line 110
      // call R1, line 110
      if (!r_R1()) {
        goto lab0Break;
      }
      // <-, line 110
      slice_from(L"heid");
      break;
    case 2:
      // (, line 113
      // call en_ending, line 113
      if (!r_en_ending()) {
        goto lab0Break;
      }
      break;
    case 3:
      // (, line 116
      // call R1, line 116
      if (!r_R1()) {
        goto lab0Break;
      }
      if (!(out_grouping_b(g_v_j, 97, 232))) {
        goto lab0Break;
      }
      // delete, line 116
      slice_del();
      break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  // do, line 120
  v_2 = limit - cursor;
  do {
    // call e_ending, line 120
    if (!r_e_ending()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 122
  v_3 = limit - cursor;
  do {
    // (, line 122
    // [, line 122
    ket = cursor;
    // literal, line 122
    if (!(eq_s_b(4, L"heid"))) {
      goto lab2Break;
    }
    // ], line 122
    bra = cursor;
    // call R2, line 122
    if (!r_R2()) {
      goto lab2Break;
    }
    {
      // not, line 122
      v_4 = limit - cursor;
      do {
        // literal, line 122
        if (!(eq_s_b(1, L"c"))) {
          goto lab3Break;
        }
        goto lab2Break;
      } while (false);
    lab3Continue:;
    lab3Break:
      cursor = limit - v_4;
    }
    // delete, line 122
    slice_del();
    // [, line 123
    ket = cursor;
    // literal, line 123
    if (!(eq_s_b(2, L"en"))) {
      goto lab2Break;
    }
    // ], line 123
    bra = cursor;
    // call en_ending, line 123
    if (!r_en_ending()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 126
  v_5 = limit - cursor;
  do {
    // (, line 126
    // [, line 127
    ket = cursor;
    // substring, line 127
    among_var = find_among_b(a_4, 6);
    if (among_var == 0) {
      goto lab4Break;
    }
    // ], line 127
    bra = cursor;
    switch (among_var) {
    case 0:
      goto lab4Break;
    case 1:
      // (, line 129
      // call R2, line 129
      if (!r_R2()) {
        goto lab4Break;
      }
      // delete, line 129
      slice_del();
      // or, line 130
      do {
        v_6 = limit - cursor;
        do {
          // (, line 130
          // [, line 130
          ket = cursor;
          // literal, line 130
          if (!(eq_s_b(2, L"ig"))) {
            goto lab6Break;
          }
          // ], line 130
          bra = cursor;
          // call R2, line 130
          if (!r_R2()) {
            goto lab6Break;
          }
          {
            // not, line 130
            v_7 = limit - cursor;
            do {
              // literal, line 130
              if (!(eq_s_b(1, L"e"))) {
                goto lab7Break;
              }
              goto lab6Break;
            } while (false);
          lab7Continue:;
          lab7Break:
            cursor = limit - v_7;
          }
          // delete, line 130
          slice_del();
          goto lab5Break;
        } while (false);
      lab6Continue:;
      lab6Break:
        cursor = limit - v_6;
        // call undouble, line 130
        if (!r_undouble()) {
          goto lab4Break;
        }
      } while (false);
    lab5Continue:;
    lab5Break:
      break;
    case 2:
      // (, line 133
      // call R2, line 133
      if (!r_R2()) {
        goto lab4Break;
      }
      {
        // not, line 133
        v_8 = limit - cursor;
        do {
          // literal, line 133
          if (!(eq_s_b(1, L"e"))) {
            goto lab8Break;
          }
          goto lab4Break;
        } while (false);
      lab8Continue:;
      lab8Break:
        cursor = limit - v_8;
      }
      // delete, line 133
      slice_del();
      break;
    case 3:
      // (, line 136
      // call R2, line 136
      if (!r_R2()) {
        goto lab4Break;
      }
      // delete, line 136
      slice_del();
      // call e_ending, line 136
      if (!r_e_ending()) {
        goto lab4Break;
      }
      break;
    case 4:
      // (, line 139
      // call R2, line 139
      if (!r_R2()) {
        goto lab4Break;
      }
      // delete, line 139
      slice_del();
      break;
    case 5:
      // (, line 142
      // call R2, line 142
      if (!r_R2()) {
        goto lab4Break;
      }
      // Boolean test e_found, line 142
      if (!(B_e_found)) {
        goto lab4Break;
      }
      // delete, line 142
      slice_del();
      break;
    }
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_5;
  // do, line 146
  v_9 = limit - cursor;
  do {
    // (, line 146
    if (!(out_grouping_b(g_v_I, 73, 232))) {
      goto lab9Break;
    }
    // test, line 148
    v_10 = limit - cursor;
    // (, line 148
    // among, line 149
    if (find_among_b(a_5, 4) == 0) {
      goto lab9Break;
    }
    if (!(out_grouping_b(g_v, 97, 232))) {
      goto lab9Break;
    }
    cursor = limit - v_10;
    // [, line 152
    ket = cursor;
    // next, line 152
    if (cursor <= limit_backward) {
      goto lab9Break;
    }
    cursor--;
    // ], line 152
    bra = cursor;
    // delete, line 152
    slice_del();
  } while (false);
lab9Continue:;
lab9Break:
  cursor = limit - v_9;
  return true;
}

bool DutchStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 157
  // do, line 159
  v_1 = cursor;
  do {
    // call prelude, line 159
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 160
  v_2 = cursor;
  do {
    // call mark_regions, line 160
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 161
  limit_backward = cursor;
  cursor = limit;
  // do, line 162
  v_3 = limit - cursor;
  do {
    // call standard_suffix, line 162
    if (!r_standard_suffix()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  cursor = limit_backward; // do, line 163
  v_4 = cursor;
  do {
    // call postlude, line 163
    if (!r_postlude()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = v_4;
  return true;
}

bool DutchStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<DutchStemmer>(o) != nullptr;
}

int DutchStemmer::hashCode()
{
  return DutchStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext