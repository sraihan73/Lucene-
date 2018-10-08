using namespace std;

#include "TestPayloadSpans.h"

namespace org::apache::lucene::queries::payloads
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanFirstQuery = org::apache::lucene::search::spans::SpanFirstQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Spans = org::apache::lucene::search::spans::Spans;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPayloadSpans::setUp() 
{
  LuceneTestCase::setUp();
  shared_ptr<PayloadHelper> helper = make_shared<PayloadHelper>();
  searcher = helper->setUp(random(), similarity, 1000);
  indexReader = searcher->getIndexReader();
}

void TestPayloadSpans::testSpanTermQuery() 
{
  shared_ptr<SpanTermQuery> stq;
  shared_ptr<Spans> spans;
  stq = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"seventy"));

  spans = stq->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 100, 1, 1, 1);

  stq = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::NO_PAYLOAD_FIELD, L"seventy"));
  spans = stq->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 100, 0, 0, 0);
}

void TestPayloadSpans::testSpanFirst() 
{

  shared_ptr<SpanQuery> match;
  shared_ptr<SpanFirstQuery> sfq;
  match = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"one"));
  sfq = make_shared<SpanFirstQuery>(match, 2);
  shared_ptr<Spans> spans =
      sfq->createWeight(searcher, false, 1.0f)
          ->getSpans(searcher->getIndexReader()->leaves()[0],
                     SpanWeight::Postings::PAYLOADS);
  checkSpans(spans, 109, 1, 1, 1);
  // Test more complicated subclause
  std::deque<std::shared_ptr<SpanQuery>> clauses(2);
  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"one"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"hundred"));
  match = make_shared<SpanNearQuery>(clauses, 0, true);
  sfq = make_shared<SpanFirstQuery>(match, 2);
  checkSpans(sfq->createWeight(searcher, false, 1.0f)
                 ->getSpans(searcher->getIndexReader()->leaves()[0],
                            SpanWeight::Postings::PAYLOADS),
             100, 2, 1, 1);

  match = make_shared<SpanNearQuery>(clauses, 0, false);
  sfq = make_shared<SpanFirstQuery>(match, 2);
  checkSpans(sfq->createWeight(searcher, false, 1.0f)
                 ->getSpans(searcher->getIndexReader()->leaves()[0],
                            SpanWeight::Postings::PAYLOADS),
             100, 2, 1, 1);
}

void TestPayloadSpans::testSpanNot() 
{
  std::deque<std::shared_ptr<SpanQuery>> clauses(2);
  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"one"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"three"));
  shared_ptr<SpanQuery> spq = make_shared<SpanNearQuery>(clauses, 5, true);
  shared_ptr<SpanNotQuery> snq = make_shared<SpanNotQuery>(
      spq, make_shared<SpanTermQuery>(
               make_shared<Term>(PayloadHelper::FIELD, L"two")));

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<PayloadAnalyzer>())
          ->setSimilarity(similarity));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(PayloadHelper::FIELD,
                              L"one two three one four three",
                              Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> reader = getOnlyLeafReader(writer->getReader());
  delete writer;

  checkSpans(
      snq->createWeight(newSearcher(reader, false), false, 1.0f)
          ->getSpans(reader->leaves()[0], SpanWeight::Postings::PAYLOADS),
      1, std::deque<int>{2});
  delete reader;
  delete directory;
}

void TestPayloadSpans::testNestedSpans() 
{
  shared_ptr<SpanTermQuery> stq;
  shared_ptr<Spans> spans;
  shared_ptr<IndexSearcher> searcher = getSearcher();

  stq = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"mark"));
  spans = stq->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);
  assertNull(spans);

  std::deque<std::shared_ptr<SpanQuery>> clauses(3);
  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"rr"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"yy"));
  clauses[2] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"xx"));
  shared_ptr<SpanNearQuery> spanNearQuery =
      make_shared<SpanNearQuery>(clauses, 12, false);

  spans = spanNearQuery->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 2, std::deque<int>{3, 3});

  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"xx"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"rr"));
  clauses[2] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"yy"));

  spanNearQuery = make_shared<SpanNearQuery>(clauses, 6, true);

  spans = spanNearQuery->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);

  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 1, std::deque<int>{3});

  clauses = std::deque<std::shared_ptr<SpanQuery>>(2);

  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"xx"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"rr"));

  spanNearQuery = make_shared<SpanNearQuery>(clauses, 6, true);

  // xx within 6 of rr

  std::deque<std::shared_ptr<SpanQuery>> clauses2(2);

  clauses2[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"yy"));
  clauses2[1] = spanNearQuery;

  shared_ptr<SpanNearQuery> nestedSpanNearQuery =
      make_shared<SpanNearQuery>(clauses2, 6, false);

  // yy within 6 of xx within 6 of rr
  spans = nestedSpanNearQuery->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 2, std::deque<int>{3, 3});
  delete closeIndexReader;
  delete directory;
}

