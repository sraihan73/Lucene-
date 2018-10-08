using namespace std;

#include "TestDocValuesScoring.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using DocValues = org::apache::lucene::index::DocValues;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDocValuesScoring::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newTextField(L"foo", L"", Field::Store::NO);
  doc->push_back(field);
  shared_ptr<Field> dvField =
      make_shared<FloatDocValuesField>(L"foo_boost", 0.0F);
  doc->push_back(dvField);
  shared_ptr<Field> field2 = newTextField(L"bar", L"", Field::Store::NO);
  doc->push_back(field2);

  field->setStringValue(L"quick brown fox");
  field2->setStringValue(L"quick brown fox");
  dvField->setFloatValue(2.0f); // boost x2
  iw->addDocument(doc);
  field->setStringValue(L"jumps over lazy brown dog");
  field2->setStringValue(L"jumps over lazy brown dog");
  dvField->setFloatValue(4.0f); // boost x4
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  // no boosting
  shared_ptr<IndexSearcher> searcher1 = newSearcher(ir, false);
  shared_ptr<Similarity> *const base = searcher1->getSimilarity(true);
  // boosting
  shared_ptr<IndexSearcher> searcher2 = newSearcher(ir, false);
  searcher2->setSimilarity(
      make_shared<PerFieldSimilarityWrapperAnonymousInnerClass>(
          shared_from_this(), field, base));

  // in this case, we searched on field "foo". first document should have 2x the
  // score.
  shared_ptr<TermQuery> tq =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"quick"));
  QueryUtils::check(random(), tq, searcher1);
  QueryUtils::check(random(), tq, searcher2);

  shared_ptr<TopDocs> noboost = searcher1->search(tq, 10);
  shared_ptr<TopDocs> boost = searcher2->search(tq, 10);
  assertEquals(1, noboost->totalHits);
  assertEquals(1, boost->totalHits);

  // System.out.println(searcher2.explain(tq, boost.scoreDocs[0].doc));
  assertEquals(boost->scoreDocs[0]->score, noboost->scoreDocs[0]->score * 2.0f,
               SCORE_EPSILON);

  // this query matches only the second document, which should have 4x the
  // score.
  tq = make_shared<TermQuery>(make_shared<Term>(L"foo", L"jumps"));
  QueryUtils::check(random(), tq, searcher1);
  QueryUtils::check(random(), tq, searcher2);

  noboost = searcher1->search(tq, 10);
  boost = searcher2->search(tq, 10);
  assertEquals(1, noboost->totalHits);
  assertEquals(1, boost->totalHits);

  assertEquals(boost->scoreDocs[0]->score, noboost->scoreDocs[0]->score * 4.0f,
               SCORE_EPSILON);

  // search on on field bar just for kicks, nothing should happen, since we
  // setup our sim provider to only use foo_boost for field foo.
  tq = make_shared<TermQuery>(make_shared<Term>(L"bar", L"quick"));
  QueryUtils::check(random(), tq, searcher1);
  QueryUtils::check(random(), tq, searcher2);

  noboost = searcher1->search(tq, 10);
  boost = searcher2->search(tq, 10);
  assertEquals(1, noboost->totalHits);
  assertEquals(1, boost->totalHits);

  assertEquals(boost->scoreDocs[0]->score, noboost->scoreDocs[0]->score,
               SCORE_EPSILON);

  delete ir;
  delete dir;
}

TestDocValuesScoring::PerFieldSimilarityWrapperAnonymousInnerClass::
    PerFieldSimilarityWrapperAnonymousInnerClass(
        shared_ptr<TestDocValuesScoring> outerInstance, shared_ptr<Field> field,
        shared_ptr<Similarity> base)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->base = base;
  fooSim = make_shared<BoostingSimilarity>(base, L"foo_boost");
}

shared_ptr<Similarity>
TestDocValuesScoring::PerFieldSimilarityWrapperAnonymousInnerClass::get(
    const wstring &field)
{
  return L"foo" == field ? fooSim : base;
}

TestDocValuesScoring::BoostingSimilarity::BoostingSimilarity(
    shared_ptr<Similarity> sim, const wstring &boostField)
    : sim(sim), boostField(boostField)
{
}

int64_t TestDocValuesScoring::BoostingSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return sim->computeNorm(state);
}

shared_ptr<Similarity::SimWeight>
TestDocValuesScoring::BoostingSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return sim->computeWeight(boost, collectionStats, {termStats});
}

shared_ptr<Similarity::SimScorer>
TestDocValuesScoring::BoostingSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> stats,
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Similarity::SimScorer> *const sub = sim->simScorer(stats, context);
  shared_ptr<NumericDocValues> *const values =
      DocValues::getNumeric(context->reader(), boostField);

  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this(), sub,
                                                   values);
}

TestDocValuesScoring::BoostingSimilarity::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<BoostingSimilarity> outerInstance,
                                 shared_ptr<Similarity::SimScorer> sub,
                                 shared_ptr<NumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->sub = sub;
  this->values = values;
}

float TestDocValuesScoring::BoostingSimilarity::SimScorerAnonymousInnerClass::
    getValueForDoc(int doc) 
{
  int curDocID = values->docID();
  if (doc < curDocID) {
    throw invalid_argument(L"doc=" + to_wstring(doc) + L" is before curDocID=" +
                           to_wstring(curDocID));
  }
  if (doc > curDocID) {
    curDocID = values->advance(doc);
  }
  if (curDocID == doc) {
    return Float::intBitsToFloat(static_cast<int>(values->longValue()));
  } else {
    return 0.0f;
  }
}

float TestDocValuesScoring::BoostingSimilarity::SimScorerAnonymousInnerClass::
    score(int doc, float freq) 
{
  return getValueForDoc(doc) * sub->score(doc, freq);
}

float TestDocValuesScoring::BoostingSimilarity::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return sub->computeSlopFactor(distance);
}

float TestDocValuesScoring::BoostingSimilarity::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  return sub->computePayloadFactor(doc, start, end, payload);
}

shared_ptr<Explanation>
TestDocValuesScoring::BoostingSimilarity::SimScorerAnonymousInnerClass::explain(
    int doc, shared_ptr<Explanation> freq) 
{
  shared_ptr<Explanation> boostExplanation =
      Explanation::match(getValueForDoc(doc),
                         L"indexDocValue(" + outerInstance->boostField + L")");
  shared_ptr<Explanation> simExplanation = sub->explain(doc, freq);
  return Explanation::match(boostExplanation->getValue() *
                                simExplanation->getValue(),
                            L"product of:", {boostExplanation, simExplanation});
}
} // namespace org::apache::lucene::search