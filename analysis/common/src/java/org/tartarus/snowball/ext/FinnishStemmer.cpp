using namespace std;

#include "FinnishStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    FinnishStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"pa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sti", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kaan", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"han", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kin", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"h\u00E4n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k\u00E4\u00E4n", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ko", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"p\u00E4", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k\u00F6", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"lla", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"na", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ssa", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ta", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lta", 3, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sta", 3, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"ll\u00E4", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u00E4", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ss\u00E4", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00E4", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lt\u00E4", 3, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"st\u00E4", 3, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                               L"lle", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"ine", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"nsa", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mme", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nne", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ni", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"si", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"an", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E4n", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ns\u00E4", -1, 3, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_5 = {
        make_shared<org::tartarus::snowball::Among>(L"aa", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ee", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ii", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oo", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uu", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E4\u00E4", -1, -1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6\u00F6", -1, -1,
                                                    L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lla", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"na", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ssa", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ta", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lta", 4, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sta", 4, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tta", 4, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lle", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ine", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ksi", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"han", 11, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"den", 11, -1, L"r_VI",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"seen", 11, -1, L"r_LONG",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hen", 11, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tten", 11, -1, L"r_VI",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hin", 11, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"siin", 11, -1, L"r_VI",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hon", 11, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"h\u00E4n", 11, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"h\u00F6n", 11, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E4", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ll\u00E4", 22, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u00E4", 22, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ss\u00E4", 22, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00E4", 22, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lt\u00E4", 26, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"st\u00E4", 26, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tt\u00E4", 26, 9, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_7 = {
        make_shared<org::tartarus::snowball::Among>(L"eja", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mma", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imma", 1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mpa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"impa", 3, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mmi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"immi", 5, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mpi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"impi", 7, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ej\u00E4", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mm\u00E4", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imm\u00E4", 10, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mp\u00E4", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imp\u00E4", 12, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_8 = {make_shared<org::tartarus::snowball::Among>(
                               L"i", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"j", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FinnishStemmer::a_9 = {make_shared<org::tartarus::snowball::Among>(
                               L"mma", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"imma", 0, -1, L"", methodObject)};
std::deque<wchar_t> const FinnishStemmer::g_AEI = {17, 1, 0, 0, 0, 0, 0, 0, 0,
                                                    0,  0, 0, 0, 0, 0, 0, 8};
std::deque<wchar_t> const FinnishStemmer::g_V1 = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32};
std::deque<wchar_t> const FinnishStemmer::g_V2 = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32};
std::deque<wchar_t> const FinnishStemmer::g_particle_end = {
    17, 97, 24, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32};

void FinnishStemmer::copy_from(shared_ptr<FinnishStemmer> other)
{
  B_ending_removed = other->B_ending_removed;
  S_x = other->S_x;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool FinnishStemmer::r_mark_regions()
{
  int v_1;
  int v_3;
  // (, line 41
  I_p1 = limit;
  I_p2 = limit;
  // goto, line 46
  while (true) {
    v_1 = cursor;
    do {
      if (!(in_grouping(g_V1, 97, 246))) {
        goto lab1Break;
      }
      cursor = v_1;
      goto golab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab0Continue:;
  }
golab0Break:
  // gopast, line 46
  while (true) {
    do {
      if (!(out_grouping(g_V1, 97, 246))) {
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
  // setmark p1, line 46
  I_p1 = cursor;
  // goto, line 47
  while (true) {
    v_3 = cursor;
    do {
      if (!(in_grouping(g_V1, 97, 246))) {
        goto lab5Break;
      }
      cursor = v_3;
      goto golab4Break;
    } while (false);
  lab5Continue:;
  lab5Break:
    cursor = v_3;
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab4Continue:;
  }
golab4Break:
  // gopast, line 47
  while (true) {
    do {
      if (!(out_grouping(g_V1, 97, 246))) {
        goto lab7Break;
      }
      goto golab6Break;
    } while (false);
  lab7Continue:;
  lab7Break:
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab6Continue:;
  }
golab6Break:
  // setmark p2, line 47
  I_p2 = cursor;
  return true;
}

bool FinnishStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool FinnishStemmer::r_particle_etc()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 54
  // setlimit, line 55
  v_1 = limit - cursor;
  // tomark, line 55
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 55
  // [, line 55
  ket = cursor;
  // substring, line 55
  among_var = find_among_b(a_0, 10);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 55
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 62
    if (!(in_grouping_b(g_particle_end, 97, 246))) {
      return false;
    }
    break;
  case 2:
    // (, line 64
    // call R2, line 64
    if (!r_R2()) {
      return false;
    }
    break;
  }
  // delete, line 66
  slice_del();
  return true;
}

bool FinnishStemmer::r_possessive()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  // (, line 68
  // setlimit, line 69
  v_1 = limit - cursor;
  // tomark, line 69
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 69
  // [, line 69
  ket = cursor;
  // substring, line 69
  among_var = find_among_b(a_4, 9);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 69
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 72
    // not, line 72
    {
      v_3 = limit - cursor;
      do {
        // literal, line 72
        if (!(eq_s_b(1, L"k"))) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_3;
    }
    // delete, line 72
    slice_del();
    break;
  case 2:
    // (, line 74
    // delete, line 74
    slice_del();
    // [, line 74
    ket = cursor;
    // literal, line 74
    if (!(eq_s_b(3, L"kse"))) {
      return false;
    }
    // ], line 74
    bra = cursor;
    // <-, line 74
    slice_from(L"ksi");
    break;
  case 3:
    // (, line 78
    // delete, line 78
    slice_del();
    break;
  case 4:
    // (, line 81
    // among, line 81
    if (find_among_b(a_1, 6) == 0) {
      return false;
    }
    // delete, line 81
    slice_del();
    break;
  case 5:
    // (, line 83
    // among, line 83
    if (find_among_b(a_2, 6) == 0) {
      return false;
    }
    // delete, line 84
    slice_del();
    break;
  case 6:
    // (, line 86
    // among, line 86
    if (find_among_b(a_3, 2) == 0) {
      return false;
    }
    // delete, line 86
    slice_del();
    break;
  }
  return true;
}

bool FinnishStemmer::r_LONG()
{
  // among, line 91
  if (find_among_b(a_5, 7) == 0) {
    return false;
  }
  return true;
}

bool FinnishStemmer::r_VI()
{
  // (, line 93
  // literal, line 93
  if (!(eq_s_b(1, L"i"))) {
    return false;
  }
  if (!(in_grouping_b(g_V2, 97, 246))) {
    return false;
  }
  return true;
}

bool FinnishStemmer::r_case_ending()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 95
  // setlimit, line 96
  v_1 = limit - cursor;
  // tomark, line 96
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 96
  // [, line 96
  ket = cursor;
  // substring, line 96
  among_var = find_among_b(a_6, 30);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 96
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 98
    // literal, line 98
    if (!(eq_s_b(1, L"a"))) {
      return false;
    }
    break;
  case 2:
    // (, line 99
    // literal, line 99
    if (!(eq_s_b(1, L"e"))) {
      return false;
    }
    break;
  case 3:
    // (, line 100
    // literal, line 100
    if (!(eq_s_b(1, L"i"))) {
      return false;
    }
    break;
  case 4:
    // (, line 101
    // literal, line 101
    if (!(eq_s_b(1, L"o"))) {
      return false;
    }
    break;
  case 5:
    // (, line 102
    // literal, line 102
    if (!(eq_s_b(1, L"\u00E4"))) {
      return false;
    }
    break;
  case 6:
    // (, line 103
    // literal, line 103
    if (!(eq_s_b(1, L"\u00F6"))) {
      return false;
    }
    break;
  case 7:
    // (, line 111
    // try, line 111
    v_3 = limit - cursor;
    do {
      // (, line 111
      // and, line 113
      v_4 = limit - cursor;
      // or, line 112
      do {
        v_5 = limit - cursor;
        do {
          // call LONG, line 111
          if (!r_LONG()) {
            goto lab2Break;
          }
          goto lab1Break;
        } while (false);
      lab2Continue:;
      lab2Break:
        cursor = limit - v_5;
        // literal, line 112
        if (!(eq_s_b(2, L"ie"))) {
          cursor = limit - v_3;
          goto lab0Break;
        }
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_4;
      // next, line 113
      if (cursor <= limit_backward) {
        cursor = limit - v_3;
        goto lab0Break;
      }
      cursor--;
      // ], line 113
      bra = cursor;
    } while (false);
  lab0Continue:;
  lab0Break:
    break;
  case 8:
    // (, line 119
    if (!(in_grouping_b(g_V1, 97, 246))) {
      return false;
    }
    if (!(out_grouping_b(g_V1, 97, 246))) {
      return false;
    }
    break;
  case 9:
    // (, line 121
    // literal, line 121
    if (!(eq_s_b(1, L"e"))) {
      return false;
    }
    break;
  }
  // delete, line 138
  slice_del();
  // set ending_removed, line 139
  B_ending_removed = true;
  return true;
}

bool FinnishStemmer::r_other_endings()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  // (, line 141
  // setlimit, line 142
  v_1 = limit - cursor;
  // tomark, line 142
  if (cursor < I_p2) {
    return false;
  }
  cursor = I_p2;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 142
  // [, line 142
  ket = cursor;
  // substring, line 142
  among_var = find_among_b(a_7, 14);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 142
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 146
    // not, line 146
    {
      v_3 = limit - cursor;
      do {
        // literal, line 146
        if (!(eq_s_b(2, L"po"))) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_3;
    }
    break;
  }
  // delete, line 151
  slice_del();
  return true;
}

bool FinnishStemmer::r_i_plural()
{
  int v_1;
  int v_2;
  // (, line 153
  // setlimit, line 154
  v_1 = limit - cursor;
  // tomark, line 154
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 154
  // [, line 154
  ket = cursor;
  // substring, line 154
  if (find_among_b(a_8, 2) == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 154
  bra = cursor;
  limit_backward = v_2;
  // delete, line 158
  slice_del();
  return true;
}

bool FinnishStemmer::r_t_plural()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  // (, line 160
  // setlimit, line 161
  v_1 = limit - cursor;
  // tomark, line 161
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 161
  // [, line 162
  ket = cursor;
  // literal, line 162
  if (!(eq_s_b(1, L"t"))) {
    limit_backward = v_2;
    return false;
  }
  // ], line 162
  bra = cursor;
  // test, line 162
  v_3 = limit - cursor;
  if (!(in_grouping_b(g_V1, 97, 246))) {
    limit_backward = v_2;
    return false;
  }
  cursor = limit - v_3;
  // delete, line 163
  slice_del();
  limit_backward = v_2;
  // setlimit, line 165
  v_4 = limit - cursor;
  // tomark, line 165
  if (cursor < I_p2) {
    return false;
  }
  cursor = I_p2;
  v_5 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_4;
  // (, line 165
  // [, line 165
  ket = cursor;
  // substring, line 165
  among_var = find_among_b(a_9, 2);
  if (among_var == 0) {
    limit_backward = v_5;
    return false;
  }
  // ], line 165
  bra = cursor;
  limit_backward = v_5;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 167
    // not, line 167
    {
      v_6 = limit - cursor;
      do {
        // literal, line 167
        if (!(eq_s_b(2, L"po"))) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_6;
    }
    break;
  }
  // delete, line 170
  slice_del();
  return true;
}

bool FinnishStemmer::r_tidy()
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
  // (, line 172
  // setlimit, line 173
  v_1 = limit - cursor;
  // tomark, line 173
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 173
  // do, line 174
  v_3 = limit - cursor;
  do {
    // (, line 174
    // and, line 174
    v_4 = limit - cursor;
    // call LONG, line 174
    if (!r_LONG()) {
      goto lab0Break;
    }
    cursor = limit - v_4;
    // (, line 174
    // [, line 174
    ket = cursor;
    // next, line 174
    if (cursor <= limit_backward) {
      goto lab0Break;
    }
    cursor--;
    // ], line 174
    bra = cursor;
    // delete, line 174
    slice_del();
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_3;
  // do, line 175
  v_5 = limit - cursor;
  do {
    // (, line 175
    // [, line 175
    ket = cursor;
    if (!(in_grouping_b(g_AEI, 97, 228))) {
      goto lab1Break;
    }
    // ], line 175
    bra = cursor;
    if (!(out_grouping_b(g_V1, 97, 246))) {
      goto lab1Break;
    }
    // delete, line 175
    slice_del();
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_5;
  // do, line 176
  v_6 = limit - cursor;
  do {
    // (, line 176
    // [, line 176
    ket = cursor;
    // literal, line 176
    if (!(eq_s_b(1, L"j"))) {
      goto lab2Break;
    }
    // ], line 176
    bra = cursor;
    // or, line 176
    do {
      v_7 = limit - cursor;
      do {
        // literal, line 176
        if (!(eq_s_b(1, L"o"))) {
          goto lab4Break;
        }
        goto lab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      cursor = limit - v_7;
      // literal, line 176
      if (!(eq_s_b(1, L"u"))) {
        goto lab2Break;
      }
    } while (false);
  lab3Continue:;
  lab3Break:
    // delete, line 176
    slice_del();
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_6;
  // do, line 177
  v_8 = limit - cursor;
  do {
    // (, line 177
    // [, line 177
    ket = cursor;
    // literal, line 177
    if (!(eq_s_b(1, L"o"))) {
      goto lab5Break;
    }
    // ], line 177
    bra = cursor;
    // literal, line 177
    if (!(eq_s_b(1, L"j"))) {
      goto lab5Break;
    }
    // delete, line 177
    slice_del();
  } while (false);
lab5Continue:;
lab5Break:
  cursor = limit - v_8;
  limit_backward = v_2;
  // goto, line 179
  while (true) {
    v_9 = limit - cursor;
    do {
      if (!(out_grouping_b(g_V1, 97, 246))) {
        goto lab7Break;
      }
      cursor = limit - v_9;
      goto golab6Break;
    } while (false);
  lab7Continue:;
  lab7Break:
    cursor = limit - v_9;
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
  golab6Continue:;
  }
golab6Break:
  // [, line 179
  ket = cursor;
  // next, line 179
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // ], line 179
  bra = cursor;
  // -> x, line 179
  S_x = slice_to(S_x);
  // name x, line 179
  if (!(eq_v_b(S_x))) {
    return false;
  }
  // delete, line 179
  slice_del();
  return true;
}

bool FinnishStemmer::stem()
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
  // (, line 183
  // do, line 185
  v_1 = cursor;
  do {
    // call mark_regions, line 185
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // unset ending_removed, line 186
  B_ending_removed = false;
  // backwards, line 187
  limit_backward = cursor;
  cursor = limit;
  // (, line 187
  // do, line 188
  v_2 = limit - cursor;
  do {
    // call particle_etc, line 188
    if (!r_particle_etc()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 189
  v_3 = limit - cursor;
  do {
    // call possessive, line 189
    if (!r_possessive()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 190
  v_4 = limit - cursor;
  do {
    // call case_ending, line 190
    if (!r_case_ending()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  // do, line 191
  v_5 = limit - cursor;
  do {
    // call other_endings, line 191
    if (!r_other_endings()) {
      goto lab4Break;
    }
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_5;
  // or, line 192
  do {
    v_6 = limit - cursor;
    do {
      // (, line 192
      // Boolean test ending_removed, line 192
      if (!(B_ending_removed)) {
        goto lab6Break;
      }
      // do, line 192
      v_7 = limit - cursor;
      do {
        // call i_plural, line 192
        if (!r_i_plural()) {
          goto lab7Break;
        }
      } while (false);
    lab7Continue:;
    lab7Break:
      cursor = limit - v_7;
      goto lab5Break;
    } while (false);
  lab6Continue:;
  lab6Break:
    cursor = limit - v_6;
    // do, line 192
    v_8 = limit - cursor;
    do {
      // call t_plural, line 192
      if (!r_t_plural()) {
        goto lab8Break;
      }
    } while (false);
  lab8Continue:;
  lab8Break:
    cursor = limit - v_8;
  } while (false);
lab5Continue:;
lab5Break:
  // do, line 193
  v_9 = limit - cursor;
  do {
    // call tidy, line 193
    if (!r_tidy()) {
      goto lab9Break;
    }
  } while (false);
lab9Continue:;
lab9Break:
  cursor = limit - v_9;
  cursor = limit_backward;
  return true;
}

bool FinnishStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<FinnishStemmer>(o) != nullptr;
}

int FinnishStemmer::hashCode()
{
  return FinnishStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext