using namespace std;

#include "HungarianStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    HungarianStemmer::methodObject =
        java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"cs", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dzs", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gy", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ly", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ny", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ty", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zs", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                                 L"\u00E1", -1, 1, L"", methodObject),
                             make_shared<org::tartarus::snowball::Among>(
                                 L"\u00E9", -1, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"bb", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cc", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dd", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ff", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gg", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jj", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kk", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ll", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mm", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pp", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rr", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ccs", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ss", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zzs", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tt", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vv", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ggy", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lly", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nny", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tty", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ssz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zz", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                                 L"al", -1, 1, L"", methodObject),
                             make_shared<org::tartarus::snowball::Among>(
                                 L"el", -1, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_4 = {
        make_shared<org::tartarus::snowball::Among>(L"ba", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ra", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"be", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"re", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ig", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nak", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nek", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"val", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vel", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ul", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u00E1l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u00E9l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"b\u00F3l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"r\u00F3l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00F3l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"b\u00F5l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"r\u00F5l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u00F5l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FCl", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"an", 19, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ban", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", 19, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ben", 22, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k\u00E9ppen", 22, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"on", 19, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6n", 19, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k\u00E9pp", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kor", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"et", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k\u00E9nt", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ank\u00E9nt", 32, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enk\u00E9nt", 32, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"onk\u00E9nt", 32, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ot", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9rt", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6t", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hez", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hoz", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"h\u00F6z", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"v\u00E1", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"v\u00E9", -1, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_5 = {
        make_shared<org::tartarus::snowball::Among>(L"\u00E1n", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1nk\u00E9nt", -1, 3,
                                                    L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"stul", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"astul", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1stul", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"st\u00FCl", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"est\u00FCl", 3, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9st\u00FCl", 3, 4,
                                                    L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_7 = {make_shared<org::tartarus::snowball::Among>(
                                 L"\u00E1", -1, 1, L"", methodObject),
                             make_shared<org::tartarus::snowball::Among>(
                                 L"\u00E9", -1, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_8 = {
        make_shared<org::tartarus::snowball::Among>(L"k", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ak", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ek", 0, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ok", 0, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1k", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9k", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6k", 0, 3, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_9 = {
        make_shared<org::tartarus::snowball::Among>(L"\u00E9i", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1\u00E9i", 0, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9\u00E9i", 0, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k\u00E9", 3, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ak\u00E9", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ek\u00E9", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ok\u00E9", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1k\u00E9", 4, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9k\u00E9", 4, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6k\u00E9", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9\u00E9", 3, 8, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_10 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ja", 0, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d", -1, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ad", 2, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ed", 2, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"od", 2, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1d", 2, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9d", 2, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F6d", 2, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"je", 9, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nk", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"unk", 11, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1nk", 11, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9nk", 11, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FCnk", 11, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uk", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"juk", 16, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1juk", 17, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FCk", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"j\u00FCk", 19, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9j\u00FCk", 20, 6,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"m", -1, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am", 22, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"em", 22, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"om", 22, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1m", 22, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9m", 22, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"o", -1, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1", -1, 19, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9", -1, 20, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    HungarianStemmer::a_11 = {
        make_shared<org::tartarus::snowball::Among>(L"id", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aid", 0, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jaid", 1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eid", 0, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jeid", 3, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1id", 0, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9id", 0, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i", -1, 15, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ai", 7, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jai", 8, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ei", 7, 14, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jei", 10, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1i", 7, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9i", 7, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itek", -1, 24, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eitek", 14, 21, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jeitek", 15, 20, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9itek", 14, 23, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ik", -1, 29, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aik", 18, 26, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jaik", 19, 25, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eik", 18, 26, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jeik", 21, 25, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1ik", 18, 27, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ik", 18, 28, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ink", -1, 20, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aink", 25, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jaink", 26, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eink", 25, 17, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jeink", 28, 16, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1ink", 25, 18, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ink", 25, 19, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aitok", -1, 21, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jaitok", 32, 20, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1itok", -1, 22, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"im", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aim", 35, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jaim", 36, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eim", 35, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jeim", 38, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1im", 35, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9im", 35, 3, L"",
                                                    methodObject)};
std::deque<wchar_t> const HungarianStemmer::g_v = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 17, 52, 14};

void HungarianStemmer::copy_from(shared_ptr<HungarianStemmer> other)
{
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool HungarianStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  int v_3;
  // (, line 44
  I_p1 = limit;
  // or, line 51
  do {
    v_1 = cursor;
    do {
      // (, line 48
      if (!(in_grouping(g_v, 97, 252))) {
        goto lab1Break;
      }
      // goto, line 48
      while (true) {
        v_2 = cursor;
        do {
          if (!(out_grouping(g_v, 97, 252))) {
            goto lab3Break;
          }
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
      // or, line 49
      do {
        v_3 = cursor;
        do {
          // among, line 49
          if (find_among(a_0, 8) == 0) {
            goto lab5Break;
          }
          goto lab4Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = v_3;
        // next, line 49
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
      } while (false);
    lab4Continue:;
    lab4Break:
      // setmark p1, line 50
      I_p1 = cursor;
      goto lab0Break;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    // (, line 53
    if (!(out_grouping(g_v, 97, 252))) {
      return false;
    }
    // gopast, line 53
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 252))) {
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
    // setmark p1, line 53
    I_p1 = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  return true;
}

bool HungarianStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool HungarianStemmer::r_v_ending()
{
  int among_var;
  // (, line 60
  // [, line 61
  ket = cursor;
  // substring, line 61
  among_var = find_among_b(a_1, 2);
  if (among_var == 0) {
    return false;
  }
  // ], line 61
  bra = cursor;
  // call R1, line 61
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 62
    // <-, line 62
    slice_from(L"a");
    break;
  case 2:
    // (, line 63
    // <-, line 63
    slice_from(L"e");
    break;
  }
  return true;
}

bool HungarianStemmer::r_double()
{
  int v_1;
  // (, line 67
  // test, line 68
  v_1 = limit - cursor;
  // among, line 68
  if (find_among_b(a_2, 23) == 0) {
    return false;
  }
  cursor = limit - v_1;
  return true;
}

bool HungarianStemmer::r_undouble()
{
  // (, line 72
  // next, line 73
  if (cursor <= limit_backward) {
    return false;
  }
  cursor--;
  // [, line 73
  ket = cursor;
  // hop, line 73
  {
    int c = cursor - 1;
    if (limit_backward > c || c > limit) {
      return false;
    }
    cursor = c;
  }
  // ], line 73
  bra = cursor;
  // delete, line 73
  slice_del();
  return true;
}

bool HungarianStemmer::r_instrum()
{
  int among_var;
  // (, line 76
  // [, line 77
  ket = cursor;
  // substring, line 77
  among_var = find_among_b(a_3, 2);
  if (among_var == 0) {
    return false;
  }
  // ], line 77
  bra = cursor;
  // call R1, line 77
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 78
    // call double, line 78
    if (!r_double()) {
      return false;
    }
    break;
  case 2:
    // (, line 79
    // call double, line 79
    if (!r_double()) {
      return false;
    }
    break;
  }
  // delete, line 81
  slice_del();
  // call undouble, line 82
  if (!r_undouble()) {
    return false;
  }
  return true;
}

bool HungarianStemmer::r_case()
{
  // (, line 86
  // [, line 87
  ket = cursor;
  // substring, line 87
  if (find_among_b(a_4, 44) == 0) {
    return false;
  }
  // ], line 87
  bra = cursor;
  // call R1, line 87
  if (!r_R1()) {
    return false;
  }
  // delete, line 111
  slice_del();
  // call v_ending, line 112
  if (!r_v_ending()) {
    return false;
  }
  return true;
}

bool HungarianStemmer::r_case_special()
{
  int among_var;
  // (, line 115
  // [, line 116
  ket = cursor;
  // substring, line 116
  among_var = find_among_b(a_5, 3);
  if (among_var == 0) {
    return false;
  }
  // ], line 116
  bra = cursor;
  // call R1, line 116
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 117
    // <-, line 117
    slice_from(L"e");
    break;
  case 2:
    // (, line 118
    // <-, line 118
    slice_from(L"a");
    break;
  case 3:
    // (, line 119
    // <-, line 119
    slice_from(L"a");
    break;
  }
  return true;
}

bool HungarianStemmer::r_case_other()
{
  int among_var;
  // (, line 123
  // [, line 124
  ket = cursor;
  // substring, line 124
  among_var = find_among_b(a_6, 6);
  if (among_var == 0) {
    return false;
  }
  // ], line 124
  bra = cursor;
  // call R1, line 124
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 125
    // delete, line 125
    slice_del();
    break;
  case 2:
    // (, line 126
    // delete, line 126
    slice_del();
    break;
  case 3:
    // (, line 127
    // <-, line 127
    slice_from(L"a");
    break;
  case 4:
    // (, line 128
    // <-, line 128
    slice_from(L"e");
    break;
  }
  return true;
}

bool HungarianStemmer::r_factive()
{
  int among_var;
  // (, line 132
  // [, line 133
  ket = cursor;
  // substring, line 133
  among_var = find_among_b(a_7, 2);
  if (among_var == 0) {
    return false;
  }
  // ], line 133
  bra = cursor;
  // call R1, line 133
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 134
    // call double, line 134
    if (!r_double()) {
      return false;
    }
    break;
  case 2:
    // (, line 135
    // call double, line 135
    if (!r_double()) {
      return false;
    }
    break;
  }
  // delete, line 137
  slice_del();
  // call undouble, line 138
  if (!r_undouble()) {
    return false;
  }
  return true;
}

bool HungarianStemmer::r_plural()
{
  int among_var;
  // (, line 141
  // [, line 142
  ket = cursor;
  // substring, line 142
  among_var = find_among_b(a_8, 7);
  if (among_var == 0) {
    return false;
  }
  // ], line 142
  bra = cursor;
  // call R1, line 142
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 143
    // <-, line 143
    slice_from(L"a");
    break;
  case 2:
    // (, line 144
    // <-, line 144
    slice_from(L"e");
    break;
  case 3:
    // (, line 145
    // delete, line 145
    slice_del();
    break;
  case 4:
    // (, line 146
    // delete, line 146
    slice_del();
    break;
  case 5:
    // (, line 147
    // delete, line 147
    slice_del();
    break;
  case 6:
    // (, line 148
    // delete, line 148
    slice_del();
    break;
  case 7:
    // (, line 149
    // delete, line 149
    slice_del();
    break;
  }
  return true;
}

bool HungarianStemmer::r_owned()
{
  int among_var;
  // (, line 153
  // [, line 154
  ket = cursor;
  // substring, line 154
  among_var = find_among_b(a_9, 12);
  if (among_var == 0) {
    return false;
  }
  // ], line 154
  bra = cursor;
  // call R1, line 154
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 155
    // delete, line 155
    slice_del();
    break;
  case 2:
    // (, line 156
    // <-, line 156
    slice_from(L"e");
    break;
  case 3:
    // (, line 157
    // <-, line 157
    slice_from(L"a");
    break;
  case 4:
    // (, line 158
    // delete, line 158
    slice_del();
    break;
  case 5:
    // (, line 159
    // <-, line 159
    slice_from(L"e");
    break;
  case 6:
    // (, line 160
    // <-, line 160
    slice_from(L"a");
    break;
  case 7:
    // (, line 161
    // delete, line 161
    slice_del();
    break;
  case 8:
    // (, line 162
    // <-, line 162
    slice_from(L"e");
    break;
  case 9:
    // (, line 163
    // delete, line 163
    slice_del();
    break;
  }
  return true;
}

bool HungarianStemmer::r_sing_owner()
{
  int among_var;
  // (, line 167
  // [, line 168
  ket = cursor;
  // substring, line 168
  among_var = find_among_b(a_10, 31);
  if (among_var == 0) {
    return false;
  }
  // ], line 168
  bra = cursor;
  // call R1, line 168
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 169
    // delete, line 169
    slice_del();
    break;
  case 2:
    // (, line 170
    // <-, line 170
    slice_from(L"a");
    break;
  case 3:
    // (, line 171
    // <-, line 171
    slice_from(L"e");
    break;
  case 4:
    // (, line 172
    // delete, line 172
    slice_del();
    break;
  case 5:
    // (, line 173
    // <-, line 173
    slice_from(L"a");
    break;
  case 6:
    // (, line 174
    // <-, line 174
    slice_from(L"e");
    break;
  case 7:
    // (, line 175
    // delete, line 175
    slice_del();
    break;
  case 8:
    // (, line 176
    // delete, line 176
    slice_del();
    break;
  case 9:
    // (, line 177
    // delete, line 177
    slice_del();
    break;
  case 10:
    // (, line 178
    // <-, line 178
    slice_from(L"a");
    break;
  case 11:
    // (, line 179
    // <-, line 179
    slice_from(L"e");
    break;
  case 12:
    // (, line 180
    // delete, line 180
    slice_del();
    break;
  case 13:
    // (, line 181
    // delete, line 181
    slice_del();
    break;
  case 14:
    // (, line 182
    // <-, line 182
    slice_from(L"a");
    break;
  case 15:
    // (, line 183
    // <-, line 183
    slice_from(L"e");
    break;
  case 16:
    // (, line 184
    // delete, line 184
    slice_del();
    break;
  case 17:
    // (, line 185
    // delete, line 185
    slice_del();
    break;
  case 18:
    // (, line 186
    // delete, line 186
    slice_del();
    break;
  case 19:
    // (, line 187
    // <-, line 187
    slice_from(L"a");
    break;
  case 20:
    // (, line 188
    // <-, line 188
    slice_from(L"e");
    break;
  }
  return true;
}

bool HungarianStemmer::r_plur_owner()
{
  int among_var;
  // (, line 192
  // [, line 193
  ket = cursor;
  // substring, line 193
  among_var = find_among_b(a_11, 42);
  if (among_var == 0) {
    return false;
  }
  // ], line 193
  bra = cursor;
  // call R1, line 193
  if (!r_R1()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 194
    // delete, line 194
    slice_del();
    break;
  case 2:
    // (, line 195
    // <-, line 195
    slice_from(L"a");
    break;
  case 3:
    // (, line 196
    // <-, line 196
    slice_from(L"e");
    break;
  case 4:
    // (, line 197
    // delete, line 197
    slice_del();
    break;
  case 5:
    // (, line 198
    // delete, line 198
    slice_del();
    break;
  case 6:
    // (, line 199
    // delete, line 199
    slice_del();
    break;
  case 7:
    // (, line 200
    // <-, line 200
    slice_from(L"a");
    break;
  case 8:
    // (, line 201
    // <-, line 201
    slice_from(L"e");
    break;
  case 9:
    // (, line 202
    // delete, line 202
    slice_del();
    break;
  case 10:
    // (, line 203
    // delete, line 203
    slice_del();
    break;
  case 11:
    // (, line 204
    // delete, line 204
    slice_del();
    break;
  case 12:
    // (, line 205
    // <-, line 205
    slice_from(L"a");
    break;
  case 13:
    // (, line 206
    // <-, line 206
    slice_from(L"e");
    break;
  case 14:
    // (, line 207
    // delete, line 207
    slice_del();
    break;
  case 15:
    // (, line 208
    // delete, line 208
    slice_del();
    break;
  case 16:
    // (, line 209
    // delete, line 209
    slice_del();
    break;
  case 17:
    // (, line 210
    // delete, line 210
    slice_del();
    break;
  case 18:
    // (, line 211
    // <-, line 211
    slice_from(L"a");
    break;
  case 19:
    // (, line 212
    // <-, line 212
    slice_from(L"e");
    break;
  case 20:
    // (, line 214
    // delete, line 214
    slice_del();
    break;
  case 21:
    // (, line 215
    // delete, line 215
    slice_del();
    break;
  case 22:
    // (, line 216
    // <-, line 216
    slice_from(L"a");
    break;
  case 23:
    // (, line 217
    // <-, line 217
    slice_from(L"e");
    break;
  case 24:
    // (, line 218
    // delete, line 218
    slice_del();
    break;
  case 25:
    // (, line 219
    // delete, line 219
    slice_del();
    break;
  case 26:
    // (, line 220
    // delete, line 220
    slice_del();
    break;
  case 27:
    // (, line 221
    // <-, line 221
    slice_from(L"a");
    break;
  case 28:
    // (, line 222
    // <-, line 222
    slice_from(L"e");
    break;
  case 29:
    // (, line 223
    // delete, line 223
    slice_del();
    break;
  }
  return true;
}

bool HungarianStemmer::stem()
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
  // (, line 228
  // do, line 229
  v_1 = cursor;
  do {
    // call mark_regions, line 229
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 230
  limit_backward = cursor;
  cursor = limit;
  // (, line 230
  // do, line 231
  v_2 = limit - cursor;
  do {
    // call instrum, line 231
    if (!r_instrum()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 232
  v_3 = limit - cursor;
  do {
    // call case, line 232
    if (!r_case()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 233
  v_4 = limit - cursor;
  do {
    // call case_special, line 233
    if (!r_case_special()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  // do, line 234
  v_5 = limit - cursor;
  do {
    // call case_other, line 234
    if (!r_case_other()) {
      goto lab4Break;
    }
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_5;
  // do, line 235
  v_6 = limit - cursor;
  do {
    // call factive, line 235
    if (!r_factive()) {
      goto lab5Break;
    }
  } while (false);
lab5Continue:;
lab5Break:
  cursor = limit - v_6;
  // do, line 236
  v_7 = limit - cursor;
  do {
    // call owned, line 236
    if (!r_owned()) {
      goto lab6Break;
    }
  } while (false);
lab6Continue:;
lab6Break:
  cursor = limit - v_7;
  // do, line 237
  v_8 = limit - cursor;
  do {
    // call sing_owner, line 237
    if (!r_sing_owner()) {
      goto lab7Break;
    }
  } while (false);
lab7Continue:;
lab7Break:
  cursor = limit - v_8;
  // do, line 238
  v_9 = limit - cursor;
  do {
    // call plur_owner, line 238
    if (!r_plur_owner()) {
      goto lab8Break;
    }
  } while (false);
lab8Continue:;
lab8Break:
  cursor = limit - v_9;
  // do, line 239
  v_10 = limit - cursor;
  do {
    // call plural, line 239
    if (!r_plural()) {
      goto lab9Break;
    }
  } while (false);
lab9Continue:;
lab9Break:
  cursor = limit - v_10;
  cursor = limit_backward;
  return true;
}

bool HungarianStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<HungarianStemmer>(o) != nullptr;
}

int HungarianStemmer::hashCode()
{
  return HungarianStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext