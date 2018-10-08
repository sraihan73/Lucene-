using namespace std;

#include "BlendedInfixSuggesterTest.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Input = org::apache::lucene::search::suggest::Input;
using InputArrayIterator =
    org::apache::lucene::search::suggest::InputArrayIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void BlendedInfixSuggesterTest::testBlendedSort() 
{

  shared_ptr<BytesRef> payload = make_shared<BytesRef>(L"star");

  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(
      L"star wars: episode v - the empire strikes back", 8, payload)};

  shared_ptr<Path> tempDir = createTempDir(L"BlendedInfixSuggesterTest");

  shared_ptr<Analyzer> a =
      make_shared<StandardAnalyzer>(CharArraySet::EMPTY_SET);
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(
          newFSDirectory(tempDir), a, a,
          AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
          BlendedInfixSuggester::BlenderType::POSITION_LINEAR,
          BlendedInfixSuggester::DEFAULT_NUM_FACTOR, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  // we query for star wars and check that the weight
  // is smaller when we search for tokens that are far from the beginning

  int64_t w0 = getInResults(suggester, L"star ", payload, 1);
  int64_t w1 = getInResults(suggester, L"war", payload, 1);
  int64_t w2 = getInResults(suggester, L"empire ba", payload, 1);
  int64_t w3 = getInResults(suggester, L"back", payload, 1);
  int64_t w4 = getInResults(suggester, L"bacc", payload, 1);

  assertTrue(w0 > w1);
  assertTrue(w1 > w2);
  assertTrue(w2 > w3);

  assertTrue(w4 < 0);

  delete suggester;
}

void BlendedInfixSuggesterTest::testBlendingType() 
{

  shared_ptr<BytesRef> pl = make_shared<BytesRef>(L"lake");
  int64_t w = 20;

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"top of the lake", w, pl)};

  shared_ptr<Path> tempDir = createTempDir(L"BlendedInfixSuggesterTest");
  shared_ptr<Analyzer> a =
      make_shared<StandardAnalyzer>(CharArraySet::EMPTY_SET);

  // BlenderType.LINEAR is used by default (remove position*10%)
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(newFSDirectory(tempDir), a);
  suggester->build(make_shared<InputArrayIterator>(keys));

  TestUtil::assertEquals(w, getInResults(suggester, L"top", pl, 1));
  TestUtil::assertEquals(static_cast<int>(w * (1 - 0.10 * 2)),
                         getInResults(suggester, L"the", pl, 1));
  TestUtil::assertEquals(static_cast<int>(w * (1 - 0.10 * 3)),
                         getInResults(suggester, L"lake", pl, 1));

  delete suggester;

  // BlenderType.RECIPROCAL is using 1/(1+p) * w where w is weight and p the
  // position of the word
  suggester = make_shared<BlendedInfixSuggester>(
      newFSDirectory(tempDir), a, a,
      AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
      BlendedInfixSuggester::BlenderType::POSITION_RECIPROCAL, 1, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  TestUtil::assertEquals(w, getInResults(suggester, L"top", pl, 1));
  TestUtil::assertEquals(static_cast<int>(w * 1 / (1 + 2)),
                         getInResults(suggester, L"the", pl, 1));
  TestUtil::assertEquals(static_cast<int>(w * 1 / (1 + 3)),
                         getInResults(suggester, L"lake", pl, 1));
  delete suggester;

  // BlenderType.EXPONENTIAL_RECIPROCAL is using 1/(pow(1+p, exponent)) * w
  // where w is weight and p the position of the word
  suggester = make_shared<BlendedInfixSuggester>(
      newFSDirectory(tempDir), a, a,
      AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
      BlendedInfixSuggester::BlenderType::POSITION_EXPONENTIAL_RECIPROCAL, 1,
      4.0, false, true, false);

  suggester->build(make_shared<InputArrayIterator>(keys));

  TestUtil::assertEquals(w, getInResults(suggester, L"top", pl, 1));
  TestUtil::assertEquals(static_cast<int>(w * 1 / (pow(1 + 2, 4.0))),
                         getInResults(suggester, L"the", pl, 1));
  TestUtil::assertEquals(static_cast<int>(w * 1 / (pow(1 + 3, 4.0))),
                         getInResults(suggester, L"lake", pl, 1));

  delete suggester;
}

