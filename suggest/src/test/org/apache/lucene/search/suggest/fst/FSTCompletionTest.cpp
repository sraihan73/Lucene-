using namespace std;

#include "FSTCompletionTest.h"

namespace org::apache::lucene::search::suggest::fst
{
using namespace org::apache::lucene::search::suggest;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using Completion =
    org::apache::lucene::search::suggest::fst::FSTCompletion::Completion;
using Directory = org::apache::lucene::store::Directory;
using namespace org::apache::lucene::util;

shared_ptr<Input> FSTCompletionTest::tf(const wstring &t, int v)
{
  return make_shared<Input>(t, v);
}

void FSTCompletionTest::setUp() 
{
  LuceneTestCase::setUp();

  shared_ptr<FSTCompletionBuilder> builder =
      make_shared<FSTCompletionBuilder>();
  for (auto tf : evalKeys()) {
    builder->add(tf->term, static_cast<int>(tf->v));
  }
  completion = builder->build();
  completionAlphabetical =
      make_shared<FSTCompletion>(completion->getFST(), false, true);
}

std::deque<std::shared_ptr<Input>> FSTCompletionTest::evalKeys()
{
  std::deque<std::shared_ptr<Input>> keys = {
      tf(L"one", 0),      tf(L"oneness", 1),  tf(L"onerous", 1),
      tf(L"onesimus", 1), tf(L"two", 1),      tf(L"twofold", 1),
      tf(L"twonk", 1),    tf(L"thrive", 1),   tf(L"through", 1),
      tf(L"threat", 1),   tf(L"three", 1),    tf(L"foundation", 1),
      tf(L"fourblah", 1), tf(L"fourteen", 1), tf(L"four", 0),
      tf(L"fourier", 0),  tf(L"fourty", 0),   tf(L"xo", 1)};
  return keys;
}

void FSTCompletionTest::testExactMatchHighPriority() 
{
  assertMatchEquals(completion->lookup(stringToCharSequence(L"two"), 1),
                    {L"two/1.0"});
}

void FSTCompletionTest::testExactMatchLowPriority() 
{
  assertMatchEquals(completion->lookup(stringToCharSequence(L"one"), 2),
                    {L"one/0.0", L"oneness/1.0"});
}

void FSTCompletionTest::testExactMatchReordering() 
{
  // Check reordering of exact matches.
  assertMatchEquals(
      completion->lookup(stringToCharSequence(L"four"), 4),
      {L"four/0.0", L"fourblah/1.0", L"fourteen/1.0", L"fourier/0.0"});
}

void FSTCompletionTest::testRequestedCount() 
{
  // 'one' is promoted after collecting two higher ranking results.
  assertMatchEquals(completion->lookup(stringToCharSequence(L"one"), 2),
                    {L"one/0.0", L"oneness/1.0"});

  // 'four' is collected in a bucket and then again as an exact match.
  assertMatchEquals(completion->lookup(stringToCharSequence(L"four"), 2),
                    {L"four/0.0", L"fourblah/1.0"});

  // Check reordering of exact matches.
  assertMatchEquals(
      completion->lookup(stringToCharSequence(L"four"), 4),
      {L"four/0.0", L"fourblah/1.0", L"fourteen/1.0", L"fourier/0.0"});

  // 'one' is at the top after collecting all alphabetical results.
  assertMatchEquals(
      completionAlphabetical->lookup(stringToCharSequence(L"one"), 2),
      {L"one/0.0", L"oneness/1.0"});

  // 'one' is not promoted after collecting two higher ranking results.
  shared_ptr<FSTCompletion> noPromotion =
      make_shared<FSTCompletion>(completion->getFST(), true, false);
  assertMatchEquals(noPromotion->lookup(stringToCharSequence(L"one"), 2),
                    {L"oneness/1.0", L"onerous/1.0"});

  // 'one' is at the top after collecting all alphabetical results.
  assertMatchEquals(
      completionAlphabetical->lookup(stringToCharSequence(L"one"), 2),
      {L"one/0.0", L"oneness/1.0"});
}

void FSTCompletionTest::testMiss() 
{
  assertMatchEquals(completion->lookup(stringToCharSequence(L"xyz"), 1));
}

void FSTCompletionTest::testAlphabeticWithWeights() 
{
  assertEquals(
      0,
      completionAlphabetical->lookup(stringToCharSequence(L"xyz"), 1).size());
}

void FSTCompletionTest::testFullMatchList() 
{
  assertMatchEquals(
      completion->lookup(stringToCharSequence(L"one"),
                         numeric_limits<int>::max()),
      {L"oneness/1.0", L"onerous/1.0", L"onesimus/1.0", L"one/0.0"});
}

void FSTCompletionTest::testThreeByte() 
{
  wstring key = wstring(
      std::deque<char>{static_cast<char>(0xF0), static_cast<char>(0xA4),
                        static_cast<char>(0xAD), static_cast<char>(0xA2)},
      StandardCharsets::UTF_8);
  shared_ptr<FSTCompletionBuilder> builder =
      make_shared<FSTCompletionBuilder>();
  builder->add(make_shared<BytesRef>(key), 0);

  shared_ptr<FSTCompletion> lookup = builder->build();
  deque<std::shared_ptr<Completion>> result =
      lookup->lookup(stringToCharSequence(key), 1);
  assertEquals(1, result.size());
}

void FSTCompletionTest::testLargeInputConstantWeights() 
{
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FSTCompletionLookup> lookup =
      make_shared<FSTCompletionLookup>(tempDir, L"fst", 10, true);

  shared_ptr<Random> r = random();
  deque<std::shared_ptr<Input>> keys = deque<std::shared_ptr<Input>>();
  for (int i = 0; i < 5000; i++) {
    keys.push_back(make_shared<Input>(TestUtil::randomSimpleString(r), -1));
  }

  lookup->build(make_shared<InputArrayIterator>(keys));

  // All the weights were constant, so all returned buckets must be constant,
  // whatever they are.
  optional<int64_t> previous = nullopt;
  for (auto tf : keys) {
    optional<int64_t> current =
        (any_cast<std::shared_ptr<Number>>(
             lookup->get(TestUtil::bytesToCharSequence(tf->term, random()))))
            .longValue();
    if (previous) {
      assertEquals(previous, current);
    }
    previous = current;
  }
  delete tempDir;
}

void FSTCompletionTest::testMultilingualInput() 
{
  deque<std::shared_ptr<Input>> input = LookupBenchmarkTest::readTop50KWiki();

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FSTCompletionLookup> lookup =
      make_shared<FSTCompletionLookup>(tempDir, L"fst");
  lookup->build(make_shared<InputArrayIterator>(input));
  assertEquals(input.size(), lookup->getCount());
  for (auto tf : input) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertNotNull(
        L"Not found: " + tf->term->toString(),
        lookup->get(TestUtil::bytesToCharSequence(tf->term, random())));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(tf->term->utf8ToString(),
                 lookup
                     ->lookup(TestUtil::bytesToCharSequence(tf->term, random()),
                              true, 1)[0]
                     ->key->toString());
  }

  deque<std::shared_ptr<LookupResult>> result =
      lookup->lookup(stringToCharSequence(L"wit"), true, 5);
  assertEquals(5, result.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(result[0]->key->toString()->equals(L"wit")); // exact match.
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(result[1]->key->toString()->equals(L"with")); // highest count.
  delete tempDir;
}

void FSTCompletionTest::testEmptyInput() 
{
  completion = (make_shared<FSTCompletionBuilder>())->build();
  assertMatchEquals(completion->lookup(stringToCharSequence(L""), 10));
}

void FSTCompletionTest::testRandom() 
{
  deque<std::shared_ptr<Input>> freqs = deque<std::shared_ptr<Input>>();
  shared_ptr<Random> rnd = random();
  for (int i = 0; i < 2500 + rnd->nextInt(2500); i++) {
    int weight = rnd->nextInt(100);
    freqs.push_back(make_shared<Input>(L"" + rnd->nextLong(), weight));
  }

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FSTCompletionLookup> lookup =
      make_shared<FSTCompletionLookup>(tempDir, L"fst");
  lookup->build(make_shared<InputArrayIterator>(
      freqs.toArray(std::deque<std::shared_ptr<Input>>(freqs.size()))));

  for (auto tf : freqs) {
    const wstring term = tf->term->utf8ToString();
    for (int i = 1; i < term.length(); i++) {
      wstring prefix = term.substr(0, i);
      for (auto lr : lookup->lookup(stringToCharSequence(prefix), true, 10)) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        assertTrue(lr->key->toString()->startsWith(prefix));
      }
    }
  }
  delete tempDir;
}

shared_ptr<std::wstring>
FSTCompletionTest::stringToCharSequence(const wstring &prefix)
{
  return TestUtil::stringToCharSequence(prefix, random());
}

void FSTCompletionTest::assertMatchEquals(
    deque<std::shared_ptr<Completion>> &res, deque<wstring> &expected)
{
  std::deque<wstring> result(res.size());
  for (int i = 0; i < res.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    result[i] = res[i]->toString();
  }

  if (!Arrays::equals(stripScore(expected), stripScore(result))) {
    int colLen = max(maxLen(expected), maxLen(result));

    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    wstring format =
        L"%" + to_wstring(colLen) + L"s  " + L"%" + to_wstring(colLen) + L"s\n";
    b->append(wstring::format(Locale::ROOT, format, L"Expected", L"Result"));
    for (int i = 0; i < max(result.size(), expected->length); i++) {
      b->append(wstring::format(Locale::ROOT, format,
                                i < expected->length ? expected[i] : L"--",
                                i < result.size() ? result[i] : L"--"));
    }

    System::err::println(b->toString());
    fail(L"Expected different output:\n" + b->toString());
  }
}

std::deque<wstring>
FSTCompletionTest::stripScore(std::deque<wstring> &expected)
{
  std::deque<wstring> result(expected.size());
  for (int i = 0; i < result.size(); i++) {
    result[i] = expected[i].replaceAll(L"\\/[0-9\\.]+", L"");
  }
  return result;
}

int FSTCompletionTest::maxLen(std::deque<wstring> &result)
{
  int len = 0;
  for (auto s : result) {
    len = max(len, s.length());
  }
  return len;
}

shared_ptr<Directory> FSTCompletionTest::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::search::suggest::fst