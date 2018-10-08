using namespace std;

#include "NorwegianStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    NorwegianStemmer::methodObject =
        java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    NorwegianStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ede", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ande", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ende", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ane", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ene", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hetene", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erte", 1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heten", 9, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heter", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", 14, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", 14, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"edes", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"endes", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enes", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hetenes", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ens", 14, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hetens", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ers", 14, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ets", 14, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"et", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"het", 25, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ert", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ast", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    NorwegianStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                                 L"dt", -1, -1, L"", methodObject),
                             make_shared<org::tartarus::snowball::Among>(
                                 L"vt", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    NorwegianStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"leg", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eleg", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ig", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eig", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lig", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"elig", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"els", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lov", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"elov", 7, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"slov", 7, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hetslov", 9, 1, L"",
                                                    methodObject)};
std::deque<wchar_t> const NorwegianStemmer::g_v = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 128};
std::deque<wchar_t> const NorwegianStemmer::g_s_ending = {119, 125, 149, 1};

void NorwegianStemmer::copy_from(shared_ptr<NorwegianStemmer> other)
{
  I_x = other->I_x;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool NorwegianStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  // (, line 26
  I_p1 = limit;
  // test, line 30
  v_1 = cursor;
  // (, line 30
  // hop, line 30
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // setmark x, line 30
  I_x = cursor;
  cursor = v_1;
  // goto, line 31
  while (true) {
    v_2 = cursor;
    do {
      if (!(in_grouping(g_v, 97, 248))) {
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
  // gopast, line 31
  while (true) {
    do {
      if (!(out_grouping(g_v, 97, 248))) {
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
  // setmark p1, line 31
  I_p1 = cursor;
  // try, line 32
  do {
    // (, line 32
    if (!(I_p1 < I_x)) {
      goto lab4Break;
    }
    I_p1 = I_x;
  } while (false);
lab4Continue:;
lab4Break:
  return true;
}

bool NorwegianStemmer::r_main_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  // (, line 37
  // setlimit, line 38
  v_1 = limit - cursor;
  // tomark, line 38
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 38
  // [, line 38
  ket = cursor;
  // substring, line 38
  among_var = find_among_b(a_0, 29);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 38
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 44
    // delete, line 44
    slice_del();
    break;
  case 2:
    // (, line 46
    // or, line 46
    do {
      v_3 = limit - cursor;
      do {
        if (!(in_grouping_b(g_s_ending, 98, 122))) {
          goto lab1Break;
        }
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_3;
      // (, line 46
      // literal, line 46
      if (!(eq_s_b(1, L"k"))) {
        return false;
      }
      if (!(out_grouping_b(g_v, 97, 248))) {
        return false;
      }
    } while (false);
  lab0Continue:;
  lab0Break:
    // delete, line 46
    slice_del();
    break;
  case 3:
    // (, line 48
    // <-, line 48
    slice_from(L"er");
    break;
  }
  return true;
}

bool NorwegianStemmer::r_consonant_pair()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 52
  // test, line 53
  v_1 = limit - cursor;
  // (, line 53
  // setlimit, line 54
  v_2 = limit - cursor;
  // tomark, line 54
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_3 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_2;
  // (, line 54
  // [, line 54
  ket = cursor;
  // substring, line 54
  if (find_among_b(a_1, 2) == 0) {
    limit_backward = v_3;
    return false;
  }
  // ], line 54
  bra = cursor;
  limit_backward = v_3;
  cursor = limit - v_1;
  // next, line 59
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // ], line 59
  bra = cursor;
  // delete, line 59
  slice_del();
  return true;
}

bool NorwegianStemmer::r_other_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 62
  // setlimit, line 63
  v_1 = limit - cursor;
  // tomark, line 63
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 63
  // [, line 63
  ket = cursor;
  // substring, line 63
  among_var = find_among_b(a_2, 11);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 63
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 67
    // delete, line 67
    slice_del();
    break;
  }
  return true;
}

bool NorwegianStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 72
  // do, line 74
  v_1 = cursor;
  do {
    // call mark_regions, line 74
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 75
  limit_backward = cursor;
  cursor = limit;
  // (, line 75
  // do, line 76
  v_2 = limit - cursor;
  do {
    // call main_suffix, line 76
    if (!r_main_suffix()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 77
  v_3 = limit - cursor;
  do {
    // call consonant_pair, line 77
    if (!r_consonant_pair()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 78
  v_4 = limit - cursor;
  do {
    // call other_suffix, line 78
    if (!r_other_suffix()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  cursor = limit_backward;
  return true;
}

bool NorwegianStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<NorwegianStemmer>(o) != nullptr;
}

int NorwegianStemmer::hashCode()
{
  return NorwegianStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext