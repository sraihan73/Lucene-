using namespace std;

#include "KpStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    KpStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<
    std::shared_ptr<org::tartarus::snowball::Among>> const KpStemmer::a_0 = {
    make_shared<org::tartarus::snowball::Among>(L"nde", -1, 7, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"en", -1, 6, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"s", -1, 2, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"'s", 2, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"es", 2, 4, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"ies", 4, 3, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"aus", 2, 5, L"",
                                                methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"de", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ge", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ische", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"je", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lijke", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"le", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ene", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"re", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"se", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"te", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieve", -1, 11, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"heid", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fie", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gie", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atie", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isme", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ing", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arij", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erij", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sel", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rder", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ster", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iteit", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dst", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tst", -1, 9, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"end", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atief", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erig", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"achtig", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioneel", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"baar", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"laar", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"naar", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"raar", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eriger", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"achtiger", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lijker", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tant", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erigst", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"achtigst", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lijkst", -1, 8, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_4 = {make_shared<org::tartarus::snowball::Among>(
                          L"ig", -1, 1, L"", methodObject),
                      make_shared<org::tartarus::snowball::Among>(
                          L"iger", -1, 1, L"", methodObject),
                      make_shared<org::tartarus::snowball::Among>(
                          L"igst", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_5 = {make_shared<org::tartarus::snowball::Among>(
                          L"ft", -1, 2, L"", methodObject),
                      make_shared<org::tartarus::snowball::Among>(
                          L"kt", -1, 1, L"", methodObject),
                      make_shared<org::tartarus::snowball::Among>(
                          L"pt", -1, 3, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"bb", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cc", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dd", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ff", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gg", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hh", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jj", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kk", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ll", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mm", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nn", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pp", -1, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qq", -1, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rr", -1, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ss", -1, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tt", -1, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"v", -1, 21, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vv", 16, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ww", -1, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"xx", -1, 19, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"z", -1, 22, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zz", 20, 20, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    KpStemmer::a_7 = {make_shared<org::tartarus::snowball::Among>(
                          L"d", -1, 1, L"", methodObject),
                      make_shared<org::tartarus::snowball::Among>(
                          L"t", -1, 2, L"", methodObject)};
std::deque<wchar_t> const KpStemmer::g_v = {17, 65, 16, 1};
std::deque<wchar_t> const KpStemmer::g_v_WX = {17, 65, 208, 1};
std::deque<wchar_t> const KpStemmer::g_AOU = {1, 64, 16};
std::deque<wchar_t> const KpStemmer::g_AIOU = {1, 65, 16};

void KpStemmer::copy_from(shared_ptr<KpStemmer> other)
{
  B_GE_removed = other->B_GE_removed;
  B_stemmed = other->B_stemmed;
  B_Y_found = other->B_Y_found;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_x = other->I_x;
  S_ch = other->S_ch;
  SnowballProgram::copy_from(other);
}

bool KpStemmer::r_R1()
{
  // (, line 32
  // setmark x, line 32
  I_x = cursor;
  if (!(I_x >= I_p1)) {
    return false;
  }
  return true;
}

bool KpStemmer::r_R2()
{
  // (, line 33
  // setmark x, line 33
  I_x = cursor;
  if (!(I_x >= I_p2)) {
    return false;
  }
  return true;
}

bool KpStemmer::r_V()
{
  int v_1;
  int v_2;
  // test, line 35
  v_1 = limit - cursor;
  // (, line 35
  // or, line 35
  do {
    v_2 = limit - cursor;
    do {
      if (!(in_grouping_b(g_v, 97, 121))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // literal, line 35
    if (!(eq_s_b(2, L"ij"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  return true;
}

bool KpStemmer::r_VX()
{
  int v_1;
  int v_2;
  // test, line 36
  v_1 = limit - cursor;
  // (, line 36
  // next, line 36
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // or, line 36
  do {
    v_2 = limit - cursor;
    do {
      if (!(in_grouping_b(g_v, 97, 121))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_2;
    // literal, line 36
    if (!(eq_s_b(2, L"ij"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  return true;
}

bool KpStemmer::r_C()
{
  int v_1;
  int v_2;
  // test, line 37
  v_1 = limit - cursor;
  // (, line 37
  // not, line 37
  {
    v_2 = limit - cursor;
    do {
      // literal, line 37
      if (!(eq_s_b(2, L"ij"))) {
        goto lab0Break;
      }
      return false;
    } while (false);
  lab0Continue:;
  lab0Break:
    cursor = limit - v_2;
  }
  if (!(out_grouping_b(g_v, 97, 121))) {
    return false;
  }
  cursor = limit - v_1;
  return true;
}

bool KpStemmer::r_lengthen_V()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  // do, line 39
  v_1 = limit - cursor;
  do {
    // (, line 39
    if (!(out_grouping_b(g_v_WX, 97, 121))) {
      goto lab0Break;
    }
    // [, line 40
    ket = cursor;
    // or, line 40
    do {
      v_2 = limit - cursor;
      do {
        // (, line 40
        if (!(in_grouping_b(g_AOU, 97, 117))) {
          goto lab2Break;
        }
        // ], line 40
        bra = cursor;
        // test, line 40
        v_3 = limit - cursor;
        // (, line 40
        // or, line 40
        do {
          v_4 = limit - cursor;
          do {
            if (!(out_grouping_b(g_v, 97, 121))) {
              goto lab4Break;
            }
            goto lab3Break;
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = limit - v_4;
          // atlimit, line 40
          if (cursor > limit_backward) {
            goto lab2Break;
          }
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = limit - v_3;
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_2;
      // (, line 41
      // literal, line 41
      if (!(eq_s_b(1, L"e"))) {
        goto lab0Break;
      }
      // ], line 41
      bra = cursor;
      // test, line 41
      v_5 = limit - cursor;
      // (, line 41
      // or, line 41
      do {
        v_6 = limit - cursor;
        do {
          if (!(out_grouping_b(g_v, 97, 121))) {
            goto lab6Break;
          }
          goto lab5Break;
        } while (false);
      lab6Continue:;
      lab6Break:
        cursor = limit - v_6;
        // atlimit, line 41
        if (cursor > limit_backward) {
          goto lab0Break;
        }
      } while (false);
    lab5Continue:;
    lab5Break : {
      // not, line 42
      v_7 = limit - cursor;
      do {
        if (!(in_grouping_b(g_AIOU, 97, 117))) {
          goto lab7Break;
        }
        goto lab0Break;
      } while (false);
    lab7Continue:;
    lab7Break:
      cursor = limit - v_7;
    }
      {
        // not, line 43
        v_8 = limit - cursor;
        do {
          // (, line 43
          // next, line 43
          if (cursor <= limit_backward) {
            goto lab8Break;
          }
          cursor--;
          if (!(in_grouping_b(g_AIOU, 97, 117))) {
            goto lab8Break;
          }
          if (!(out_grouping_b(g_v, 97, 121))) {
            goto lab8Break;
          }
          goto lab0Break;
        } while (false);
      lab8Continue:;
      lab8Break:
        cursor = limit - v_8;
      }
      cursor = limit - v_5;
    } while (false);
  lab1Continue:;
  lab1Break:
    // -> ch, line 44
    S_ch = slice_to(S_ch);
    // <+ ch, line 44
    {
      int c = cursor;
      insert(cursor, cursor, S_ch);
      cursor = c;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  return true;
}

bool KpStemmer::r_Step_1()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 48
  // [, line 49
  ket = cursor;
  // among, line 49
  among_var = find_among_b(a_0, 7);
  if (among_var == 0) {
    return false;
  }
  // (, line 49
  // ], line 49
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 51
    // delete, line 51
    slice_del();
    break;
  case 2:
    // (, line 52
    // call R1, line 52
    if (!r_R1()) {
      return false;
    }
    {
      // not, line 52
      v_1 = limit - cursor;
      do {
        // (, line 52
        // literal, line 52
        if (!(eq_s_b(1, L"t"))) {
          goto lab0Break;
        }
        // call R1, line 52
        if (!r_R1()) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_1;
    }
    // call C, line 52
    if (!r_C()) {
      return false;
    }
    // delete, line 52
    slice_del();
    break;
  case 3:
    // (, line 53
    // call R1, line 53
    if (!r_R1()) {
      return false;
    }
    // <-, line 53
    slice_from(L"ie");
    break;
  case 4:
    // (, line 55
    // or, line 55
    do {
      v_2 = limit - cursor;
      do {
        // (, line 55
        // literal, line 55
        if (!(eq_s_b(2, L"ar"))) {
          goto lab2Break;
        }
        // call R1, line 55
        if (!r_R1()) {
          goto lab2Break;
        }
        // call C, line 55
        if (!r_C()) {
          goto lab2Break;
        }
        // ], line 55
        bra = cursor;
        // delete, line 55
        slice_del();
        // call lengthen_V, line 55
        if (!r_lengthen_V()) {
          goto lab2Break;
        }
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_2;
      do {
        // (, line 56
        // literal, line 56
        if (!(eq_s_b(2, L"er"))) {
          goto lab3Break;
        }
        // call R1, line 56
        if (!r_R1()) {
          goto lab3Break;
        }
        // call C, line 56
        if (!r_C()) {
          goto lab3Break;
        }
        // ], line 56
        bra = cursor;
        // delete, line 56
        slice_del();
        goto lab1Break;
      } while (false);
    lab3Continue:;
    lab3Break:
      cursor = limit - v_2;
      // (, line 57
      // call R1, line 57
      if (!r_R1()) {
        return false;
      }
      // call C, line 57
      if (!r_C()) {
        return false;
      }
      // <-, line 57
      slice_from(L"e");
    } while (false);
  lab1Continue:;
  lab1Break:
    break;
  case 5:
    // (, line 59
    // call R1, line 59
    if (!r_R1()) {
      return false;
    }
    // call V, line 59
    if (!r_V()) {
      return false;
    }
    // <-, line 59
    slice_from(L"au");
    break;
  case 6:
    // (, line 60
    // or, line 60
    do {
      v_3 = limit - cursor;
      do {
        // (, line 60
        // literal, line 60
        if (!(eq_s_b(3, L"hed"))) {
          goto lab5Break;
        }
        // call R1, line 60
        if (!r_R1()) {
          goto lab5Break;
        }
        // ], line 60
        bra = cursor;
        // <-, line 60
        slice_from(L"heid");
        goto lab4Break;
      } while (false);
    lab5Continue:;
    lab5Break:
      cursor = limit - v_3;
      do {
        // (, line 61
        // literal, line 61
        if (!(eq_s_b(2, L"nd"))) {
          goto lab6Break;
        }
        // delete, line 61
        slice_del();
        goto lab4Break;
      } while (false);
    lab6Continue:;
    lab6Break:
      cursor = limit - v_3;
      do {
        // (, line 62
        // literal, line 62
        if (!(eq_s_b(1, L"d"))) {
          goto lab7Break;
        }
        // call R1, line 62
        if (!r_R1()) {
          goto lab7Break;
        }
        // call C, line 62
        if (!r_C()) {
          goto lab7Break;
        }
        // ], line 62
        bra = cursor;
        // delete, line 62
        slice_del();
        goto lab4Break;
      } while (false);
    lab7Continue:;
    lab7Break:
      cursor = limit - v_3;
      do {
        // (, line 63
        // or, line 63
        do {
          v_4 = limit - cursor;
          do {
            // literal, line 63
            if (!(eq_s_b(1, L"i"))) {
              goto lab10Break;
            }
            goto lab9Break;
          } while (false);
        lab10Continue:;
        lab10Break:
          cursor = limit - v_4;
          // literal, line 63
          if (!(eq_s_b(1, L"j"))) {
            goto lab8Break;
          }
        } while (false);
      lab9Continue:;
      lab9Break:
        // call V, line 63
        if (!r_V()) {
          goto lab8Break;
        }
        // delete, line 63
        slice_del();
        goto lab4Break;
      } while (false);
    lab8Continue:;
    lab8Break:
      cursor = limit - v_3;
      // (, line 64
      // call R1, line 64
      if (!r_R1()) {
        return false;
      }
      // call C, line 64
      if (!r_C()) {
        return false;
      }
      // delete, line 64
      slice_del();
      // call lengthen_V, line 64
      if (!r_lengthen_V()) {
        return false;
      }
    } while (false);
  lab4Continue:;
  lab4Break:
    break;
  case 7:
    // (, line 65
    // <-, line 65
    slice_from(L"nd");
    break;
  }
  return true;
}

bool KpStemmer::r_Step_2()
{
  int among_var;
  int v_1;
  // (, line 70
  // [, line 71
  ket = cursor;
  // among, line 71
  among_var = find_among_b(a_1, 11);
  if (among_var == 0) {
    return false;
  }
  // (, line 71
  // ], line 71
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 72
    // or, line 72
    do {
      v_1 = limit - cursor;
      do {
        // (, line 72
        // literal, line 72
        if (!(eq_s_b(2, L"'t"))) {
          goto lab1Break;
        }
        // ], line 72
        bra = cursor;
        // delete, line 72
        slice_del();
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_1;
      do {
        // (, line 73
        // literal, line 73
        if (!(eq_s_b(2, L"et"))) {
          goto lab2Break;
        }
        // ], line 73
        bra = cursor;
        // call R1, line 73
        if (!r_R1()) {
          goto lab2Break;
        }
        // call C, line 73
        if (!r_C()) {
          goto lab2Break;
        }
        // delete, line 73
        slice_del();
        goto lab0Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_1;
      do {
        // (, line 74
        // literal, line 74
        if (!(eq_s_b(3, L"rnt"))) {
          goto lab3Break;
        }
        // ], line 74
        bra = cursor;
        // <-, line 74
        slice_from(L"rn");
        goto lab0Break;
      } while (false);
    lab3Continue:;
    lab3Break:
      cursor = limit - v_1;
      do {
        // (, line 75
        // literal, line 75
        if (!(eq_s_b(1, L"t"))) {
          goto lab4Break;
        }
        // ], line 75
        bra = cursor;
        // call R1, line 75
        if (!r_R1()) {
          goto lab4Break;
        }
        // call VX, line 75
        if (!r_VX()) {
          goto lab4Break;
        }
        // delete, line 75
        slice_del();
        goto lab0Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      cursor = limit - v_1;
      do {
        // (, line 76
        // literal, line 76
        if (!(eq_s_b(3, L"ink"))) {
          goto lab5Break;
        }
        // ], line 76
        bra = cursor;
        // <-, line 76
        slice_from(L"ing");
        goto lab0Break;
      } while (false);
    lab5Continue:;
    lab5Break:
      cursor = limit - v_1;
      do {
        // (, line 77
        // literal, line 77
        if (!(eq_s_b(2, L"mp"))) {
          goto lab6Break;
        }
        // ], line 77
        bra = cursor;
        // <-, line 77
        slice_from(L"m");
        goto lab0Break;
      } while (false);
    lab6Continue:;
    lab6Break:
      cursor = limit - v_1;
      do {
        // (, line 78
        // literal, line 78
        if (!(eq_s_b(1, L"'"))) {
          goto lab7Break;
        }
        // ], line 78
        bra = cursor;
        // call R1, line 78
        if (!r_R1()) {
          goto lab7Break;
        }
        // delete, line 78
        slice_del();
        goto lab0Break;
      } while (false);
    lab7Continue:;
    lab7Break:
      cursor = limit - v_1;
      // (, line 79
      // ], line 79
      bra = cursor;
      // call R1, line 79
      if (!r_R1()) {
        return false;
      }
      // call C, line 79
      if (!r_C()) {
        return false;
      }
      // delete, line 79
      slice_del();
    } while (false);
  lab0Continue:;
  lab0Break:
    break;
  case 2:
    // (, line 80
    // call R1, line 80
    if (!r_R1()) {
      return false;
    }
    // <-, line 80
    slice_from(L"g");
    break;
  case 3:
    // (, line 81
    // call R1, line 81
    if (!r_R1()) {
      return false;
    }
    // <-, line 81
    slice_from(L"lijk");
    break;
  case 4:
    // (, line 82
    // call R1, line 82
    if (!r_R1()) {
      return false;
    }
    // <-, line 82
    slice_from(L"isch");
    break;
  case 5:
    // (, line 83
    // call R1, line 83
    if (!r_R1()) {
      return false;
    }
    // call C, line 83
    if (!r_C()) {
      return false;
    }
    // delete, line 83
    slice_del();
    break;
  case 6:
    // (, line 84
    // call R1, line 84
    if (!r_R1()) {
      return false;
    }
    // <-, line 84
    slice_from(L"t");
    break;
  case 7:
    // (, line 85
    // call R1, line 85
    if (!r_R1()) {
      return false;
    }
    // <-, line 85
    slice_from(L"s");
    break;
  case 8:
    // (, line 86
    // call R1, line 86
    if (!r_R1()) {
      return false;
    }
    // <-, line 86
    slice_from(L"r");
    break;
  case 9:
    // (, line 87
    // call R1, line 87
    if (!r_R1()) {
      return false;
    }
    // delete, line 87
    slice_del();
    // attach, line 87
    insert(cursor, cursor, L"l");
    // call lengthen_V, line 87
    if (!r_lengthen_V()) {
      return false;
    }
    break;
  case 10:
    // (, line 88
    // call R1, line 88
    if (!r_R1()) {
      return false;
    }
    // call C, line 88
    if (!r_C()) {
      return false;
    }
    // delete, line 88
    slice_del();
    // attach, line 88
    insert(cursor, cursor, L"en");
    // call lengthen_V, line 88
    if (!r_lengthen_V()) {
      return false;
    }
    break;
  case 11:
    // (, line 89
    // call R1, line 89
    if (!r_R1()) {
      return false;
    }
    // call C, line 89
    if (!r_C()) {
      return false;
    }
    // <-, line 89
    slice_from(L"ief");
    break;
  }
  return true;
}

bool KpStemmer::r_Step_3()
{
  int among_var;
  // (, line 94
  // [, line 95
  ket = cursor;
  // among, line 95
  among_var = find_among_b(a_2, 14);
  if (among_var == 0) {
    return false;
  }
  // (, line 95
  // ], line 95
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 96
    // call R1, line 96
    if (!r_R1()) {
      return false;
    }
    // <-, line 96
    slice_from(L"eer");
    break;
  case 2:
    // (, line 97
    // call R1, line 97
    if (!r_R1()) {
      return false;
    }
    // delete, line 97
    slice_del();
    // call lengthen_V, line 97
    if (!r_lengthen_V()) {
      return false;
    }
    break;
  case 3:
    // (, line 100
    // call R1, line 100
    if (!r_R1()) {
      return false;
    }
    // delete, line 100
    slice_del();
    break;
  case 4:
    // (, line 101
    // <-, line 101
    slice_from(L"r");
    break;
  case 5:
    // (, line 104
    // call R1, line 104
    if (!r_R1()) {
      return false;
    }
    // delete, line 104
    slice_del();
    // call lengthen_V, line 104
    if (!r_lengthen_V()) {
      return false;
    }
    break;
  case 6:
    // (, line 105
    // call R1, line 105
    if (!r_R1()) {
      return false;
    }
    // call C, line 105
    if (!r_C()) {
      return false;
    }
    // <-, line 105
    slice_from(L"aar");
    break;
  case 7:
    // (, line 106
    // call R2, line 106
    if (!r_R2()) {
      return false;
    }
    // delete, line 106
    slice_del();
    // attach, line 106
    insert(cursor, cursor, L"f");
    // call lengthen_V, line 106
    if (!r_lengthen_V()) {
      return false;
    }
    break;
  case 8:
    // (, line 107
    // call R2, line 107
    if (!r_R2()) {
      return false;
    }
    // delete, line 107
    slice_del();
    // attach, line 107
    insert(cursor, cursor, L"g");
    // call lengthen_V, line 107
    if (!r_lengthen_V()) {
      return false;
    }
    break;
  case 9:
    // (, line 108
    // call R1, line 108
    if (!r_R1()) {
      return false;
    }
    // call C, line 108
    if (!r_C()) {
      return false;
    }
    // <-, line 108
    slice_from(L"t");
    break;
  case 10:
    // (, line 109
    // call R1, line 109
    if (!r_R1()) {
      return false;
    }
    // call C, line 109
    if (!r_C()) {
      return false;
    }
    // <-, line 109
    slice_from(L"d");
    break;
  }
  return true;
}

bool KpStemmer::r_Step_4()
{
  int among_var;
  int v_1;
  // (, line 114
  // or, line 134
  do {
    v_1 = limit - cursor;
    do {
      // (, line 115
      // [, line 115
      ket = cursor;
      // among, line 115
      among_var = find_among_b(a_3, 16);
      if (among_var == 0) {
        goto lab1Break;
      }
      // (, line 115
      // ], line 115
      bra = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 116
        // call R1, line 116
        if (!r_R1()) {
          goto lab1Break;
        }
        // <-, line 116
        slice_from(L"ie");
        break;
      case 2:
        // (, line 117
        // call R1, line 117
        if (!r_R1()) {
          goto lab1Break;
        }
        // <-, line 117
        slice_from(L"eer");
        break;
      case 3:
        // (, line 118
        // call R1, line 118
        if (!r_R1()) {
          goto lab1Break;
        }
        // delete, line 118
        slice_del();
        break;
      case 4:
        // (, line 119
        // call R1, line 119
        if (!r_R1()) {
          goto lab1Break;
        }
        // call V, line 119
        if (!r_V()) {
          goto lab1Break;
        }
        // <-, line 119
        slice_from(L"n");
        break;
      case 5:
        // (, line 120
        // call R1, line 120
        if (!r_R1()) {
          goto lab1Break;
        }
        // call V, line 120
        if (!r_V()) {
          goto lab1Break;
        }
        // <-, line 120
        slice_from(L"l");
        break;
      case 6:
        // (, line 121
        // call R1, line 121
        if (!r_R1()) {
          goto lab1Break;
        }
        // call V, line 121
        if (!r_V()) {
          goto lab1Break;
        }
        // <-, line 121
        slice_from(L"r");
        break;
      case 7:
        // (, line 122
        // call R1, line 122
        if (!r_R1()) {
          goto lab1Break;
        }
        // <-, line 122
        slice_from(L"teer");
        break;
      case 8:
        // (, line 124
        // call R1, line 124
        if (!r_R1()) {
          goto lab1Break;
        }
        // <-, line 124
        slice_from(L"lijk");
        break;
      case 9:
        // (, line 127
        // call R1, line 127
        if (!r_R1()) {
          goto lab1Break;
        }
        // delete, line 127
        slice_del();
        break;
      case 10:
        // (, line 131
        // call R1, line 131
        if (!r_R1()) {
          goto lab1Break;
        }
        // call C, line 131
        if (!r_C()) {
          goto lab1Break;
        }
        // delete, line 131
        slice_del();
        // call lengthen_V, line 131
        if (!r_lengthen_V()) {
          goto lab1Break;
        }
        break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 135
    // [, line 135
    ket = cursor;
    // among, line 135
    among_var = find_among_b(a_4, 3);
    if (among_var == 0) {
      return false;
    }
    // (, line 135
    // ], line 135
    bra = cursor;
    switch (among_var) {
    case 0:
      return false;
    case 1:
      // (, line 138
      // call R1, line 138
      if (!r_R1()) {
        return false;
      }
      // call C, line 138
      if (!r_C()) {
        return false;
      }
      // delete, line 138
      slice_del();
      // call lengthen_V, line 138
      if (!r_lengthen_V()) {
        return false;
      }
      break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool KpStemmer::r_Step_7()
{
  int among_var;
  // (, line 144
  // [, line 145
  ket = cursor;
  // among, line 145
  among_var = find_among_b(a_5, 3);
  if (among_var == 0) {
    return false;
  }
  // (, line 145
  // ], line 145
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 146
    // <-, line 146
    slice_from(L"k");
    break;
  case 2:
    // (, line 147
    // <-, line 147
    slice_from(L"f");
    break;
  case 3:
    // (, line 148
    // <-, line 148
    slice_from(L"p");
    break;
  }
  return true;
}

bool KpStemmer::r_Step_6()
{
  int among_var;
  // (, line 153
  // [, line 154
  ket = cursor;
  // among, line 154
  among_var = find_among_b(a_6, 22);
  if (among_var == 0) {
    return false;
  }
  // (, line 154
  // ], line 154
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 155
    // <-, line 155
    slice_from(L"b");
    break;
  case 2:
    // (, line 156
    // <-, line 156
    slice_from(L"c");
    break;
  case 3:
    // (, line 157
    // <-, line 157
    slice_from(L"d");
    break;
  case 4:
    // (, line 158
    // <-, line 158
    slice_from(L"f");
    break;
  case 5:
    // (, line 159
    // <-, line 159
    slice_from(L"g");
    break;
  case 6:
    // (, line 160
    // <-, line 160
    slice_from(L"h");
    break;
  case 7:
    // (, line 161
    // <-, line 161
    slice_from(L"j");
    break;
  case 8:
    // (, line 162
    // <-, line 162
    slice_from(L"k");
    break;
  case 9:
    // (, line 163
    // <-, line 163
    slice_from(L"l");
    break;
  case 10:
    // (, line 164
    // <-, line 164
    slice_from(L"m");
    break;
  case 11:
    // (, line 165
    // <-, line 165
    slice_from(L"n");
    break;
  case 12:
    // (, line 166
    // <-, line 166
    slice_from(L"p");
    break;
  case 13:
    // (, line 167
    // <-, line 167
    slice_from(L"q");
    break;
  case 14:
    // (, line 168
    // <-, line 168
    slice_from(L"r");
    break;
  case 15:
    // (, line 169
    // <-, line 169
    slice_from(L"s");
    break;
  case 16:
    // (, line 170
    // <-, line 170
    slice_from(L"t");
    break;
  case 17:
    // (, line 171
    // <-, line 171
    slice_from(L"v");
    break;
  case 18:
    // (, line 172
    // <-, line 172
    slice_from(L"w");
    break;
  case 19:
    // (, line 173
    // <-, line 173
    slice_from(L"x");
    break;
  case 20:
    // (, line 174
    // <-, line 174
    slice_from(L"z");
    break;
  case 21:
    // (, line 175
    // <-, line 175
    slice_from(L"f");
    break;
  case 22:
    // (, line 176
    // <-, line 176
    slice_from(L"s");
    break;
  }
  return true;
}

bool KpStemmer::r_Step_1c()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 181
  // [, line 182
  ket = cursor;
  // among, line 182
  among_var = find_among_b(a_7, 2);
  if (among_var == 0) {
    return false;
  }
  // (, line 182
  // ], line 182
  bra = cursor;
  // call R1, line 182
  if (!r_R1()) {
    return false;
  }
  // call C, line 182
  if (!r_C()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 183
    // not, line 183
    {
      v_1 = limit - cursor;
      do {
        // (, line 183
        // literal, line 183
        if (!(eq_s_b(1, L"n"))) {
          goto lab0Break;
        }
        // call R1, line 183
        if (!r_R1()) {
          goto lab0Break;
        }
        return false;
      } while (false);
    lab0Continue:;
    lab0Break:
      cursor = limit - v_1;
    }
    // delete, line 183
    slice_del();
    break;
  case 2:
    // (, line 184
    // not, line 184
    {
      v_2 = limit - cursor;
      do {
        // (, line 184
        // literal, line 184
        if (!(eq_s_b(1, L"h"))) {
          goto lab1Break;
        }
        // call R1, line 184
        if (!r_R1()) {
          goto lab1Break;
        }
        return false;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_2;
    }
    // delete, line 184
    slice_del();
    break;
  }
  return true;
}

bool KpStemmer::r_Lose_prefix()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 189
  // [, line 190
  bra = cursor;
  // literal, line 190
  if (!(eq_s(2, L"ge"))) {
    return false;
  }
  // ], line 190
  ket = cursor;
  // test, line 190
  v_1 = cursor;
  // hop, line 190
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = v_1;
  // (, line 190
  // goto, line 190
  while (true) {
    v_2 = cursor;
    do {
      if (!(in_grouping(g_v, 97, 121))) {
        goto lab1Break;
      }
      cursor = v_2;
      goto golab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_2;
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab0Continue:;
  }
golab0Break:
  // goto, line 190
  while (true) {
    v_3 = cursor;
    do {
      if (!(out_grouping(g_v, 97, 121))) {
        goto lab3Break;
      }
      cursor = v_3;
      goto golab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = v_3;
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab2Continue:;
  }
golab2Break:
  // set GE_removed, line 191
  B_GE_removed = true;
  // delete, line 192
  slice_del();
  return true;
}

bool KpStemmer::r_Lose_infix()
{
  int v_2;
  int v_3;
  int v_4;
  // (, line 195
  // next, line 196
  if (cursor >= limit) {
    return false;
  }
  cursor++;
  // gopast, line 197
  while (true) {
    do {
      // (, line 197
      // [, line 197
      bra = cursor;
      // literal, line 197
      if (!(eq_s(2, L"ge"))) {
        goto lab1Break;
      }
      // ], line 197
      ket = cursor;
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
  // test, line 197
  v_2 = cursor;
  // hop, line 197
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  cursor = v_2;
  // (, line 197
  // goto, line 197
  while (true) {
    v_3 = cursor;
    do {
      if (!(in_grouping(g_v, 97, 121))) {
        goto lab3Break;
      }
      cursor = v_3;
      goto golab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = v_3;
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab2Continue:;
  }
golab2Break:
  // goto, line 197
  while (true) {
    v_4 = cursor;
    do {
      if (!(out_grouping(g_v, 97, 121))) {
        goto lab5Break;
      }
      cursor = v_4;
      goto golab4Break;
    } while (false);
  lab5Continue:;
  lab5Break:
    cursor = v_4;
    if (cursor >= limit) {
      return false;
    }
    cursor++;
  golab4Continue:;
  }
golab4Break:
  // set GE_removed, line 198
  B_GE_removed = true;
  // delete, line 199
  slice_del();
  return true;
}

bool KpStemmer::r_measure()
{
  int v_1;
  int v_2;
  int v_5;
  int v_6;
  int v_9;
  int v_10;
  // (, line 202
  // do, line 203
  v_1 = cursor;
  do {
    // (, line 203
    // tolimit, line 204
    cursor = limit;
    // setmark p1, line 205
    I_p1 = cursor;
    // setmark p2, line 206
    I_p2 = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 208
  v_2 = cursor;
  do {
    // (, line 208
    // repeat, line 209
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 121))) {
          goto lab3Break;
        }
        goto replab2Continue;
      } while (false);
    lab3Continue:;
    lab3Break:
      goto replab2Break;
    replab2Continue:;
    }
  replab2Break : {
    // atleast, line 209
    int v_4 = 1;
    // atleast, line 209
    while (true) {
      v_5 = cursor;
      do {
        // (, line 209
        // or, line 209
        do {
          v_6 = cursor;
          do {
            // literal, line 209
            if (!(eq_s(2, L"ij"))) {
              goto lab7Break;
            }
            goto lab6Break;
          } while (false);
        lab7Continue:;
        lab7Break:
          cursor = v_6;
          if (!(in_grouping(g_v, 97, 121))) {
            goto lab5Break;
          }
        } while (false);
      lab6Continue:;
      lab6Break:
        v_4--;
        goto replab4Continue;
      } while (false);
    lab5Continue:;
    lab5Break:
      cursor = v_5;
      goto replab4Break;
    replab4Continue:;
    }
  replab4Break:
    if (v_4 > 0) {
      goto lab1Break;
    }
  }
    if (!(out_grouping(g_v, 97, 121))) {
      goto lab1Break;
    }
    // setmark p1, line 209
    I_p1 = cursor;
    // repeat, line 210
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 121))) {
          goto lab9Break;
        }
        goto replab8Continue;
      } while (false);
    lab9Continue:;
    lab9Break:
      goto replab8Break;
    replab8Continue:;
    }
  replab8Break : {
    // atleast, line 210
    int v_8 = 1;
    // atleast, line 210
    while (true) {
      v_9 = cursor;
      do {
        // (, line 210
        // or, line 210
        do {
          v_10 = cursor;
          do {
            // literal, line 210
            if (!(eq_s(2, L"ij"))) {
              goto lab13Break;
            }
            goto lab12Break;
          } while (false);
        lab13Continue:;
        lab13Break:
          cursor = v_10;
          if (!(in_grouping(g_v, 97, 121))) {
            goto lab11Break;
          }
        } while (false);
      lab12Continue:;
      lab12Break:
        v_8--;
        goto replab10Continue;
      } while (false);
    lab11Continue:;
    lab11Break:
      cursor = v_9;
      goto replab10Break;
    replab10Continue:;
    }
  replab10Break:
    if (v_8 > 0) {
      goto lab1Break;
    }
  }
    if (!(out_grouping(g_v, 97, 121))) {
      goto lab1Break;
    }
    // setmark p2, line 210
    I_p2 = cursor;
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  return true;
}

bool KpStemmer::stem()
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
  int v_18;
  int v_19;
  int v_20;
  // (, line 214
  // unset Y_found, line 216
  B_Y_found = false;
  // unset stemmed, line 217
  B_stemmed = false;
  // do, line 218
  v_1 = cursor;
  do {
    // (, line 218
    // [, line 218
    bra = cursor;
    // literal, line 218
    if (!(eq_s(1, L"y"))) {
      goto lab0Break;
    }
    // ], line 218
    ket = cursor;
    // <-, line 218
    slice_from(L"Y");
    // set Y_found, line 218
    B_Y_found = true;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 219
  v_2 = cursor;
  do {
    // repeat, line 219
    while (true) {
      v_3 = cursor;
      do {
        // (, line 219
        // goto, line 219
        while (true) {
          v_4 = cursor;
          do {
            // (, line 219
            if (!(in_grouping(g_v, 97, 121))) {
              goto lab5Break;
            }
            // [, line 219
            bra = cursor;
            // literal, line 219
            if (!(eq_s(1, L"y"))) {
              goto lab5Break;
            }
            // ], line 219
            ket = cursor;
            cursor = v_4;
            goto golab4Break;
          } while (false);
        lab5Continue:;
        lab5Break:
          cursor = v_4;
          if (cursor >= limit) {
            goto lab3Break;
          }
          cursor++;
        golab4Continue:;
        }
      golab4Break:
        // <-, line 219
        slice_from(L"Y");
        // set Y_found, line 219
        B_Y_found = true;
        goto replab2Continue;
      } while (false);
    lab3Continue:;
    lab3Break:
      cursor = v_3;
      goto replab2Break;
    replab2Continue:;
    }
  replab2Break:;
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // call measure, line 221
  if (!r_measure()) {
    return false;
  }
  // backwards, line 223
  limit_backward = cursor;
  cursor = limit;
  // (, line 223
  // do, line 224
  v_5 = limit - cursor;
  do {
    // (, line 224
    // call Step_1, line 224
    if (!r_Step_1()) {
      goto lab6Break;
    }
    // set stemmed, line 224
    B_stemmed = true;
  } while (false);
lab6Continue:;
lab6Break:
  cursor = limit - v_5;
  // do, line 225
  v_6 = limit - cursor;
  do {
    // (, line 225
    // call Step_2, line 225
    if (!r_Step_2()) {
      goto lab7Break;
    }
    // set stemmed, line 225
    B_stemmed = true;
  } while (false);
lab7Continue:;
lab7Break:
  cursor = limit - v_6;
  // do, line 226
  v_7 = limit - cursor;
  do {
    // (, line 226
    // call Step_3, line 226
    if (!r_Step_3()) {
      goto lab8Break;
    }
    // set stemmed, line 226
    B_stemmed = true;
  } while (false);
lab8Continue:;
lab8Break:
  cursor = limit - v_7;
  // do, line 227
  v_8 = limit - cursor;
  do {
    // (, line 227
    // call Step_4, line 227
    if (!r_Step_4()) {
      goto lab9Break;
    }
    // set stemmed, line 227
    B_stemmed = true;
  } while (false);
lab9Continue:;
lab9Break:
  cursor = limit - v_8;
  cursor = limit_backward; // unset GE_removed, line 229
  B_GE_removed = false;
  // do, line 230
  v_9 = cursor;
  do {
    // (, line 230
    // and, line 230
    v_10 = cursor;
    // call Lose_prefix, line 230
    if (!r_Lose_prefix()) {
      goto lab10Break;
    }
    cursor = v_10;
    // call measure, line 230
    if (!r_measure()) {
      goto lab10Break;
    }
  } while (false);
lab10Continue:;
lab10Break:
  cursor = v_9;
  // backwards, line 231
  limit_backward = cursor;
  cursor = limit;
  // (, line 231
  // do, line 232
  v_11 = limit - cursor;
  do {
    // (, line 232
    // Boolean test GE_removed, line 232
    if (!(B_GE_removed)) {
      goto lab11Break;
    }
    // call Step_1c, line 232
    if (!r_Step_1c()) {
      goto lab11Break;
    }
  } while (false);
lab11Continue:;
lab11Break:
  cursor = limit - v_11;
  cursor = limit_backward; // unset GE_removed, line 234
  B_GE_removed = false;
  // do, line 235
  v_12 = cursor;
  do {
    // (, line 235
    // and, line 235
    v_13 = cursor;
    // call Lose_infix, line 235
    if (!r_Lose_infix()) {
      goto lab12Break;
    }
    cursor = v_13;
    // call measure, line 235
    if (!r_measure()) {
      goto lab12Break;
    }
  } while (false);
lab12Continue:;
lab12Break:
  cursor = v_12;
  // backwards, line 236
  limit_backward = cursor;
  cursor = limit;
  // (, line 236
  // do, line 237
  v_14 = limit - cursor;
  do {
    // (, line 237
    // Boolean test GE_removed, line 237
    if (!(B_GE_removed)) {
      goto lab13Break;
    }
    // call Step_1c, line 237
    if (!r_Step_1c()) {
      goto lab13Break;
    }
  } while (false);
lab13Continue:;
lab13Break:
  cursor = limit - v_14;
  cursor = limit_backward; // backwards, line 239
  limit_backward = cursor;
  cursor = limit;
  // (, line 239
  // do, line 240
  v_15 = limit - cursor;
  do {
    // (, line 240
    // call Step_7, line 240
    if (!r_Step_7()) {
      goto lab14Break;
    }
    // set stemmed, line 240
    B_stemmed = true;
  } while (false);
lab14Continue:;
lab14Break:
  cursor = limit - v_15;
  // do, line 241
  v_16 = limit - cursor;
  do {
    // (, line 241
    // or, line 241
    do {
      do {
        // Boolean test stemmed, line 241
        if (!(B_stemmed)) {
          goto lab17Break;
        }
        goto lab16Break;
      } while (false);
    lab17Continue:;
    lab17Break:
      // Boolean test GE_removed, line 241
      if (!(B_GE_removed)) {
        goto lab15Break;
      }
    } while (false);
  lab16Continue:;
  lab16Break:
    // call Step_6, line 241
    if (!r_Step_6()) {
      goto lab15Break;
    }
  } while (false);
lab15Continue:;
lab15Break:
  cursor = limit - v_16;
  cursor = limit_backward; // do, line 243
  v_18 = cursor;
  do {
    // (, line 243
    // Boolean test Y_found, line 243
    if (!(B_Y_found)) {
      goto lab18Break;
    }
    // repeat, line 243
    while (true) {
      v_19 = cursor;
      do {
        // (, line 243
        // goto, line 243
        while (true) {
          v_20 = cursor;
          do {
            // (, line 243
            // [, line 243
            bra = cursor;
            // literal, line 243
            if (!(eq_s(1, L"Y"))) {
              goto lab22Break;
            }
            // ], line 243
            ket = cursor;
            cursor = v_20;
            goto golab21Break;
          } while (false);
        lab22Continue:;
        lab22Break:
          cursor = v_20;
          if (cursor >= limit) {
            goto lab20Break;
          }
          cursor++;
        golab21Continue:;
        }
      golab21Break:
        // <-, line 243
        slice_from(L"y");
        goto replab19Continue;
      } while (false);
    lab20Continue:;
    lab20Break:
      cursor = v_19;
      goto replab19Break;
    replab19Continue:;
    }
  replab19Break:;
  } while (false);
lab18Continue:;
lab18Break:
  cursor = v_18;
  return true;
}

bool KpStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<KpStemmer>(o) != nullptr;
}

int KpStemmer::hashCode() { return KpStemmer::typeid->getName().hashCode(); }
} // namespace org::tartarus::snowball::ext