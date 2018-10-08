using namespace std;

#include "TestUTF32ToUTF8.h"

namespace org::apache::lucene::util::automaton
{
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using Util = org::apache::lucene::util::fst::Util;

void TestUTF32ToUTF8::setUp() ; }

bool TestUTF32ToUTF8::matches(shared_ptr<ByteRunAutomaton> a, int code)
{
  std::deque<wchar_t> chars = Character::toChars(code);
  std::deque<char> b(UnicodeUtil::maxUTF8Length(chars.size()));
  constexpr int len = UnicodeUtil::UTF16toUTF8(chars, 0, chars.size(), b);
  return a->run(b, 0, len);
}

void TestUTF32ToUTF8::testOne(shared_ptr<Random> r,
                              shared_ptr<ByteRunAutomaton> a, int startCode,
                              int endCode, int iters)
{

  // Verify correct ints are accepted
  constexpr int nonSurrogateCount;
  constexpr bool ovSurStart;
  if (endCode < UnicodeUtil::UNI_SUR_HIGH_START ||
      startCode > UnicodeUtil::UNI_SUR_LOW_END) {
    // no overlap w/ surrogates
    nonSurrogateCount = endCode - startCode + 1;
    ovSurStart = false;
  } else if (isSurrogate(startCode)) {
    // start of range overlaps surrogates
    nonSurrogateCount = endCode - startCode + 1 -
                        (UnicodeUtil::UNI_SUR_LOW_END - startCode + 1);
    ovSurStart = false;
  } else if (isSurrogate(endCode)) {
    // end of range overlaps surrogates
    ovSurStart = true;
    nonSurrogateCount = endCode - startCode + 1 -
                        (endCode - UnicodeUtil::UNI_SUR_HIGH_START + 1);
  } else {
    // range completely subsumes surrogates
    ovSurStart = true;
    nonSurrogateCount =
        endCode - startCode + 1 -
        (UnicodeUtil::UNI_SUR_LOW_END - UnicodeUtil::UNI_SUR_HIGH_START + 1);
  }

  assert(nonSurrogateCount > 0);

  for (int iter = 0; iter < iters; iter++) {
    // pick random code point in-range

    int code = startCode + r->nextInt(nonSurrogateCount);
    if (isSurrogate(code)) {
      if (ovSurStart) {
        code = UnicodeUtil::UNI_SUR_LOW_END + 1 +
               (code - UnicodeUtil::UNI_SUR_HIGH_START);
      } else {
        code = UnicodeUtil::UNI_SUR_LOW_END + 1 + (code - startCode);
      }
    }

    assert((code >= startCode && code <= endCode,
            L"code=" + to_wstring(code) + L" start=" + to_wstring(startCode) +
                L" end=" + to_wstring(endCode)));
    assert(!isSurrogate(code));

    assertTrue(L"DFA for range " + to_wstring(startCode) + L"-" +
                   to_wstring(endCode) + L" failed to match code=" +
                   to_wstring(code),
               matches(a, code));
  }

  // Verify invalid ints are not accepted
  constexpr int invalidRange = MAX_UNICODE - (endCode - startCode + 1);
  if (invalidRange > 0) {
    for (int iter = 0; iter < iters; iter++) {
      int x = TestUtil::nextInt(r, 0, invalidRange - 1);
      constexpr int code;
      if (x >= startCode) {
        code = endCode + 1 + x - startCode;
      } else {
        code = x;
      }
      if ((code >= UnicodeUtil::UNI_SUR_HIGH_START &&
           code <= UnicodeUtil::UNI_SUR_HIGH_END) |
          (code >= UnicodeUtil::UNI_SUR_LOW_START &&
           code <= UnicodeUtil::UNI_SUR_LOW_END)) {
        iter--;
        continue;
      }
      assertFalse(L"DFA for range " + to_wstring(startCode) + L"-" +
                      to_wstring(endCode) + L" matched invalid code=" +
                      to_wstring(code),
                  matches(a, code));
    }
  }
}

int TestUTF32ToUTF8::getCodeStart(shared_ptr<Random> r)
{
  switch (r->nextInt(4)) {
  case 0:
    return TestUtil::nextInt(r, 0, 128);
  case 1:
    return TestUtil::nextInt(r, 128, 2048);
  case 2:
    return TestUtil::nextInt(r, 2048, 65536);
  default:
    return TestUtil::nextInt(r, 65536, 1 + MAX_UNICODE);
  }
}

bool TestUTF32ToUTF8::isSurrogate(int code)
{
  return code >= UnicodeUtil::UNI_SUR_HIGH_START &&
         code <= UnicodeUtil::UNI_SUR_LOW_END;
}

void TestUTF32ToUTF8::testRandomRanges() 
{
  shared_ptr<Random> *const r = random();
  int ITERS = atLeast(10);
  int ITERS_PER_DFA = atLeast(100);
  for (int iter = 0; iter < ITERS; iter++) {
    int x1 = getCodeStart(r);
    int x2 = getCodeStart(r);
    constexpr int startCode, endCode;

    if (x1 < x2) {
      startCode = x1;
      endCode = x2;
    } else {
      startCode = x2;
      endCode = x1;
    }

    if (isSurrogate(startCode) && isSurrogate(endCode)) {
      iter--;
      continue;
    }

    shared_ptr<Automaton> a = Automata::makeCharRange(startCode, endCode);
    testOne(r, make_shared<ByteRunAutomaton>(a), startCode, endCode,
            ITERS_PER_DFA);
  }
}

