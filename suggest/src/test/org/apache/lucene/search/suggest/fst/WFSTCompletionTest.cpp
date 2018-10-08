using namespace std;

#include "WFSTCompletionTest.h"

namespace org::apache::lucene::search::suggest::fst
{
using Input = org::apache::lucene::search::suggest::Input;
using InputArrayIterator =
    org::apache::lucene::search::suggest::InputArrayIterator;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void WFSTCompletionTest::testBasic() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"foo", 50), make_shared<Input>(L"bar", 10),
      make_shared<Input>(L"barbar", 12), make_shared<Input>(L"barbara", 6)};

  shared_ptr<Random> random =
      make_shared<Random>(WFSTCompletionTest::random()->nextLong());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<WFSTCompletionLookup> suggester =
      make_shared<WFSTCompletionLookup>(tempDir, L"wfst");
  suggester->build(make_shared<InputArrayIterator>(keys));

  // top N of 2, but only foo is available
  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(TestUtil::stringToCharSequence(L"f", random), false, 2);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo", results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // make sure we don't get a dup exact suggestion:
  results = suggester->lookup(TestUtil::stringToCharSequence(L"foo", random),
                              false, 2);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo", results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // top N of 1 for 'bar': we return this even though barbar is higher
  results = suggester->lookup(TestUtil::stringToCharSequence(L"bar", random),
                              false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bar", results[0]->key->toString());
  assertEquals(10, results[0]->value, 0.01F);

  // top N Of 2 for 'b'
  results =
      suggester->lookup(TestUtil::stringToCharSequence(L"b", random), false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bar", results[1]->key->toString());
  assertEquals(10, results[1]->value, 0.01F);

  // top N of 3 for 'ba'
  results = suggester->lookup(TestUtil::stringToCharSequence(L"ba", random),
                              false, 3);
  TestUtil::assertEquals(3, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bar", results[1]->key->toString());
  assertEquals(10, results[1]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbara", results[2]->key->toString());
  assertEquals(6, results[2]->value, 0.01F);
  delete tempDir;
}

void WFSTCompletionTest::testExactFirst() 
{

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<WFSTCompletionLookup> suggester =
      make_shared<WFSTCompletionLookup>(tempDir, L"wfst", true);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"x y", 20), make_shared<Input>(L"x", 2)}));

  for (int topN = 1; topN < 4; topN++) {
    deque<std::shared_ptr<LookupResult>> results =
        suggester->lookup(L"x", false, topN);

    TestUtil::assertEquals(min(topN, 2), results.size());

    TestUtil::assertEquals(L"x", results[0]->key);
    TestUtil::assertEquals(2, results[0]->value);

    if (topN > 1) {
      TestUtil::assertEquals(L"x y", results[1]->key);
      TestUtil::assertEquals(20, results[1]->value);
    }
  }
  delete tempDir;
}

void WFSTCompletionTest::testNonExactFirst() 
{

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<WFSTCompletionLookup> suggester =
      make_shared<WFSTCompletionLookup>(tempDir, L"wfst", false);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"x y", 20), make_shared<Input>(L"x", 2)}));

  for (int topN = 1; topN < 4; topN++) {
    deque<std::shared_ptr<LookupResult>> results =
        suggester->lookup(L"x", false, topN);

    TestUtil::assertEquals(min(topN, 2), results.size());

    TestUtil::assertEquals(L"x y", results[0]->key);
    TestUtil::assertEquals(20, results[0]->value);

    if (topN > 1) {
      TestUtil::assertEquals(L"x", results[1]->key);
      TestUtil::assertEquals(2, results[1]->value);
    }
  }
  delete tempDir;
}

