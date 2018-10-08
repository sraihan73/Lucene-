using namespace std;

#include "TestPayloadCheckQuery.h"

namespace org::apache::lucene::queries::payloads
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using SimplePayloadFilter = org::apache::lucene::analysis::SimplePayloadFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanPositionRangeQuery =
    org::apache::lucene::search::spans::SpanPositionRangeQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestPayloadCheckQuery::searcher;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestPayloadCheckQuery::reader;
shared_ptr<org::apache::lucene::store::Directory>
    TestPayloadCheckQuery::directory;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestPayloadCheckQuery::beforeClass() 
{
  shared_ptr<Analyzer> simplePayloadAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass>();

  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(simplePayloadAnalyzer)
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 100, 1000))
          ->setMergePolicy(newLogMergePolicy()));
  // writer.infoStream = System.out;
  for (int i = 0; i < 2000; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field", English::intToEnglish(i), Field::Store::YES));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

TestPayloadCheckQuery::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass()
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloadCheckQuery::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SimplePayloadFilter>(tokenizer));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestPayloadCheckQuery::afterClass() 
{
  delete reader;
  delete directory;
  searcher.reset();
  reader.reset();
  directory.reset();
}

void TestPayloadCheckQuery::checkHits(
    shared_ptr<Query> query, std::deque<int> &results) 
{
  CheckHits::checkHits(random(), query, L"field", searcher, results);
}

void TestPayloadCheckQuery::testSpanPayloadCheck() 
{
  shared_ptr<SpanQuery> term1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"five"));
  shared_ptr<BytesRef> pay = make_shared<BytesRef>(L"pos: " + to_wstring(5));
  shared_ptr<SpanQuery> query = make_shared<SpanPayloadCheckQuery>(
      term1, Collections::singletonList(pay));
  checkHits(query, std::deque<int>{
                       1125, 1135, 1145, 1155, 1165, 1175, 1185, 1195, 1225,
                       1235, 1245, 1255, 1265, 1275, 1285, 1295, 1325, 1335,
                       1345, 1355, 1365, 1375, 1385, 1395, 1425, 1435, 1445,
                       1455, 1465, 1475, 1485, 1495, 1525, 1535, 1545, 1555,
                       1565, 1575, 1585, 1595, 1625, 1635, 1645, 1655, 1665,
                       1675, 1685, 1695, 1725, 1735, 1745, 1755, 1765, 1775,
                       1785, 1795, 1825, 1835, 1845, 1855, 1865, 1875, 1885,
                       1895, 1925, 1935, 1945, 1955, 1965, 1975, 1985, 1995});
  assertTrue(searcher->explain(query, 1125)->getValue() > 0.0f);

  shared_ptr<SpanTermQuery> term2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"hundred"));
  shared_ptr<SpanNearQuery> snq;
  std::deque<std::shared_ptr<SpanQuery>> clauses;
  deque<std::shared_ptr<BytesRef>> deque;
  shared_ptr<BytesRef> pay2;
  clauses = std::deque<std::shared_ptr<SpanQuery>>(2);
  clauses[0] = term1;
  clauses[1] = term2;
  snq = make_shared<SpanNearQuery>(clauses, 0, true);
  pay = make_shared<BytesRef>(L"pos: " + to_wstring(0));
  pay2 = make_shared<BytesRef>(L"pos: " + to_wstring(1));
  deque = deque<>();
  deque.push_back(pay);
  deque.push_back(pay2);
  query = make_shared<SpanPayloadCheckQuery>(snq, deque);
  checkHits(query,
            std::deque<int>{
                500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512,
                513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525,
                526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538,
                539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551,
                552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564,
                565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577,
                578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590,
                591, 592, 593, 594, 595, 596, 597, 598, 599});
  clauses = std::deque<std::shared_ptr<SpanQuery>>(3);
  clauses[0] = term1;
  clauses[1] = term2;
  clauses[2] = make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"five"));
  snq = make_shared<SpanNearQuery>(clauses, 0, true);
  pay = make_shared<BytesRef>(L"pos: " + to_wstring(0));
  pay2 = make_shared<BytesRef>(L"pos: " + to_wstring(1));
  shared_ptr<BytesRef> pay3 = make_shared<BytesRef>(L"pos: " + to_wstring(2));
  deque = deque<>();
  deque.push_back(pay);
  deque.push_back(pay2);
  deque.push_back(pay3);
  query = make_shared<SpanPayloadCheckQuery>(snq, deque);
  checkHits(query, std::deque<int>{505});
}

void TestPayloadCheckQuery::testUnorderedPayloadChecks() 
{

  shared_ptr<SpanTermQuery> term5 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"five"));
  shared_ptr<SpanTermQuery> term100 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"hundred"));
  shared_ptr<SpanTermQuery> term4 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"four"));
  shared_ptr<SpanNearQuery> nearQuery = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term5, term100, term4}, 0, false);

  deque<std::shared_ptr<BytesRef>> payloads =
      deque<std::shared_ptr<BytesRef>>();
  payloads.push_back(make_shared<BytesRef>(L"pos: " + to_wstring(2)));
  payloads.push_back(make_shared<BytesRef>(L"pos: " + to_wstring(1)));
  payloads.push_back(make_shared<BytesRef>(L"pos: " + to_wstring(0)));

  shared_ptr<SpanPayloadCheckQuery> payloadQuery =
      make_shared<SpanPayloadCheckQuery>(nearQuery, payloads);
  checkHits(payloadQuery, std::deque<int>{405});

  payloads.clear();
  payloads.push_back(make_shared<BytesRef>(L"pos: " + to_wstring(0)));
  payloads.push_back(make_shared<BytesRef>(L"pos: " + to_wstring(1)));
  payloads.push_back(make_shared<BytesRef>(L"pos: " + to_wstring(2)));

  payloadQuery = make_shared<SpanPayloadCheckQuery>(nearQuery, payloads);
  checkHits(payloadQuery, std::deque<int>{504});
}

void TestPayloadCheckQuery::testComplexSpanChecks() 
{
  shared_ptr<SpanTermQuery> one =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"one"));
  shared_ptr<SpanTermQuery> thous =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"thousand"));
  // should be one position in between
  shared_ptr<SpanTermQuery> hundred =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"hundred"));
  shared_ptr<SpanTermQuery> three =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"three"));

  shared_ptr<SpanNearQuery> oneThous = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{one, thous}, 0, true);
  shared_ptr<SpanNearQuery> hundredThree = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{hundred, three}, 0, true);
  shared_ptr<SpanNearQuery> oneThousHunThree = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{oneThous, hundredThree}, 1, true);
  shared_ptr<SpanQuery> query;
  // this one's too small
  query = make_shared<SpanPositionRangeQuery>(oneThousHunThree, 1, 2);
  checkHits(query, std::deque<int>());
  // this one's just right
  query = make_shared<SpanPositionRangeQuery>(oneThousHunThree, 0, 6);
  checkHits(query, std::deque<int>{1103, 1203, 1303, 1403, 1503, 1603, 1703,
                                    1803, 1903});

  deque<std::shared_ptr<BytesRef>> payloads =
      deque<std::shared_ptr<BytesRef>>();
  shared_ptr<BytesRef> pay = make_shared<BytesRef>(
      (L"pos: " + to_wstring(0))->getBytes(StandardCharsets::UTF_8));
  shared_ptr<BytesRef> pay2 = make_shared<BytesRef>(
      (L"pos: " + to_wstring(1))->getBytes(StandardCharsets::UTF_8));
  shared_ptr<BytesRef> pay3 = make_shared<BytesRef>(
      (L"pos: " + to_wstring(3))->getBytes(StandardCharsets::UTF_8));
  shared_ptr<BytesRef> pay4 = make_shared<BytesRef>(
      (L"pos: " + to_wstring(4))->getBytes(StandardCharsets::UTF_8));
  payloads.push_back(pay);
  payloads.push_back(pay2);
  payloads.push_back(pay3);
  payloads.push_back(pay4);
  query = make_shared<SpanPayloadCheckQuery>(oneThousHunThree, payloads);
  checkHits(query, std::deque<int>{1103, 1203, 1303, 1403, 1503, 1603, 1703,
                                    1803, 1903});
}

void TestPayloadCheckQuery::testEquality()
{
  shared_ptr<SpanQuery> sq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"one"));
  shared_ptr<SpanQuery> sq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"two"));
  shared_ptr<BytesRef> payload1 = make_shared<BytesRef>(L"pay1");
  shared_ptr<BytesRef> payload2 = make_shared<BytesRef>(L"pay2");
  shared_ptr<SpanQuery> query1 = make_shared<SpanPayloadCheckQuery>(
      sq1, Collections::singletonList(payload1));
  shared_ptr<SpanQuery> query2 = make_shared<SpanPayloadCheckQuery>(
      sq2, Collections::singletonList(payload1));
  shared_ptr<SpanQuery> query3 = make_shared<SpanPayloadCheckQuery>(
      sq1, Collections::singletonList(payload2));
  shared_ptr<SpanQuery> query4 = make_shared<SpanPayloadCheckQuery>(
      sq2, Collections::singletonList(payload2));
  shared_ptr<SpanQuery> query5 = make_shared<SpanPayloadCheckQuery>(
      sq1, Collections::singletonList(payload1));

  TestUtil::assertEquals(query1, query5);
  assertFalse(query1->equals(query2));
  assertFalse(query1->equals(query3));
  assertFalse(query1->equals(query4));
  assertFalse(query2->equals(query3));
  assertFalse(query2->equals(query4));
  assertFalse(query3->equals(query4));
}

void TestPayloadCheckQuery::testRewrite() 
{
  shared_ptr<SpanMultiTermQueryWrapper> fiv =
      make_shared<SpanMultiTermQueryWrapper>(
          make_shared<WildcardQuery>(make_shared<Term>(L"field", L"fiv*")));
  shared_ptr<SpanMultiTermQueryWrapper> hund =
      make_shared<SpanMultiTermQueryWrapper>(
          make_shared<WildcardQuery>(make_shared<Term>(L"field", L"hund*")));
  shared_ptr<SpanMultiTermQueryWrapper> twent =
      make_shared<SpanMultiTermQueryWrapper>(
          make_shared<WildcardQuery>(make_shared<Term>(L"field", L"twent*")));
  shared_ptr<SpanMultiTermQueryWrapper> nin =
      make_shared<SpanMultiTermQueryWrapper>(
          make_shared<WildcardQuery>(make_shared<Term>(L"field", L"nin*")));

  shared_ptr<SpanNearQuery> sq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{fiv, hund, twent, nin}, 0, true);

  deque<std::shared_ptr<BytesRef>> payloads =
      deque<std::shared_ptr<BytesRef>>();
  payloads.push_back(make_shared<BytesRef>(L"pos: 0"));
  payloads.push_back(make_shared<BytesRef>(L"pos: 1"));
  payloads.push_back(make_shared<BytesRef>(L"pos: 2"));
  payloads.push_back(make_shared<BytesRef>(L"pos: 3"));

  shared_ptr<SpanPayloadCheckQuery> query =
      make_shared<SpanPayloadCheckQuery>(sq, payloads);

  // if query wasn't rewritten properly, the query would have failed with
  // "Rewrite first!"
  checkHits(query, std::deque<int>{529});
}
} // namespace org::apache::lucene::queries::payloads