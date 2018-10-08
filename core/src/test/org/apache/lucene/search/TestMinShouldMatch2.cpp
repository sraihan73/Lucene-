using namespace std;

#include "TestMinShouldMatch2.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using SimWeight =
    org::apache::lucene::search::similarities::Similarity::SimWeight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> TestMinShouldMatch2::dir;
shared_ptr<org::apache::lucene::index::DirectoryReader> TestMinShouldMatch2::r;
shared_ptr<org::apache::lucene::index::LeafReader> TestMinShouldMatch2::reader;
shared_ptr<IndexSearcher> TestMinShouldMatch2::searcher;
std::deque<wstring> const TestMinShouldMatch2::alwaysTerms = {L"a"};
std::deque<wstring> const TestMinShouldMatch2::commonTerms = {L"b", L"c",
                                                               L"d"};
std::deque<wstring> const TestMinShouldMatch2::mediumTerms = {L"e", L"f",
                                                               L"g"};
std::deque<wstring> const TestMinShouldMatch2::rareTerms = {
    L"h", L"i", L"j", L"k", L"l", L"m", L"n", L"o", L"p", L"q",
    L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z"};

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestMinShouldMatch2::beforeClass() 
{
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = atLeast(300);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();

    addSome(doc, alwaysTerms);

    if (random()->nextInt(100) < 90) {
      addSome(doc, commonTerms);
    }
    if (random()->nextInt(100) < 50) {
      addSome(doc, mediumTerms);
    }
    if (random()->nextInt(100) < 10) {
      addSome(doc, rareTerms);
    }
    iw->addDocument(doc);
  }
  iw->forceMerge(1);
  delete iw;
  r = DirectoryReader::open(dir);
  reader = getOnlyLeafReader(r);
  searcher = make_shared<IndexSearcher>(reader);
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestMinShouldMatch2::afterClass() 
{
  delete reader;
  delete dir;
  searcher.reset();
  reader.reset();
  r.reset();
  dir.reset();
}

void TestMinShouldMatch2::addSome(shared_ptr<Document> doc,
                                  std::deque<wstring> &values)
{
  deque<wstring> deque = Arrays::asList(values);
  Collections::shuffle(deque, random());
  int howMany = TestUtil::nextInt(random(), 1, deque.size());
  for (int i = 0; i < howMany; i++) {
    doc->push_back(
        make_shared<StringField>(L"field", deque[i], Field::Store::NO));
    doc->push_back(make_shared<SortedSetDocValuesField>(
        L"dv", make_shared<BytesRef>(deque[i])));
  }
}

shared_ptr<Scorer> TestMinShouldMatch2::scorer(std::deque<wstring> &values,
                                               int minShouldMatch,
                                               Mode mode) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (auto value : values) {
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", value)),
            BooleanClause::Occur::SHOULD);
  }
  bq->setMinimumNumberShouldMatch(minShouldMatch);

  shared_ptr<BooleanWeight> weight = std::static_pointer_cast<BooleanWeight>(
      searcher->createWeight(searcher->rewrite(bq->build()), true, 1));

  switch (mode) {
  case org::apache::lucene::search::TestMinShouldMatch2::Mode::DOC_VALUES:
    return make_shared<SlowMinShouldMatchScorer>(weight, reader, searcher);
  case org::apache::lucene::search::TestMinShouldMatch2::Mode::SCORER:
    return weight->scorer(reader->getContext());
  case org::apache::lucene::search::TestMinShouldMatch2::Mode::BULK_SCORER: {
    shared_ptr<BulkScorer> *const bulkScorer =
        weight->optionalBulkScorer(reader->getContext());
    if (bulkScorer == nullptr) {
      if (weight->scorer(reader->getContext()) != nullptr) {
        throw make_shared<AssertionError>(
            L"BooleanScorer should be applicable for this query");
      }
      return nullptr;
    }
    return make_shared<BulkScorerWrapperScorer>(
        weight, bulkScorer, TestUtil::nextInt(random(), 1, 100));
  }
  default:
    throw make_shared<AssertionError>();
  }
}

void TestMinShouldMatch2::assertNext(
    shared_ptr<Scorer> expected, shared_ptr<Scorer> actual) 
{
  if (actual == nullptr) {
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                           expected->begin().nextDoc());
    return;
  }
  int doc;
  shared_ptr<DocIdSetIterator> expectedIt = expected->begin();
  shared_ptr<DocIdSetIterator> actualIt = actual->begin();
  while ((doc = expectedIt->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    TestUtil::assertEquals(doc, actualIt->nextDoc());
    float expectedScore = expected->score();
    float actualScore = actual->score();
    assertEquals(expectedScore, actualScore,
                 CheckHits::explainToleranceDelta(expectedScore, actualScore));
  }
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, actualIt->nextDoc());
}

void TestMinShouldMatch2::assertAdvance(shared_ptr<Scorer> expected,
                                        shared_ptr<Scorer> actual,
                                        int amount) 
{
  if (actual == nullptr) {
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                           expected->begin().nextDoc());
    return;
  }
  shared_ptr<DocIdSetIterator> expectedIt = expected->begin();
  shared_ptr<DocIdSetIterator> actualIt = actual->begin();
  int prevDoc = 0;
  int doc;
  while ((doc = expectedIt->advance(prevDoc + amount)) !=
         DocIdSetIterator::NO_MORE_DOCS) {
    TestUtil::assertEquals(doc, actualIt->advance(prevDoc + amount));
    float expectedScore = expected->score();
    float actualScore = actual->score();
    assertEquals(expectedScore, actualScore,
                 CheckHits::explainToleranceDelta(expectedScore, actualScore));
    prevDoc = doc;
  }
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         actualIt->advance(prevDoc + amount));
}

void TestMinShouldMatch2::testNextCMR2() 
{
  for (int common = 0; common < commonTerms.size(); common++) {
    for (int medium = 0; medium < mediumTerms.size(); medium++) {
      for (int rare = 0; rare < rareTerms.size(); rare++) {
        shared_ptr<Scorer> expected =
            scorer(std::deque<wstring>{commonTerms[common],
                                        mediumTerms[medium], rareTerms[rare]},
                   2, Mode::DOC_VALUES);
        shared_ptr<Scorer> actual =
            scorer(std::deque<wstring>{commonTerms[common],
                                        mediumTerms[medium], rareTerms[rare]},
                   2, Mode::SCORER);
        assertNext(expected, actual);

        expected =
            scorer(std::deque<wstring>{commonTerms[common],
                                        mediumTerms[medium], rareTerms[rare]},
                   2, Mode::DOC_VALUES);
        actual =
            scorer(std::deque<wstring>{commonTerms[common],
                                        mediumTerms[medium], rareTerms[rare]},
                   2, Mode::BULK_SCORER);
        assertNext(expected, actual);
      }
    }
  }
}

void TestMinShouldMatch2::testAdvanceCMR2() 
{
  for (int amount = 25; amount < 200; amount += 25) {
    for (int common = 0; common < commonTerms.size(); common++) {
      for (int medium = 0; medium < mediumTerms.size(); medium++) {
        for (int rare = 0; rare < rareTerms.size(); rare++) {
          shared_ptr<Scorer> expected =
              scorer(std::deque<wstring>{commonTerms[common],
                                          mediumTerms[medium], rareTerms[rare]},
                     2, Mode::DOC_VALUES);
          shared_ptr<Scorer> actual =
              scorer(std::deque<wstring>{commonTerms[common],
                                          mediumTerms[medium], rareTerms[rare]},
                     2, Mode::SCORER);
          assertAdvance(expected, actual, amount);

          expected =
              scorer(std::deque<wstring>{commonTerms[common],
                                          mediumTerms[medium], rareTerms[rare]},
                     2, Mode::DOC_VALUES);
          actual =
              scorer(std::deque<wstring>{commonTerms[common],
                                          mediumTerms[medium], rareTerms[rare]},
                     2, Mode::BULK_SCORER);
          assertAdvance(expected, actual, amount);
        }
      }
    }
  }
}

void TestMinShouldMatch2::testNextAllTerms() 
{
  deque<wstring> termsList = deque<wstring>();
  termsList.addAll(Arrays::asList(commonTerms));
  termsList.addAll(Arrays::asList(mediumTerms));
  termsList.addAll(Arrays::asList(rareTerms));
  std::deque<wstring> terms = termsList.toArray(std::deque<wstring>(0));

  for (int minNrShouldMatch = 1; minNrShouldMatch <= terms.size();
       minNrShouldMatch++) {
    shared_ptr<Scorer> expected =
        scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
    shared_ptr<Scorer> actual = scorer(terms, minNrShouldMatch, Mode::SCORER);
    assertNext(expected, actual);

    expected = scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
    actual = scorer(terms, minNrShouldMatch, Mode::BULK_SCORER);
    assertNext(expected, actual);
  }
}

void TestMinShouldMatch2::testAdvanceAllTerms() 
{
  deque<wstring> termsList = deque<wstring>();
  termsList.addAll(Arrays::asList(commonTerms));
  termsList.addAll(Arrays::asList(mediumTerms));
  termsList.addAll(Arrays::asList(rareTerms));
  std::deque<wstring> terms = termsList.toArray(std::deque<wstring>(0));

  for (int amount = 25; amount < 200; amount += 25) {
    for (int minNrShouldMatch = 1; minNrShouldMatch <= terms.size();
         minNrShouldMatch++) {
      shared_ptr<Scorer> expected =
          scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
      shared_ptr<Scorer> actual = scorer(terms, minNrShouldMatch, Mode::SCORER);
      assertAdvance(expected, actual, amount);

      expected = scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
      actual = scorer(terms, minNrShouldMatch, Mode::BULK_SCORER);
      assertAdvance(expected, actual, amount);
    }
  }
}

void TestMinShouldMatch2::testNextVaryingNumberOfTerms() 
{
  deque<wstring> termsList = deque<wstring>();
  termsList.addAll(Arrays::asList(commonTerms));
  termsList.addAll(Arrays::asList(mediumTerms));
  termsList.addAll(Arrays::asList(rareTerms));
  Collections::shuffle(termsList, random());
  for (int numTerms = 2; numTerms <= termsList.size(); numTerms++) {
    std::deque<wstring> terms =
        termsList.subList(0, numTerms).toArray(std::deque<wstring>(0));
    for (int minNrShouldMatch = 1; minNrShouldMatch <= terms.size();
         minNrShouldMatch++) {
      shared_ptr<Scorer> expected =
          scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
      shared_ptr<Scorer> actual = scorer(terms, minNrShouldMatch, Mode::SCORER);
      assertNext(expected, actual);

      expected = scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
      actual = scorer(terms, minNrShouldMatch, Mode::BULK_SCORER);
      assertNext(expected, actual);
    }
  }
}

void TestMinShouldMatch2::testAdvanceVaryingNumberOfTerms() 
{
  deque<wstring> termsList = deque<wstring>();
  termsList.addAll(Arrays::asList(commonTerms));
  termsList.addAll(Arrays::asList(mediumTerms));
  termsList.addAll(Arrays::asList(rareTerms));
  Collections::shuffle(termsList, random());

  for (int amount = 25; amount < 200; amount += 25) {
    for (int numTerms = 2; numTerms <= termsList.size(); numTerms++) {
      std::deque<wstring> terms =
          termsList.subList(0, numTerms).toArray(std::deque<wstring>(0));
      for (int minNrShouldMatch = 1; minNrShouldMatch <= terms.size();
           minNrShouldMatch++) {
        shared_ptr<Scorer> expected =
            scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
        shared_ptr<Scorer> actual =
            scorer(terms, minNrShouldMatch, Mode::SCORER);
        assertAdvance(expected, actual, amount);

        expected = scorer(terms, minNrShouldMatch, Mode::DOC_VALUES);
        actual = scorer(terms, minNrShouldMatch, Mode::SCORER);
        assertAdvance(expected, actual, amount);
      }
    }
  }
}

TestMinShouldMatch2::SlowMinShouldMatchScorer::SlowMinShouldMatchScorer(
    shared_ptr<BooleanWeight> weight, shared_ptr<LeafReader> reader,
    shared_ptr<IndexSearcher> searcher) 
    : Scorer(weight), dv(reader->getSortedSetDocValues(L"dv")),
      maxDoc(reader->maxDoc()), sims(std::deque<std::shared_ptr<SimScorer>>(
                                    static_cast<int>(dv->getValueCount()))),
      minNrShouldMatch(bq::getMinimumNumberShouldMatch())
{
  shared_ptr<BooleanQuery> bq =
      std::static_pointer_cast<BooleanQuery>(weight->getQuery());
  for (auto clause : bq->clauses()) {
    assert(!clause->isProhibited());
    assert(!clause->isRequired());
    shared_ptr<Term> term =
        (std::static_pointer_cast<TermQuery>(clause->getQuery()))->getTerm();
    int64_t ord = dv->lookupTerm(term->bytes());
    if (ord >= 0) {
      bool success = ords->add(ord);
      assert(success); // no dups
      shared_ptr<TermContext> context =
          TermContext::build(reader->getContext(), term);
      shared_ptr<SimWeight> w = weight->similarity->computeWeight(
          1.0f, searcher->collectionStatistics(L"field"),
          {searcher->termStatistics(term, context)});
      sims[static_cast<int>(ord)] =
          weight->similarity->simScorer(w, reader->getContext());
    }
  }
}

float TestMinShouldMatch2::SlowMinShouldMatchScorer::score() 
{
  assert((score_ != 0, currentMatched));
  return static_cast<float>(score_);
}

int TestMinShouldMatch2::SlowMinShouldMatchScorer::docID()
{
  return currentDoc;
}

shared_ptr<DocIdSetIterator>
TestMinShouldMatch2::SlowMinShouldMatchScorer::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

TestMinShouldMatch2::SlowMinShouldMatchScorer::
    DocIdSetIteratorAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<SlowMinShouldMatchScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestMinShouldMatch2::SlowMinShouldMatchScorer::
    DocIdSetIteratorAnonymousInnerClass::nextDoc() 
{
  assert(outerInstance->currentDoc != NO_MORE_DOCS);
  for (outerInstance->currentDoc = outerInstance->currentDoc + 1;
       outerInstance->currentDoc < outerInstance->maxDoc;
       outerInstance->currentDoc++) {
    outerInstance->currentMatched = 0;
    outerInstance->score_ = 0;
    if (outerInstance->currentDoc > outerInstance->dv->docID()) {
      outerInstance->dv->advance(outerInstance->currentDoc);
    }
    if (outerInstance->currentDoc != outerInstance->dv->docID()) {
      continue;
    }
    int64_t ord;
    while ((ord = outerInstance->dv->nextOrd()) !=
           SortedSetDocValues::NO_MORE_ORDS) {
      if (outerInstance->ords->contains(ord)) {
        outerInstance->currentMatched++;
        outerInstance->score_ +=
            outerInstance->sims[static_cast<int>(ord)]->score(
                outerInstance->currentDoc, 1);
      }
    }
    if (outerInstance->currentMatched >= outerInstance->minNrShouldMatch) {
      return outerInstance->currentDoc;
    }
  }
  return outerInstance->currentDoc = NO_MORE_DOCS;
}

int TestMinShouldMatch2::SlowMinShouldMatchScorer::
    DocIdSetIteratorAnonymousInnerClass::advance(int target) 
{
  int doc;
  while ((doc = nextDoc()) < target) {
  }
  return doc;
}

int64_t TestMinShouldMatch2::SlowMinShouldMatchScorer::
    DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int TestMinShouldMatch2::SlowMinShouldMatchScorer::
    DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->currentDoc;
}
} // namespace org::apache::lucene::search