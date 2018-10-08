using namespace std;

#include "TestPayloadTermQuery.h"

namespace org::apache::lucene::queries::payloads
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Spans = org::apache::lucene::search::spans::Spans;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestPayloadTermQuery::searcher;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestPayloadTermQuery::reader;
shared_ptr<org::apache::lucene::search::similarities::Similarity>
    TestPayloadTermQuery::similarity = make_shared<BoostingSimilarity>();
std::deque<char> const TestPayloadTermQuery::payloadField =
    std::deque<char>{1};
std::deque<char> const TestPayloadTermQuery::payloadMultiField1 =
    std::deque<char>{2};
std::deque<char> const TestPayloadTermQuery::payloadMultiField2 =
    std::deque<char>{4};
shared_ptr<org::apache::lucene::store::Directory>
    TestPayloadTermQuery::directory;

TestPayloadTermQuery::PayloadAnalyzer::PayloadAnalyzer()
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloadTermQuery::PayloadAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      result, make_shared<PayloadFilter>(result, fieldName));
}

TestPayloadTermQuery::PayloadFilter::PayloadFilter(
    shared_ptr<TokenStream> input, const wstring &fieldName)
    : org::apache::lucene::analysis::TokenFilter(input), fieldName(fieldName),
      payloadAtt(addAttribute(PayloadAttribute::typeid))
{
}

bool TestPayloadTermQuery::PayloadFilter::incrementToken() 
{
  bool hasNext = input->incrementToken();
  if (hasNext) {
    if (fieldName == L"field") {
      payloadAtt->setPayload(make_shared<BytesRef>(payloadField));
    } else if (fieldName == L"multiField") {
      if (numSeen % 2 == 0) {
        payloadAtt->setPayload(make_shared<BytesRef>(payloadMultiField1));
      } else {
        payloadAtt->setPayload(make_shared<BytesRef>(payloadMultiField2));
      }
      numSeen++;
    }
    return true;
  } else {
    return false;
  }
}

void TestPayloadTermQuery::PayloadFilter::reset() 
{
  TokenFilter::reset();
  this->numSeen = 0;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestPayloadTermQuery::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<PayloadAnalyzer>())
          ->setSimilarity(similarity)
          ->setMergePolicy(newLogMergePolicy()));
  // writer.infoStream = System.out;
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> noPayloadField =
        newTextField(PayloadHelper::NO_PAYLOAD_FIELD, English::intToEnglish(i),
                     Field::Store::YES);
    // noPayloadField.setBoost(0);
    doc->push_back(noPayloadField);
    doc->push_back(
        newTextField(L"field", English::intToEnglish(i), Field::Store::YES));
    doc->push_back(newTextField(L"multiField",
                                English::intToEnglish(i) + L"  " +
                                    English::intToEnglish(i),
                                Field::Store::YES));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = writer->getReader();
  delete writer;

  searcher = newSearcher(getOnlyLeafReader(reader));
  searcher->setSimilarity(similarity);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestPayloadTermQuery::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
}

void TestPayloadTermQuery::test() 
{
  shared_ptr<SpanQuery> query = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"seventy")),
      make_shared<MaxPayloadFunction>());
  shared_ptr<TopDocs> hits = searcher->search(query, 100);
  assertTrue(L"hits is null and it shouldn't be", hits != nullptr);
  assertTrue(L"hits Size: " + to_wstring(hits->totalHits) + L" is not: " +
                 to_wstring(100),
             hits->totalHits == 100);

  // they should all have the exact same score, because they all contain seventy
  // once, and we set all the other similarity factors to be 1

  assertTrue(to_wstring(hits->getMaxScore()) + L" does not equal: " +
                 to_wstring(1),
             hits->getMaxScore() == 1);
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    shared_ptr<ScoreDoc> doc = hits->scoreDocs[i];
    assertTrue(to_wstring(doc->score) + L" does not equal: " + to_wstring(1),
               doc->score == 1);
  }
  CheckHits::checkExplanations(query, PayloadHelper::FIELD, searcher, true);
  shared_ptr<Spans> spans =
      query->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  /*float score = hits.score(0);
  for (int i =1; i < hits.length(); i++)
  {
    assertTrue("scores are not equal and they should be", score ==
  hits.score(i));
  }*/
}

void TestPayloadTermQuery::testQuery()
{
  shared_ptr<SpanQuery> boostingFuncTermQuery = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(
          make_shared<Term>(PayloadHelper::MULTI_FIELD, L"seventy")),
      make_shared<MaxPayloadFunction>());
  QueryUtils::check(boostingFuncTermQuery);

  shared_ptr<SpanTermQuery> spanTermQuery = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::MULTI_FIELD, L"seventy"));

  assertTrue(boostingFuncTermQuery->equals(spanTermQuery) ==
             spanTermQuery->equals(boostingFuncTermQuery));

  shared_ptr<SpanQuery> boostingFuncTermQuery2 = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(
          make_shared<Term>(PayloadHelper::MULTI_FIELD, L"seventy")),
      make_shared<AveragePayloadFunction>());

  QueryUtils::checkUnequal(boostingFuncTermQuery, boostingFuncTermQuery2);
}