void BlendedInfixSuggesterTest::testRequiresMore() 
{

  shared_ptr<BytesRef> lake = make_shared<BytesRef>(L"lake");
  shared_ptr<BytesRef> star = make_shared<BytesRef>(L"star");
  shared_ptr<BytesRef> ret = make_shared<BytesRef>(L"ret");

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"top of the lake", 18, lake),
      make_shared<Input>(L"star wars: episode v - the empire strikes back", 12,
                         star),
      make_shared<Input>(L"the returned", 10, ret)};

  shared_ptr<Path> tempDir = createTempDir(L"BlendedInfixSuggesterTest");
  shared_ptr<Analyzer> a =
      make_shared<StandardAnalyzer>(CharArraySet::EMPTY_SET);

  // if factor is small, we don't get the expected element
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(
          newFSDirectory(tempDir), a, a,
          AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
          BlendedInfixSuggester::BlenderType::POSITION_RECIPROCAL, 1, false);

  suggester->build(make_shared<InputArrayIterator>(keys));

  // we don't find it for in the 2 first
  TestUtil::assertEquals(2, suggester->lookup(L"the", 2, true, false).size());
  int64_t w0 = getInResults(suggester, L"the", ret, 2);
  assertTrue(w0 < 0);

  // but it's there if we search for 3 elements
  TestUtil::assertEquals(3, suggester->lookup(L"the", 3, true, false).size());
  int64_t w1 = getInResults(suggester, L"the", ret, 3);
  assertTrue(w1 > 0);

  delete suggester;

  // if we increase the factor we have it
  suggester = make_shared<BlendedInfixSuggester>(
      newFSDirectory(tempDir), a, a,
      AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
      BlendedInfixSuggester::BlenderType::POSITION_RECIPROCAL, 2, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  // we have it
  int64_t w2 = getInResults(suggester, L"the", ret, 2);
  assertTrue(w2 > 0);

  // but we don't have the other
  int64_t w3 = getInResults(suggester, L"the", star, 2);
  assertTrue(w3 < 0);

  delete suggester;
}

void BlendedInfixSuggesterTest::testNullPrefixToken() 
{

  shared_ptr<BytesRef> payload = make_shared<BytesRef>(L"lake");

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"top of the lake", 8, payload)};

  shared_ptr<Path> tempDir = createTempDir(L"BlendedInfixSuggesterTest");

  shared_ptr<Analyzer> a =
      make_shared<StandardAnalyzer>(CharArraySet::EMPTY_SET);
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(
          newFSDirectory(tempDir), a, a,
          AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
          BlendedInfixSuggester::BlenderType::POSITION_LINEAR,
          BlendedInfixSuggester::DEFAULT_NUM_FACTOR, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  getInResults(suggester, L"of ", payload, 1);
  getInResults(suggester, L"the ", payload, 1);
  getInResults(suggester, L"lake ", payload, 1);

  delete suggester;
}

