using namespace std;

#include "FrenchStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    FrenchStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_0 = {make_shared<org::tartarus::snowball::Among>(
                              L"col", -1, -1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"par", -1, -1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"tap", -1, -1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                              L"", -1, 4, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"I", 0, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"U", 0, 2, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"Y", 0, 3, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"iqU", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abl", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"I\u00E8r", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00E8r", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eus", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iv", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                              L"ic", -1, 2, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"abil", -1, 1, L"", methodObject),
                          make_shared<org::tartarus::snowball::Among>(
                              L"iv", -1, 3, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"iqUe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atrice", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ance", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ence", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logie", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"able", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isme", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"euse", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ive", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"if", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"usion", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ation", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ution", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ateur", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iqUes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atrices", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ances", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ences", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logies", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ables", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ismes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"euses", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ives", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ifs", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"usions", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ations", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"utions", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ateurs", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ments", -1, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ements", 30, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issements", 31, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it\u00E9s", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ment", -1, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ement", 34, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issement", 35, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amment", 34, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"emment", 34, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aux", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eaux", 39, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eux", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it\u00E9", -1, 7, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_5 = {
        make_shared<org::tartarus::snowball::Among>(L"ira", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ie", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isse", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issante", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irai", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iras", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EEmes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isses", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issantes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EEtes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"is", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irais", 13, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issais", 13, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irions", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issions", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irons", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issons", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issants", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irait", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issait", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issant", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iraIent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issaIent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iront", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EEt", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iriez", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issiez", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irez", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issez", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"era", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asse", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ante", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9e", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ai", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erai", 5, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eras", 8, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2mes", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asses", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antes", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2tes", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9es", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ais", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erais", 15, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ions", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erions", 17, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assions", 17, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erons", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ants", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9s", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ait", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erait", 23, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aIent", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eraIent", 26, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8rent", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assent", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eront", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2t", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ez", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iez", 32, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eriez", 33, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assiez", 33, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erez", 32, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9", -1, 2, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_7 = {
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"I\u00E8re", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00E8re", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ion", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"Ier", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ier", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EB", -1, 4, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    FrenchStemmer::a_8 = {
        make_shared<org::tartarus::snowball::Among>(L"ell", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eill", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"onn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ett", -1, -1, L"",
                                                    methodObject)};
std::deque<wchar_t> const FrenchStemmer::g_v = {
    17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 130, 103, 8, 5};
std::deque<wchar_t> const FrenchStemmer::g_keep_with_s = {
    1, 65, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128};

void FrenchStemmer::copy_from(shared_ptr<FrenchStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool FrenchStemmer::r_prelude()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // repeat, line 38
  while (true) {
    v_1 = cursor;
    do {
      // goto, line 38
      while (true) {
        v_2 = cursor;
        do {
          // (, line 38
          // or, line 44
          do {
            v_3 = cursor;
            do {
              // (, line 40
              if (!(in_grouping(g_v, 97, 251))) {
                goto lab5Break;
              }
              // [, line 40
              bra = cursor;
              // or, line 40
              do {
                v_4 = cursor;
                do {
                  // (, line 40
                  // literal, line 40
                  if (!(eq_s(1, L"u"))) {
                    goto lab7Break;
                  }
                  // ], line 40
                  ket = cursor;
                  if (!(in_grouping(g_v, 97, 251))) {
                    goto lab7Break;
                  }
                  // <-, line 40
                  slice_from(L"U");
                  goto lab6Break;
                } while (false);
              lab7Continue:;
              lab7Break:
                cursor = v_4;
                do {
                  // (, line 41
                  // literal, line 41
                  if (!(eq_s(1, L"i"))) {
                    goto lab8Break;
                  }
                  // ], line 41
                  ket = cursor;
                  if (!(in_grouping(g_v, 97, 251))) {
                    goto lab8Break;
                  }
                  // <-, line 41
                  slice_from(L"I");
                  goto lab6Break;
                } while (false);
              lab8Continue:;
              lab8Break:
                cursor = v_4;
                // (, line 42
                // literal, line 42
                if (!(eq_s(1, L"y"))) {
                  goto lab5Break;
                }
                // ], line 42
                ket = cursor;
                // <-, line 42
                slice_from(L"Y");
              } while (false);
            lab6Continue:;
            lab6Break:
              goto lab4Break;
            } while (false);
          lab5Continue:;
          lab5Break:
            cursor = v_3;
            do {
              // (, line 45
              // [, line 45
              bra = cursor;
              // literal, line 45
              if (!(eq_s(1, L"y"))) {
                goto lab9Break;
              }
              // ], line 45
              ket = cursor;
              if (!(in_grouping(g_v, 97, 251))) {
                goto lab9Break;
              }
              // <-, line 45
              slice_from(L"Y");
              goto lab4Break;
            } while (false);
          lab9Continue:;
          lab9Break:
            cursor = v_3;
            // (, line 47
            // literal, line 47
            if (!(eq_s(1, L"q"))) {
              goto lab3Break;
            }
            // [, line 47
            bra = cursor;
            // literal, line 47
            if (!(eq_s(1, L"u"))) {
              goto lab3Break;
            }
            // ], line 47
            ket = cursor;
            // <-, line 47
            slice_from(L"U");
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = v_2;
          goto golab2Break;
        } while (false);
      lab3Continue:;
      lab3Break:
        cursor = v_2;
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
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool FrenchStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  int v_4;
  // (, line 50
  I_pV = limit;
  I_p1 = limit;
  I_p2 = limit;
  // do, line 56
  v_1 = cursor;
  do {
    // (, line 56
    // or, line 58
    do {
      v_2 = cursor;
      do {
        // (, line 57
        if (!(in_grouping(g_v, 97, 251))) {
          goto lab2Break;
        }
        if (!(in_grouping(g_v, 97, 251))) {
          goto lab2Break;
        }
        // next, line 57
        if (cursor >= limit) {
          goto lab2Break;
        }
        cursor++;
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = v_2;
      do {
        // among, line 59
        if (find_among(a_0, 3) == 0) {
          goto lab3Break;
        }
        goto lab1Break;
      } while (false);
    lab3Continue:;
    lab3Break:
      cursor = v_2;
      // (, line 66
      // next, line 66
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
      // gopast, line 66
      while (true) {
        do {
          if (!(in_grouping(g_v, 97, 251))) {
            goto lab5Break;
          }
          goto golab4Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        if (cursor >= limit) {
          goto lab0Break;
        }
        cursor++;
      golab4Continue:;
      }
    golab4Break:;
    } while (false);
  lab1Continue:;
  lab1Break:
    // setmark pV, line 67
    I_pV = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 69
  v_4 = cursor;
  do {
    // (, line 69
    // gopast, line 70
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 251))) {
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
    // gopast, line 70
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 251))) {
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
    // setmark p1, line 70
    I_p1 = cursor;
    // gopast, line 71
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 251))) {
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
    // gopast, line 71
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 251))) {
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
    // setmark p2, line 71
    I_p2 = cursor;
  } while (false);
lab6Continue:;
lab6Break:
  cursor = v_4;
  return true;
}

bool FrenchStemmer::r_postlude()
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
      among_var = find_among(a_1, 4);
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
        slice_from(L"i");
        break;
      case 2:
        // (, line 79
        // <-, line 79
        slice_from(L"u");
        break;
      case 3:
        // (, line 80
        // <-, line 80
        slice_from(L"y");
        break;
      case 4:
        // (, line 81
        // next, line 81
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

bool FrenchStemmer::r_RV()
{
  if (!(I_pV <= cursor)) {
    return false;
  }
  return true;
}

bool FrenchStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool FrenchStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool FrenchStemmer::r_standard_suffix()
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
  int v_11;
  // (, line 91
  // [, line 92
  ket = cursor;
  // substring, line 92
  among_var = find_among_b(a_4, 43);
  if (among_var == 0) {
    return false;
  }
  // ], line 92
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 96
    // call R2, line 96
    if (!r_R2()) {
      return false;
    }
    // delete, line 96
    slice_del();
    break;
  case 2:
    // (, line 99
    // call R2, line 99
    if (!r_R2()) {
      return false;
    }
    // delete, line 99
    slice_del();
    // try, line 100
    v_1 = limit - cursor;
    do {
      // (, line 100
      // [, line 100
      ket = cursor;
      // literal, line 100
      if (!(eq_s_b(2, L"ic"))) {
        cursor = limit - v_1;
        goto lab0Break;
      }
      // ], line 100
      bra = cursor;
      // or, line 100
      do {
        v_2 = limit - cursor;
        do {
          // (, line 100
          // call R2, line 100
          if (!r_R2()) {
            goto lab2Break;
          }
          // delete, line 100
          slice_del();
          goto lab1Break;
        } while (false);
      lab2Continue:;
      lab2Break:
        cursor = limit - v_2;
        // <-, line 100
        slice_from(L"iqU");
      } while (false);
    lab1Continue:;
    lab1Break:;
    } while (false);
  lab0Continue:;
  lab0Break:
    break;
  case 3:
    // (, line 104
    // call R2, line 104
    if (!r_R2()) {
      return false;
    }
    // <-, line 104
    slice_from(L"log");
    break;
  case 4:
    // (, line 107
    // call R2, line 107
    if (!r_R2()) {
      return false;
    }
    // <-, line 107
    slice_from(L"u");
    break;
  case 5:
    // (, line 110
    // call R2, line 110
    if (!r_R2()) {
      return false;
    }
    // <-, line 110
    slice_from(L"ent");
    break;
  case 6:
    // (, line 113
    // call RV, line 114
    if (!r_RV()) {
      return false;
    }
    // delete, line 114
    slice_del();
    // try, line 115
    v_3 = limit - cursor;
    do {
      // (, line 115
      // [, line 116
      ket = cursor;
      // substring, line 116
      among_var = find_among_b(a_2, 6);
      if (among_var == 0) {
        cursor = limit - v_3;
        goto lab3Break;
      }
      // ], line 116
      bra = cursor;
      switch (among_var) {
      case 0:
        cursor = limit - v_3;
        goto lab3Break;
      case 1:
        // (, line 117
        // call R2, line 117
        if (!r_R2()) {
          cursor = limit - v_3;
          goto lab3Break;
        }
        // delete, line 117
        slice_del();
        // [, line 117
        ket = cursor;
        // literal, line 117
        if (!(eq_s_b(2, L"at"))) {
          cursor = limit - v_3;
          goto lab3Break;
        }
        // ], line 117
        bra = cursor;
        // call R2, line 117
        if (!r_R2()) {
          cursor = limit - v_3;
          goto lab3Break;
        }
        // delete, line 117
        slice_del();
        break;
      case 2:
        // (, line 118
        // or, line 118
        do {
          v_4 = limit - cursor;
          do {
            // (, line 118
            // call R2, line 118
            if (!r_R2()) {
              goto lab5Break;
            }
            // delete, line 118
            slice_del();
            goto lab4Break;
          } while (false);
        lab5Continue:;
        lab5Break:
          cursor = limit - v_4;
          // (, line 118
          // call R1, line 118
          if (!r_R1()) {
            cursor = limit - v_3;
            goto lab3Break;
          }
          // <-, line 118
          slice_from(L"eux");
        } while (false);
      lab4Continue:;
      lab4Break:
        break;
      case 3:
        // (, line 120
        // call R2, line 120
        if (!r_R2()) {
          cursor = limit - v_3;
          goto lab3Break;
        }
        // delete, line 120
        slice_del();
        break;
      case 4:
        // (, line 122
        // call RV, line 122
        if (!r_RV()) {
          cursor = limit - v_3;
          goto lab3Break;
        }
        // <-, line 122
        slice_from(L"i");
        break;
      }
    } while (false);
  lab3Continue:;
  lab3Break:
    break;
  case 7:
    // (, line 128
    // call R2, line 129
    if (!r_R2()) {
      return false;
    }
    // delete, line 129
    slice_del();
    // try, line 130
    v_5 = limit - cursor;
    do {
      // (, line 130
      // [, line 131
      ket = cursor;
      // substring, line 131
      among_var = find_among_b(a_3, 3);
      if (among_var == 0) {
        cursor = limit - v_5;
        goto lab6Break;
      }
      // ], line 131
      bra = cursor;
      switch (among_var) {
      case 0:
        cursor = limit - v_5;
        goto lab6Break;
      case 1:
        // (, line 132
        // or, line 132
        do {
          v_6 = limit - cursor;
          do {
            // (, line 132
            // call R2, line 132
            if (!r_R2()) {
              goto lab8Break;
            }
            // delete, line 132
            slice_del();
            goto lab7Break;
          } while (false);
        lab8Continue:;
        lab8Break:
          cursor = limit - v_6;
          // <-, line 132
          slice_from(L"abl");
        } while (false);
      lab7Continue:;
      lab7Break:
        break;
      case 2:
        // (, line 133
        // or, line 133
        do {
          v_7 = limit - cursor;
          do {
            // (, line 133
            // call R2, line 133
            if (!r_R2()) {
              goto lab10Break;
            }
            // delete, line 133
            slice_del();
            goto lab9Break;
          } while (false);
        lab10Continue:;
        lab10Break:
          cursor = limit - v_7;
          // <-, line 133
          slice_from(L"iqU");
        } while (false);
      lab9Continue:;
      lab9Break:
        break;
      case 3:
        // (, line 134
        // call R2, line 134
        if (!r_R2()) {
          cursor = limit - v_5;
          goto lab6Break;
        }
        // delete, line 134
        slice_del();
        break;
      }
    } while (false);
  lab6Continue:;
  lab6Break:
    break;
  case 8:
    // (, line 140
    // call R2, line 141
    if (!r_R2()) {
      return false;
    }
    // delete, line 141
    slice_del();
    // try, line 142
    v_8 = limit - cursor;
    do {
      // (, line 142
      // [, line 142
      ket = cursor;
      // literal, line 142
      if (!(eq_s_b(2, L"at"))) {
        cursor = limit - v_8;
        goto lab11Break;
      }
      // ], line 142
      bra = cursor;
      // call R2, line 142
      if (!r_R2()) {
        cursor = limit - v_8;
        goto lab11Break;
      }
      // delete, line 142
      slice_del();
      // [, line 142
      ket = cursor;
      // literal, line 142
      if (!(eq_s_b(2, L"ic"))) {
        cursor = limit - v_8;
        goto lab11Break;
      }
      // ], line 142
      bra = cursor;
      // or, line 142
      do {
        v_9 = limit - cursor;
        do {
          // (, line 142
          // call R2, line 142
          if (!r_R2()) {
            goto lab13Break;
          }
          // delete, line 142
          slice_del();
          goto lab12Break;
        } while (false);
      lab13Continue:;
      lab13Break:
        cursor = limit - v_9;
        // <-, line 142
        slice_from(L"iqU");
      } while (false);
    lab12Continue:;
    lab12Break:;
    } while (false);
  lab11Continue:;
  lab11Break:
    break;
  case 9:
    // (, line 144
    // <-, line 144
    slice_from(L"eau");
    break;
  case 10:
    // (, line 145
    // call R1, line 145
    if (!r_R1()) {
      return false;
    }
    // <-, line 145
    slice_from(L"al");
    break;
  case 11:
    // (, line 147
    // or, line 147
    do {
      v_10 = limit - cursor;
      do {
        // (, line 147
        // call R2, line 147
        if (!r_R2()) {
          goto lab15Break;
        }
        // delete, line 147
        slice_del();
        goto lab14Break;
      } while (false);
    lab15Continue:;
    lab15Break:
      cursor = limit - v_10;
      // (, line 147
      // call R1, line 147
      if (!r_R1()) {
        return false;
      }
      // <-, line 147
      slice_from(L"eux");
    } while (false);
  lab14Continue:;
  lab14Break:
    break;
  case 12:
    // (, line 150
    // call R1, line 150
    if (!r_R1()) {
      return false;
    }
    if (!(out_grouping_b(g_v, 97, 251))) {
      return false;
    }
    // delete, line 150
    slice_del();
    break;
  case 13:
    // (, line 155
    // call RV, line 155
    if (!r_RV()) {
      return false;
    }
    // fail, line 155
    // (, line 155
    // <-, line 155
    slice_from(L"ant");
    return false;
  case 14:
    // (, line 156
    // call RV, line 156
    if (!r_RV()) {
      return false;
    }
    // fail, line 156
    // (, line 156
    // <-, line 156
    slice_from(L"ent");
    return false;
  case 15:
    // (, line 158
    // test, line 158
    v_11 = limit - cursor;
    // (, line 158
    if (!(in_grouping_b(g_v, 97, 251))) {
      return false;
    }
    // call RV, line 158
    if (!r_RV()) {
      return false;
    }
    cursor = limit - v_11;
    // fail, line 158
    // (, line 158
    // delete, line 158
    slice_del();
    return false;
  }
  return true;
}

