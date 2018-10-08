using namespace std;

#include "TestSloppyPhraseQuery.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestSloppyPhraseQuery::S_1 = L"A A A";
const wstring TestSloppyPhraseQuery::S_2 = L"A 1 2 3 A 4 5 6 A";
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_1 = makeDocument(L"X " + S_1 + L" Y");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_2 = makeDocument(L"X " + S_2 + L" Y");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_3 = makeDocument(L"X " + S_1 + L" A Y");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_1_B =
        makeDocument(L"X " + S_1 + L" Y N N N N " + S_1 + L" Z");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_2_B =
        makeDocument(L"X " + S_2 + L" Y N N N N " + S_2 + L" Z");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_3_B =
        makeDocument(L"X " + S_1 + L" A Y N N N N " + S_1 + L" A Y");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_4 =
        makeDocument(L"A A X A X B A X B B A A X B A A");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_5_3 =
        makeDocument(L"H H H X X X H H H X X X H H H");
const shared_ptr<org::apache::lucene::document::Document>
    TestSloppyPhraseQuery::DOC_5_4 = makeDocument(L"H H H H");
const shared_ptr<PhraseQuery> TestSloppyPhraseQuery::QUERY_1 =
    makePhraseQuery(S_1);
const shared_ptr<PhraseQuery> TestSloppyPhraseQuery::QUERY_2 =
    makePhraseQuery(S_2);
const shared_ptr<PhraseQuery> TestSloppyPhraseQuery::QUERY_4 =
    makePhraseQuery(L"X A A");
const shared_ptr<PhraseQuery> TestSloppyPhraseQuery::QUERY_5_4 =
    makePhraseQuery(L"H H H H");

void TestSloppyPhraseQuery::testDoc4_Query4_All_Slops_Should_match() throw(
    runtime_error)
{
  for (int slop = 0; slop < 30; slop++) {
    int numResultsExpected = slop < 1 ? 0 : 1;
    checkPhraseQuery(DOC_4, QUERY_4, slop, numResultsExpected);
  }
}

void TestSloppyPhraseQuery::testDoc1_Query1_All_Slops_Should_match() throw(
    runtime_error)
{
  for (int slop = 0; slop < 30; slop++) {
    float freq1 = checkPhraseQuery(DOC_1, QUERY_1, slop, 1);
    float freq2 = checkPhraseQuery(DOC_1_B, QUERY_1, slop, 1);
    assertTrue(L"slop=" + to_wstring(slop) + L" freq2=" + to_wstring(freq2) +
                   L" should be greater than score1 " + to_wstring(freq1),
               freq2 > freq1);
  }
}

void TestSloppyPhraseQuery::testDoc2_Query1_Slop_6_or_more_Should_match() throw(
    runtime_error)
{
  for (int slop = 0; slop < 30; slop++) {
    int numResultsExpected = slop < 6 ? 0 : 1;
    float freq1 = checkPhraseQuery(DOC_2, QUERY_1, slop, numResultsExpected);
    if (numResultsExpected > 0) {
      float freq2 = checkPhraseQuery(DOC_2_B, QUERY_1, slop, 1);
      assertTrue(L"slop=" + to_wstring(slop) + L" freq2=" + to_wstring(freq2) +
                     L" should be greater than freq1 " + to_wstring(freq1),
                 freq2 > freq1);
    }
  }
}

void TestSloppyPhraseQuery::testDoc2_Query2_All_Slops_Should_match() throw(
    runtime_error)
{
  for (int slop = 0; slop < 30; slop++) {
    float freq1 = checkPhraseQuery(DOC_2, QUERY_2, slop, 1);
    float freq2 = checkPhraseQuery(DOC_2_B, QUERY_2, slop, 1);
    assertTrue(L"slop=" + to_wstring(slop) + L" freq2=" + to_wstring(freq2) +
                   L" should be greater than freq1 " + to_wstring(freq1),
               freq2 > freq1);
  }
}

