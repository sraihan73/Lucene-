using namespace std;

#include "TestFunctionScoreQuery.h"

namespace org::apache::lucene::queries::function
{
using Expression = org::apache::lucene::expressions::Expression;
using SimpleBindings = org::apache::lucene::expressions::SimpleBindings;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestFunctionScoreQuery::reader;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestFunctionScoreQuery::searcher;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFunctionScoreQuery::beforeClass() 
{
  createIndex(true);
  reader = DirectoryReader::open(dir);
  searcher = make_shared<IndexSearcher>(reader);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestFunctionScoreQuery::afterClass() 
{
  delete reader;
}

void TestFunctionScoreQuery::testEqualities()
{

  shared_ptr<Query> q1 = make_shared<FunctionScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      DoubleValuesSource::constant(1));
  shared_ptr<Query> q2 = make_shared<FunctionScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"b")),
      DoubleValuesSource::constant(1));
  shared_ptr<Query> q3 = make_shared<FunctionScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"b")),
      DoubleValuesSource::constant(2));
  shared_ptr<Query> q4 = make_shared<FunctionScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"b")),
      DoubleValuesSource::constant(2));

  QueryUtils::check(q1);
  QueryUtils::checkUnequal(q1, q3);
  QueryUtils::checkUnequal(q1, q2);
  QueryUtils::checkUnequal(q2, q3);
  QueryUtils::checkEqual(q3, q4);

  shared_ptr<Query> bq1 = FunctionScoreQuery::boostByValue(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      DoubleValuesSource::constant(2));
  QueryUtils::check(bq1);
  shared_ptr<Query> bq2 = FunctionScoreQuery::boostByValue(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      DoubleValuesSource::constant(4));
  QueryUtils::checkUnequal(bq1, bq2);
  shared_ptr<Query> bq3 = FunctionScoreQuery::boostByValue(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"b")),
      DoubleValuesSource::constant(4));
  QueryUtils::checkUnequal(bq1, bq3);
  QueryUtils::checkUnequal(bq2, bq3);
  shared_ptr<Query> bq4 = FunctionScoreQuery::boostByValue(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"b")),
      DoubleValuesSource::constant(4));
  QueryUtils::checkEqual(bq3, bq4);

  shared_ptr<Query> qq1 = FunctionScoreQuery::boostByQuery(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"z")), 0.1f);
  QueryUtils::check(qq1);
  shared_ptr<Query> qq2 = FunctionScoreQuery::boostByQuery(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"z")), 0.2f);
  QueryUtils::checkUnequal(qq1, qq2);
  shared_ptr<Query> qq3 = FunctionScoreQuery::boostByQuery(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"b")),
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"z")), 0.1f);
  QueryUtils::checkUnequal(qq1, qq3);
  QueryUtils::checkUnequal(qq2, qq3);
  shared_ptr<Query> qq4 = FunctionScoreQuery::boostByQuery(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"zz")), 0.1f);
  QueryUtils::checkUnequal(qq1, qq4);
  QueryUtils::checkUnequal(qq2, qq4);
  QueryUtils::checkUnequal(qq3, qq4);
  shared_ptr<Query> qq5 = FunctionScoreQuery::boostByQuery(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"a")),
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"z")), 0.1f);
  QueryUtils::checkEqual(qq1, qq5);
}

void TestFunctionScoreQuery::testSimpleSourceScore() 
{

  shared_ptr<FunctionScoreQuery> q = make_shared<FunctionScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"first")),
      DoubleValuesSource::fromIntField(INT_FIELD));

  QueryUtils::check(random(), q, searcher, rarely());

  std::deque<int> expectedDocs = {4, 7, 9};
  shared_ptr<TopDocs> docs = searcher->search(q, 4);
  assertEquals(expectedDocs.size(), docs->totalHits);
  for (int i = 0; i < expectedDocs.size(); i++) {
    assertEquals(docs->scoreDocs[i]->doc, expectedDocs[i]);
  }
}

void TestFunctionScoreQuery::testScoreModifyingSource() 
{

  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"first")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"text")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> plain = searcher->search(bq, 1);

  shared_ptr<FunctionScoreQuery> fq = FunctionScoreQuery::boostByValue(
      bq, DoubleValuesSource::fromIntField(L"iii"));

  QueryUtils::check(random(), fq, searcher, rarely());

  std::deque<int> expectedDocs = {4, 7, 9, 8, 12};
  shared_ptr<TopDocs> docs = searcher->search(fq, 5);
  assertEquals(plain->totalHits, docs->totalHits);
  for (int i = 0; i < expectedDocs.size(); i++) {
    assertEquals(expectedDocs[i], docs->scoreDocs[i]->doc);
  }

  shared_ptr<Explanation> expl = searcher->explain(fq, 4);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(expl->toString()->find(L"first") != wstring::npos);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(expl->toString()->find(L"iii") != wstring::npos);
}

void TestFunctionScoreQuery::testCombiningMultipleQueryScores() throw(
    runtime_error)
{

  shared_ptr<TermQuery> q =
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"text"));
  shared_ptr<TopDocs> plain = searcher->search(q, 1);

  shared_ptr<FunctionScoreQuery> fq = FunctionScoreQuery::boostByQuery(
      q, make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"rechecking")),
      100.0f);

  QueryUtils::check(random(), fq, searcher, rarely());

  std::deque<int> expectedDocs = {6, 1, 0, 2, 8};
  shared_ptr<TopDocs> docs = searcher->search(fq, 20);
  assertEquals(plain->totalHits, docs->totalHits);
  for (int i = 0; i < expectedDocs.size(); i++) {
    assertEquals(expectedDocs[i], docs->scoreDocs[i]->doc);
  }

  shared_ptr<Explanation> expl = searcher->explain(fq, 6);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(expl->toString()->find(L"rechecking") != wstring::npos);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(expl->toString()->find(L"text") != wstring::npos);
}

void TestFunctionScoreQuery::testBoostsAreAppliedLast() 
{

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"score", DoubleValuesSource::SCORES);
  shared_ptr<Expression> expr = JavascriptCompiler::compile(L"ln(score + 4)");

  shared_ptr<Query> q1 = make_shared<FunctionScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(TEXT_FIELD, L"text")),
      expr->getDoubleValuesSource(bindings));
  shared_ptr<TopDocs> plain = searcher->search(q1, 5);

  shared_ptr<Query> boosted = make_shared<BoostQuery>(q1, 2);
  shared_ptr<TopDocs> afterboost = searcher->search(boosted, 5);
  assertEquals(plain->totalHits, afterboost->totalHits);
  for (int i = 0; i < 5; i++) {
    assertEquals(plain->scoreDocs[i]->doc, afterboost->scoreDocs[i]->doc);
    assertEquals(plain->scoreDocs[i]->score,
                 afterboost->scoreDocs[i]->score / 2, 0.0001);
  }
}
} // namespace org::apache::lucene::queries::function