bool FrenchStemmer::r_i_verb_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  // setlimit, line 163
  v_1 = limit - cursor;
  // tomark, line 163
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 163
  // [, line 164
  ket = cursor;
  // substring, line 164
  among_var = find_among_b(a_5, 35);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 164
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_2;
    return false;
  case 1:
    // (, line 170
    if (!(out_grouping_b(g_v, 97, 251))) {
      limit_backward = v_2;
      return false;
    }
    // delete, line 170
    slice_del();
    break;
  }
  limit_backward = v_2;
  return true;
}

bool FrenchStemmer::r_verb_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  // setlimit, line 174
  v_1 = limit - cursor;
  // tomark, line 174
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 174
  // [, line 175
  ket = cursor;
  // substring, line 175
  among_var = find_among_b(a_6, 38);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 175
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_2;
    return false;
  case 1:
    // (, line 177
    // call R2, line 177
    if (!r_R2()) {
      limit_backward = v_2;
      return false;
    }
    // delete, line 177
    slice_del();
    break;
  case 2:
    // (, line 185
    // delete, line 185
    slice_del();
    break;
  case 3:
    // (, line 190
    // delete, line 190
    slice_del();
    // try, line 191
    v_3 = limit - cursor;
    do {
      // (, line 191
      // [, line 191
      ket = cursor;
      // literal, line 191
      if (!(eq_s_b(1, L"e"))) {
        cursor = limit - v_3;
        goto lab0Break;
      }
      // ], line 191
      bra = cursor;
      // delete, line 191
      slice_del();
    } while (false);
  lab0Continue:;
  lab0Break:
    break;
  }
  limit_backward = v_2;
  return true;
}

