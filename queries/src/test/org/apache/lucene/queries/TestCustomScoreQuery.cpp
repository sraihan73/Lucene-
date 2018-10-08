using namespace std;

#include "TestCustomScoreQuery.h"

namespace org::apache::lucene::queries
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using Term = org::apache::lucene::index::Term;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using FunctionTestSetup =
    org::apache::lucene::queries::function::FunctionTestSetup;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using CheckHits = org::apache::lucene::search::CheckHits;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using org::junit::BeforeClass;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestCustomScoreQuery::beforeClass() 
{
  createIndex(true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCustomScoreInt() throws Exception
void TestCustomScoreQuery::testCustomScoreInt() 
{
  doTestCustomScore(INT_VALUESOURCE, 1.0);
  doTestCustomScore(INT_VALUESOURCE, 4.0);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCustomScoreFloat() throws Exception
void TestCustomScoreQuery::testCustomScoreFloat() 
{
  doTestCustomScore(FLOAT_VALUESOURCE, 1.0);
  doTestCustomScore(FLOAT_VALUESOURCE, 6.0);
}

TestCustomScoreQuery::CustomAddQuery::CustomAddQuery(
    shared_ptr<Query> q, shared_ptr<FunctionQuery> qValSrc)
    : CustomScoreQuery(q, qValSrc)
{
}

wstring TestCustomScoreQuery::CustomAddQuery::name() { return L"customAdd"; }

shared_ptr<CustomScoreProvider>
TestCustomScoreQuery::CustomAddQuery::getCustomScoreProvider(
    shared_ptr<LeafReaderContext> context)
{
  return make_shared<CustomScoreProviderAnonymousInnerClass>(shared_from_this(),
                                                             context);
}

TestCustomScoreQuery::CustomAddQuery::CustomScoreProviderAnonymousInnerClass::
    CustomScoreProviderAnonymousInnerClass(
        shared_ptr<CustomAddQuery> outerInstance,
        shared_ptr<LeafReaderContext> context)
    : CustomScoreProvider(context)
{
  this->outerInstance = outerInstance;
}

float TestCustomScoreQuery::CustomAddQuery::
    CustomScoreProviderAnonymousInnerClass::customScore(int doc,
                                                        float subQueryScore,
                                                        float valSrcScore)
{
  return subQueryScore + valSrcScore;
}

shared_ptr<Explanation>
TestCustomScoreQuery::CustomAddQuery::CustomScoreProviderAnonymousInnerClass::
    customExplain(int doc, shared_ptr<Explanation> subQueryExpl,
                  shared_ptr<Explanation> valSrcExpl)
{
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  subs.push_back(subQueryExpl);
  if (valSrcExpl != nullptr) {
    subs.push_back(valSrcExpl);
  }
  float valSrcScore = valSrcExpl == nullptr ? 0 : valSrcExpl->getValue();
  return Explanation::match(valSrcScore + subQueryExpl->getValue(),
                            L"custom score: sum of:", subs);
}

TestCustomScoreQuery::CustomMulAddQuery::CustomMulAddQuery(
    shared_ptr<Query> q, shared_ptr<FunctionQuery> qValSrc1,
    shared_ptr<FunctionQuery> qValSrc2)
    : CustomScoreQuery(q, qValSrc1, qValSrc2)
{
}

wstring TestCustomScoreQuery::CustomMulAddQuery::name()
{
  return L"customMulAdd";
}

shared_ptr<CustomScoreProvider>
TestCustomScoreQuery::CustomMulAddQuery::getCustomScoreProvider(
    shared_ptr<LeafReaderContext> context)
{
  return make_shared<CustomScoreProviderAnonymousInnerClass>(shared_from_this(),
                                                             context);
}

TestCustomScoreQuery::CustomMulAddQuery::
    CustomScoreProviderAnonymousInnerClass::
        CustomScoreProviderAnonymousInnerClass(
            shared_ptr<CustomMulAddQuery> outerInstance,
            shared_ptr<LeafReaderContext> context)
    : CustomScoreProvider(context)
{
  this->outerInstance = outerInstance;
}

float TestCustomScoreQuery::CustomMulAddQuery::
    CustomScoreProviderAnonymousInnerClass::customScore(
        int doc, float subQueryScore, std::deque<float> &valSrcScores)
{
  if (valSrcScores.empty()) {
    return subQueryScore;
  }
  if (valSrcScores.size() == 1) {
    return subQueryScore + valSrcScores[0];
    // confirm that skipping beyond the last doc, on the
    // previous reader, hits NO_MORE_DOCS
  }
  return (subQueryScore + valSrcScores[0]) *
         valSrcScores[1]; // we know there are two
}

shared_ptr<Explanation> TestCustomScoreQuery::CustomMulAddQuery::
    CustomScoreProviderAnonymousInnerClass::customExplain(
        int doc, shared_ptr<Explanation> subQueryExpl,
        std::deque<std::shared_ptr<Explanation>> &valSrcExpls)
{
  if (valSrcExpls.empty()) {
    return subQueryExpl;
  }
  if (valSrcExpls.size() == 1) {
    return Explanation::match(
        valSrcExpls[0]->getValue() + subQueryExpl->getValue(),
        L"CustomMulAdd, sum of:", {subQueryExpl, valSrcExpls[0]});
  } else {
    shared_ptr<Explanation> exp = Explanation::match(
        valSrcExpls[0]->getValue() + subQueryExpl->getValue(), L"sum of:",
        {subQueryExpl, valSrcExpls[0]});
    return Explanation::match(valSrcExpls[1]->getValue() * exp->getValue(),
                              L"custom score: product of:",
                              {valSrcExpls[1], exp});
  }
}

shared_ptr<CustomScoreProvider>
TestCustomScoreQuery::CustomExternalQuery::getCustomScoreProvider(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<NumericDocValues> *const values =
      DocValues::getNumeric(context->reader(), INT_FIELD);
  return make_shared<CustomScoreProviderAnonymousInnerClass>(shared_from_this(),
                                                             context, values);
}

TestCustomScoreQuery::CustomExternalQuery::
    CustomScoreProviderAnonymousInnerClass::
        CustomScoreProviderAnonymousInnerClass(
            shared_ptr<CustomExternalQuery> outerInstance,
            shared_ptr<LeafReaderContext> context,
            shared_ptr<NumericDocValues> values)
    : CustomScoreProvider(context)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->values = values;
}

float TestCustomScoreQuery::CustomExternalQuery::
    CustomScoreProviderAnonymousInnerClass::customScore(
        int doc, float subScore, float valSrcScore) 
{
  assertTrue(doc <= context->reader()->maxDoc());
  if (values->docID() < doc) {
    values->advance(doc);
  }
  if (doc == values->docID()) {
    return values->longValue();
  } else {
    return 0;
  }
}

TestCustomScoreQuery::CustomExternalQuery::CustomExternalQuery(
    shared_ptr<Query> q)
    : CustomScoreQuery(q)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCustomExternalQuery() throws Exception
void TestCustomScoreQuery::testCustomExternalQuery() 
{
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"first")),
          BooleanClause::Occur::SHOULD);
  q1->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"aid")),
          BooleanClause::Occur::SHOULD);
  q1->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"text")),
          BooleanClause::Occur::SHOULD);

  shared_ptr<Query> *const q = make_shared<CustomExternalQuery>(q1->build());
  log(q);

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<TopDocs> hits = s->search(q, 1000);
  assertEquals(N_DOCS, hits->totalHits);
  for (int i = 0; i < N_DOCS; i++) {
    constexpr int doc = hits->scoreDocs[i]->doc;
    constexpr float score = hits->scoreDocs[i]->score;
    assertEquals(L"doc=" + to_wstring(doc),
                 static_cast<float>(1) + (4 * doc) % N_DOCS, score, 0.0001);
  }
  delete r;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRewrite() throws Exception
