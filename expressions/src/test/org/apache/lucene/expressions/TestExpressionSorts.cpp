using namespace std;

#include "TestExpressionSorts.h"

namespace org::apache::lucene::expressions
{
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestExpressionSorts::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  int numDocs = TestUtil::nextInt(random(), 2049, 4000);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(
        newTextField(L"english", English::intToEnglish(i), Field::Store::NO));
    document->push_back(newTextField(
        L"oddeven", (i % 2 == 0) ? L"even" : L"odd", Field::Store::NO));
    document->push_back(
        make_shared<NumericDocValuesField>(L"int", random()->nextInt()));
    document->push_back(
        make_shared<NumericDocValuesField>(L"long", random()->nextLong()));
    document->push_back(
        make_shared<FloatDocValuesField>(L"float", random()->nextFloat()));
    document->push_back(
        make_shared<DoubleDocValuesField>(L"double", random()->nextDouble()));
    iw->addDocument(document);
  }
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);
}

void TestExpressionSorts::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestExpressionSorts::testQueries() 
{
  int n = atLeast(4);
  for (int i = 0; i < n; i++) {
    assertQuery(make_shared<MatchAllDocsQuery>());
    assertQuery(make_shared<TermQuery>(make_shared<Term>(L"english", L"one")));
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"english", L"one")),
            BooleanClause::Occur::SHOULD);
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"oddeven", L"even")),
            BooleanClause::Occur::SHOULD);
    assertQuery(bq->build());
    // force in order
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"english", L"two")),
            BooleanClause::Occur::SHOULD);
    bq->setMinimumNumberShouldMatch(2);
    assertQuery(bq->build());
  }
}

void TestExpressionSorts::assertQuery(shared_ptr<Query> query) throw(
    runtime_error)
{
  for (int i = 0; i < 10; i++) {
    bool reversed = random()->nextBoolean();
    std::deque<std::shared_ptr<SortField>> fields = {
        make_shared<SortField>(L"int", SortField::Type::INT, reversed),
        make_shared<SortField>(L"long", SortField::Type::LONG, reversed),
        make_shared<SortField>(L"float", SortField::Type::FLOAT, reversed),
        make_shared<SortField>(L"double", SortField::Type::DOUBLE, reversed),
        make_shared<SortField>(L"score", SortField::Type::SCORE)};
    Collections::shuffle(Arrays::asList(fields), random());
    int numSorts = TestUtil::nextInt(random(), 1, fields.size());
    assertQuery(query,
                make_shared<Sort>(Arrays::copyOfRange(fields, 0, numSorts)));
  }
}

void TestExpressionSorts::assertQuery(
    shared_ptr<Query> query, shared_ptr<Sort> sort) 
{
  int size =
      TestUtil::nextInt(random(), 1, searcher->getIndexReader()->maxDoc() / 5);
  shared_ptr<TopDocs> expected = searcher->search(
      query, size, sort, random()->nextBoolean(), random()->nextBoolean());

  // make our actual sort, mutating original by replacing some of the
  // sortfields with equivalent expressions

  std::deque<std::shared_ptr<SortField>> original = sort->getSort();
  std::deque<std::shared_ptr<SortField>> mutated(original.size());
  for (int i = 0; i < mutated.size(); i++) {
    if (random()->nextInt(3) > 0) {
      shared_ptr<SortField> s = original[i];
      shared_ptr<Expression> expr = JavascriptCompiler::compile(s->getField());
      shared_ptr<SimpleBindings> simpleBindings = make_shared<SimpleBindings>();
      simpleBindings->add(s);
      bool reverse = s->getType() == SortField::Type::SCORE || s->getReverse();
      mutated[i] = expr->getSortField(simpleBindings, reverse);
    } else {
      mutated[i] = original[i];
    }
  }

  shared_ptr<Sort> mutatedSort = make_shared<Sort>(mutated);
  shared_ptr<TopDocs> actual =
      searcher->search(query, size, mutatedSort, random()->nextBoolean(),
                       random()->nextBoolean());
  CheckHits::checkEqual(query, expected->scoreDocs, actual->scoreDocs);

  if (size < actual->totalHits) {
    expected =
        searcher->searchAfter(expected->scoreDocs[size - 1], query, size, sort);
    actual = searcher->searchAfter(actual->scoreDocs[size - 1], query, size,
                                   mutatedSort);
    CheckHits::checkEqual(query, expected->scoreDocs, actual->scoreDocs);
  }
}
} // namespace org::apache::lucene::expressions