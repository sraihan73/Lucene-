using namespace std;

#include "TestFunctionQuerySort.h"

namespace org::apache::lucene::queries::function
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using DoubleConstValueSource =
    org::apache::lucene::queries::function::valuesource::DoubleConstValueSource;
using DoubleFieldSource =
    org::apache::lucene::queries::function::valuesource::DoubleFieldSource;
using FloatFieldSource =
    org::apache::lucene::queries::function::valuesource::FloatFieldSource;
using IntFieldSource =
    org::apache::lucene::queries::function::valuesource::IntFieldSource;
using LongFieldSource =
    org::apache::lucene::queries::function::valuesource::LongFieldSource;
using SumFloatFunction =
    org::apache::lucene::queries::function::valuesource::SumFloatFunction;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Type = org::apache::lucene::search::SortField::Type;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFunctionQuerySort::testOptimizedFieldSourceFunctionSorting() throw(
    IOException)
{
  // index contents don't matter for this test.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  constexpr bool reverse = random()->nextBoolean();
  shared_ptr<ValueSource> vs;
  shared_ptr<SortField> sf, vssf;

  vs = make_shared<IntFieldSource>(L"int_field");
  sf = make_shared<SortField>(L"int_field", SortField::Type::INT, reverse);
  vssf = vs->getSortField(reverse);
  assertEquals(sf, vssf);
  sf = sf->rewrite(searcher);
  vssf = vssf->rewrite(searcher);
  assertEquals(sf, vssf);

  vs = make_shared<FloatFieldSource>(L"float_field");
  sf = make_shared<SortField>(L"float_field", SortField::Type::FLOAT, reverse);
  vssf = vs->getSortField(reverse);
  assertEquals(sf, vssf);
  sf = sf->rewrite(searcher);
  vssf = vssf->rewrite(searcher);
  assertEquals(sf, vssf);

  vs = make_shared<DoubleFieldSource>(L"double_field");
  sf =
      make_shared<SortField>(L"double_field", SortField::Type::DOUBLE, reverse);
  vssf = vs->getSortField(reverse);
  assertEquals(sf, vssf);
  sf = sf->rewrite(searcher);
  vssf = vssf->rewrite(searcher);
  assertEquals(sf, vssf);

  vs = make_shared<LongFieldSource>(L"long_field");
  sf = make_shared<SortField>(L"long_field", SortField::Type::LONG, reverse);
  vssf = vs->getSortField(reverse);
  assertEquals(sf, vssf);
  sf = sf->rewrite(searcher);
  vssf = vssf->rewrite(searcher);
  assertEquals(sf, vssf);

  delete reader;
  delete dir;
}

void TestFunctionQuerySort::testSearchAfterWhenSortingByFunctionValues() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  iwc->setMergePolicy(newLogMergePolicy()); // depends on docid order
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = make_shared<StoredField>(L"value", 0);
  shared_ptr<Field> dvField = make_shared<NumericDocValuesField>(L"value", 0);
  doc->push_back(field);
  doc->push_back(dvField);

  // Save docs unsorted (decreasing value n, n-1, ...)
  constexpr int NUM_VALS = 5;
  for (int val = NUM_VALS; val > 0; val--) {
    field->setIntValue(val);
    dvField->setLongValue(val);
    writer->addDocument(doc);
  }

  // Open index
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // Trivial ValueSource function that bypasses single field ValueSource sort
  // optimization
  shared_ptr<ValueSource> src =
      make_shared<SumFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
          make_shared<IntFieldSource>(L"value"),
          make_shared<DoubleConstValueSource>(1.0)});
  // ...and make it a sort criterion
  shared_ptr<SortField> sf = src->getSortField(false)->rewrite(searcher);
  shared_ptr<Sort> orderBy = make_shared<Sort>(sf);

  // Get hits sorted by our FunctionValues (ascending values)
  shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
  shared_ptr<TopDocs> hits = searcher->search(q, reader->maxDoc(), orderBy);
  assertEquals(NUM_VALS, hits->scoreDocs.size());
  // Verify that sorting works in general
  int i = 0;
  for (auto hit : hits->scoreDocs) {
    int valueFromDoc =
        static_cast<Integer>(reader->document(hit->doc)[L"value"]);
    assertEquals(++i, valueFromDoc);
  }

  // Now get hits after hit #2 using IS.searchAfter()
  int afterIdx = 1;
  shared_ptr<FieldDoc> afterHit =
      std::static_pointer_cast<FieldDoc>(hits->scoreDocs[afterIdx]);
  hits = searcher->searchAfter(afterHit, q, reader->maxDoc(), orderBy);

  // Expected # of hits: NUM_VALS - 2
  assertEquals(NUM_VALS - (afterIdx + 1), hits->scoreDocs.size());

  // Verify that hits are actually "after"
  int afterValue = (any_cast<optional<double>>(afterHit->fields[0])).value();
  for (auto hit : hits->scoreDocs) {
    int val = static_cast<Integer>(reader->document(hit->doc)[L"value"]);
    assertTrue(afterValue <= val);
    assertFalse(hit->doc == afterHit->doc);
  }
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::queries::function