void TestCustomScoreQuery::testRewrite() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> *const s = newSearcher(r);

  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"first"));
  shared_ptr<CustomScoreQuery> original = make_shared<CustomScoreQuery>(q);
  shared_ptr<CustomScoreQuery> rewritten =
      std::static_pointer_cast<CustomScoreQuery>(
          original->rewrite(s->getIndexReader()));
  assertTrue(
      L"rewritten query should be identical, as TermQuery does not rewrite",
      original == rewritten);
  assertTrue(L"no hits for query", s->search(rewritten, 1)->totalHits > 0);
  assertEquals(s->search(q, 1)->totalHits, s->search(rewritten, 1)->totalHits);

  q = make_shared<TermRangeQuery>(TEXT_FIELD, nullptr, nullptr, true,
                                  true); // everything
  original = make_shared<CustomScoreQuery>(q);
  rewritten = std::static_pointer_cast<CustomScoreQuery>(
      original->rewrite(s->getIndexReader()));
  assertTrue(
      L"rewritten query should not be identical, as TermRangeQuery rewrites",
      original != rewritten);
  assertTrue(L"no hits for query", s->search(rewritten, 1)->totalHits > 0);
  assertEquals(s->search(q, 1)->totalHits, s->search(original, 1)->totalHits);
  assertEquals(s->search(q, 1)->totalHits, s->search(rewritten, 1)->totalHits);

  delete r;
}

void TestCustomScoreQuery::doTestCustomScore(
    shared_ptr<ValueSource> valueSource, double dboost) 
{
  float boost = static_cast<float>(dboost);
  shared_ptr<FunctionQuery> functionQuery =
      make_shared<FunctionQuery>(valueSource);
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> s = newSearcher(r);

  // regular (bool) query.
  shared_ptr<BooleanQuery::Builder> q1b = make_shared<BooleanQuery::Builder>();
  q1b->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"first")),
           BooleanClause::Occur::SHOULD);
  q1b->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"aid")),
           BooleanClause::Occur::SHOULD);
  q1b->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"text")),
           BooleanClause::Occur::SHOULD);
  shared_ptr<Query> q1 = q1b->build();
  log(q1);

  // custom query, that should score the same as q1.
  shared_ptr<BooleanQuery::Builder> q2CustomNeutralB =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<Query> q2CustomNeutralInner = make_shared<CustomScoreQuery>(q1);
  q2CustomNeutralB->add(
      make_shared<BoostQuery>(q2CustomNeutralInner,
                              static_cast<float>(sqrt(dboost))),
      BooleanClause::Occur::SHOULD);
  // a little tricky: we split the boost across an outer BQ and CustomScoreQuery
  // this ensures boosting is correct across all these functions (see
  // LUCENE-4935)
  shared_ptr<Query> q2CustomNeutral = q2CustomNeutralB->build();
  q2CustomNeutral = make_shared<BoostQuery>(q2CustomNeutral,
                                            static_cast<float>(sqrt(dboost)));
  log(q2CustomNeutral);

  // custom query, that should (by default) multiply the scores of q1 by that of
  // the field
  shared_ptr<Query> q3CustomMul;
  {
    shared_ptr<CustomScoreQuery> csq =
        make_shared<CustomScoreQuery>(q1, functionQuery);
    q3CustomMul = csq;
  }
  q3CustomMul = make_shared<BoostQuery>(q3CustomMul, boost);
  log(q3CustomMul);

  // custom query, that should add the scores of q1 to that of the field
  shared_ptr<Query> q4CustomAdd;
  {
    shared_ptr<CustomScoreQuery> csq =
        make_shared<CustomAddQuery>(q1, functionQuery);
    q4CustomAdd = csq;
  }
  q4CustomAdd = make_shared<BoostQuery>(q4CustomAdd, boost);
  log(q4CustomAdd);

  // custom query, that multiplies and adds the field score to that of q1
  shared_ptr<Query> q5CustomMulAdd;
  {
    shared_ptr<CustomScoreQuery> csq =
        make_shared<CustomMulAddQuery>(q1, functionQuery, functionQuery);
    q5CustomMulAdd = csq;
  }
  q5CustomMulAdd = make_shared<BoostQuery>(q5CustomMulAdd, boost);
  log(q5CustomMulAdd);

  // do al the searches
  shared_ptr<TopDocs> td1 = s->search(q1, 1000);
  shared_ptr<TopDocs> td2CustomNeutral = s->search(q2CustomNeutral, 1000);
  shared_ptr<TopDocs> td3CustomMul = s->search(q3CustomMul, 1000);
  shared_ptr<TopDocs> td4CustomAdd = s->search(q4CustomAdd, 1000);
  shared_ptr<TopDocs> td5CustomMulAdd = s->search(q5CustomMulAdd, 1000);

  // put results in map_obj so we can verify the scores although they have changed
  unordered_map<int, float> h1 = topDocsToMap(td1);
  unordered_map<int, float> h2CustomNeutral = topDocsToMap(td2CustomNeutral);
  unordered_map<int, float> h3CustomMul = topDocsToMap(td3CustomMul);
  unordered_map<int, float> h4CustomAdd = topDocsToMap(td4CustomAdd);
  unordered_map<int, float> h5CustomMulAdd = topDocsToMap(td5CustomMulAdd);

  verifyResults(boost, s, h1, h2CustomNeutral, h3CustomMul, h4CustomAdd,
                h5CustomMulAdd, q1, q2CustomNeutral, q3CustomMul, q4CustomAdd,
                q5CustomMulAdd);
  delete r;
}

void TestCustomScoreQuery::verifyResults(
    float boost, shared_ptr<IndexSearcher> s, unordered_map<int, float> &h1,
    unordered_map<int, float> &h2customNeutral,
    unordered_map<int, float> &h3CustomMul,
    unordered_map<int, float> &h4CustomAdd,
    unordered_map<int, float> &h5CustomMulAdd, shared_ptr<Query> q1,
    shared_ptr<Query> q2, shared_ptr<Query> q3, shared_ptr<Query> q4,
    shared_ptr<Query> q5) 
{

  // verify numbers of matches
  log(L"#hits = " + h1.size());
  assertEquals(L"queries should have same #hits", h1.size(),
               h2customNeutral.size());
  assertEquals(L"queries should have same #hits", h1.size(),
               h3CustomMul.size());
  assertEquals(L"queries should have same #hits", h1.size(),
               h4CustomAdd.size());
  assertEquals(L"queries should have same #hits", h1.size(),
               h5CustomMulAdd.size());

  QueryUtils::check(random(), q1, s, rarely());
  QueryUtils::check(random(), q2, s, rarely());
  QueryUtils::check(random(), q3, s, rarely());
  QueryUtils::check(random(), q4, s, rarely());
  QueryUtils::check(random(), q5, s, rarely());

  // verify scores ratios
  for (shared_ptr<> : : optional<int> doc : h1.keySet()) {

    log(L"doc = " + doc);

    float fieldScore =
        expectedFieldScore(s->getIndexReader()->document(doc)[ID_FIELD]);
    log(L"fieldScore = " + to_wstring(fieldScore));
    assertTrue(L"fieldScore should not be 0", fieldScore > 0);

    float score1 = h1[doc];
    logResult(L"score1=", s, q1, doc, score1);

    float score2 = h2customNeutral[doc];
    logResult(L"score2=", s, q2, doc, score2);
    assertEquals(L"same score (just boosted) for neutral", boost * score1,
                 score2,
                 CheckHits::explainToleranceDelta(boost * score1, score2));

    float score3 = h3CustomMul[doc];
    logResult(L"score3=", s, q3, doc, score3);
    assertEquals(
        L"new score for custom mul", boost * fieldScore * score1, score3,
        CheckHits::explainToleranceDelta(boost * fieldScore * score1, score3));

    float score4 = h4CustomAdd[doc];
    logResult(L"score4=", s, q4, doc, score4);
    assertEquals(L"new score for custom add", boost * (fieldScore + score1),
                 score4,
                 CheckHits::explainToleranceDelta(boost * (fieldScore + score1),
                                                  score4));

    float score5 = h5CustomMulAdd[doc];
    logResult(L"score5=", s, q5, doc, score5);
    assertEquals(L"new score for custom mul add",
                 boost * fieldScore * (score1 + fieldScore), score5,
                 CheckHits::explainToleranceDelta(
                     boost * fieldScore * (score1 + fieldScore), score5));
  }
}

void TestCustomScoreQuery::logResult(const wstring &msg,
                                     shared_ptr<IndexSearcher> s,
                                     shared_ptr<Query> q, int doc,
                                     float score1) 
{
  log(msg + L" " + to_wstring(score1));
  log(L"Explain by: " + q);
  log(s->explain(q, doc));
}

unordered_map<int, float>
TestCustomScoreQuery::topDocsToMap(shared_ptr<TopDocs> td)
{
  unordered_map<int, float> h = unordered_map<int, float>();
  for (int i = 0; i < td->totalHits; i++) {
    h.emplace(td->scoreDocs[i]->doc, td->scoreDocs[i]->score);
  }
  return h;
}
} // namespace org::apache::lucene::queries