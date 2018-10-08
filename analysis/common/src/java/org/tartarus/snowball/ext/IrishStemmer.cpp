using namespace std;

#include "IrishStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    IrishStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    IrishStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"b'", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bh", -1, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bhf", 1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bp", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ch", -1, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d'", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d'fh", 5, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dh", -1, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dt", -1, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fh", -1, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gc", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gh", -1, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"h-", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"m'", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mb", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mh", -1, 19, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n-", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nd", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ng", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ph", -1, 20, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sh", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t-", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"th", -1, 21, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ts", -1, 12, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    IrishStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"\u00EDochta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u00EDochta", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ire", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aire", 2, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abh", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eabh", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibh", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aibh", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amh", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eamh", 8, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imh", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aimh", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDocht", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u00EDocht", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00ED", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"air\u00ED", 14, 2, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    IrishStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"\u00F3ideacha", -1, 6,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"patacha", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"achta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arcachta", 2, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eachta", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"grafa\u00EDochta", -1, 4,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"paite", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ach", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"each", 7, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3ideach", 8, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gineach", 8, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"patach", 7, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"grafa\u00EDoch", -1, 4,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pataigh", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3idigh", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acht\u00FAil", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eacht\u00FAil", 15, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gineas", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ginis", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acht", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arcacht", 19, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eacht", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"grafa\u00EDocht", -1, 4,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arcachta\u00ED", -1, 2,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"grafa\u00EDochta\u00ED",
                                                    -1, 4, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    IrishStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"imid", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aimid", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDmid", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u00EDmid", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adh", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eadh", 4, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"faidh", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fidh", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1il", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ain", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tear", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tar", -1, 2, L"",
                                                    methodObject)};
std::deque<wchar_t> const IrishStemmer::g_v = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 17, 4, 2};

void IrishStemmer::copy_from(shared_ptr<IrishStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool IrishStemmer::r_mark_regions()
{
  int v_1;
  int v_3;
  // (, line 28
  I_pV = limit;
  I_p1 = limit;
  I_p2 = limit;
  // do, line 34
  v_1 = cursor;
  do {
    // (, line 34
    // gopast, line 35
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab2Break;
        }
        goto golab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab1Continue:;
    }
  golab1Break:
    // setmark pV, line 35
    I_pV = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 37
  v_3 = cursor;
  do {
    // (, line 37
    // gopast, line 38
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab5Break;
        }
        goto golab4Break;
      } while (false);
    lab5Continue:;
    lab5Break:
      if (cursor >= limit) {
        goto lab3Break;
      }
      cursor++;
    golab4Continue:;
    }
  golab4Break:
    // gopast, line 38
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 250))) {
          goto lab7Break;
        }
        goto golab6Break;
      } while (false);
    lab7Continue:;
    lab7Break:
      if (cursor >= limit) {
        goto lab3Break;
      }
      cursor++;
    golab6Continue:;
    }
  golab6Break:
    // setmark p1, line 38
    I_p1 = cursor;
    // gopast, line 39
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab9Break;
        }
        goto golab8Break;
      } while (false);
    lab9Continue:;
    lab9Break:
      if (cursor >= limit) {
        goto lab3Break;
      }
      cursor++;
    golab8Continue:;
    }
  golab8Break:
    // gopast, line 39
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 250))) {
          goto lab11Break;
        }
        goto golab10Break;
      } while (false);
    lab11Continue:;
    lab11Break:
      if (cursor >= limit) {
        goto lab3Break;
      }
      cursor++;
    golab10Continue:;
    }
  golab10Break:
    // setmark p2, line 39
    I_p2 = cursor;
  } while (false);
lab3Continue:;
lab3Break:
  cursor = v_3;
  return true;
}

bool IrishStemmer::r_initial_morph()
{
  int among_var;
  // (, line 43
  // [, line 44
  bra = cursor;
  // substring, line 44
  among_var = find_among(a_0, 24);
  if (among_var == 0) {
    return false;
  }
  // ], line 44
  ket = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 46
    // delete, line 46
    slice_del();
    break;
  case 2:
    // (, line 50
    // delete, line 50
    slice_del();
    break;
  case 3:
    // (, line 52
    // <-, line 52
    slice_from(L"f");
    break;
  case 4:
    // (, line 55
    // delete, line 55
    slice_del();
    break;
  case 5:
    // (, line 58
    // <-, line 58
    slice_from(L"s");
    break;
  case 6:
    // (, line 61
    // <-, line 61
    slice_from(L"b");
    break;
  case 7:
    // (, line 63
    // <-, line 63
    slice_from(L"c");
    break;
  case 8:
    // (, line 65
    // <-, line 65
    slice_from(L"d");
    break;
  case 9:
    // (, line 67
    // <-, line 67
    slice_from(L"f");
    break;
  case 10:
    // (, line 69
    // <-, line 69
    slice_from(L"g");
    break;
  case 11:
    // (, line 71
    // <-, line 71
    slice_from(L"p");
    break;
  case 12:
    // (, line 73
    // <-, line 73
    slice_from(L"s");
    break;
  case 13:
    // (, line 75
    // <-, line 75
    slice_from(L"t");
    break;
  case 14:
    // (, line 79
    // <-, line 79
    slice_from(L"b");
    break;
  case 15:
    // (, line 81
    // <-, line 81
    slice_from(L"c");
    break;
  case 16:
    // (, line 83
    // <-, line 83
    slice_from(L"d");
    break;
  case 17:
    // (, line 85
    // <-, line 85
    slice_from(L"f");
    break;
  case 18:
    // (, line 87
    // <-, line 87
    slice_from(L"g");
    break;
  case 19:
    // (, line 89
    // <-, line 89
    slice_from(L"m");
    break;
  case 20:
    // (, line 91
    // <-, line 91
    slice_from(L"p");
    break;
  case 21:
    // (, line 93
    // <-, line 93
    slice_from(L"t");
    break;
  }
  return true;
}

bool IrishStemmer::r_RV()
{
  if (!(I_pV <= cursor)) {
    return false;
  }
  return true;
}

bool IrishStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool IrishStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool IrishStemmer::r_noun_sfx()
{
  int among_var;
  // (, line 103
  // [, line 104
  ket = cursor;
  // substring, line 104
  among_var = find_among_b(a_1, 16);
  if (among_var == 0) {
    return false;
  }
  // ], line 104
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 108
    // call R1, line 108
    if (!r_R1()) {
      return false;
    }
    // delete, line 108
    slice_del();
    break;
  case 2:
    // (, line 110
    // call R2, line 110
    if (!r_R2()) {
      return false;
    }
    // delete, line 110
    slice_del();
    break;
  }
  return true;
}

bool IrishStemmer::r_deriv()
{
  int among_var;
  // (, line 113
  // [, line 114
  ket = cursor;
  // substring, line 114
  among_var = find_among_b(a_2, 25);
  if (among_var == 0) {
    return false;
  }
  // ], line 114
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 116
    // call R2, line 116
    if (!r_R2()) {
      return false;
    }
    // delete, line 116
    slice_del();
    break;
  case 2:
    // (, line 118
    // <-, line 118
    slice_from(L"arc");
    break;
  case 3:
    // (, line 120
    // <-, line 120
    slice_from(L"gin");
    break;
  case 4:
    // (, line 122
    // <-, line 122
    slice_from(L"graf");
    break;
  case 5:
    // (, line 124
    // <-, line 124
    slice_from(L"paite");
    break;
  case 6:
    // (, line 126
    // <-, line 126
    slice_from(L"\u00F3id");
    break;
  }
  return true;
}

bool IrishStemmer::r_verb_sfx()
{
  int among_var;
  // (, line 129
  // [, line 130
  ket = cursor;
  // substring, line 130
  among_var = find_among_b(a_3, 12);
  if (among_var == 0) {
    return false;
  }
  // ], line 130
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 133
    // call RV, line 133
    if (!r_RV()) {
      return false;
    }
    // delete, line 133
    slice_del();
    break;
  case 2:
    // (, line 138
    // call R1, line 138
    if (!r_R1()) {
      return false;
    }
    // delete, line 138
    slice_del();
    break;
  }
  return true;
}

bool IrishStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 143
  // do, line 144
  v_1 = cursor;
  do {
    // call initial_morph, line 144
    if (!r_initial_morph()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 145
  v_2 = cursor;
  do {
    // call mark_regions, line 145
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 146
  limit_backward = cursor;
  cursor = limit;
  // (, line 146
  // do, line 147
  v_3 = limit - cursor;
  do {
    // call noun_sfx, line 147
    if (!r_noun_sfx()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 148
  v_4 = limit - cursor;
  do {
    // call deriv, line 148
    if (!r_deriv()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  // do, line 149
  v_5 = limit - cursor;
  do {
    // call verb_sfx, line 149
    if (!r_verb_sfx()) {
      goto lab4Break;
    }
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_5;
  cursor = limit_backward;
  return true;
}

bool IrishStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<IrishStemmer>(o) != nullptr;
}

int IrishStemmer::hashCode()
{
  return IrishStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext