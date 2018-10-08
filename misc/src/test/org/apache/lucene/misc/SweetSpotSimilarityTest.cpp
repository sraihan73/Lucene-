using namespace std;

#include "SweetSpotSimilarityTest.h"

namespace org::apache::lucene::misc
{
using Store = org::apache::lucene::document::Field::Store;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

float SweetSpotSimilarityTest::computeNorm(shared_ptr<Similarity> sim,
                                           const wstring &field,
                                           int length) 
{
  wstring value = IntStream::range(0, length)
                      .mapToObj([&](any i) { L"a"; })
                      .collect(Collectors::joining(L" "));
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig()->setSimilarity(sim));
  w->addDocument(Collections::singleton(newTextField(field, value, Store::NO)));
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(w);
  delete w;
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  searcher->setSimilarity(sim);
  shared_ptr<Explanation> expl = searcher->explain(
      make_shared<TermQuery>(make_shared<Term>(field, L"a")), 0);
  reader->close();
  delete dir;
  shared_ptr<Explanation> norm = findExplanation(expl, L"fieldNorm");
  assertNotNull(norm);
  return norm->getValue();
}

shared_ptr<Explanation>
SweetSpotSimilarityTest::findExplanation(shared_ptr<Explanation> expl,
                                         const wstring &text)
{
  if (StringHelper::startsWith(expl->getDescription(), text)) {
    return expl;
  } else {
    for (auto sub : expl->getDetails()) {
      shared_ptr<Explanation> match = findExplanation(sub, text);
      if (match != nullptr) {
        return match;
      }
    }
  }
  return nullptr;
}

void SweetSpotSimilarityTest::testSweetSpotComputeNorm() 
{

  shared_ptr<SweetSpotSimilarity> *const ss =
      make_shared<SweetSpotSimilarity>();
  ss->setLengthNormFactors(1, 1, 0.5f, true);

  shared_ptr<Similarity> d = make_shared<ClassicSimilarity>();
  shared_ptr<Similarity> s = ss;

  // base case, should degrade
  for (int i = 1; i < 1000; i++) {
    assertEquals(L"base case: i=" + to_wstring(i), computeNorm(d, L"bogus", i),
                 computeNorm(s, L"bogus", i), 0.0f);
  }

  // make a sweet spot

  ss->setLengthNormFactors(3, 10, 0.5f, true);

  for (int i = 3; i <= 10; i++) {
    assertEquals(L"3,10: spot i=" + to_wstring(i), 1.0f,
                 computeNorm(ss, L"bogus", i), 0.0f);
  }

  for (int i = 10; i < 1000; i++) {
    constexpr float normD = computeNorm(d, L"bogus", i - 9);
    constexpr float normS = computeNorm(s, L"bogus", i);
    assertEquals(L"3,10: 10<x : i=" + to_wstring(i), normD, normS, 0.01f);
  }

  // separate sweet spot for certain fields

  shared_ptr<SweetSpotSimilarity> *const ssBar =
      make_shared<SweetSpotSimilarity>();
  ssBar->setLengthNormFactors(8, 13, 0.5f, false);
  shared_ptr<SweetSpotSimilarity> *const ssYak =
      make_shared<SweetSpotSimilarity>();
  ssYak->setLengthNormFactors(6, 9, 0.5f, false);
  shared_ptr<SweetSpotSimilarity> *const ssA =
      make_shared<SweetSpotSimilarity>();
  ssA->setLengthNormFactors(5, 8, 0.5f, false);
  shared_ptr<SweetSpotSimilarity> *const ssB =
      make_shared<SweetSpotSimilarity>();
  ssB->setLengthNormFactors(5, 8, 0.1f, false);

  shared_ptr<Similarity> sp =
      make_shared<PerFieldSimilarityWrapperAnonymousInnerClass>(
          shared_from_this(), ss, ssBar, ssYak, ssA, ssB);

  for (int i = 3; i <= 10; i++) {
    assertEquals(L"f: 3,10: spot i=" + to_wstring(i), 1.0f,
                 computeNorm(sp, L"foo", i), 0.0f);
  }

  for (int i = 10; i < 1000; i++) {
    constexpr float normD = computeNorm(d, L"foo", i - 9);
    constexpr float normS = computeNorm(sp, L"foo", i);
    assertEquals(L"f: 3,10: 10<x : i=" + to_wstring(i), normD, normS, 0.01f);
  }

  for (int i = 8; i <= 13; i++) {
    assertEquals(L"f: 8,13: spot i=" + to_wstring(i), 1.0f,
                 computeNorm(sp, L"bar", i), 0.01f);
  }

  for (int i = 6; i <= 9; i++) {
    assertEquals(L"f: 6,9: spot i=" + to_wstring(i), 1.0f,
                 computeNorm(sp, L"yak", i), 0.01f);
  }

  for (int i = 13; i < 1000; i++) {
    constexpr float normD = computeNorm(d, L"bar", i - 12);
    constexpr float normS = computeNorm(sp, L"bar", i);
    assertEquals(L"f: 8,13: 13<x : i=" + to_wstring(i), normD, normS, 0.01f);
  }

  for (int i = 9; i < 1000; i++) {
    constexpr float normD = computeNorm(d, L"yak", i - 8);
    constexpr float normS = computeNorm(sp, L"yak", i);
    assertEquals(L"f: 6,9: 9<x : i=" + to_wstring(i), normD, normS, 0.01f);
  }

  // steepness

  for (int i = 9; i < 1000; i++) {
    constexpr float normSS = computeNorm(sp, L"a", i);
    constexpr float normS = computeNorm(sp, L"b", i);
    assertTrue(L"s: i=" + to_wstring(i) + L" : a=" + to_wstring(normSS) +
                   L" < b=" + to_wstring(normS),
               normSS < normS);
  }
}

