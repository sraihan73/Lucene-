using namespace std;

#include "TestExpressionValueSource.h"

namespace org::apache::lucene::expressions
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using SortField = org::apache::lucene::search::SortField;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestExpressionValueSource::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"some contents and more contents",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 5));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  doc->push_back(newTextField(
      L"body", L"another document with different contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 20));
  doc->push_back(make_shared<NumericDocValuesField>(L"count", 1));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"crappy contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 2));
  iw->addDocument(doc);
  iw->forceMerge(1);

  reader = iw->getReader();
  delete iw;
}

void TestExpressionValueSource::tearDown() 
{
  reader->close();
  delete dir;
  LuceneTestCase::tearDown();
}

void TestExpressionValueSource::testDoubleValuesSourceTypes() throw(
    runtime_error)
{
  shared_ptr<Expression> expr =
      JavascriptCompiler::compile(L"2*popularity + count");
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"popularity", SortField::Type::LONG));
  bindings->add(make_shared<SortField>(L"count", SortField::Type::LONG));
  shared_ptr<DoubleValuesSource> vs = expr->getDoubleValuesSource(bindings);

  assertEquals(1, reader->leaves()->size());
  shared_ptr<LeafReaderContext> leaf = reader->leaves()->get(0);
  shared_ptr<DoubleValues> values = vs->getValues(leaf, nullptr);

  assertTrue(values->advanceExact(0));
  assertEquals(10, values->doubleValue(), 0);
  assertTrue(values->advanceExact(1));
  assertEquals(41, values->doubleValue(), 0);
  assertTrue(values->advanceExact(2));
  assertEquals(4, values->doubleValue(), 0);
}

void TestExpressionValueSource::testDoubleValuesSourceEquals() throw(
    runtime_error)
{
  shared_ptr<Expression> expr = JavascriptCompiler::compile(L"sqrt(a) + ln(b)");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"a", SortField::Type::INT));
  bindings->add(make_shared<SortField>(L"b", SortField::Type::INT));

  shared_ptr<DoubleValuesSource> vs1 = expr->getDoubleValuesSource(bindings);
  // same instance
  assertEquals(vs1, vs1);
  // null
  assertFalse(vs1->equals(nullptr));
  // other object
  assertFalse(vs1->equals(L"foobar"));
  // same bindings and expression instances
  shared_ptr<DoubleValuesSource> vs2 = expr->getDoubleValuesSource(bindings);
  assertEquals(vs1->hashCode(), vs2->hashCode());
  assertEquals(vs1, vs2);
  // equiv bindings (different instance)
  shared_ptr<SimpleBindings> bindings2 = make_shared<SimpleBindings>();
  bindings2->add(make_shared<SortField>(L"a", SortField::Type::INT));
  bindings2->add(make_shared<SortField>(L"b", SortField::Type::INT));
  shared_ptr<DoubleValuesSource> vs3 = expr->getDoubleValuesSource(bindings2);
  assertEquals(vs1, vs3);
  // different bindings (same names, different types)
  shared_ptr<SimpleBindings> bindings3 = make_shared<SimpleBindings>();
  bindings3->add(make_shared<SortField>(L"a", SortField::Type::LONG));
  bindings3->add(make_shared<SortField>(L"b", SortField::Type::FLOAT));
  shared_ptr<DoubleValuesSource> vs4 = expr->getDoubleValuesSource(bindings3);
  assertFalse(vs1->equals(vs4));
}
} // namespace org::apache::lucene::expressions