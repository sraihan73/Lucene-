using namespace std;

#include "DanishStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    DanishStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DanishStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"hed", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ethed", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ered", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erede", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ende", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erende", 5, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ene", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erne", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ere", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heden", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eren", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heder", 13, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erer", 13, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heds", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"endes", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erendes", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enes", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ernes", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eres", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ens", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hedens", 24, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erens", 24, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ers", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ets", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erets", 28, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"et", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eret", 30, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DanishStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                              L"gd", -1, -1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"dt", -1, -1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"gt", -1, -1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"kt", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    DanishStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"ig", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lig", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"elig", 1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"els", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"l\u00F8st", -1, 2, L"",
                                                    methodObject)};
std::deque<wchar_t> const DanishStemmer::g_v = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 128};
std::deque<wchar_t> const DanishStemmer::g_s_ending = {
    239, 254, 42, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16};

void DanishStemmer::copy_from(shared_ptr<DanishStemmer> other)
{
  I_x = other->I_x;
  I_p1 = other->I_p1;
  S_ch = other->S_ch;
  SnowballProgram::copy_from(other);
}

bool DanishStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  // (, line 29
  I_p1 = limit;
  // test, line 33
  v_1 = cursor;
  // (, line 33
  // hop, line 33
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // setmark x, line 33
  I_x = cursor;
  cursor = v_1;
  // goto, line 34
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
  // gopast, line 34
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
  // setmark p1, line 34
  I_p1 = cursor;
  // try, line 35
  do {
    // (, line 35
    if (!(I_p1 < I_x)) {
      goto lab4Break;
    }
    I_p1 = I_x;
  } while (false);
lab4Continue:;
lab4Break:
  return true;
}

bool DanishStemmer::r_main_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 40
  // setlimit, line 41
  v_1 = limit - cursor;
  // tomark, line 41
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 41
  // [, line 41
  ket = cursor;
  // substring, line 41
  among_var = find_among_b(a_0, 32);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 41
  bra = cursor;
  limit_backward = v_2;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 48
    // delete, line 48
    slice_del();
    break;
  case 2:
    // (, line 50
    if (!(in_grouping_b(g_s_ending, 97, 229))) {
      return false;
    }
    // delete, line 50
    slice_del();
    break;
  }
  return true;
}

bool DanishStemmer::r_consonant_pair()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 54
  // test, line 55
  v_1 = limit - cursor;
  // (, line 55
  // setlimit, line 56
  v_2 = limit - cursor;
  // tomark, line 56
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_3 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_2;
  // (, line 56
  // [, line 56
  ket = cursor;
  // substring, line 56
  if (find_among_b(a_1, 4) == 0) {
    limit_backward = v_3;
    return false;
  }
  // ], line 56
  bra = cursor;
  limit_backward = v_3;
  cursor = limit - v_1;
  // next, line 62
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // ], line 62
  bra = cursor;
  // delete, line 62
  slice_del();
  return true;
}

bool DanishStemmer::r_other_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 65
  // do, line 66
  v_1 = limit - cursor;
  do {
    // (, line 66
    // [, line 66
    ket = cursor;
    // literal, line 66
    if (!(eq_s_b(2, L"st"))) {
      goto lab0Break;
    }
    // ], line 66
    bra = cursor;
    // literal, line 66
    if (!(eq_s_b(2, L"ig"))) {
      goto lab0Break;
    }
    // delete, line 66
    slice_del();
  } while (false);
lab0Continue:;
lab0Break:
  cursor = limit - v_1;
  // setlimit, line 67
  v_2 = limit - cursor;
  // tomark, line 67
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_3 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_2;
  // (, line 67
  // [, line 67
  ket = cursor;
  // substring, line 67
  among_var = find_among_b(a_2, 5);
  if (among_var == 0) {
    limit_backward = v_3;
    return false;
  }
  // ], line 67
  bra = cursor;
  limit_backward = v_3;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 70
    // delete, line 70
    slice_del();
    // do, line 70
    v_4 = limit - cursor;
    do {
      // call consonant_pair, line 70
      if (!r_consonant_pair()) {
        goto lab1Break;
      }
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_4;
    break;
  case 2:
    // (, line 72
    // <-, line 72
    slice_from(L"l\u00F8s");
    break;
  }
  return true;
}

bool DanishStemmer::r_undouble()
{
  int v_1;
  int v_2;
  // (, line 75
  // setlimit, line 76
  v_1 = limit - cursor;
  // tomark, line 76
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 76
  // [, line 76
  ket = cursor;
  if (!(out_grouping_b(g_v, 97, 248))) {
    limit_backward = v_2;
    return false;
  }
  // ], line 76
  bra = cursor;
  // -> ch, line 76
  S_ch = slice_to(S_ch);
  limit_backward = v_2;
  // name ch, line 77
  if (!(eq_v_b(S_ch))) {
    return false;
  }
  // delete, line 78
  slice_del();
  return true;
}

bool DanishStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 82
  // do, line 84
  v_1 = cursor;
  do {
    // call mark_regions, line 84
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 85
  limit_backward = cursor;
  cursor = limit;
  // (, line 85
  // do, line 86
  v_2 = limit - cursor;
  do {
    // call main_suffix, line 86
    if (!r_main_suffix()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 87
  v_3 = limit - cursor;
  do {
    // call consonant_pair, line 87
    if (!r_consonant_pair()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 88
  v_4 = limit - cursor;
  do {
    // call other_suffix, line 88
    if (!r_other_suffix()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  // do, line 89
  v_5 = limit - cursor;
  do {
    // call undouble, line 89
    if (!r_undouble()) {
      goto lab4Break;
    }
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_5;
  cursor = limit_backward;
  return true;
}

bool DanishStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<DanishStemmer>(o) != nullptr;
}

int DanishStemmer::hashCode()
{
  return DanishStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext