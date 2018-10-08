using namespace std;

#include "CommonTermsQueryTest.h"

namespace org::apache::lucene::queries
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using LeafReader = org::apache::lucene::index::LeafReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

void CommonTermsQueryTest::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  std::deque<wstring> docs = {
      L"this is the end of the world right", L"is this it or maybe not",
      L"this is the end of the universe as we know it",
      L"there is the famous restaurant at the end of the universe"};
  for (int i = 0; i < docs.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    doc->push_back(newTextField(L"field", docs[i], Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 3);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
    TestUtil::assertEquals(L"2", r->document(search->scoreDocs[1]->doc)[L"id"]);
    TestUtil::assertEquals(L"3", r->document(search->scoreDocs[2]->doc)[L"id"]);
  }

  { // only high freq
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 2);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
    TestUtil::assertEquals(L"2", r->document(search->scoreDocs[1]->doc)[L"id"]);
  }

  { // low freq is mandatory
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::MUST, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));

    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 1);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
  }

  { // low freq is mandatory
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::MUST, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"restaurant"));
    query->add(make_shared<Term>(L"field", L"universe"));

    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 1);
    TestUtil::assertEquals(L"3", r->document(search->scoreDocs[0]->doc)[L"id"]);
  }
  IOUtils::close({r, w, dir, analyzer});
}

void CommonTermsQueryTest::testEqualsHashCode()
{
  shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
      randomOccur(random()), randomOccur(random()), random()->nextFloat());
  int terms = atLeast(2);
  for (int i = 0; i < terms; i++) {
    query->add(
        make_shared<Term>(TestUtil::randomRealisticUnicodeString(random()),
                          TestUtil::randomRealisticUnicodeString(random())));
  }
  QueryUtils::checkHashEquals(query);
  QueryUtils::checkUnequal(make_shared<CommonTermsQuery>(randomOccur(random()),
                                                         randomOccur(random()),
                                                         random()->nextFloat()),
                           query);

  {
    constexpr int64_t seed = random()->nextLong();
    shared_ptr<Random> r = make_shared<Random>(seed);
    shared_ptr<CommonTermsQuery> left = make_shared<CommonTermsQuery>(
        randomOccur(r), randomOccur(r), r->nextFloat());
    int leftTerms = atLeast(r, 2);
    for (int i = 0; i < leftTerms; i++) {
      left->add(make_shared<Term>(TestUtil::randomRealisticUnicodeString(r),
                                  TestUtil::randomRealisticUnicodeString(r)));
    }
    left->setHighFreqMinimumNumberShouldMatch(r->nextInt(4));
    left->setLowFreqMinimumNumberShouldMatch(r->nextInt(4));

    r = make_shared<Random>(seed);
    shared_ptr<CommonTermsQuery> right = make_shared<CommonTermsQuery>(
        randomOccur(r), randomOccur(r), r->nextFloat());
    int rightTerms = atLeast(r, 2);
    for (int i = 0; i < rightTerms; i++) {
      right->add(make_shared<Term>(TestUtil::randomRealisticUnicodeString(r),
                                   TestUtil::randomRealisticUnicodeString(r)));
    }
    right->setHighFreqMinimumNumberShouldMatch(r->nextInt(4));
    right->setLowFreqMinimumNumberShouldMatch(r->nextInt(4));
    QueryUtils::checkEqual(left, right);
  }
}

Occur CommonTermsQueryTest::randomOccur(shared_ptr<Random> random)
{
  return random->nextBoolean() ? Occur::MUST : Occur::SHOULD;
}

void CommonTermsQueryTest::testNullTerm()
{
  shared_ptr<Random> random = CommonTermsQueryTest::random();
  shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
      randomOccur(random), randomOccur(random),
      CommonTermsQueryTest::random()->nextFloat());
  // null values are not supported
  expectThrows(invalid_argument::typeid, [&]() { query->add(nullptr); });
}

void CommonTermsQueryTest::testMinShouldMatch() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  std::deque<wstring> docs = {
      L"this is the end of the world right", L"is this it or maybe not",
      L"this is the end of the universe as we know it",
      L"there is the famous restaurant at the end of the universe"};
  for (int i = 0; i < docs.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    doc->push_back(newTextField(L"field", docs[i], Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    query->setLowFreqMinimumNumberShouldMatch(0.5f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 1);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
  }
  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    query->setLowFreqMinimumNumberShouldMatch(2.0f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 1);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
  }

  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    query->setLowFreqMinimumNumberShouldMatch(0.49f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 3);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
    TestUtil::assertEquals(L"2", r->document(search->scoreDocs[1]->doc)[L"id"]);
    TestUtil::assertEquals(L"3", r->document(search->scoreDocs[2]->doc)[L"id"]);
  }

  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    query->setLowFreqMinimumNumberShouldMatch(1.0f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 3);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
    TestUtil::assertEquals(L"2", r->document(search->scoreDocs[1]->doc)[L"id"]);
    TestUtil::assertEquals(L"3", r->document(search->scoreDocs[2]->doc)[L"id"]);
    assertTrue(search->scoreDocs[1]->score >= search->scoreDocs[2]->score);
  }

  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    query->setLowFreqMinimumNumberShouldMatch(1.0f);
    query->setHighFreqMinimumNumberShouldMatch(4.0f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 3);
    assertEquals(search->scoreDocs[1]->score, search->scoreDocs[2]->score,
                 0.0f);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
    // doc 2 and 3 only get a score from low freq terms
    TestUtil::assertEquals(unordered_set<>(Arrays::asList(L"2", L"3")),
                           unordered_set<>(Arrays::asList(
                               r->document(search->scoreDocs[1]->doc)[L"id"],
                               r->document(search->scoreDocs[2]->doc)[L"id"])));
  }

  {
    // only high freq terms around - check that min should match is applied
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"the"));
    query->setLowFreqMinimumNumberShouldMatch(1.0f);
    query->setHighFreqMinimumNumberShouldMatch(2.0f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 4);
  }

  {
    // only high freq terms around - check that min should match is applied
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::MUST, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"the"));
    query->setLowFreqMinimumNumberShouldMatch(1.0f);
    query->setHighFreqMinimumNumberShouldMatch(2.0f);
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 2);
    TestUtil::assertEquals(unordered_set<>(Arrays::asList(L"0", L"2")),
                           unordered_set<>(Arrays::asList(
                               r->document(search->scoreDocs[0]->doc)[L"id"],
                               r->document(search->scoreDocs[1]->doc)[L"id"])));
  }
  IOUtils::close({r, w, dir, analyzer});
}

void CommonTermsQueryTest::testIllegalOccur()
{
  shared_ptr<Random> random = CommonTermsQueryTest::random();

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<CommonTermsQuery>(Occur::MUST_NOT, randomOccur(random),
                                  CommonTermsQueryTest::random()->nextFloat());
  });

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<CommonTermsQuery>(randomOccur(random), Occur::MUST_NOT,
                                  CommonTermsQueryTest::random()->nextFloat());
  });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExtend() throws java.io.IOException
void CommonTermsQueryTest::testExtend() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  std::deque<wstring> docs = {
      L"this is the end of the world right", L"is this it or maybe not",
      L"this is the end of the universe as we know it",
      L"there is the famous restaurant at the end of the universe"};
  for (int i = 0; i < docs.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    doc->push_back(newTextField(L"field", docs[i], Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  // don't use a randomized similarity, e.g. stopwords for DFI can get scored as
  // 0, so boosting them is kind of crazy
  s->setSimilarity(make_shared<BM25Similarity>());
  {
    shared_ptr<CommonTermsQuery> query = make_shared<CommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 3);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[0]->doc)[L"id"]);
    TestUtil::assertEquals(L"2", r->document(search->scoreDocs[1]->doc)[L"id"]);
    TestUtil::assertEquals(L"3", r->document(search->scoreDocs[2]->doc)[L"id"]);
  }

  {
    // this one boosts the termQuery("field" "universe") by 10x
    shared_ptr<CommonTermsQuery> query = make_shared<ExtendedCommonTermsQuery>(
        Occur::SHOULD, Occur::SHOULD, random()->nextBoolean() ? 2.0f : 0.5f);
    query->add(make_shared<Term>(L"field", L"is"));
    query->add(make_shared<Term>(L"field", L"this"));
    query->add(make_shared<Term>(L"field", L"end"));
    query->add(make_shared<Term>(L"field", L"world"));
    query->add(make_shared<Term>(L"field", L"universe"));
    query->add(make_shared<Term>(L"field", L"right"));
    shared_ptr<TopDocs> search = s->search(query, 10);
    TestUtil::assertEquals(search->totalHits, 3);
    TestUtil::assertEquals(L"2", r->document(search->scoreDocs[0]->doc)[L"id"]);
    TestUtil::assertEquals(L"3", r->document(search->scoreDocs[1]->doc)[L"id"]);
    TestUtil::assertEquals(L"0", r->document(search->scoreDocs[2]->doc)[L"id"]);
  }
  IOUtils::close({r, w, dir, analyzer});
}

void CommonTermsQueryTest::testRandomIndex() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  createRandomIndex(atLeast(50), w, random()->nextLong());
  w->forceMerge(1);
  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<LeafReader> wrapper = getOnlyLeafReader(reader);
  wstring field = L"body";
  shared_ptr<Terms> terms = wrapper->terms(field);
  shared_ptr<PriorityQueue<std::shared_ptr<TermAndFreq>>> lowFreqQueue =
      make_shared<PriorityQueueAnonymousInnerClass>(shared_from_this());
  shared_ptr<PriorityQueue<std::shared_ptr<TermAndFreq>>> highFreqQueue =
      make_shared<PriorityQueueAnonymousInnerClass2>(shared_from_this());
  try {
    shared_ptr<TermsEnum> iterator = terms->begin();
    while (iterator->next() != nullptr) {
      if (highFreqQueue->size() < 5) {
        highFreqQueue->push_back(make_shared<TermAndFreq>(
            BytesRef::deepCopyOf(iterator->term()), iterator->docFreq()));
        lowFreqQueue->push_back(make_shared<TermAndFreq>(
            BytesRef::deepCopyOf(iterator->term()), iterator->docFreq()));
      } else {
        if (highFreqQueue->top()->freq < iterator->docFreq()) {
          highFreqQueue->top()->freq = iterator->docFreq();
          highFreqQueue->top()->term = BytesRef::deepCopyOf(iterator->term());
          highFreqQueue->updateTop();
        }

        if (lowFreqQueue->top()->freq > iterator->docFreq()) {
          lowFreqQueue->top()->freq = iterator->docFreq();
          lowFreqQueue->top()->term = BytesRef::deepCopyOf(iterator->term());
          lowFreqQueue->updateTop();
        }
      }
    }
    int lowFreq = lowFreqQueue->top()->freq;
    int highFreq = highFreqQueue->top()->freq;
    assumeTrue(L"unlucky index", highFreq - 1 > lowFreq);
    deque<std::shared_ptr<TermAndFreq>> highTerms = queueToList(highFreqQueue);
    deque<std::shared_ptr<TermAndFreq>> lowTerms = queueToList(lowFreqQueue);

    shared_ptr<IndexSearcher> searcher = newSearcher(reader);
    Occur lowFreqOccur = randomOccur(random());
    shared_ptr<BooleanQuery::Builder> verifyQuery =
        make_shared<BooleanQuery::Builder>();
    shared_ptr<CommonTermsQuery> cq = make_shared<CommonTermsQuery>(
        randomOccur(random()), lowFreqOccur, highFreq - 1);
    for (auto termAndFreq : lowTerms) {
      cq->add(make_shared<Term>(field, termAndFreq->term));
      verifyQuery->add(make_shared<BooleanClause>(
          make_shared<TermQuery>(make_shared<Term>(field, termAndFreq->term)),
          lowFreqOccur));
    }
    for (auto termAndFreq : highTerms) {
      cq->add(make_shared<Term>(field, termAndFreq->term));
    }

    shared_ptr<TopDocs> cqSearch = searcher->search(cq, reader->maxDoc());

    shared_ptr<TopDocs> verifySearch =
        searcher->search(verifyQuery->build(), reader->maxDoc());
    TestUtil::assertEquals(verifySearch->totalHits, cqSearch->totalHits);
    shared_ptr<Set<int>> hits = unordered_set<int>();
    for (auto doc : verifySearch->scoreDocs) {
      hits->add(doc->doc);
    }

    for (auto doc : cqSearch->scoreDocs) {
      assertTrue(hits->remove(doc->doc));
    }

    assertTrue(hits->isEmpty());

    /*
     *  need to force merge here since QueryUtils adds checks based
     *  on leave readers which have different statistics than the top
     *  level reader if we have more than one segment. This could
     *  result in a different query / results.
     */
    w->forceMerge(1);
    shared_ptr<DirectoryReader> reader2 = w->getReader();
    QueryUtils::check(random(), cq, newSearcher(reader2));
    reader2->close();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({reader, w, dir, analyzer});
  }
}

CommonTermsQueryTest::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(
        shared_ptr<CommonTermsQueryTest> outerInstance)
    : org::apache::lucene::util::PriorityQueue<
          CommonTermsQueryTest::TermAndFreq>(5)
{
  this->outerInstance = outerInstance;
}

bool CommonTermsQueryTest::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<TermAndFreq> a, shared_ptr<TermAndFreq> b)
{
  return a->freq > b->freq;
}

CommonTermsQueryTest::PriorityQueueAnonymousInnerClass2::
    PriorityQueueAnonymousInnerClass2(
        shared_ptr<CommonTermsQueryTest> outerInstance)
    : org::apache::lucene::util::PriorityQueue<
          CommonTermsQueryTest::TermAndFreq>(5)
{
  this->outerInstance = outerInstance;
}

bool CommonTermsQueryTest::PriorityQueueAnonymousInnerClass2::lessThan(
    shared_ptr<TermAndFreq> a, shared_ptr<TermAndFreq> b)
{
  return a->freq < b->freq;
}

deque<std::shared_ptr<TermAndFreq>> CommonTermsQueryTest::queueToList(
    shared_ptr<PriorityQueue<std::shared_ptr<TermAndFreq>>> queue)
{
  deque<std::shared_ptr<TermAndFreq>> terms =
      deque<std::shared_ptr<TermAndFreq>>();
  while (queue->size() > 0) {
    terms.push_back(queue->pop());
  }
  return terms;
}

CommonTermsQueryTest::TermAndFreq::TermAndFreq(shared_ptr<BytesRef> term,
                                               int freq)
{
  this->term = term;
  this->freq = freq;
}

void CommonTermsQueryTest::createRandomIndex(
    int numdocs, shared_ptr<RandomIndexWriter> writer,
    int64_t seed) 
{
  shared_ptr<Random> random = make_shared<Random>(seed);
  // primary source for our data is from linefiledocs, it's realistic.
  shared_ptr<LineFileDocs> lineFileDocs = make_shared<LineFileDocs>(random);

  // TODO: we should add other fields that use things like docs&freqs but omit
  // positions,
  // because linefiledocs doesn't cover all the possibilities.
  for (int i = 0; i < numdocs; i++) {
    writer->addDocument(lineFileDocs->nextDoc());
  }

  delete lineFileDocs;
}

CommonTermsQueryTest::ExtendedCommonTermsQuery::ExtendedCommonTermsQuery(
    Occur highFreqOccur, Occur lowFreqOccur, float maxTermFrequency)
    : CommonTermsQuery(highFreqOccur, lowFreqOccur, maxTermFrequency)
{
}

shared_ptr<Query> CommonTermsQueryTest::ExtendedCommonTermsQuery::newTermQuery(
    shared_ptr<Term> term, shared_ptr<TermContext> context)
{
  shared_ptr<Query> query = CommonTermsQuery::newTermQuery(term, context);
  if (term->text() == L"universe") {
    query = make_shared<BoostQuery>(query, 100.0f);
  }
  return query;
}
} // namespace org::apache::lucene::queries