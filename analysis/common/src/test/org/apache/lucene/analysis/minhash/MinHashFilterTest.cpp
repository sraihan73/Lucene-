using namespace std;

#include "MinHashFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/RegExp.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/minhash/MinHashFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/minhash/MinHashFilterFactory.h"

namespace org::apache::lucene::analysis::minhash
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using org::apache::lucene::analysis::minhash::MinHashFilter::FixedSizeTreeSet;
using LongPair =
    org::apache::lucene::analysis::minhash::MinHashFilter::LongPair;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIntHash()
void MinHashFilterTest::testIntHash()
{
  shared_ptr<LongPair> hash = make_shared<LongPair>();
  MinHashFilter::murmurhash3_x64_128(MinHashFilter::getBytes(0), 0, 4, 0, hash);
  assertEquals(-3485513579396041028LL, hash->val1);
  assertEquals(6383328099726337777LL, hash->val2);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testStringHash() throws
// java.io.UnsupportedEncodingException
void MinHashFilterTest::testStringHash() 
{
  shared_ptr<LongPair> hash = make_shared<LongPair>();
  std::deque<char> bytes =
      (wstring(L"woof woof woof woof woof")).getBytes(L"UTF-16LE");
  MinHashFilter::murmurhash3_x64_128(bytes, 0, bytes.size(), 0, hash);
  assertEquals(7638079586852243959LL, hash->val1);
  assertEquals(4378804943379391304LL, hash->val2);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimpleOrder() throws
// java.io.UnsupportedEncodingException
void MinHashFilterTest::testSimpleOrder() 
{
  shared_ptr<LongPair> hash1 = make_shared<LongPair>();
  hash1->val1 = 1;
  hash1->val2 = 2;
  shared_ptr<LongPair> hash2 = make_shared<LongPair>();
  hash2->val1 = 2;
  hash2->val2 = 1;
  assert(hash1->compareTo(hash2) > 0);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testHashOrder()
void MinHashFilterTest::testHashOrder()
{
  assertTrue(!MinHashFilter::isLessThanUnsigned(0LL, 0LL));
  assertTrue(MinHashFilter::isLessThanUnsigned(0LL, -1LL));
  assertTrue(MinHashFilter::isLessThanUnsigned(1LL, -1LL));
  assertTrue(MinHashFilter::isLessThanUnsigned(-2LL, -1LL));
  assertTrue(MinHashFilter::isLessThanUnsigned(1LL, 2LL));
  assertTrue(MinHashFilter::isLessThanUnsigned(
      numeric_limits<int64_t>::max(), numeric_limits<int64_t>::min()));

  shared_ptr<FixedSizeTreeSet<std::shared_ptr<LongPair>>> minSet =
      make_shared<FixedSizeTreeSet<std::shared_ptr<LongPair>>>(500);
  unordered_set<std::shared_ptr<LongPair>> unadded =
      unordered_set<std::shared_ptr<LongPair>>();
  for (int i = 0; i < 100; i++) {
    shared_ptr<LongPair> hash = make_shared<LongPair>();
    MinHashFilter::murmurhash3_x64_128(MinHashFilter::getBytes(i), 0, 4, 0,
                                       hash);
    shared_ptr<LongPair> peek = nullptr;
    if (minSet->size() > 0) {
      peek = minSet->last();
    }

    if (!minSet->add(hash)) {
      unadded.insert(hash);
    } else {
      if (peek != nullptr) {
        if ((minSet->size() == 500) && !peek->equals(minSet->last())) {
          unadded.insert(peek);
        }
      }
    }
  }
  assertEquals(100, minSet->size());
  assertEquals(0, unadded.size());

  unordered_set<std::shared_ptr<LongPair>> collisionDetection =
      unordered_set<std::shared_ptr<LongPair>>();
  unadded = unordered_set<std::shared_ptr<LongPair>>();
  minSet = make_shared<FixedSizeTreeSet<std::shared_ptr<LongPair>>>(500);
  for (int i = 0; i < 1000000; i++) {
    shared_ptr<LongPair> hash = make_shared<LongPair>();
    MinHashFilter::murmurhash3_x64_128(MinHashFilter::getBytes(i), 0, 4, 0,
                                       hash);
    collisionDetection.insert(hash);
    shared_ptr<LongPair> peek = nullptr;
    if (minSet->size() > 0) {
      peek = minSet->last();
    }

    if (!minSet->add(hash)) {
      unadded.insert(hash);
    } else {
      if (peek != nullptr) {
        if ((minSet->size() == 500) && !peek->equals(minSet->last())) {
          unadded.insert(peek);
        }
      }
    }
  }
  assertEquals(1000000, collisionDetection.size());
  assertEquals(500, minSet->size());
  assertEquals(999500, unadded.size());

  shared_ptr<LongPair> last = nullptr;
  shared_ptr<LongPair> current = nullptr;
  while ((current = minSet->pollLast()) != nullptr) {
    if (last != nullptr) {
      assertTrue(isLessThan(current, last));
    }
    last = current;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testHashNotRepeated()
void MinHashFilterTest::testHashNotRepeated()
{
  shared_ptr<FixedSizeTreeSet<std::shared_ptr<LongPair>>> minSet =
      make_shared<FixedSizeTreeSet<std::shared_ptr<LongPair>>>(500);
  unordered_set<std::shared_ptr<LongPair>> unadded =
      unordered_set<std::shared_ptr<LongPair>>();
  for (int i = 0; i < 10000; i++) {
    shared_ptr<LongPair> hash = make_shared<LongPair>();
    MinHashFilter::murmurhash3_x64_128(MinHashFilter::getBytes(i), 0, 4, 0,
                                       hash);
    shared_ptr<LongPair> peek = nullptr;
    if (minSet->size() > 0) {
      peek = minSet->last();
    }
    if (!minSet->add(hash)) {
      unadded.insert(hash);
    } else {
      if (peek != nullptr) {
        if ((minSet->size() == 500) && !peek->equals(minSet->last())) {
          unadded.insert(peek);
        }
      }
    }
  }
  assertEquals(500, minSet->size());

  shared_ptr<LongPair> last = nullptr;
  shared_ptr<LongPair> current = nullptr;
  while ((current = minSet->pollLast()) != nullptr) {
    if (last != nullptr) {
      assertTrue(isLessThan(current, last));
    }
    last = current;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMockShingleTokenizer() throws
// java.io.IOException
void MinHashFilterTest::testMockShingleTokenizer() 
{
  shared_ptr<Tokenizer> mockShingleTokenizer =
      createMockShingleTokenizer(5, wstring(L"woof woof woof woof woof") +
                                        L" " + L"woof woof woof woof puff");
  assertTokenStreamContents(mockShingleTokenizer,
                            std::deque<wstring>{L"woof woof woof woof woof",
                                                 L"woof woof woof woof puff"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStreamSingleInput() throws
// java.io.IOException
void MinHashFilterTest::testTokenStreamSingleInput() 
{
  std::deque<wstring> hashes = {L"℁팽徭聙↝ꇁ홱杯"};
  shared_ptr<TokenStream> ts =
      createTokenStream(5, L"woof woof woof woof woof", 1, 1, 100, false);
  assertTokenStreamContents(
      ts, hashes, std::deque<int>{0}, std::deque<int>{24},
      std::deque<wstring>{MinHashFilter::MIN_HASH_TYPE}, std::deque<int>{1},
      std::deque<int>{1}, 24, 0, nullptr, true, nullptr);

  ts = createTokenStream(5, L"woof woof woof woof woof", 2, 1, 1, false);
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{wstring(std::deque<wchar_t>{
                               0, 0, 8449, 54077, 64133, 32857, 8605, 41409}),
                           wstring(std::deque<wchar_t>{
                               0, 1, 16887, 58164, 39536, 14926, 6529, 17276})},
      std::deque<int>{0, 0}, std::deque<int>{24, 24},
      std::deque<wstring>{MinHashFilter::MIN_HASH_TYPE,
                           MinHashFilter::MIN_HASH_TYPE},
      std::deque<int>{1, 0}, std::deque<int>{1, 1}, 24, 0, nullptr, true,
      nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream1() throws
// java.io.IOException
void MinHashFilterTest::testTokenStream1() 
{
  std::deque<wstring> hashes = {
      L"℁팽徭聙↝ꇁ홱杯",
      wstring(std::deque<wchar_t>{36347, 63457, 43013, 56843, 52284, 34231,
                                   57934, 42302})};
  // characters!

  shared_ptr<TokenStream> ts = createTokenStream(
      5,
      wstring(L"woof woof woof woof woof") + L" " + L"woof woof woof woof puff",
      1, 1, 100, false);
  assertTokenStreamContents(ts, hashes, std::deque<int>{0, 0},
                            std::deque<int>{49, 49},
                            std::deque<wstring>{MinHashFilter::MIN_HASH_TYPE,
                                                 MinHashFilter::MIN_HASH_TYPE},
                            std::deque<int>{1, 0}, std::deque<int>{1, 1}, 49,
                            0, nullptr, true, nullptr);
}

deque<wstring>
MinHashFilterTest::getTokens(shared_ptr<TokenStream> ts) 
{
  deque<wstring> tokens = deque<wstring>();
  ts->reset();
  while (ts->incrementToken()) {
    shared_ptr<CharTermAttribute> termAttribute =
        ts->getAttribute(CharTermAttribute::typeid);
    wstring token =
        wstring(termAttribute->buffer(), 0, termAttribute->length());
    tokens.push_back(token);
  }
  ts->end();
  delete ts;

  return tokens;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream2() throws
// java.io.IOException
void MinHashFilterTest::testTokenStream2() 
{
  shared_ptr<TokenStream> ts = createTokenStream(
      5,
      wstring(L"woof woof woof woof woof") + L" " + L"woof woof woof woof puff",
      100, 1, 1, false);
  deque<wstring> tokens = getTokens(ts);
  delete ts;

  assertEquals(100, tokens.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream3() throws
// java.io.IOException
void MinHashFilterTest::testTokenStream3() 
{
  shared_ptr<TokenStream> ts = createTokenStream(
      5,
      wstring(L"woof woof woof woof woof") + L" " + L"woof woof woof woof puff",
      10, 1, 10, false);
  deque<wstring> tokens = getTokens(ts);
  delete ts;

  assertEquals(20, tokens.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream4() throws
// java.io.IOException
void MinHashFilterTest::testTokenStream4() 
{
  shared_ptr<TokenStream> ts = createTokenStream(
      5,
      wstring(L"woof woof woof woof woof") + L" " + L"woof woof woof woof puff",
      10, 10, 1, false);
  deque<wstring> tokens = getTokens(ts);
  delete ts;

  assertEquals(20, tokens.size());

  ts = createTokenStream(5,
                         wstring(L"woof woof woof woof woof") + L" " +
                             L"woof woof woof woof puff",
                         10, 10, 1, true);
  tokens = getTokens(ts);
  delete ts;

  assertEquals(100, tokens.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenStream5() throws
// java.io.IOException
void MinHashFilterTest::testTokenStream5() 
{
  shared_ptr<TokenStream> ts = createTokenStream(
      5,
      wstring(L"woof woof woof woof woof") + L" " + L"woof woof woof woof puff",
      1, 100, 1, false);
  deque<wstring> tokens = getTokens(ts);
  delete ts;

  assertEquals(2, tokens.size());

  ts = createTokenStream(5,
                         wstring(L"woof woof woof woof woof") + L" " +
                             L"woof woof woof woof puff",
                         1, 100, 1, true);
  tokens = getTokens(ts);
  delete ts;

  assertEquals(100, tokens.size());
  unordered_set<wstring> set = unordered_set<wstring>(tokens);
  assertEquals(2, set.size());

  bool rolled = false;
  wstring first = L"";
  wstring last = L"";
  for (auto current : tokens) {
    if (first == L"") {
      first = current;
    }
    if (last != L"") {
      if (!rolled) {
        if (current.compareTo(last) >= 0) {
          // fine
        } else if (current.equals(first)) {
          rolled = true;
        } else {
          fail(L"Incorrect hash order");
        }
      } else {
        if (!current.equals(first)) {
          fail(L"Incorrect hash order");
        }
      }
    }
    last = current;
  }
}

shared_ptr<TokenStream>
MinHashFilterTest::createTokenStream(int shingleSize, const wstring &shingles,
                                     int hashCount, int bucketCount,
                                     int hashSetSize, bool withRotation)
{
  shared_ptr<Tokenizer> tokenizer =
      createMockShingleTokenizer(shingleSize, shingles);
  unordered_map<wstring, wstring> lshffargs = unordered_map<wstring, wstring>();
  lshffargs.emplace(L"hashCount", L"" + to_wstring(hashCount));
  lshffargs.emplace(L"bucketCount", L"" + to_wstring(bucketCount));
  lshffargs.emplace(L"hashSetSize", L"" + to_wstring(hashSetSize));
  lshffargs.emplace(L"withRotation",
                    L"" + StringHelper::toString(withRotation));
  shared_ptr<MinHashFilterFactory> lshff =
      make_shared<MinHashFilterFactory>(lshffargs);
  return lshff->create(tokenizer);
}

shared_ptr<Tokenizer>
MinHashFilterTest::createMockShingleTokenizer(int shingleSize,
                                              const wstring &shingles)
{
  shared_ptr<MockTokenizer> tokenizer = make_shared<MockTokenizer>(
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"[^ \t\r\n]+([ \t\r\n]+[^ \t\r\n]+){" +
                               to_wstring(shingleSize - 1) + L"}"))
              ->toAutomaton()),
      true);
  tokenizer->setEnableChecks(true);
  if (shingles != L"") {
    tokenizer->setReader(make_shared<StringReader>(shingles));
  }
  return tokenizer;
}

bool MinHashFilterTest::isLessThan(shared_ptr<LongPair> hash1,
                                   shared_ptr<LongPair> hash2)
{
  return MinHashFilter::isLessThanUnsigned(hash1->val2, hash2->val2) ||
         hash1->val2 == hash2->val2 &&
             (MinHashFilter::isLessThanUnsigned(hash1->val1, hash2->val1));
}
} // namespace org::apache::lucene::analysis::minhash