void TestPayloadTermQuery::testMultipleMatchesPerDoc() 
{
  shared_ptr<SpanQuery> query = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(
          make_shared<Term>(PayloadHelper::MULTI_FIELD, L"seventy")),
      make_shared<MaxPayloadFunction>());
  shared_ptr<TopDocs> hits = searcher->search(query, 100);
  assertTrue(L"hits is null and it shouldn't be", hits != nullptr);
  assertTrue(L"hits Size: " + to_wstring(hits->totalHits) + L" is not: " +
                 to_wstring(100),
             hits->totalHits == 100);

  // they should all have the exact same score, because they all contain seventy
  // once, and we set all the other similarity factors to be 1

  // System.out.println("Hash: " + seventyHash + " Twice Hash: " +
  // 2*seventyHash);
  assertTrue(to_wstring(hits->getMaxScore()) + L" does not equal: " +
                 to_wstring(4.0),
             hits->getMaxScore() == 4.0);
  // there should be exactly 10 items that score a 4, all the rest should score
  // a 2 The 10 items are: 70 + i*100 where i in [0-9]
  int numTens = 0;
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    shared_ptr<ScoreDoc> doc = hits->scoreDocs[i];
    if (doc->doc % 10 == 0) {
      numTens++;
      assertTrue(to_wstring(doc->score) + L" does not equal: " +
                     to_wstring(4.0),
                 doc->score == 4.0);
    } else {
      assertTrue(to_wstring(doc->score) + L" does not equal: " + to_wstring(2),
                 doc->score == 2);
    }
  }
  assertTrue(to_wstring(numTens) + L" does not equal: " + to_wstring(10),
             numTens == 10);
  CheckHits::checkExplanations(query, L"field", searcher, true);
  shared_ptr<Spans> spans =
      query->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::POSITIONS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  // should be two matches per document
  int count = 0;
  // 100 hits times 2 matches per hit, we should have 200 in count
  while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
    while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      count++;
    }
  }
  assertTrue(to_wstring(count) + L" does not equal: " + to_wstring(200),
             count == 200);
}

void TestPayloadTermQuery::testNoMatch() 
{
  shared_ptr<SpanQuery> query = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(
          make_shared<Term>(PayloadHelper::FIELD, L"junk")),
      make_shared<MaxPayloadFunction>());
  shared_ptr<TopDocs> hits = searcher->search(query, 100);
  assertTrue(L"hits is null and it shouldn't be", hits != nullptr);
  assertTrue(L"hits Size: " + to_wstring(hits->totalHits) + L" is not: " +
                 to_wstring(0),
             hits->totalHits == 0);
}

void TestPayloadTermQuery::testNoPayload() 
{
  shared_ptr<SpanQuery> q1 = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(
          make_shared<Term>(PayloadHelper::NO_PAYLOAD_FIELD, L"zero")),
      make_shared<MaxPayloadFunction>());
  shared_ptr<SpanQuery> q2 = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(
          make_shared<Term>(PayloadHelper::NO_PAYLOAD_FIELD, L"foo")),
      make_shared<MaxPayloadFunction>());
  shared_ptr<BooleanClause> c1 =
      make_shared<BooleanClause>(q1, BooleanClause::Occur::MUST);
  shared_ptr<BooleanClause> c2 =
      make_shared<BooleanClause>(q2, BooleanClause::Occur::MUST_NOT);
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(c1);
  query->add(c2);
  shared_ptr<TopDocs> hits = searcher->search(query->build(), 100);
  assertTrue(L"hits is null and it shouldn't be", hits != nullptr);
  assertTrue(L"hits Size: " + to_wstring(hits->totalHits) + L" is not: " +
                 to_wstring(1),
             hits->totalHits == 1);
  std::deque<int> results(1);
  results[0] = 0; // hits.scoreDocs[0].doc;
  CheckHits::checkHitCollector(random(), query->build(),
                               PayloadHelper::NO_PAYLOAD_FIELD, searcher,
                               results);
}

float TestPayloadTermQuery::BoostingSimilarity::scorePayload(
    int docId, int start, int end, shared_ptr<BytesRef> payload)
{
  // we know it is size 4 here, so ignore the offset/length
  return payload->bytes[payload->offset];
}

float TestPayloadTermQuery::BoostingSimilarity::lengthNorm(int length)
{
  return 1;
}

float TestPayloadTermQuery::BoostingSimilarity::sloppyFreq(int distance)
{
  return 1;
}

float TestPayloadTermQuery::BoostingSimilarity::idf(int64_t docFreq,
                                                    int64_t docCount)
{
  return 1;
}

float TestPayloadTermQuery::BoostingSimilarity::tf(float freq)
{
  return freq == 0 ? 0 : 1;
}

float TestPayloadTermQuery::FullSimilarity::scorePayload(
    int docId, const wstring &fieldName, std::deque<char> &payload, int offset,
    int length)
{
  // we know it is size 4 here, so ignore the offset/length
  return payload[offset];
}
} // namespace org::apache::lucene::queries::payloads