void TestSloppyPhraseQuery::testDoc3_Query1_All_Slops_Should_match() throw(
    runtime_error)
{
  for (int slop = 0; slop < 30; slop++) {
    float freq1 = checkPhraseQuery(DOC_3, QUERY_1, slop, 1);
    float freq2 = checkPhraseQuery(DOC_3_B, QUERY_1, slop, 1);
    assertTrue(L"slop=" + to_wstring(slop) + L" freq2=" + to_wstring(freq2) +
                   L" should be greater than freq1 " + to_wstring(freq1),
               freq2 > freq1);
  }
}

void TestSloppyPhraseQuery::
    testDoc5_Query5_Any_Slop_Should_be_consistent() 
{
  int nRepeats = 5;
  for (int slop = 0; slop < 3; slop++) {
    for (int trial = 0; trial < nRepeats; trial++) {
      // should steadily always find this one
      checkPhraseQuery(DOC_5_4, QUERY_5_4, slop, 1);
    }
    for (int trial = 0; trial < nRepeats; trial++) {
      // should steadily never find this one
      checkPhraseQuery(DOC_5_3, QUERY_5_4, slop, 0);
    }
  }
}

float TestSloppyPhraseQuery::checkPhraseQuery(
    shared_ptr<Document> doc, shared_ptr<PhraseQuery> query, int slop,
    int expectedNumResults) 
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  std::deque<std::shared_ptr<Term>> terms = query->getTerms();
  std::deque<int> positions = query->getPositions();
  for (int i = 0; i < terms.size(); ++i) {
    builder->add(terms[i], positions[i]);
  }
  builder->setSlop(slop);
  query = builder->build();

  shared_ptr<MockDirectoryWrapper> ramDir =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), ramDir,
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<MaxFreqCollector> c = make_shared<MaxFreqCollector>();
  searcher->search(query, c);
  assertEquals(L"slop: " + to_wstring(slop) + L"  query: " + query +
                   L"  doc: " + doc + L"  Wrong number of hits",
               expectedNumResults, c->totalHits);

  // QueryUtils.check(query,searcher);
  delete writer;
  delete reader;
  delete ramDir;

  // returns the max Scorer.freq() found, because even though norms are omitted,
  // many index stats are different with these different
  // tokens/distributions/lengths.. otherwise this test is very fragile.
  return c->max;
}

shared_ptr<Document> TestSloppyPhraseQuery::makeDocument(const wstring &docText)
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> f = make_shared<Field>(L"f", docText, customType);
  doc->push_back(f);
  return doc;
}

shared_ptr<PhraseQuery>
TestSloppyPhraseQuery::makePhraseQuery(const wstring &terms)
{
  std::deque<wstring> t = terms.split(L" +");
  return make_shared<PhraseQuery>(L"f", t);
}

TestSloppyPhraseQuery::MaxFreqCollector::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass()
{
}

float TestSloppyPhraseQuery::MaxFreqCollector::SimScorerAnonymousInnerClass::
    score(int doc, float freq) 
{
  throw make_shared<UnsupportedOperationException>();
}

float TestSloppyPhraseQuery::MaxFreqCollector::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return 1.0f / (1.0f + distance);
}

float TestSloppyPhraseQuery::MaxFreqCollector::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestSloppyPhraseQuery::MaxFreqCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
  while (std::dynamic_pointer_cast<AssertingScorer>(this->scorer) != nullptr) {
    this->scorer =
        (std::static_pointer_cast<AssertingScorer>(this->scorer))->getIn();
  }
}

void TestSloppyPhraseQuery::MaxFreqCollector::collect(int doc) throw(
    IOException)
{
  totalHits++;
  shared_ptr<PhraseScorer> ps = std::static_pointer_cast<PhraseScorer>(scorer);
  float freq = ps->matcher->sloppyWeight(simScorer);
  while (ps->matcher->nextMatch()) {
    freq += ps->matcher->sloppyWeight(simScorer);
  }
  max = max(max, freq);
}

bool TestSloppyPhraseQuery::MaxFreqCollector::needsScores() { return true; }