void TestPayloadSpans::testFirstClauseWithoutPayload() 
{
  shared_ptr<Spans> spans;
  shared_ptr<IndexSearcher> searcher = getSearcher();

  std::deque<std::shared_ptr<SpanQuery>> clauses(3);
  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"nopayload"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"qq"));
  clauses[2] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"ss"));

  shared_ptr<SpanNearQuery> spanNearQuery =
      make_shared<SpanNearQuery>(clauses, 6, true);

  std::deque<std::shared_ptr<SpanQuery>> clauses2(2);

  clauses2[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"pp"));
  clauses2[1] = spanNearQuery;

  shared_ptr<SpanNearQuery> snq =
      make_shared<SpanNearQuery>(clauses2, 6, false);

  std::deque<std::shared_ptr<SpanQuery>> clauses3(2);

  clauses3[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"np"));
  clauses3[1] = snq;

  shared_ptr<SpanNearQuery> nestedSpanNearQuery =
      make_shared<SpanNearQuery>(clauses3, 6, false);
  spans = nestedSpanNearQuery->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);

  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 1, std::deque<int>{3});
  delete closeIndexReader;
  delete directory;
}

void TestPayloadSpans::testHeavilyNestedSpanQuery() 
{
  shared_ptr<Spans> spans;
  shared_ptr<IndexSearcher> searcher = getSearcher();

  std::deque<std::shared_ptr<SpanQuery>> clauses(3);
  clauses[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"one"));
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"two"));
  clauses[2] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"three"));

  shared_ptr<SpanNearQuery> spanNearQuery =
      make_shared<SpanNearQuery>(clauses, 5, true);

  clauses = std::deque<std::shared_ptr<SpanQuery>>(3);
  clauses[0] = spanNearQuery;
  clauses[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"five"));
  clauses[2] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"six"));

  shared_ptr<SpanNearQuery> spanNearQuery2 =
      make_shared<SpanNearQuery>(clauses, 6, true);

  std::deque<std::shared_ptr<SpanQuery>> clauses2(2);
  clauses2[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"eleven"));
  clauses2[1] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"ten"));
  shared_ptr<SpanNearQuery> spanNearQuery3 =
      make_shared<SpanNearQuery>(clauses2, 2, false);

  std::deque<std::shared_ptr<SpanQuery>> clauses3(3);
  clauses3[0] = make_shared<SpanTermQuery>(
      make_shared<Term>(PayloadHelper::FIELD, L"nine"));
  clauses3[1] = spanNearQuery2;
  clauses3[2] = spanNearQuery3;

  shared_ptr<SpanNearQuery> nestedSpanNearQuery =
      make_shared<SpanNearQuery>(clauses3, 6, false);

  spans = nestedSpanNearQuery->createWeight(searcher, false, 1.0f)
              ->getSpans(searcher->getIndexReader()->leaves()[0],
                         SpanWeight::Postings::PAYLOADS);
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  checkSpans(spans, 2, std::deque<int>{8, 8});
  delete closeIndexReader;
  delete directory;
}

void TestPayloadSpans::testShrinkToAfterShortestMatch() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<TestPayloadAnalyzer>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", make_shared<StringReader>(L"a b c d e f g h i j a k")));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> is = newSearcher(getOnlyLeafReader(reader), false);
  delete writer;

  shared_ptr<SpanTermQuery> stq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"a"));
  shared_ptr<SpanTermQuery> stq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"k"));
  std::deque<std::shared_ptr<SpanQuery>> sqs = {stq1, stq2};
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(sqs, 1, true);
  shared_ptr<VerifyingCollector> collector = make_shared<VerifyingCollector>();
  shared_ptr<Spans> spans = snq->createWeight(is, false, 1.0f)
                                ->getSpans(is->getIndexReader()->leaves()[0],
                                           SpanWeight::Postings::PAYLOADS);

  shared_ptr<TopDocs> topDocs = is->search(snq, 1);
  shared_ptr<Set<wstring>> payloadSet = unordered_set<wstring>();
  for (int i = 0; i < topDocs->scoreDocs.size(); i++) {
    while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
      while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
        collector->reset();
        spans->collect(collector);
        for (auto payload : collector->payloads) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          payloadSet->add(Term::toString(payload));
        }
      }
    }
  }
  assertEquals(2, payloadSet->size());
  assertTrue(payloadSet->contains(L"a:Noise:10"));
  assertTrue(payloadSet->contains(L"k:Noise:11"));
  delete reader;
  delete directory;
}

void TestPayloadSpans::testShrinkToAfterShortestMatch2() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<TestPayloadAnalyzer>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", make_shared<StringReader>(L"a b a d k f a h i k a k")));
  writer->addDocument(doc);
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> is = newSearcher(getOnlyLeafReader(reader), false);
  delete writer;

  shared_ptr<SpanTermQuery> stq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"a"));
  shared_ptr<SpanTermQuery> stq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"k"));
  std::deque<std::shared_ptr<SpanQuery>> sqs = {stq1, stq2};
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(sqs, 0, true);
  shared_ptr<VerifyingCollector> collector = make_shared<VerifyingCollector>();
  shared_ptr<Spans> spans = snq->createWeight(is, false, 1.0f)
                                ->getSpans(is->getIndexReader()->leaves()[0],
                                           SpanWeight::Postings::PAYLOADS);

  shared_ptr<TopDocs> topDocs = is->search(snq, 1);
  shared_ptr<Set<wstring>> payloadSet = unordered_set<wstring>();
  for (int i = 0; i < topDocs->scoreDocs.size(); i++) {
    while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
      while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
        collector->reset();
        spans->collect(collector);
        for (auto payload : collector->payloads) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          payloadSet->add(Term::toString(payload));
        }
      }
    }
  }
  assertEquals(2, payloadSet->size());
  assertTrue(payloadSet->contains(L"a:Noise:10"));
  assertTrue(payloadSet->contains(L"k:Noise:11"));
  delete reader;
  delete directory;
}

void TestPayloadSpans::testShrinkToAfterShortestMatch3() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<TestPayloadAnalyzer>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content",
      make_shared<StringReader>(L"j k a l f k k p a t a k l k t a")));
  writer->addDocument(doc);
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> is = newSearcher(getOnlyLeafReader(reader), false);
  delete writer;

  shared_ptr<SpanTermQuery> stq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"a"));
  shared_ptr<SpanTermQuery> stq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"k"));
  std::deque<std::shared_ptr<SpanQuery>> sqs = {stq1, stq2};
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(sqs, 0, true);
  shared_ptr<Spans> spans = snq->createWeight(is, false, 1.0f)
                                ->getSpans(is->getIndexReader()->leaves()[0],
                                           SpanWeight::Postings::PAYLOADS);

  shared_ptr<TopDocs> topDocs = is->search(snq, 1);
  shared_ptr<Set<wstring>> payloadSet = unordered_set<wstring>();
  shared_ptr<VerifyingCollector> collector = make_shared<VerifyingCollector>();
  for (int i = 0; i < topDocs->scoreDocs.size(); i++) {
    while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
      while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
        collector->reset();
        spans->collect(collector);
        for (auto payload : collector->payloads) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          payloadSet->add(Term::toString(payload));
        }
      }
    }
  }
  assertEquals(2, payloadSet->size());
  if (VERBOSE) {
    for (auto payload : payloadSet) {
      wcout << L"match:" << payload << endl;
    }
  }
  assertTrue(payloadSet->contains(L"a:Noise:10"));
  assertTrue(payloadSet->contains(L"k:Noise:11"));
  delete reader;
  delete directory;
}

void TestPayloadSpans::VerifyingCollector::collectLeaf(
    shared_ptr<PostingsEnum> postings, int position,
    shared_ptr<Term> term) 
{
  if (postings->getPayload() != nullptr) {
    payloads.push_back(BytesRef::deepCopyOf(postings->getPayload()));
  }
}

void TestPayloadSpans::VerifyingCollector::reset() { payloads.clear(); }

void TestPayloadSpans::VerifyingCollector::verify(int expectedLength,
                                                  int expectedFirstByte)
{
  for (auto payload : payloads) {
    assertEquals(L"Incorrect payload length", expectedLength, payload->length);
    assertEquals(L"Incorrect first byte", expectedFirstByte, payload->bytes[0]);
  }
}

void TestPayloadSpans::checkSpans(shared_ptr<Spans> spans, int expectedNumSpans,
                                  int expectedNumPayloads,
                                  int expectedPayloadLength,
                                  int expectedFirstByte) 
{
  assertTrue(L"spans is null and it shouldn't be", spans != nullptr);
  // each position match should have a span associated with it, since there is
  // just one underlying term query, there should only be one entry in the span
  shared_ptr<VerifyingCollector> collector = make_shared<VerifyingCollector>();
  int seen = 0;
  while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
    while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      collector->reset();
      spans->collect(collector);
      collector->verify(expectedPayloadLength, expectedFirstByte);
      assertEquals(L"expectedNumPayloads", expectedNumPayloads,
                   collector->payloads.size());
      seen++;
    }
  }
  assertEquals(L"expectedNumSpans", expectedNumSpans, seen);
}

shared_ptr<IndexSearcher> TestPayloadSpans::getSearcher() 
{
  directory = newDirectory();
  std::deque<wstring> docs = {
      L"xx rr yy mm  pp", L"xx yy mm rr pp", L"nopayload qq ss pp np",
      L"one two three four five six seven eight nine ten eleven",
      L"nine one two three four five six seven eight eleven ten"};
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<PayloadAnalyzer>())
          ->setSimilarity(similarity));

  shared_ptr<Document> doc;
  for (int i = 0; i < docs.size(); i++) {
    doc = make_shared<Document>();
    wstring docText = docs[i];
    doc->push_back(
        newTextField(PayloadHelper::FIELD, docText, Field::Store::YES));
    writer->addDocument(doc);
  }

  writer->forceMerge(1);
  closeIndexReader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(closeIndexReader, false);
  return searcher;
}

void TestPayloadSpans::checkSpans(
    shared_ptr<Spans> spans, int numSpans,
    std::deque<int> &numPayloads) 
{
  int cnt = 0;
  shared_ptr<VerifyingCollector> collector = make_shared<VerifyingCollector>();
  while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
    while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      if (VERBOSE) {
        wcout << L"\nSpans Dump --" << endl;
      }
      collector->reset();
      spans->collect(collector);
      assertEquals(L"payload size", numPayloads[cnt],
                   collector->payloads.size());

      cnt++;
    }
  }

  assertEquals(L"expected numSpans", numSpans, cnt);
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloadSpans::PayloadAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      result, make_shared<PayloadFilter>(result));
}

TestPayloadSpans::PayloadFilter::PayloadFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  pos = 0;
  entities->add(L"xx");
  entities->add(L"one");
  nopayload->add(L"nopayload");
  nopayload->add(L"np");
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  payloadAtt = addAttribute(PayloadAttribute::typeid);
}

bool TestPayloadSpans::PayloadFilter::incrementToken() 
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring token = termAtt->toString();

    if (!nopayload->contains(token)) {
      if (entities->contains(token)) {
        payloadAtt->setPayload(
            make_shared<BytesRef>(token + L":Entity:" + to_wstring(pos)));
      } else {
        payloadAtt->setPayload(
            make_shared<BytesRef>(token + L":Noise:" + to_wstring(pos)));
      }
    }
    pos += posIncrAtt->getPositionIncrement();
    return true;
  }
  return false;
}

void TestPayloadSpans::PayloadFilter::reset() 
{
  TokenFilter::reset();
  this->pos = 0;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloadSpans::TestPayloadAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      result, make_shared<PayloadFilter>(result));
}
} // namespace org::apache::lucene::queries::payloads