void BlendedInfixSuggesterTest::
    testBlendedInfixSuggesterDedupsOnWeightTitleAndPayload() throw(
        runtime_error)
{

  // exactly same inputs
  std::deque<std::shared_ptr<Input>> inputDocuments = {
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid1")),
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid1"))};
  duplicateCheck(inputDocuments, 1);

  // inputs differ on payload
  inputDocuments = std::deque<std::shared_ptr<Input>>{
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid1")),
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid2"))};
  duplicateCheck(inputDocuments, 2);

  // exactly same input without payloads
  inputDocuments = std::deque<std::shared_ptr<Input>>{
      make_shared<Input>(L"lend me your ear", 7),
      make_shared<Input>(L"lend me your ear", 7)};
  duplicateCheck(inputDocuments, 1);

  // Same input with first has payloads, second does not
  inputDocuments = std::deque<std::shared_ptr<Input>>{
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid1")),
      make_shared<Input>(L"lend me your ear", 7)};
  duplicateCheck(inputDocuments, 2);

  /**same input, first not having a payload, the second having payload
   * we would expect 2 entries out but we are getting only 1 because
   * the InputArrayIterator#hasPayloads() returns false because the first
   * item has no payload, therefore, when ingested, none of the 2 input has
   * payload and become 1
   */
  inputDocuments = std::deque<std::shared_ptr<Input>>{
      make_shared<Input>(L"lend me your ear", 7),
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid2"))};
  deque<std::shared_ptr<Lookup::LookupResult>> results =
      duplicateCheck(inputDocuments, 1);
  assertNull(results[0]->payload);

  // exactly same inputs but different weight
  inputDocuments = std::deque<std::shared_ptr<Input>>{
      make_shared<Input>(L"lend me your ear", 1,
                         make_shared<BytesRef>(L"uid1")),
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid1"))};
  duplicateCheck(inputDocuments, 2);

  // exactly same inputs but different text
  inputDocuments = std::deque<std::shared_ptr<Input>>{
      make_shared<Input>(L"lend me your earings", 7,
                         make_shared<BytesRef>(L"uid1")),
      make_shared<Input>(L"lend me your ear", 7,
                         make_shared<BytesRef>(L"uid1"))};
  duplicateCheck(inputDocuments, 2);
}

void BlendedInfixSuggesterTest::testSuggesterCountForAllLookups() throw(
    IOException)
{

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ears", 1),
      make_shared<Input>(L"as you sow so shall you reap", 1)};

  shared_ptr<Path> tempDir = createTempDir(L"BlendedInfixSuggesterTest");
  shared_ptr<Analyzer> a =
      make_shared<StandardAnalyzer>(CharArraySet::EMPTY_SET);

  // BlenderType.LINEAR is used by default (remove position*10%)
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(newFSDirectory(tempDir), a);
  suggester->build(make_shared<InputArrayIterator>(keys));

  wstring term = L"you";

  deque<std::shared_ptr<Lookup::LookupResult>> responses =
      suggester->lookup(term, false, 1);
  TestUtil::assertEquals(1, responses.size());

  responses = suggester->lookup(term, false, 2);
  TestUtil::assertEquals(2, responses.size());

  responses = suggester->lookup(term, 1, false, false);
  TestUtil::assertEquals(1, responses.size());

  responses = suggester->lookup(term, 2, false, false);
  TestUtil::assertEquals(2, responses.size());

  responses = suggester->lookup(
      term,
      static_cast<
          unordered_map<std::shared_ptr<BytesRef>, BooleanClause::Occur>>(
          nullptr),
      1, false, false);
  TestUtil::assertEquals(1, responses.size());

  responses = suggester->lookup(
      term,
      static_cast<
          unordered_map<std::shared_ptr<BytesRef>, BooleanClause::Occur>>(
          nullptr),
      2, false, false);
  TestUtil::assertEquals(2, responses.size());

  responses = suggester->lookup(
      term, std::static_pointer_cast<Set<std::shared_ptr<BytesRef>>>(nullptr),
      1, false, false);
  TestUtil::assertEquals(1, responses.size());

  responses = suggester->lookup(
      term, std::static_pointer_cast<Set<std::shared_ptr<BytesRef>>>(nullptr),
      2, false, false);
  TestUtil::assertEquals(2, responses.size());

  responses = suggester->lookup(term, nullptr, false, 1);
  TestUtil::assertEquals(1, responses.size());

  responses = suggester->lookup(term, nullptr, false, 2);
  TestUtil::assertEquals(2, responses.size());

  responses = suggester->lookup(
      term, std::static_pointer_cast<BooleanQuery>(nullptr), 1, false, false);
  TestUtil::assertEquals(1, responses.size());

  responses = suggester->lookup(
      term, std::static_pointer_cast<BooleanQuery>(nullptr), 2, false, false);
  TestUtil::assertEquals(2, responses.size());

  delete suggester;
}

void BlendedInfixSuggesterTest::rying() 
{

  shared_ptr<BytesRef> lake = make_shared<BytesRef>(L"lake");
  shared_ptr<BytesRef> star = make_shared<BytesRef>(L"star");
  shared_ptr<BytesRef> ret = make_shared<BytesRef>(L"ret");

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"top of the lake", 15, lake),
      make_shared<Input>(L"star wars: episode v - the empire strikes back", 12,
                         star),
      make_shared<Input>(L"the returned", 10, ret)};

  shared_ptr<Path> tempDir = createTempDir(L"BlendedInfixSuggesterTest");
  shared_ptr<Analyzer> a =
      make_shared<StandardAnalyzer>(CharArraySet::EMPTY_SET);

  // if factor is small, we don't get the expected element
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(
          newFSDirectory(tempDir), a, a,
          AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
          BlendedInfixSuggester::BlenderType::POSITION_RECIPROCAL,
          BlendedInfixSuggester::DEFAULT_NUM_FACTOR, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<Lookup::LookupResult>> responses =
      suggester->lookup(L"the", 4, true, false);

  for (auto response : responses) {
    wcout << response << endl;
  }

  delete suggester;
}

int64_t BlendedInfixSuggesterTest::getInResults(
    shared_ptr<BlendedInfixSuggester> suggester, const wstring &prefix,
    shared_ptr<BytesRef> payload, int num) 
{

  deque<std::shared_ptr<Lookup::LookupResult>> responses =
      suggester->lookup(prefix, num, true, false);

  for (auto response : responses) {
    if (response->payload->equals(payload)) {
      return response->value;
    }
  }

  return -1;
}

deque<std::shared_ptr<Lookup::LookupResult>>
BlendedInfixSuggesterTest::duplicateCheck(
    std::deque<std::shared_ptr<Input>> &inputs,
    int expectedSuggestionCount) 
{

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<BlendedInfixSuggester> suggester =
      make_shared<BlendedInfixSuggester>(
          newDirectory(), a, a,
          AnalyzingInfixSuggester::DEFAULT_MIN_PREFIX_CHARS,
          BlendedInfixSuggester::BlenderType::POSITION_RECIPROCAL, 10, false);

  shared_ptr<InputArrayIterator> inputArrayIterator =
      make_shared<InputArrayIterator>(inputs);
  suggester->build(inputArrayIterator);

  deque<std::shared_ptr<Lookup::LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
  TestUtil::assertEquals(expectedSuggestionCount, results.size());

  delete suggester;
  delete a;

  return results;
}
} // namespace org::apache::lucene::search::suggest::analyzing