void WFSTCompletionTest::testRandom() 
{
  int numWords = atLeast(1000);

  const map_obj<wstring, int64_t> slowCompletor = map_obj<wstring, int64_t>();
  const set<wstring> allPrefixes = set<wstring>();

  std::deque<std::shared_ptr<Input>> keys(numWords);

  for (int i = 0; i < numWords; i++) {
    wstring s;
    while (true) {
      // TODO: would be nice to fix this slowCompletor/comparator to
      // use full range, but we might lose some coverage too...
      s = TestUtil::randomSimpleString(random());
      if (slowCompletor.find(s) == slowCompletor.end()) {
        break;
      }
    }

    for (int j = 1; j < s.length(); j++) {
      allPrefixes.insert(s.substr(0, j));
    }
    // we can probably do Integer.MAX_VALUE here, but why worry.
    int weight = random()->nextInt(1 << 24);
    slowCompletor.emplace(s, static_cast<int64_t>(weight));
    keys[i] = make_shared<Input>(s, weight);
  }

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<WFSTCompletionLookup> suggester =
      make_shared<WFSTCompletionLookup>(tempDir, L"wfst", false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  TestUtil::assertEquals(numWords, suggester->getCount());
  shared_ptr<Random> random =
      make_shared<Random>(WFSTCompletionTest::random()->nextLong());
  for (auto prefix : allPrefixes) {
    constexpr int topN = TestUtil::nextInt(random, 1, 10);
    deque<std::shared_ptr<LookupResult>> r = suggester->lookup(
        TestUtil::stringToCharSequence(prefix, random), false, topN);

    // 2. go thru whole treemap (slowCompletor) and check it's actually the best
    // suggestion
    const deque<std::shared_ptr<LookupResult>> matches =
        deque<std::shared_ptr<LookupResult>>();

    // TODO: could be faster... but it's slowCompletor for a reason
    for (auto e : slowCompletor) {
      if (e->getKey()->startsWith(prefix)) {
        matches.push_back(
            make_shared<LookupResult>(e->getKey(), e->getValue().longValue()));
      }
    }

    assertTrue(matches.size() > 0);
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: Collections.sort(matches, new
    // Comparator<org.apache.lucene.search.suggest.Lookup.LookupResult>()
    sort(matches.begin(), matches.end(),
         make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));
    if (matches.size() > topN) {
      matches.subList(topN, matches.size())->clear();
    }

    TestUtil::assertEquals(matches.size(), r.size());

    for (int hit = 0; hit < r.size(); hit++) {
      // System.out.println("  check hit " + hit);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(matches[hit]->key->toString(),
                             r[hit]->key->toString());
      assertEquals(matches[hit]->value, r[hit]->value, 0.0f);
    }
  }
  delete tempDir;
}

WFSTCompletionTest::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(shared_ptr<WFSTCompletionTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

int WFSTCompletionTest::ComparatorAnonymousInnerClass::compare(
    shared_ptr<LookupResult> left, shared_ptr<LookupResult> right)
{
  int cmp = Float::compare(right->value, left->value);
  if (cmp == 0) {
    return left->compareTo(right);
  } else {
    return cmp;
  }
}

void WFSTCompletionTest::test0ByteKeys() 
{
  shared_ptr<BytesRef> key1 = make_shared<BytesRef>(4);
  key1->length = 4;
  shared_ptr<BytesRef> key2 = make_shared<BytesRef>(3);
  key1->length = 3;

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<WFSTCompletionLookup> suggester =
      make_shared<WFSTCompletionLookup>(tempDir, L"wfst", false);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(key1, 50), make_shared<Input>(key2, 50)}));
  delete tempDir;
}

void WFSTCompletionTest::testEmpty() 
{
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<WFSTCompletionLookup> suggester =
      make_shared<WFSTCompletionLookup>(tempDir, L"wfst", false);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0)));
  TestUtil::assertEquals(0, suggester->getCount());
  deque<std::shared_ptr<LookupResult>> result =
      suggester->lookup(L"a", false, 20);
  assertTrue(result.empty());
  delete tempDir;
}

shared_ptr<Directory> WFSTCompletionTest::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::search::suggest::fst