using namespace std;

#include "CollationTestBase.h"

namespace org::apache::lucene::analysis
{
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void CollationTestBase::testFarsiRangeFilterCollating(
    shared_ptr<Analyzer> analyzer, shared_ptr<BytesRef> firstBeg,
    shared_ptr<BytesRef> firstEnd, shared_ptr<BytesRef> secondBeg,
    shared_ptr<BytesRef> secondEnd) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"content", L"\u0633\u0627\u0628",
                                        Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"body", L"body", Field::Store::YES));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"body"));

  // Unicode order would include U+0633 in [ U+062F - U+0698 ], but Farsi
  // orders the U+0698 character before the U+0633 character, so the single
  // index Term below should NOT be returned by a TermRangeFilter with a Farsi
  // Collator (or an Arabic one for the case when Farsi searcher not
  // supported).
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(query, Occur::MUST);
  bq->add(
      make_shared<TermRangeQuery>(L"content", firstBeg, firstEnd, true, true),
      Occur::FILTER);
  std::deque<std::shared_ptr<ScoreDoc>> result =
      searcher->search(bq->build(), 1)->scoreDocs;
  assertEquals(L"The index Term should not be included.", 0, result.size());

  bq = make_shared<BooleanQuery::Builder>();
  bq->add(query, Occur::MUST);
  bq->add(
      make_shared<TermRangeQuery>(L"content", secondBeg, secondEnd, true, true),
      Occur::FILTER);
  result = searcher->search(bq->build(), 1)->scoreDocs;
  assertEquals(L"The index Term should be included.", 1, result.size());

  delete reader;
  delete dir;
}

void CollationTestBase::testFarsiRangeQueryCollating(
    shared_ptr<Analyzer> analyzer, shared_ptr<BytesRef> firstBeg,
    shared_ptr<BytesRef> firstEnd, shared_ptr<BytesRef> secondBeg,
    shared_ptr<BytesRef> secondEnd) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();

  // Unicode order would include U+0633 in [ U+062F - U+0698 ], but Farsi
  // orders the U+0698 character before the U+0633 character, so the single
  // index Term below should NOT be returned by a TermRangeQuery with a Farsi
  // Collator (or an Arabic one for the case when Farsi is not supported).
  doc->push_back(make_shared<TextField>(L"content", L"\u0633\u0627\u0628",
                                        Field::Store::YES));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);

  shared_ptr<Query> query =
      make_shared<TermRangeQuery>(L"content", firstBeg, firstEnd, true, true);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"The index Term should not be included.", 0, hits.size());

  query =
      make_shared<TermRangeQuery>(L"content", secondBeg, secondEnd, true, true);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"The index Term should be included.", 1, hits.size());
  delete reader;
  delete dir;
}

void CollationTestBase::testFarsiTermRangeQuery(
    shared_ptr<Analyzer> analyzer, shared_ptr<BytesRef> firstBeg,
    shared_ptr<BytesRef> firstEnd, shared_ptr<BytesRef> secondBeg,
    shared_ptr<BytesRef> secondEnd) 
{

  shared_ptr<Directory> farsiIndex = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      farsiIndex, make_shared<IndexWriterConfig>(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"content", L"\u0633\u0627\u0628",
                                        Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"body", L"body", Field::Store::YES));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(farsiIndex);
  shared_ptr<IndexSearcher> search = newSearcher(reader);

  // Unicode order would include U+0633 in [ U+062F - U+0698 ], but Farsi
  // orders the U+0698 character before the U+0633 character, so the single
  // index Term below should NOT be returned by a TermRangeQuery
  // with a Farsi Collator (or an Arabic one for the case when Farsi is
  // not supported).
  shared_ptr<Query> csrq =
      make_shared<TermRangeQuery>(L"content", firstBeg, firstEnd, true, true);
  std::deque<std::shared_ptr<ScoreDoc>> result =
      search->search(csrq, 1000)->scoreDocs;
  assertEquals(L"The index Term should not be included.", 0, result.size());

  csrq =
      make_shared<TermRangeQuery>(L"content", secondBeg, secondEnd, true, true);
  result = search->search(csrq, 1000)->scoreDocs;
  assertEquals(L"The index Term should be included.", 1, result.size());
  delete reader;
  delete farsiIndex;
}

void CollationTestBase::assertMatches(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> query,
    shared_ptr<Sort> sort, const wstring &expectedResult) 
{
  std::deque<std::shared_ptr<ScoreDoc>> result =
      searcher->search(query, 1000, sort)->scoreDocs;
  shared_ptr<StringBuilder> buff = make_shared<StringBuilder>(10);
  int n = result.size();
  for (int i = 0; i < n; ++i) {
    shared_ptr<Document> doc = searcher->doc(result[i]->doc);
    std::deque<std::shared_ptr<IndexableField>> v = doc->getFields(L"tracer");
    for (int j = 0; j < v.size(); ++j) {
      buff->append(v[j]->stringValue());
    }
  }
  TestUtil::assertEquals(expectedResult, buff->toString());
}

void CollationTestBase::assertThreadSafe(shared_ptr<Analyzer> analyzer) throw(
    runtime_error)
{
  int numTestPoints = 100;
  int numThreads = TestUtil::nextInt(random(), 3, 5);
  const unordered_map<wstring, std::shared_ptr<BytesRef>> map_obj =
      unordered_map<wstring, std::shared_ptr<BytesRef>>();

  // create a map_obj<std::wstring,SortKey> up front.
  // then with multiple threads, generate sort keys for all the keys in the map_obj
  // and ensure they are the same as the ones we produced in serial fashion.

  for (int i = 0; i < numTestPoints; i++) {
    wstring term = TestUtil::randomSimpleString(random());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (TokenStream ts =
    // analyzer.tokenStream("fake", term))
    {
      TokenStream ts = analyzer->tokenStream(L"fake", term);
      shared_ptr<TermToBytesRefAttribute> termAtt =
          ts->addAttribute(TermToBytesRefAttribute::typeid);
      ts->reset();
      assertTrue(ts->incrementToken());
      // ensure we make a copy of the actual bytes too
      map_obj.emplace(term, BytesRef::deepCopyOf(termAtt->getBytesRef()));
      assertFalse(ts->incrementToken());
      ts->end();
    }
  }

  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int i = 0; i < numThreads; i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(shared_from_this(),
                                                        analyzer, map_obj, ts);
  }
  for (int i = 0; i < numThreads; i++) {
    threads[i]->start();
  }
  for (int i = 0; i < numThreads; i++) {
    threads[i]->join();
  }
}

CollationTestBase::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<CollationTestBase> outerInstance,
    shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer,
    unordered_map<wstring, std::shared_ptr<BytesRef>> &map_obj,
    shared_ptr<org::apache::lucene::analysis::TokenStream> ts)
{
  this->outerInstance = outerInstance;
  this->analyzer = analyzer;
  this->map_obj = map_obj;
  this->ts = ts;
}

void CollationTestBase::ThreadAnonymousInnerClass::run()
{
  try {
    for (auto mapping : map_obj) {
      wstring term = mapping.first;
      shared_ptr<BytesRef> expected = mapping.second;
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (TokenStream ts =
      // analyzer.tokenStream("fake", term))
      {
        TokenStream ts = analyzer->tokenStream(L"fake", term);
        shared_ptr<TermToBytesRefAttribute> termAtt =
            ts->addAttribute(TermToBytesRefAttribute::typeid);
        ts->reset();
        assertTrue(ts->incrementToken());
        TestUtil::assertEquals(expected, termAtt->getBytesRef());
        assertFalse(ts->incrementToken());
        ts->end();
      }
    }
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::analysis