bool FrenchStemmer::r_residual_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 198
  // try, line 199
  v_1 = limit - cursor;
  do {
    // (, line 199
    // [, line 199
    ket = cursor;
    // literal, line 199
    if (!(eq_s_b(1, L"s"))) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    // ], line 199
    bra = cursor;
    // test, line 199
    v_2 = limit - cursor;
    if (!(out_grouping_b(g_keep_with_s, 97, 232))) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    cursor = limit - v_2;
    // delete, line 199
    slice_del();
  } while (false);
lab0Continue:;
lab0Break:
  // setlimit, line 200
  v_3 = limit - cursor;
  // tomark, line 200
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_4 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_3;
  // (, line 200
  // [, line 201
  ket = cursor;
  // substring, line 201
  among_var = find_among_b(a_7, 7);
  if (among_var == 0) {
    limit_backward = v_4;
    return false;
  }
  // ], line 201
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_4;
    return false;
  case 1:
    // (, line 202
    // call R2, line 202
    if (!r_R2()) {
      limit_backward = v_4;
      return false;
    }
    // or, line 202
    do {
      v_5 = limit - cursor;
      do {
        // literal, line 202
        if (!(eq_s_b(1, L"s"))) {
          goto lab2Break;
        }
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = limit - v_5;
      // literal, line 202
      if (!(eq_s_b(1, L"t"))) {
        limit_backward = v_4;
        return false;
      }
    } while (false);
  lab1Continue:;
  lab1Break:
    // delete, line 202
    slice_del();
    break;
  case 2:
    // (, line 204
    // <-, line 204
    slice_from(L"i");
    break;
  case 3:
    // (, line 205
    // delete, line 205
    slice_del();
    break;
  case 4:
    // (, line 206
    // literal, line 206
    if (!(eq_s_b(2, L"gu"))) {
      limit_backward = v_4;
      return false;
    }
    // delete, line 206
    slice_del();
    break;
  }
  limit_backward = v_4;
  return true;
}

