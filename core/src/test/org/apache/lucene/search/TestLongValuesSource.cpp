using namespace std;

#include "TestLongValuesSource.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestLongValuesSource::setUp() 
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
    if (i == 545) {
      document->push_back(make_shared<NumericDocValuesField>(L"onefield", 45));
    }
    iw->addDocument(document);
  }
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);
}

void TestLongValuesSource::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestLongValuesSource::testSortMissing() 
{
  shared_ptr<LongValuesSource> onefield =
      LongValuesSource::fromLongField(L"onefield");
  shared_ptr<TopDocs> results =
      searcher->search(make_shared<MatchAllDocsQuery>(), 1,
                       make_shared<Sort>(onefield->getSortField(true)));
  shared_ptr<FieldDoc> first =
      std::static_pointer_cast<FieldDoc>(results->scoreDocs[0]);
  TestUtil::assertEquals(45LL, first->fields[0]);
}

void TestLongValuesSource::testSimpleFieldEquivalences() 
{
  checkSorts(make_shared<MatchAllDocsQuery>(),
             make_shared<Sort>(make_shared<SortField>(
                 L"int", SortField::Type::INT, random()->nextBoolean())));
  checkSorts(make_shared<MatchAllDocsQuery>(),
             make_shared<Sort>(make_shared<SortField>(
                 L"long", SortField::Type::LONG, random()->nextBoolean())));
}

void TestLongValuesSource::testHashCodeAndEquals()
{
  shared_ptr<LongValuesSource> vs1 = LongValuesSource::fromLongField(L"long");
  shared_ptr<LongValuesSource> vs2 = LongValuesSource::fromLongField(L"long");
  TestUtil::assertEquals(vs1, vs2);
  TestUtil::assertEquals(vs1->hashCode(), vs2->hashCode());
  shared_ptr<LongValuesSource> v3 = LongValuesSource::fromLongField(L"int");
  assertFalse(vs1->equals(v3));
}

void TestLongValuesSource::testSimpleFieldSortables() 
{
  int n = atLeast(4);
  for (int i = 0; i < n; i++) {
    shared_ptr<Sort> sort = randomSort();
    checkSorts(make_shared<MatchAllDocsQuery>(), sort);
    checkSorts(make_shared<TermQuery>(make_shared<Term>(L"english", L"one")),
               sort);
  }
}

shared_ptr<Sort> TestLongValuesSource::randomSort() 
{
  bool reversed = random()->nextBoolean();
  std::deque<std::shared_ptr<SortField>> fields = {
      make_shared<SortField>(L"int", SortField::Type::INT, reversed),
      make_shared<SortField>(L"long", SortField::Type::LONG, reversed)};
  Collections::shuffle(Arrays::asList(fields), random());
  int numSorts = TestUtil::nextInt(random(), 1, fields.size());
  return make_shared<Sort>(Arrays::copyOfRange(fields, 0, numSorts));
}

shared_ptr<Sort>
TestLongValuesSource::convertSortToSortable(shared_ptr<Sort> sort)
{
  std::deque<std::shared_ptr<SortField>> original = sort->getSort();
  std::deque<std::shared_ptr<SortField>> mutated(original.size());
  for (int i = 0; i < mutated.size(); i++) {
    if (random()->nextInt(3) > 0) {
      shared_ptr<SortField> s = original[i];
      bool reverse = s->getType() == SortField::Type::SCORE || s->getReverse();
      switch (s->getType()) {
      case INT:
        mutated[i] = LongValuesSource::fromIntField(s->getField())
                         ->getSortField(reverse);
        break;
      case LONG:
        mutated[i] = LongValuesSource::fromLongField(s->getField())
                         ->getSortField(reverse);
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

void TestLongValuesSource::checkSorts(
    shared_ptr<Query> query, shared_ptr<Sort> sort) 
{
  int size =
      TestUtil::nextInt(random(), 1, searcher->getIndexReader()->maxDoc() / 5);
  shared_ptr<Sort> mutatedSort = convertSortToSortable(sort);
  shared_ptr<TopDocs> actual =
      searcher->search(query, size, mutatedSort, random()->nextBoolean(),
                       random()->nextBoolean());
  shared_ptr<TopDocs> expected = searcher->search(
      query, size, sort, random()->nextBoolean(), random()->nextBoolean());

  CheckHits::checkEqual(query, expected->scoreDocs, actual->scoreDocs);

  if (size < actual->totalHits) {
    expected =
        searcher->searchAfter(expected->scoreDocs[size - 1], query, size, sort);
    actual = searcher->searchAfter(actual->scoreDocs[size - 1], query, size,
                                   mutatedSort);
    CheckHits::checkEqual(query, expected->scoreDocs, actual->scoreDocs);
  }
}
} // namespace org::apache::lucene::search