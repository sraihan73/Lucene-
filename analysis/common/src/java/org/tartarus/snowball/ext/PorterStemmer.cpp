using namespace std;

#include "PorterStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    PorterStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PorterStemmer::a_0 = {make_shared<org::tartarus::snowball::Among>(
                              L"s", -1, 3, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"ies", 0, 2, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"sses", 0, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"ss", 0, -1, L"", methodObject)};
std::deque<std::shared_ptr<
    org::tartarus::snowball::Among>> const PorterStemmer::a_1 = {
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
    PorterStemmer::a_2 = {make_shared<org::tartarus::snowball::Among>(
                              L"ed", -1, 2, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"eed", 0, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"ing", -1, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PorterStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"anci", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enci", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abli", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eli", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alli", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ousli", -1, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"entli", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aliti", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"biliti", -1, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iviti", -1, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tional", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ational", 10, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alism", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ation", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ization", 13, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"izer", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ator", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iveness", -1, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fulness", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ousness", -1, 12, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PorterStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"icate", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ative", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alize", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iciti", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ical", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ful", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ness", -1, 3, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PorterStemmer::a_5 = {
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
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ou", -1, 1, L"",
                                                    methodObject)};
std::deque<wchar_t> const PorterStemmer::g_v = {17, 65, 16, 1};
std::deque<wchar_t> const PorterStemmer::g_v_WXY = {1, 17, 65, 208, 1};

void PorterStemmer::copy_from(shared_ptr<PorterStemmer> other)
{
  B_Y_found = other->B_Y_found;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool PorterStemmer::r_shortv()
{
  // (, line 19
  if (!(out_grouping_b(g_v_WXY, 89, 121))) {
    return false;
  }
  if (!(in_grouping_b(g_v, 97, 121))) {
    return false;
  }
  if (!(out_grouping_b(g_v, 97, 121))) {
    return false;
  }
  return true;
}

bool PorterStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool PorterStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool PorterStemmer::r_Step_1a()
{
  int among_var;
  // (, line 24
  // [, line 25
  ket = cursor;
  // substring, line 25
  among_var = find_among_b(a_0, 4);
  if (among_var == 0) {
    return false;
  }
  // ], line 25
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 26
    // <-, line 26
    slice_from(L"ss");
    break;
  case 2:
    // (, line 27
    // <-, line 27
    slice_from(L"i");
    break;
  case 3:
    // (, line 29
    // delete, line 29
    slice_del();
    break;
  }
  return true;
}

bool PorterStemmer::r_Step_1b()
{
  int among_var;
  int v_1;
  int v_3;
  int v_4;
  // (, line 33
  // [, line 34
  ket = cursor;
  // substring, line 34
  among_var = find_among_b(a_2, 3);
  if (among_var == 0) {
    return false;
  }
  // ], line 34
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 35
    // call R1, line 35
    if (!r_R1()) {
      return false;
    }
    // <-, line 35
    slice_from(L"ee");
    break;
  case 2:
    // (, line 37
    // test, line 38
    v_1 = limit - cursor;
    // gopast, line 38
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
    // delete, line 38
    slice_del();
    // test, line 39
    v_3 = limit - cursor;
    // substring, line 39
    among_var = find_among_b(a_1, 13);
    if (among_var == 0) {
      return false;
    }
    cursor = limit - v_3;
    switch (among_var) {
    case 0:
      return false;
    case 1:
      // (, line 41
      // <+, line 41
      {
        int c = cursor;
        insert(cursor, cursor, L"e");
        cursor = c;
      }
      break;
    case 2:
      // (, line 44
      // [, line 44
      ket = cursor;
      // next, line 44
      if (cursor <= limit_backward) {
        return false;
      }
      cursor--;
      // ], line 44
      bra = cursor;
      // delete, line 44
      slice_del();
      break;
    case 3:
      // (, line 45
      // atmark, line 45
      if (cursor != I_p1) {
        return false;
      }
      // test, line 45
      v_4 = limit - cursor;
      // call shortv, line 45
      if (!r_shortv()) {
        return false;
      }
      cursor = limit - v_4;
      // <+, line 45
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

bool PorterStemmer::r_Step_1c()
{
  int v_1;
  // (, line 51
  // [, line 52
  ket = cursor;
  // or, line 52
  do {
    v_1 = limit - cursor;
    do {
      // literal, line 52
      if (!(eq_s_b(1, L"y"))) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // literal, line 52
    if (!(eq_s_b(1, L"Y"))) {
      return false;
    }
  } while (false);
lab0Continue:;
lab0Break:
  // ], line 52
  bra = cursor;
  // gopast, line 53
  while (true) {
    do {
      if (!(in_grouping_b(g_v, 97, 121))) {
        goto lab3Break;
      }
      goto golab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    if (cursor <= limit_backward) {
      return false;
    }
    cursor--;
  golab2Continue:;
  }
golab2Break:
  // <-, line 54
  slice_from(L"i");
  return true;
}

bool PorterStemmer::r_Step_2()
{
  int among_var;
  // (, line 57
  // [, line 58
  ket = cursor;
  // substring, line 58
  among_var = find_among_b(a_3, 20);
  if (among_var == 0) {
    return false;
  }
  // ], line 58
  bra = cursor;
  // call R1, line 58
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 59
    // <-, line 59
    slice_from(L"tion");
    break;
  case 2:
    // (, line 60
    // <-, line 60
    slice_from(L"ence");
    break;
  case 3:
    // (, line 61
    // <-, line 61
    slice_from(L"ance");
    break;
  case 4:
    // (, line 62
    // <-, line 62
    slice_from(L"able");
    break;
  case 5:
    // (, line 63
    // <-, line 63
    slice_from(L"ent");
    break;
  case 6:
    // (, line 64
    // <-, line 64
    slice_from(L"e");
    break;
  case 7:
    // (, line 66
    // <-, line 66
    slice_from(L"ize");
    break;
  case 8:
    // (, line 68
    // <-, line 68
    slice_from(L"ate");
    break;
  case 9:
    // (, line 69
    // <-, line 69
    slice_from(L"al");
    break;
  case 10:
    // (, line 71
    // <-, line 71
    slice_from(L"al");
    break;
  case 11:
    // (, line 72
    // <-, line 72
    slice_from(L"ful");
    break;
  case 12:
    // (, line 74
    // <-, line 74
    slice_from(L"ous");
    break;
  case 13:
    // (, line 76
    // <-, line 76
    slice_from(L"ive");
    break;
  case 14:
    // (, line 77
    // <-, line 77
    slice_from(L"ble");
    break;
  }
  return true;
}

bool PorterStemmer::r_Step_3()
{
  int among_var;
  // (, line 81
  // [, line 82
  ket = cursor;
  // substring, line 82
  among_var = find_among_b(a_4, 7);
  if (among_var == 0) {
    return false;
  }
  // ], line 82
  bra = cursor;
  // call R1, line 82
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 83
    // <-, line 83
    slice_from(L"al");
    break;
  case 2:
    // (, line 85
    // <-, line 85
    slice_from(L"ic");
    break;
  case 3:
    // (, line 87
    // delete, line 87
    slice_del();
    break;
  }
  return true;
}

bool PorterStemmer::r_Step_4()
{
  int among_var;
  int v_1;
  // (, line 91
  // [, line 92
  ket = cursor;
  // substring, line 92
  among_var = find_among_b(a_5, 19);
  if (among_var == 0) {
    return false;
  }
  // ], line 92
  bra = cursor;
  // call R2, line 92
  if (!r_R2()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 95
    // delete, line 95
    slice_del();
    break;
  case 2:
    // (, line 96
    // or, line 96
    do {
      v_1 = limit - cursor;
      do {
        // literal, line 96
        if (!(eq_s_b(1, L"s"))) {
          goto lab1Break;
        }
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_1;
      // literal, line 96
      if (!(eq_s_b(1, L"t"))) {
        return false;
      }
    } while (false);
  lab0Continue:;
  lab0Break:
    // delete, line 96
    slice_del();
    break;
  }
  return true;
}

bool PorterStemmer::r_Step_5a()
{
  int v_1;
  int v_2;
  // (, line 100
  // [, line 101
  ket = cursor;
  // literal, line 101
  if (!(eq_s_b(1, L"e"))) {
    return false;
  }
  // ], line 101
  bra = cursor;
  // or, line 102
  do {
    v_1 = limit - cursor;
    do {
      // call R2, line 102
      if (!r_R2()) {
        goto lab1Break;
      }
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    // (, line 102
    // call R1, line 102
    if (!r_R1()) {
      return false;
    }
    {
      // not, line 102
      v_2 = limit - cursor;
      do {
        // call shortv, line 102
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
  // delete, line 103
  slice_del();
  return true;
}

bool PorterStemmer::r_Step_5b()
{
  // (, line 106
  // [, line 107
  ket = cursor;
  // literal, line 107
  if (!(eq_s_b(1, L"l"))) {
    return false;
  }
  // ], line 107
  bra = cursor;
  // call R2, line 108
  if (!r_R2()) {
    return false;
  }
  // literal, line 108
  if (!(eq_s_b(1, L"l"))) {
    return false;
  }
  // delete, line 109
  slice_del();
  return true;
}

bool PorterStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
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
  // (, line 113
  // unset Y_found, line 115
  B_Y_found = false;
  // do, line 116
  v_1 = cursor;
  do {
    // (, line 116
    // [, line 116
    bra = cursor;
    // literal, line 116
    if (!(eq_s(1, L"y"))) {
      goto lab0Break;
    }
    // ], line 116
    ket = cursor;
    // <-, line 116
    slice_from(L"Y");
    // set Y_found, line 116
    B_Y_found = true;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 117
  v_2 = cursor;
  do {
    // repeat, line 117
    while (true) {
      v_3 = cursor;
      do {
        // (, line 117
        // goto, line 117
        while (true) {
          v_4 = cursor;
          do {
            // (, line 117
            if (!(in_grouping(g_v, 97, 121))) {
              goto lab5Break;
            }
            // [, line 117
            bra = cursor;
            // literal, line 117
            if (!(eq_s(1, L"y"))) {
              goto lab5Break;
            }
            // ], line 117
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
        // <-, line 117
        slice_from(L"Y");
        // set Y_found, line 117
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
  I_p1 = limit;
  I_p2 = limit;
  // do, line 121
  v_5 = cursor;
  do {
    // (, line 121
    // gopast, line 122
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
        goto lab6Break;
      }
      cursor++;
    golab7Continue:;
    }
  golab7Break:
    // gopast, line 122
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
        goto lab6Break;
      }
      cursor++;
    golab9Continue:;
    }
  golab9Break:
    // setmark p1, line 122
    I_p1 = cursor;
    // gopast, line 123
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 121))) {
          goto lab12Break;
        }
        goto golab11Break;
      } while (false);
    lab12Continue:;
    lab12Break:
      if (cursor >= limit) {
        goto lab6Break;
      }
      cursor++;
    golab11Continue:;
    }
  golab11Break:
    // gopast, line 123
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 121))) {
          goto lab14Break;
        }
        goto golab13Break;
      } while (false);
    lab14Continue:;
    lab14Break:
      if (cursor >= limit) {
        goto lab6Break;
      }
      cursor++;
    golab13Continue:;
    }
  golab13Break:
    // setmark p2, line 123
    I_p2 = cursor;
  } while (false);
lab6Continue:;
lab6Break:
  cursor = v_5;
  // backwards, line 126
  limit_backward = cursor;
  cursor = limit;
  // (, line 126
  // do, line 127
  v_10 = limit - cursor;
  do {
    // call Step_1a, line 127
    if (!r_Step_1a()) {
      goto lab15Break;
    }
  } while (false);
lab15Continue:;
lab15Break:
  cursor = limit - v_10;
  // do, line 128
  v_11 = limit - cursor;
  do {
    // call Step_1b, line 128
    if (!r_Step_1b()) {
      goto lab16Break;
    }
  } while (false);
lab16Continue:;
lab16Break:
  cursor = limit - v_11;
  // do, line 129
  v_12 = limit - cursor;
  do {
    // call Step_1c, line 129
    if (!r_Step_1c()) {
      goto lab17Break;
    }
  } while (false);
lab17Continue:;
lab17Break:
  cursor = limit - v_12;
  // do, line 130
  v_13 = limit - cursor;
  do {
    // call Step_2, line 130
    if (!r_Step_2()) {
      goto lab18Break;
    }
  } while (false);
lab18Continue:;
lab18Break:
  cursor = limit - v_13;
  // do, line 131
  v_14 = limit - cursor;
  do {
    // call Step_3, line 131
    if (!r_Step_3()) {
      goto lab19Break;
    }
  } while (false);
lab19Continue:;
lab19Break:
  cursor = limit - v_14;
  // do, line 132
  v_15 = limit - cursor;
  do {
    // call Step_4, line 132
    if (!r_Step_4()) {
      goto lab20Break;
    }
  } while (false);
lab20Continue:;
lab20Break:
  cursor = limit - v_15;
  // do, line 133
  v_16 = limit - cursor;
  do {
    // call Step_5a, line 133
    if (!r_Step_5a()) {
      goto lab21Break;
    }
  } while (false);
lab21Continue:;
lab21Break:
  cursor = limit - v_16;
  // do, line 134
  v_17 = limit - cursor;
  do {
    // call Step_5b, line 134
    if (!r_Step_5b()) {
      goto lab22Break;
    }
  } while (false);
lab22Continue:;
lab22Break:
  cursor = limit - v_17;
  cursor = limit_backward; // do, line 137
  v_18 = cursor;
  do {
    // (, line 137
    // Boolean test Y_found, line 137
    if (!(B_Y_found)) {
      goto lab23Break;
    }
    // repeat, line 137
    while (true) {
      v_19 = cursor;
      do {
        // (, line 137
        // goto, line 137
        while (true) {
          v_20 = cursor;
          do {
            // (, line 137
            // [, line 137
            bra = cursor;
            // literal, line 137
            if (!(eq_s(1, L"Y"))) {
              goto lab27Break;
            }
            // ], line 137
            ket = cursor;
            cursor = v_20;
            goto golab26Break;
          } while (false);
        lab27Continue:;
        lab27Break:
          cursor = v_20;
          if (cursor >= limit) {
            goto lab25Break;
          }
          cursor++;
        golab26Continue:;
        }
      golab26Break:
        // <-, line 137
        slice_from(L"y");
        goto replab24Continue;
      } while (false);
    lab25Continue:;
    lab25Break:
      cursor = v_19;
      goto replab24Break;
    replab24Continue:;
    }
  replab24Break:;
  } while (false);
lab23Continue:;
lab23Break:
  cursor = v_18;
  return true;
}

bool PorterStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<PorterStemmer>(o) != nullptr;
}

int PorterStemmer::hashCode()
{
  return PorterStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext