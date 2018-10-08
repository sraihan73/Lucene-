using namespace std;

#include "TestSpansEnum.h"

namespace org::apache::lucene::search::spans
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using namespace org::apache::lucene::search::spans;
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;
shared_ptr<org::apache::lucene::search::IndexSearcher> TestSpansEnum::searcher;
shared_ptr<org::apache::lucene::index::IndexReader> TestSpansEnum::reader;
shared_ptr<org::apache::lucene::store::Directory> TestSpansEnum::directory;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestSpansEnum::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 100, 1000))
          ->setMergePolicy(newLogMergePolicy()));
  // writer.infoStream = System.out;
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field", English::intToEnglish(i), Field::Store::YES));
    writer->addDocument(doc);
  }
  for (int i = 100; i < 110; i++) {
    shared_ptr<Document> doc =
        make_shared<Document>(); // doc id 10-19 have 100-109
    doc->push_back(
        newTextField(L"field", English::intToEnglish(i), Field::Store::YES));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestSpansEnum::afterClass() 
{
  delete reader;
  delete directory;
  searcher.reset();
  reader.reset();
  directory.reset();
}

void TestSpansEnum::checkHits(shared_ptr<Query> query,
                              std::deque<int> &results) 
{
  CheckHits::checkHits(random(), query, L"field", searcher, results);
}

void TestSpansEnum::testSpansEnumOr1() 
{
  checkHits(spanOrQuery(L"field", L"one", L"two"),
            std::deque<int>{1, 2, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19});
}

void TestSpansEnum::testSpansEnumOr2() 
{
  checkHits(spanOrQuery(L"field", L"one", L"eleven"),
            std::deque<int>{1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19});
}

void TestSpansEnum::testSpansEnumOr3() 
{
  checkHits(spanOrQuery(L"field", L"twelve", L"eleven"), std::deque<int>());
}

shared_ptr<SpanQuery> TestSpansEnum::spanTQ(const wstring &s)
{
  return SpanTestUtil::spanTermQuery(L"field", s);
}

void TestSpansEnum::testSpansEnumOrNot1() 
{
  checkHits(
      SpanTestUtil::spanNotQuery(spanOrQuery(L"field", L"one", L"two"),
                                 SpanTestUtil::spanTermQuery(L"field", L"one")),
      std::deque<int>{2, 12});
}

void TestSpansEnum::testSpansEnumNotBeforeAfter1() 
{
  checkHits(SpanTestUtil::spanNotQuery(
                SpanTestUtil::spanTermQuery(L"field", L"hundred"),
                SpanTestUtil::spanTermQuery(L"field", L"one")),
            std::deque<int>{10, 11, 12, 13, 14, 15, 16, 17, 18,
                             19}); // include all "one hundred ..."
}

void TestSpansEnum::testSpansEnumNotBeforeAfter2() 
{
  checkHits(SpanTestUtil::spanNotQuery(
                SpanTestUtil::spanTermQuery(L"field", L"hundred"),
                SpanTestUtil::spanTermQuery(L"field", L"one"), 1, 0),
            std::deque<int>()); // exclude all "one hundred ..."
}

void TestSpansEnum::testSpansEnumNotBeforeAfter3() 
{
  checkHits(SpanTestUtil::spanNotQuery(
                SpanTestUtil::spanTermQuery(L"field", L"hundred"),
                SpanTestUtil::spanTermQuery(L"field", L"one"), 0, 1),
            std::deque<int>{10, 12, 13, 14, 15, 16, 17, 18,
                             19}); // exclude "one hundred one"
}
} // namespace org::apache::lucene::search::spans