SweetSpotSimilarityTest::PerFieldSimilarityWrapperAnonymousInnerClass::
    PerFieldSimilarityWrapperAnonymousInnerClass(
        shared_ptr<SweetSpotSimilarityTest> outerInstance,
        shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ss,
        shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssBar,
        shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssYak,
        shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssA,
        shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssB)
{
  this->outerInstance = outerInstance;
  this->ss = ss;
  this->ssBar = ssBar;
  this->ssYak = ssYak;
  this->ssA = ssA;
  this->ssB = ssB;
}

shared_ptr<Similarity>
SweetSpotSimilarityTest::PerFieldSimilarityWrapperAnonymousInnerClass::get(
    const wstring &field)
{
  if (field == L"bar") {
    return ssBar;
  } else if (field == L"yak") {
    return ssYak;
  } else if (field == L"a") {
    return ssA;
  } else if (field == L"b") {
    return ssB;
  } else {
    return ss;
  }
}

void SweetSpotSimilarityTest::testSweetSpotTf()
{

  shared_ptr<SweetSpotSimilarity> ss = make_shared<SweetSpotSimilarity>();

  shared_ptr<TFIDFSimilarity> d = make_shared<ClassicSimilarity>();
  shared_ptr<TFIDFSimilarity> s = ss;

  // tf equal

  ss->setBaselineTfFactors(0.0f, 0.0f);

  for (int i = 1; i < 1000; i++) {
    assertEquals(L"tf: i=" + to_wstring(i), d->tf(i), s->tf(i), 0.0f);
  }

  // tf higher

  ss->setBaselineTfFactors(1.0f, 0.0f);

  for (int i = 1; i < 1000; i++) {
    assertTrue(L"tf: i=" + to_wstring(i) + L" : d=" + to_wstring(d->tf(i)) +
                   L" < s=" + to_wstring(s->tf(i)),
               d->tf(i) < s->tf(i));
  }

  // tf flat

  ss->setBaselineTfFactors(1.0f, 6.0f);
  for (int i = 1; i <= 6; i++) {
    assertEquals(L"tf flat1: i=" + to_wstring(i), 1.0f, s->tf(i), 0.0f);
  }
  ss->setBaselineTfFactors(2.0f, 6.0f);
  for (int i = 1; i <= 6; i++) {
    assertEquals(L"tf flat2: i=" + to_wstring(i), 2.0f, s->tf(i), 0.0f);
  }
  for (int i = 6; i <= 1000; i++) {
    assertTrue(L"tf: i=" + to_wstring(i) + L" : s=" + to_wstring(s->tf(i)) +
                   L" < d=" + to_wstring(d->tf(i)),
               s->tf(i) < d->tf(i));
  }

  // stupidity
  assertEquals(L"tf zero", 0.0f, s->tf(0), 0.0f);
}

void SweetSpotSimilarityTest::testHyperbolicSweetSpot()
{

  shared_ptr<SweetSpotSimilarity> ss =
      make_shared<SweetSpotSimilarityAnonymousInnerClass>(shared_from_this());
  ss->setHyperbolicTfFactors(3.3f, 7.7f, M_E, 5.0f);

  shared_ptr<TFIDFSimilarity> s = ss;

  for (int i = 1; i <= 1000; i++) {
    assertTrue(L"MIN tf: i=" + to_wstring(i) + L" : s=" + to_wstring(s->tf(i)),
               3.3f <= s->tf(i));
    assertTrue(L"MAX tf: i=" + to_wstring(i) + L" : s=" + to_wstring(s->tf(i)),
               s->tf(i) <= 7.7f);
  }
  assertEquals(L"MID tf", 3.3f + (7.7f - 3.3f) / 2.0f, s->tf(5), 0.00001f);

  // stupidity
  assertEquals(L"tf zero", 0.0f, s->tf(0), 0.0f);
}

SweetSpotSimilarityTest::SweetSpotSimilarityAnonymousInnerClass::
    SweetSpotSimilarityAnonymousInnerClass(
        shared_ptr<SweetSpotSimilarityTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

float SweetSpotSimilarityTest::SweetSpotSimilarityAnonymousInnerClass::tf(
    float freq)
{
  return hyperbolicTf(freq);
}
} // namespace org::apache::lucene::misc