void TestSloppyPhraseQuery::assertSaneScoring(
    shared_ptr<PhraseQuery> pq,
    shared_ptr<IndexSearcher> searcher) 
{
  searcher->search(
      pq, make_shared<SimpleCollectorAnonymousInnerClass>(shared_from_this()));
  QueryUtils::check(random(), pq, searcher);
}

TestSloppyPhraseQuery::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestSloppyPhraseQuery> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestSloppyPhraseQuery::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
  while (std::dynamic_pointer_cast<AssertingScorer>(this->scorer) != nullptr) {
    this->scorer =
        (std::static_pointer_cast<AssertingScorer>(this->scorer))->getIn();
  }
}

void TestSloppyPhraseQuery::SimpleCollectorAnonymousInnerClass::collect(
    int doc) 
{
  assertFalse(isinf(scorer::score()));
}

bool TestSloppyPhraseQuery::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

void TestSloppyPhraseQuery::testSlopWithHoles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> f = make_shared<Field>(L"lyrics", L"", customType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(f);
  f->setStringValue(L"drug drug");
  iw->addDocument(doc);
  f->setStringValue(L"drug druggy drug");
  iw->addDocument(doc);
  f->setStringValue(L"drug druggy druggy drug");
  iw->addDocument(doc);
  f->setStringValue(L"drug druggy drug druggy drug");
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"lyrics", L"drug"), 1);
  builder->add(make_shared<Term>(L"lyrics", L"drug"), 4);
  shared_ptr<PhraseQuery> pq = builder->build();
  // "drug the drug"~1
  assertEquals(1, is->search(pq, 4)->totalHits);
  builder->setSlop(1);
  pq = builder->build();
  assertEquals(3, is->search(pq, 4)->totalHits);
  builder->setSlop(2);
  pq = builder->build();
  assertEquals(4, is->search(pq, 4)->totalHits);
  delete ir;
  delete dir;
}

void TestSloppyPhraseQuery::testInfiniteFreq1() 
{
  wstring document = L"drug druggy drug drug drug";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"lyrics", document,
                          make_shared<FieldType>(TextField::TYPE_NOT_STORED)));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> is = newSearcher(ir);
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"lyrics", L"drug"), 1);
  builder->add(make_shared<Term>(L"lyrics", L"drug"), 3);
  builder->setSlop(1);
  shared_ptr<PhraseQuery> pq = builder->build();
  // "drug the drug"~1
  assertSaneScoring(pq, is);
  delete ir;
  delete dir;
}

void TestSloppyPhraseQuery::testInfiniteFreq2() 
{
  wstring document =
      wstring(L"So much fun to be had in my head ") + L"No more sunshine " +
      L"So much fun just lying in my bed " + L"No more sunshine " +
      L"I can't face the sunlight and the dirt outside " +
      L"Wanna stay in 666 where this darkness don't lie " +
      L"Drug drug druggy " + L"Got a feeling sweet like honey " +
      L"Drug drug druggy " + L"Need sensation like my baby " +
      L"Show me your scars you're so aware " +
      L"I'm not barbaric I just care " + L"Drug drug drug " +
      L"I need a reflection to prove I exist " + L"No more sunshine " +
      L"I am a victim of designer blitz " + L"No more sunshine " +
      L"Dance like a robot when you're chained at the knee " +
      L"The C.I.A say you're all they'll ever need " + L"Drug drug druggy " +
      L"Got a feeling sweet like honey " + L"Drug drug druggy " +
      L"Need sensation like my baby " + L"Snort your lines you're so aware " +
      L"I'm not barbaric I just care " + L"Drug drug druggy " +
      L"Got a feeling sweet like honey " + L"Drug drug druggy " +
      L"Need sensation like my baby";

  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"lyrics", document,
                          make_shared<FieldType>(TextField::TYPE_NOT_STORED)));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> is = newSearcher(ir);

  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"lyrics", L"drug"), 1);
  builder->add(make_shared<Term>(L"lyrics", L"drug"), 4);
  builder->setSlop(5);
  shared_ptr<PhraseQuery> pq = builder->build();
  // "drug the drug"~5
  assertSaneScoring(pq, is);
  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::search