void TestUTF32ToUTF8::testSpecialCase()
{
  shared_ptr<RegExp> re = make_shared<RegExp>(L".?");
  shared_ptr<Automaton> automaton = re->toAutomaton();
  shared_ptr<CharacterRunAutomaton> cra =
      make_shared<CharacterRunAutomaton>(automaton);
  shared_ptr<ByteRunAutomaton> bra = make_shared<ByteRunAutomaton>(automaton);
  // make sure character dfa accepts empty string
  assertTrue(cra->isAccept(0));
  assertTrue(cra->run(L""));
  assertTrue(cra->run(std::deque<wchar_t>(0), 0, 0));

  // make sure byte dfa accepts empty string
  assertTrue(bra->isAccept(0));
  assertTrue(bra->run(std::deque<char>(0), 0, 0));
}

void TestUTF32ToUTF8::testSpecialCase2() 
{
  shared_ptr<RegExp> re = make_shared<RegExp>(L".+\u0775");
  wstring input = L"\ufadc\ufffd\ub80b\uda5a\udc68\uf234\u0056\uda5b\udcc1"
                  L"\ufffd\ufffd\u0775";
  shared_ptr<Automaton> automaton = re->toAutomaton();
  shared_ptr<CharacterRunAutomaton> cra =
      make_shared<CharacterRunAutomaton>(automaton);
  shared_ptr<ByteRunAutomaton> bra = make_shared<ByteRunAutomaton>(automaton);

  assertTrue(cra->run(input));

  std::deque<char> bytes = input.getBytes(StandardCharsets::UTF_8);
  assertTrue(bra->run(bytes, 0, bytes.size())); // this one fails!
}

void TestUTF32ToUTF8::testSpecialCase3() 
{
  shared_ptr<RegExp> re = make_shared<RegExp>(L"(\\鯺)*(.)*\\Ӕ");
  wstring input = L"\u5cfd\ufffd\ub2f7\u0033\ue304\u51d7\u3692\udb50\udfb3"
                  L"\u0576\udae2\udc62\u0053\u0449\u04d4";
  shared_ptr<Automaton> automaton = re->toAutomaton();
  shared_ptr<CharacterRunAutomaton> cra =
      make_shared<CharacterRunAutomaton>(automaton);
  shared_ptr<ByteRunAutomaton> bra = make_shared<ByteRunAutomaton>(automaton);

  assertTrue(cra->run(input));

  std::deque<char> bytes = input.getBytes(StandardCharsets::UTF_8);
  assertTrue(bra->run(bytes, 0, bytes.size()));
}

void TestUTF32ToUTF8::testRandomRegexes() 
{
  int num = atLeast(250);
  for (int i = 0; i < num; i++) {
    assertAutomaton(
        (make_shared<RegExp>(AutomatonTestUtil::randomRegexp(random()),
                             RegExp::NONE))
            ->toAutomaton());
  }
}

void TestUTF32ToUTF8::testSingleton() 
{
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    wstring s = TestUtil::randomRealisticUnicodeString(random());
    shared_ptr<Automaton> a = Automata::makeString(s);
    shared_ptr<Automaton> utf8 = (make_shared<UTF32ToUTF8>())->convert(a);
    shared_ptr<IntsRefBuilder> ints = make_shared<IntsRefBuilder>();
    Util::toIntsRef(make_shared<BytesRef>(s), ints);
    shared_ptr<Set<std::shared_ptr<IntsRef>>> set =
        unordered_set<std::shared_ptr<IntsRef>>();
    set->add(ints->get());
    TestUtil::assertEquals(set, TestOperations::getFiniteStrings(utf8));
  }
}

void TestUTF32ToUTF8::assertAutomaton(shared_ptr<Automaton> automaton) throw(
    runtime_error)
{
  shared_ptr<CharacterRunAutomaton> cra =
      make_shared<CharacterRunAutomaton>(automaton);
  shared_ptr<ByteRunAutomaton> bra = make_shared<ByteRunAutomaton>(automaton);
  shared_ptr<AutomatonTestUtil::RandomAcceptedStrings> *const ras =
      make_shared<AutomatonTestUtil::RandomAcceptedStrings>(automaton);

  int num = atLeast(1000);
  for (int i = 0; i < num; i++) {
    const wstring string;
    if (random()->nextBoolean()) {
      // likely not accepted
      string = TestUtil::randomUnicodeString(random());
    } else {
      // will be accepted
      std::deque<int> codepoints = ras->getRandomAcceptedString(random());
      try {
        string = UnicodeUtil::newString(codepoints, 0, codepoints.size());
      } catch (const runtime_error &e) {
        wcout << codepoints.size() << L" codepoints:" << endl;
        for (int j = 0; j < codepoints.size(); j++) {
          wcout << L"  " << Integer::toHexString(codepoints[j]) << endl;
        }
        throw e;
      }
    }
    std::deque<char> bytes = string.getBytes(StandardCharsets::UTF_8);
    TestUtil::assertEquals(cra->run(string), bra->run(bytes, 0, bytes.size()));
  }
}
} // namespace org::apache::lucene::util::automaton