bool FrenchStemmer::r_un_double()
{
  int v_1;
  // (, line 211
  // test, line 212
  v_1 = limit - cursor;
  // among, line 212
  if (find_among_b(a_8, 5) == 0) {
    return false;
  }
  cursor = limit - v_1;
  // [, line 212
  ket = cursor;
  // next, line 212
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // ], line 212
  bra = cursor;
  // delete, line 212
  slice_del();
  return true;
}

bool FrenchStemmer::r_un_accent()
{
  int v_3;
  // (, line 215
  // atleast, line 216
  {
    int v_1 = 1;
    // atleast, line 216
    while (true) {
      do {
        if (!(out_grouping_b(g_v, 97, 251))) {
          goto lab1Break;
        }
        v_1--;
        goto replab0Continue;
      } while (false);
    lab1Continue:;
    lab1Break:
      goto replab0Break;
    replab0Continue:;
    }
  replab0Break:
    if (v_1 > 0) {
      return false;
    }
  }
  // [, line 217
  ket = cursor;
  // or, line 217
  do {
    v_3 = limit - cursor;
    do {
      // literal, line 217
      if (!(eq_s_b(1, L"\u00E9"))) {
        goto lab3Break;
      }
      goto lab2Break;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = limit - v_3;
    // literal, line 217
    if (!(eq_s_b(1, L"\u00E8"))) {
      return false;
    }
  } while (false);
lab2Continue:;
lab2Break:
  // ], line 217
  bra = cursor;
  // <-, line 217
  slice_from(L"e");
  return true;
}

bool FrenchStemmer::stem()
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
  // (, line 221
  // do, line 223
  v_1 = cursor;
  do {
    // call prelude, line 223
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 224
  v_2 = cursor;
  do {
    // call mark_regions, line 224
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 225
  limit_backward = cursor;
  cursor = limit;
  // (, line 225
  // do, line 227
  v_3 = limit - cursor;
  do {
    // (, line 227
    // or, line 237
    do {
      v_4 = limit - cursor;
      do {
        // (, line 228
        // and, line 233
        v_5 = limit - cursor;
        // (, line 229
        // or, line 229
        do {
          v_6 = limit - cursor;
          do {
            // call standard_suffix, line 229
            if (!r_standard_suffix()) {
              goto lab6Break;
            }
            goto lab5Break;
          } while (false);
        lab6Continue:;
        lab6Break:
          cursor = limit - v_6;
          do {
            // call i_verb_suffix, line 230
            if (!r_i_verb_suffix()) {
              goto lab7Break;
            }
            goto lab5Break;
          } while (false);
        lab7Continue:;
        lab7Break:
          cursor = limit - v_6;
          // call verb_suffix, line 231
          if (!r_verb_suffix()) {
            goto lab4Break;
          }
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_5;
        // try, line 234
        v_7 = limit - cursor;
        do {
          // (, line 234
          // [, line 234
          ket = cursor;
          // or, line 234
          do {
            v_8 = limit - cursor;
            do {
              // (, line 234
              // literal, line 234
              if (!(eq_s_b(1, L"Y"))) {
                goto lab10Break;
              }
              // ], line 234
              bra = cursor;
              // <-, line 234
              slice_from(L"i");
              goto lab9Break;
            } while (false);
          lab10Continue:;
          lab10Break:
            cursor = limit - v_8;
            // (, line 235
            // literal, line 235
            if (!(eq_s_b(1, L"\u00E7"))) {
              cursor = limit - v_7;
              goto lab8Break;
            }
            // ], line 235
            bra = cursor;
            // <-, line 235
            slice_from(L"c");
          } while (false);
        lab9Continue:;
        lab9Break:;
        } while (false);
      lab8Continue:;
      lab8Break:
        goto lab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      cursor = limit - v_4;
      // call residual_suffix, line 238
      if (!r_residual_suffix()) {
        goto lab2Break;
      }
    } while (false);
  lab3Continue:;
  lab3Break:;
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 243
  v_9 = limit - cursor;
  do {
    // call un_double, line 243
    if (!r_un_double()) {
      goto lab11Break;
    }
  } while (false);
lab11Continue:;
lab11Break:
  cursor = limit - v_9;
  // do, line 244
  v_10 = limit - cursor;
  do {
    // call un_accent, line 244
    if (!r_un_accent()) {
      goto lab12Break;
    }
  } while (false);
lab12Continue:;
lab12Break:
  cursor = limit - v_10;
  cursor = limit_backward; // do, line 246
  v_11 = cursor;
  do {
    // call postlude, line 246
    if (!r_postlude()) {
      goto lab13Break;
    }
  } while (false);
lab13Continue:;
lab13Break:
  cursor = v_11;
  return true;
}

bool FrenchStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<FrenchStemmer>(o) != nullptr;
}

int FrenchStemmer::hashCode()
{
  return FrenchStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext