using namespace std;

#include "TermInSetQueryTest.h"

namespace org::apache::lucene::search
{
using com::carrotsearch::randomizedtesting::generators::RandomStrings;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using TestUtil = org::apache::lucene::util::TestUtil;

void TermInSetQueryTest::testDuel() 
{
  constexpr int iters = atLeast(2);
  const wstring field = L"f";
  for (int iter = 0; iter < iters; ++iter) {
    const deque<std::shared_ptr<BytesRef>> allTerms =
        deque<std::shared_ptr<BytesRef>>();
    constexpr int numTerms =
        TestUtil::nextInt(random(), 1, 1 << TestUtil::nextInt(random(), 1, 10));
    for (int i = 0; i < numTerms; ++i) {
      const wstring value = TestUtil::randomAnalysisString(random(), 10, true);
      allTerms.push_back(make_shared<BytesRef>(value));
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      shared_ptr<BytesRef> *const term =
          allTerms[random()->nextInt(allTerms.size())];
      doc->push_back(make_shared<StringField>(field, term, Store::NO));
      iw->addDocument(doc);
    }
    if (numTerms > 1 && random()->nextBoolean()) {
      iw->deleteDocuments(
          make_shared<TermQuery>(make_shared<Term>(field, allTerms[0])));
    }
    iw->commit();
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    delete iw;

    if (reader->numDocs() == 0) {
      // may occasionally happen if all documents got the same term
      IOUtils::close({reader, dir});
      continue;
    }

    for (int i = 0; i < 100; ++i) {
      constexpr float boost = random()->nextFloat() * 10;
      constexpr int numQueryTerms = TestUtil::nextInt(
          random(), 1, 1 << TestUtil::nextInt(random(), 1, 8));
      deque<std::shared_ptr<BytesRef>> queryTerms =
          deque<std::shared_ptr<BytesRef>>();
      for (int j = 0; j < numQueryTerms; ++j) {
        queryTerms.push_back(allTerms[random()->nextInt(allTerms.size())]);
      }
      shared_ptr<BooleanQuery::Builder> *const bq =
          make_shared<BooleanQuery::Builder>();
      for (auto t : queryTerms) {
        bq->add(make_shared<TermQuery>(make_shared<Term>(field, t)),
                Occur::SHOULD);
      }
      shared_ptr<Query> *const q1 =
          make_shared<ConstantScoreQuery>(bq->build());
      shared_ptr<Query> *const q2 =
          make_shared<TermInSetQuery>(field, queryTerms);
      assertSameMatches(searcher, make_shared<BoostQuery>(q1, boost),
                        make_shared<BoostQuery>(q2, boost), true);
    }

    delete reader;
    delete dir;
  }
}

void TermInSetQueryTest::assertSameMatches(shared_ptr<IndexSearcher> searcher,
                                           shared_ptr<Query> q1,
                                           shared_ptr<Query> q2,
                                           bool scores) 
{
  constexpr int maxDoc = searcher->getIndexReader()->maxDoc();
  shared_ptr<TopDocs> *const td1 =
      searcher->search(q1, maxDoc, scores ? Sort::RELEVANCE : Sort::INDEXORDER);
  shared_ptr<TopDocs> *const td2 =
      searcher->search(q2, maxDoc, scores ? Sort::RELEVANCE : Sort::INDEXORDER);
  TestUtil::assertEquals(td1->totalHits, td2->totalHits);
  for (int i = 0; i < td1->scoreDocs.size(); ++i) {
    TestUtil::assertEquals(td1->scoreDocs[i]->doc, td2->scoreDocs[i]->doc);
    if (scores) {
      assertEquals(td1->scoreDocs[i]->score, td2->scoreDocs[i]->score, 10e-7);
    }
  }
}

void TermInSetQueryTest::testHashCodeAndEquals()
{
  int num = atLeast(100);
  deque<std::shared_ptr<BytesRef>> terms = deque<std::shared_ptr<BytesRef>>();
  shared_ptr<Set<std::shared_ptr<BytesRef>>> uniqueTerms =
      unordered_set<std::shared_ptr<BytesRef>>();
  for (int i = 0; i < num; i++) {
    wstring string = TestUtil::randomRealisticUnicodeString(random());
    terms.push_back(make_shared<BytesRef>(string));
    uniqueTerms->add(make_shared<BytesRef>(string));
    shared_ptr<TermInSetQuery> left =
        make_shared<TermInSetQuery>(L"field", uniqueTerms);
    Collections::shuffle(terms, random());
    shared_ptr<TermInSetQuery> right =
        make_shared<TermInSetQuery>(L"field", terms);
    TestUtil::assertEquals(right, left);
    TestUtil::assertEquals(right->hashCode(), left->hashCode());
    if (uniqueTerms->size() > 1) {
      deque<std::shared_ptr<BytesRef>> asList =
          deque<std::shared_ptr<BytesRef>>(uniqueTerms);
      asList.erase(asList.begin());
      shared_ptr<TermInSetQuery> notEqual =
          make_shared<TermInSetQuery>(L"field", asList);
      assertFalse(left->equals(notEqual));
      assertFalse(right->equals(notEqual));
    }
  }

  shared_ptr<TermInSetQuery> tq1 =
      make_shared<TermInSetQuery>(L"thing", make_shared<BytesRef>(L"apple"));
  shared_ptr<TermInSetQuery> tq2 =
      make_shared<TermInSetQuery>(L"thing", make_shared<BytesRef>(L"orange"));
  assertFalse(tq1->hashCode() == tq2->hashCode());

  // different fields with the same term should have differing hashcodes
  tq1 = make_shared<TermInSetQuery>(L"thing", make_shared<BytesRef>(L"apple"));
  tq2 = make_shared<TermInSetQuery>(L"thing2", make_shared<BytesRef>(L"apple"));
  assertFalse(tq1->hashCode() == tq2->hashCode());
}

void TermInSetQueryTest::testSimpleEquals()
{
  // Two terms with the same hash code
  TestUtil::assertEquals((wstring(L"AaAaBB")).hashCode(),
                         (wstring(L"BBBBBB")).hashCode());
  shared_ptr<TermInSetQuery> left =
      make_shared<TermInSetQuery>(L"id", make_shared<BytesRef>(L"AaAaAa"),
                                  make_shared<BytesRef>(L"AaAaBB"));
  shared_ptr<TermInSetQuery> right =
      make_shared<TermInSetQuery>(L"id", make_shared<BytesRef>(L"AaAaAa"),
                                  make_shared<BytesRef>(L"BBBBBB"));
  assertFalse(left->equals(right));
}

void TermInSetQueryTest::testToString()
{
  shared_ptr<TermInSetQuery> termsQuery = make_shared<TermInSetQuery>(
      L"field1", make_shared<BytesRef>(L"a"), make_shared<BytesRef>(L"b"),
      make_shared<BytesRef>(L"c"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field1:(a b c)", termsQuery->toString());
}

void TermInSetQueryTest::testDedup()
{
  shared_ptr<Query> query1 =
      make_shared<TermInSetQuery>(L"foo", make_shared<BytesRef>(L"bar"));
  shared_ptr<Query> query2 = make_shared<TermInSetQuery>(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"bar"));
  QueryUtils::checkEqual(query1, query2);
}

void TermInSetQueryTest::testOrderDoesNotMatter()
{
  // order of terms if different
  shared_ptr<Query> query1 = make_shared<TermInSetQuery>(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"baz"));
  shared_ptr<Query> query2 = make_shared<TermInSetQuery>(
      L"foo", make_shared<BytesRef>(L"baz"), make_shared<BytesRef>(L"bar"));
  QueryUtils::checkEqual(query1, query2);
}

void TermInSetQueryTest::testRamBytesUsed()
{
  deque<std::shared_ptr<BytesRef>> terms = deque<std::shared_ptr<BytesRef>>();
  constexpr int numTerms = 1000 + random()->nextInt(1000);
  for (int i = 0; i < numTerms; ++i) {
    terms.push_back(make_shared<BytesRef>(
        RandomStrings::randomUnicodeOfLength(random(), 10)));
  }
  shared_ptr<TermInSetQuery> query = make_shared<TermInSetQuery>(L"f", terms);
  constexpr int64_t actualRamBytesUsed = RamUsageTester::sizeOf(query);
  constexpr int64_t expectedRamBytesUsed = query->ramBytesUsed();
  // error margin within 5%
  assertEquals(actualRamBytesUsed, expectedRamBytesUsed,
               actualRamBytesUsed / 20);
}

TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingDirectoryReaderWrapper(
        shared_ptr<DirectoryReader> in_,
        shared_ptr<AtomicInteger> counter) 
    : org::apache::lucene::index::FilterDirectoryReader(
          in_, new TermsCountingSubReaderWrapper(counter)),
      counter(counter)
{
}

TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingSubReaderWrapper::TermsCountingSubReaderWrapper(
        shared_ptr<AtomicInteger> counter)
    : counter(counter)
{
}

shared_ptr<LeafReader> TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingSubReaderWrapper::wrap(shared_ptr<LeafReader> reader)
{
  return make_shared<TermsCountingLeafReaderWrapper>(reader, counter);
}

TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingLeafReaderWrapper::TermsCountingLeafReaderWrapper(
        shared_ptr<LeafReader> in_, shared_ptr<AtomicInteger> counter)
    : org::apache::lucene::index::FilterLeafReader(in_), counter(counter)
{
}

shared_ptr<Terms> TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingLeafReaderWrapper::terms(const wstring &field) throw(
        IOException)
{
  shared_ptr<Terms> terms = FilterLeafReader::terms(field);
  if (terms == nullptr) {
    return nullptr;
  }
  return make_shared<FilterTermsAnonymousInnerClass>(shared_from_this(), terms);
}

TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingLeafReaderWrapper::FilterTermsAnonymousInnerClass::
        FilterTermsAnonymousInnerClass(
            shared_ptr<TermsCountingLeafReaderWrapper> outerInstance,
            shared_ptr<Terms> terms)
    : FilterTerms(terms)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TermsEnum> TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingLeafReaderWrapper::FilterTermsAnonymousInnerClass::
        iterator() 
{
  outerInstance->counter->incrementAndGet();
  return outerInstance->super->begin();
}

shared_ptr<IndexReader::CacheHelper>
TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingLeafReaderWrapper::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<IndexReader::CacheHelper>
TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::
    TermsCountingLeafReaderWrapper::getReaderCacheHelper()
{
  return nullptr;
}

shared_ptr<DirectoryReader>
TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<TermsCountingDirectoryReaderWrapper>(in_, counter);
}

shared_ptr<IndexReader::CacheHelper>
TermInSetQueryTest::TermsCountingDirectoryReaderWrapper::getReaderCacheHelper()
{
  return nullptr;
}

void TermInSetQueryTest::testPullOneTermsEnum() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"1", Store::NO));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> reader = w->getReader();
  delete w;
  shared_ptr<AtomicInteger> *const counter = make_shared<AtomicInteger>();
  shared_ptr<DirectoryReader> wrapped =
      make_shared<TermsCountingDirectoryReaderWrapper>(reader, counter);

  const deque<std::shared_ptr<BytesRef>> terms =
      deque<std::shared_ptr<BytesRef>>();
  // enough terms to avoid the rewrite
  constexpr int numTerms = TestUtil::nextInt(
      random(), TermInSetQuery::BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD + 1, 100);
  for (int i = 0; i < numTerms; ++i) {
    shared_ptr<BytesRef> *const term = make_shared<BytesRef>(
        RandomStrings::randomUnicodeOfCodepointLength(random(), 10));
    terms.push_back(term);
  }

  TestUtil::assertEquals(
      0, (make_shared<IndexSearcher>(wrapped))
             ->count(make_shared<TermInSetQuery>(L"bar", terms)));
  TestUtil::assertEquals(0, counter->get()); // missing field
  (make_shared<IndexSearcher>(wrapped))
      ->count(make_shared<TermInSetQuery>(L"foo", terms));
  TestUtil::assertEquals(1, counter->get());
  wrapped->close();
  delete dir;
}

void TermInSetQueryTest::testBinaryToString()
{
  shared_ptr<TermInSetQuery> query = make_shared<TermInSetQuery>(
      L"field", make_shared<BytesRef>(std::deque<char>{
                    static_cast<char>(0xff), static_cast<char>(0xfe)}));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:([ff fe])", query->toString());
}

void TermInSetQueryTest::testIsConsideredCostlyByQueryCache() 
{
  shared_ptr<TermInSetQuery> query = make_shared<TermInSetQuery>(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"baz"));
  shared_ptr<UsageTrackingQueryCachingPolicy> policy =
      make_shared<UsageTrackingQueryCachingPolicy>();
  assertFalse(policy->shouldCache(query));
  policy->onUse(query);
  policy->onUse(query);
  // cached after two uses
  assertTrue(policy->shouldCache(query));
}
} // namespace org::apache::lucene::search