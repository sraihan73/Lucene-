using namespace std;

#include "SwedishStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    SwedishStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    SwedishStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arna", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erna", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heterna", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"orna", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ad", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ade", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ande", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arne", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"are", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aste", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anden", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aren", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heten", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ern", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"heter", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"or", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arnas", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ernas", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ornas", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ades", 26, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"andes", 26, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ens", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arens", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hetens", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erns", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"andet", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"het", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ast", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    SwedishStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"dd", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gd", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dt", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gt", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kt", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tt", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    SwedishStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"ig", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lig", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"els", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fullt", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"l\u00F6st", -1, 2, L"",
                                                    methodObject)};
std::deque<wchar_t> const SwedishStemmer::g_v = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 32};
std::deque<wchar_t> const SwedishStemmer::g_s_ending = {119, 127, 149};

void SwedishStemmer::copy_from(shared_ptr<SwedishStemmer> other)
{
  I_x = other->I_x;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool SwedishStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  // (, line 26
  I_p1 = limit;
  // test, line 29
  v_1 = cursor;
  // (, line 29
  // hop, line 29
  {
    int c = cursor + 3;
    if (0 > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // setmark x, line 29
  I_x = cursor;
  cursor = v_1;
  // goto, line 30
  while (true) {
    v_2 = cursor;
    do {
      if (!(in_grouping(g_v, 97, 246))) {
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
  // gopast, line 30
  while (true) {
    do {
      if (!(out_grouping(g_v, 97, 246))) {
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
  // setmark p1, line 30
  I_p1 = cursor;
  // try, line 31
  do {
    // (, line 31
    if (!(I_p1 < I_x)) {
      goto lab4Break;
    }
    I_p1 = I_x;
  } while (false);
lab4Continue:;
lab4Break:
  return true;
}

bool SwedishStemmer::r_main_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  // (, line 36
  // setlimit, line 37
  v_1 = limit - cursor;
  // tomark, line 37
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 37
  // [, line 37
  ket = cursor;
  // substring, line 37
  among_var = find_among_b(a_0, 37);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 37
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
    if (!(in_grouping_b(g_s_ending, 98, 121))) {
      return false;
    }
    // delete, line 46
    slice_del();
    break;
  }
  return true;
}

bool SwedishStemmer::r_consonant_pair()
{
  int v_1;
  int v_2;
  int v_3;
  // setlimit, line 50
  v_1 = limit - cursor;
  // tomark, line 50
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 50
  // and, line 52
  v_3 = limit - cursor;
  // among, line 51
  if (find_among_b(a_1, 7) == 0) {
    limit_backward = v_2;
    return false;
  }
  cursor = limit - v_3;
  // (, line 52
  // [, line 52
  ket = cursor;
  // next, line 52
  if (cursor <= limit_backward) {
    limit_backward = v_2;
    return false;
  }
  cursor--;
  // ], line 52
  bra = cursor;
  // delete, line 52
  slice_del();
  limit_backward = v_2;
  return true;
}

bool SwedishStemmer::r_other_suffix()
{
  int among_var;
  int v_1;
  int v_2;
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
  // [, line 56
  ket = cursor;
  // substring, line 56
  among_var = find_among_b(a_2, 5);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 56
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_2;
    return false;
  case 1:
    // (, line 57
    // delete, line 57
    slice_del();
    break;
  case 2:
    // (, line 58
    // <-, line 58
    slice_from(L"l\u00F6s");
    break;
  case 3:
    // (, line 59
    // <-, line 59
    slice_from(L"full");
    break;
  }
  limit_backward = v_2;
  return true;
}

bool SwedishStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 64
  // do, line 66
  v_1 = cursor;
  do {
    // call mark_regions, line 66
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 67
  limit_backward = cursor;
  cursor = limit;
  // (, line 67
  // do, line 68
  v_2 = limit - cursor;
  do {
    // call main_suffix, line 68
    if (!r_main_suffix()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 69
  v_3 = limit - cursor;
  do {
    // call consonant_pair, line 69
    if (!r_consonant_pair()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 70
  v_4 = limit - cursor;
  do {
    // call other_suffix, line 70
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

bool SwedishStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<SwedishStemmer>(o) != nullptr;
}

int SwedishStemmer::hashCode()
{
  return SwedishStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext