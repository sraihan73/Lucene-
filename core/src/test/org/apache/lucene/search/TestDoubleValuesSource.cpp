using namespace std;

#include "TestDoubleValuesSource.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDoubleValuesSource::setUp() 
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
    if (i == 545) {
      document->push_back(
          make_shared<DoubleDocValuesField>(L"onefield", 45.72));
    }
    iw->addDocument(document);
  }
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);
}

void TestDoubleValuesSource::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestDoubleValuesSource::testSortMissing() 
{
  shared_ptr<DoubleValuesSource> onefield =
      DoubleValuesSource::fromDoubleField(L"onefield");
  shared_ptr<TopDocs> results =
      searcher->search(make_shared<MatchAllDocsQuery>(), 1,
                       make_shared<Sort>(onefield->getSortField(true)));
  shared_ptr<FieldDoc> first =
      std::static_pointer_cast<FieldDoc>(results->scoreDocs[0]);
  TestUtil::assertEquals(45.72, first->fields[0]);
}

void TestDoubleValuesSource::testSimpleFieldEquivalences() 
{
  checkSorts(make_shared<MatchAllDocsQuery>(),
             make_shared<Sort>(make_shared<SortField>(
                 L"int", SortField::Type::INT, random()->nextBoolean())));
  checkSorts(make_shared<MatchAllDocsQuery>(),
             make_shared<Sort>(make_shared<SortField>(
                 L"long", SortField::Type::LONG, random()->nextBoolean())));
  checkSorts(make_shared<MatchAllDocsQuery>(),
             make_shared<Sort>(make_shared<SortField>(
                 L"float", SortField::Type::FLOAT, random()->nextBoolean())));
  checkSorts(make_shared<MatchAllDocsQuery>(),
             make_shared<Sort>(make_shared<SortField>(
                 L"double", SortField::Type::DOUBLE, random()->nextBoolean())));
}

void TestDoubleValuesSource::testHashCodeAndEquals()
{
  shared_ptr<DoubleValuesSource> vs1 =
      DoubleValuesSource::fromDoubleField(L"double");
  shared_ptr<DoubleValuesSource> vs2 =
      DoubleValuesSource::fromDoubleField(L"double");
  TestUtil::assertEquals(vs1, vs2);
  TestUtil::assertEquals(vs1->hashCode(), vs2->hashCode());
  shared_ptr<DoubleValuesSource> v3 =
      DoubleValuesSource::fromLongField(L"long");
  assertFalse(vs1->equals(v3));

  TestUtil::assertEquals(DoubleValuesSource::constant(5),
                         DoubleValuesSource::constant(5));
  TestUtil::assertEquals(DoubleValuesSource::constant(5)->hashCode(),
                         DoubleValuesSource::constant(5)->hashCode());
  assertFalse((DoubleValuesSource::constant(5)->equals(
      DoubleValuesSource::constant(6))));

  TestUtil::assertEquals(DoubleValuesSource::SCORES,
                         DoubleValuesSource::SCORES);
  assertFalse(
      DoubleValuesSource::constant(5)->equals(DoubleValuesSource::SCORES));
}

void TestDoubleValuesSource::testSimpleFieldSortables() 
{
  int n = atLeast(4);
  for (int i = 0; i < n; i++) {
    shared_ptr<Sort> sort = randomSort();
    checkSorts(make_shared<MatchAllDocsQuery>(), sort);
    checkSorts(make_shared<TermQuery>(make_shared<Term>(L"english", L"one")),
               sort);
  }
}

shared_ptr<Sort> TestDoubleValuesSource::randomSort() 
{
  bool reversed = random()->nextBoolean();
  std::deque<std::shared_ptr<SortField>> fields = {
      make_shared<SortField>(L"int", SortField::Type::INT, reversed),
      make_shared<SortField>(L"long", SortField::Type::LONG, reversed),
      make_shared<SortField>(L"float", SortField::Type::FLOAT, reversed),
      make_shared<SortField>(L"double", SortField::Type::DOUBLE, reversed),
      make_shared<SortField>(L"score", SortField::Type::SCORE)};
  Collections::shuffle(Arrays::asList(fields), random());
  int numSorts = TestUtil::nextInt(random(), 1, fields.size());
  return make_shared<Sort>(Arrays::copyOfRange(fields, 0, numSorts));
}

shared_ptr<Sort>
TestDoubleValuesSource::convertSortToSortable(shared_ptr<Sort> sort)
{
  std::deque<std::shared_ptr<SortField>> original = sort->getSort();
  std::deque<std::shared_ptr<SortField>> mutated(original.size());
  for (int i = 0; i < mutated.size(); i++) {
    if (random()->nextInt(3) > 0) {
      shared_ptr<SortField> s = original[i];
      bool reverse = s->getType() == SortField::Type::SCORE || s->getReverse();
      switch (s->getType()) {
      case INT:
        mutated[i] = DoubleValuesSource::fromIntField(s->getField())
                         ->getSortField(reverse);
        break;
      case LONG:
        mutated[i] = DoubleValuesSource::fromLongField(s->getField())
                         ->getSortField(reverse);
        break;
      case FLOAT:
        mutated[i] = DoubleValuesSource::fromFloatField(s->getField())
                         ->getSortField(reverse);
        break;
      case DOUBLE:
        mutated[i] = DoubleValuesSource::fromDoubleField(s->getField())
                         ->getSortField(reverse);
        break;
      case SCORE:
        mutated[i] = DoubleValuesSource::SCORES::getSortField(reverse);
        break;
      default:
        mutated[i] = original[i];
      }
    } else {
      mutated[i] = original[i];
    }
  }

  return make_shared<Sort>(mutated);
}

void TestDoubleValuesSource::checkSorts(
    shared_ptr<Query> query, shared_ptr<Sort> sort) 
{
  int size =
      TestUtil::nextInt(random(), 1, searcher->getIndexReader()->maxDoc() / 5);
  shared_ptr<TopDocs> expected = searcher->search(
      query, size, sort, random()->nextBoolean(), random()->nextBoolean());
  shared_ptr<Sort> mutatedSort = convertSortToSortable(sort);
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

std::deque<std::shared_ptr<Query>> const TestDoubleValuesSource::testQueries =
    std::deque<std::shared_ptr<Query>>{
        make_shared<MatchAllDocsQuery>(),
        make_shared<TermQuery>(
            make_shared<org::apache::lucene::index::Term>(L"oddeven", L"odd")),
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(
                      make_shared<org::apache::lucene::index::Term>(L"english",
                                                                    L"one")),
                  BooleanClause::Occur::MUST)
            ->add(make_shared<TermQuery>(
                      make_shared<org::apache::lucene::index::Term>(L"english",
                                                                    L"two")),
                  BooleanClause::Occur::MUST)
            ->build()};

void TestDoubleValuesSource::testExplanations() 
{
  for (auto q : testQueries) {
    testExplanations(q, DoubleValuesSource::fromQuery(make_shared<TermQuery>(
                            make_shared<Term>(L"english", L"one"))));
    testExplanations(q, DoubleValuesSource::fromIntField(L"int"));
    testExplanations(q, DoubleValuesSource::fromLongField(L"long"));
    testExplanations(q, DoubleValuesSource::fromFloatField(L"float"));
    testExplanations(q, DoubleValuesSource::fromDoubleField(L"double"));
    testExplanations(q, DoubleValuesSource::fromDoubleField(L"onefield"));
    testExplanations(q, DoubleValuesSource::constant(5.45));
  }
}

void TestDoubleValuesSource::testExplanations(
    shared_ptr<Query> q, shared_ptr<DoubleValuesSource> vs) 
{
  shared_ptr<DoubleValuesSource> rewritten = vs->rewrite(searcher);
  searcher->search(q, make_shared<SimpleCollectorAnonymousInnerClass>(
                          shared_from_this(), q, rewritten));
}

TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestDoubleValuesSource> outerInstance,
        shared_ptr<org::apache::lucene::search::Query> q,
        shared_ptr<org::apache::lucene::search::DoubleValuesSource> rewritten)
{
  this->outerInstance = outerInstance;
  this->q = q;
  this->rewritten = rewritten;
}

void TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass::
    doSetNextReader(shared_ptr<LeafReaderContext> context) 
{
  this->ctx = context;
}

void TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->v =
      rewritten->getValues(this->ctx, DoubleValuesSource::fromScorer(scorer));
}

void TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass::collect(
    int doc) 
{
  shared_ptr<Explanation> scoreExpl =
      outerInstance->searcher->explain(q, ctx::docBase + doc);
  if (this->v.advanceExact(doc)) {
    CheckHits::verifyExplanation(L"", doc, static_cast<float>(v::doubleValue()),
                                 true, rewritten->explain(ctx, doc, scoreExpl));
  } else {
    assertFalse(rewritten->explain(ctx, doc, scoreExpl)->isMatch());
  }
}

bool TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return rewritten->needsScores();
}

void TestDoubleValuesSource::testQueryDoubleValuesSource() 
{
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(L"english", L"two"));
  shared_ptr<DoubleValuesSource> vs =
      DoubleValuesSource::fromQuery(q)->rewrite(searcher);
  searcher->search(q, make_shared<SimpleCollectorAnonymousInnerClass2>(
                          shared_from_this(), vs));
}

TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        shared_ptr<TestDoubleValuesSource> outerInstance,
        shared_ptr<org::apache::lucene::search::DoubleValuesSource> vs)
{
  this->outerInstance = outerInstance;
  this->vs = vs;
}

void TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass2::
    doSetNextReader(shared_ptr<LeafReaderContext> context) 
{
  this->ctx = context;
}

void TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
  this->v = vs->getValues(this->ctx, DoubleValuesSource::fromScorer(scorer));
}

void TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass2::collect(
    int doc) 
{
  assertTrue(v::advanceExact(doc));
  assertEquals(scorer::score(), v::doubleValue(), 0.00001);
}

bool TestDoubleValuesSource::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return true;
}
} // namespace org::apache::lucene::search