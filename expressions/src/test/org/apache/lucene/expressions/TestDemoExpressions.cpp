using namespace std;

#include "TestDemoExpressions.h"

namespace org::apache::lucene::expressions
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using VariableContext = org::apache::lucene::expressions::js::VariableContext;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using CheckHits = org::apache::lucene::search::CheckHits;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.INT_INDEX; import
//    static org.apache.lucene.expressions.js.VariableContext.Type.MEMBER;
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.STR_INDEX;

void TestDemoExpressions::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"some contents and more contents",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 5));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"latitude", Double::doubleToRawLongBits(40.759011)));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"longitude", Double::doubleToRawLongBits(-73.9844722)));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  doc->push_back(newTextField(
      L"body", L"another document with different contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 20));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"latitude", Double::doubleToRawLongBits(40.718266)));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"longitude", Double::doubleToRawLongBits(-74.007819)));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"crappy contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 2));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"latitude", Double::doubleToRawLongBits(40.7051157)));
  doc->push_back(make_shared<NumericDocValuesField>(
      L"longitude", Double::doubleToRawLongBits(-74.0088305)));
  iw->addDocument(doc);

  reader = iw->getReader();
  searcher = make_shared<IndexSearcher>(reader);
  delete iw;
}

void TestDemoExpressions::tearDown() 
{
  reader->close();
  delete dir;
  LuceneTestCase::tearDown();
}

void TestDemoExpressions::test() 
{
  // compile an expression:
  shared_ptr<Expression> expr =
      JavascriptCompiler::compile(L"sqrt(_score) + ln(popularity)");

  // we use SimpleBindings: which just maps variables to SortField instances
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  bindings->add(make_shared<SortField>(L"popularity", SortField::Type::INT));

  // create a sort field and sort by it (reverse order)
  shared_ptr<Sort> sort = make_shared<Sort>(expr->getSortField(bindings, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  searcher->search(query, 3, sort);
}

void TestDemoExpressions::testSortValues() 
{
  shared_ptr<Expression> expr = JavascriptCompiler::compile(L"sqrt(_score)");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));

  shared_ptr<Sort> sort = make_shared<Sort>(expr->getSortField(bindings, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<TopFieldDocs> td = searcher->search(query, 3, sort, true, true);
  for (int i = 0; i < 3; i++) {
    shared_ptr<FieldDoc> d =
        std::static_pointer_cast<FieldDoc>(td->scoreDocs[i]);
    float expected = static_cast<float>(sqrt(d->score));
    float actual = (any_cast<optional<double>>(d->fields[0])).value();
    assertEquals(expected, actual,
                 CheckHits::explainToleranceDelta(expected, actual));
  }
}

void TestDemoExpressions::testTwoOfSameBinding() 
{
  shared_ptr<Expression> expr = JavascriptCompiler::compile(L"_score + _score");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));

  shared_ptr<Sort> sort = make_shared<Sort>(expr->getSortField(bindings, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<TopFieldDocs> td = searcher->search(query, 3, sort, true, true);
  for (int i = 0; i < 3; i++) {
    shared_ptr<FieldDoc> d =
        std::static_pointer_cast<FieldDoc>(td->scoreDocs[i]);
    float expected = 2 * d->score;
    float actual = (any_cast<optional<double>>(d->fields[0])).value();
    assertEquals(expected, actual,
                 CheckHits::explainToleranceDelta(expected, actual));
  }
}

void TestDemoExpressions::testDollarVariable() 
{
  shared_ptr<Expression> expr = JavascriptCompiler::compile(L"$0+$score");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"$0", SortField::Type::SCORE));
  bindings->add(make_shared<SortField>(L"$score", SortField::Type::SCORE));

  shared_ptr<Sort> sort = make_shared<Sort>(expr->getSortField(bindings, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<TopFieldDocs> td = searcher->search(query, 3, sort, true, true);
  for (int i = 0; i < 3; i++) {
    shared_ptr<FieldDoc> d =
        std::static_pointer_cast<FieldDoc>(td->scoreDocs[i]);
    float expected = 2 * d->score;
    float actual = (any_cast<optional<double>>(d->fields[0])).value();
    assertEquals(expected, actual,
                 CheckHits::explainToleranceDelta(expected, actual));
  }
}

void TestDemoExpressions::testExpressionRefersToExpression() throw(
    runtime_error)
{
  shared_ptr<Expression> expr1 = JavascriptCompiler::compile(L"_score");
  shared_ptr<Expression> expr2 = JavascriptCompiler::compile(L"2*expr1");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  bindings->add(L"expr1", expr1);

  shared_ptr<Sort> sort =
      make_shared<Sort>(expr2->getSortField(bindings, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<TopFieldDocs> td = searcher->search(query, 3, sort, true, true);
  for (int i = 0; i < 3; i++) {
    shared_ptr<FieldDoc> d =
        std::static_pointer_cast<FieldDoc>(td->scoreDocs[i]);
    float expected = 2 * d->score;
    float actual = (any_cast<optional<double>>(d->fields[0])).value();
    assertEquals(expected, actual,
                 CheckHits::explainToleranceDelta(expected, actual));
  }
}

void TestDemoExpressions::testLotsOfBindings() 
{
  doTestLotsOfBindings(numeric_limits<char>::max() - 1);
  doTestLotsOfBindings(numeric_limits<char>::max());
  doTestLotsOfBindings(numeric_limits<char>::max() + 1);
  // TODO: ideally we'd test > Short.MAX_VALUE too, but compilation is currently
  // recursive. so if we want to test such huge expressions, we need to instead
  // change parser to use an explicit Stack
}

void TestDemoExpressions::doTestLotsOfBindings(int n) 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < n; i++) {
    if (i > 0) {
      sb->append(L"+");
    }
    sb->append(L"x" + to_wstring(i));
    bindings->add(
        make_shared<SortField>(L"x" + to_wstring(i), SortField::Type::SCORE));
  }

  shared_ptr<Expression> expr = JavascriptCompiler::compile(sb->toString());
  shared_ptr<Sort> sort = make_shared<Sort>(expr->getSortField(bindings, true));
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<TopFieldDocs> td = searcher->search(query, 3, sort, true, true);
  for (int i = 0; i < 3; i++) {
    shared_ptr<FieldDoc> d =
        std::static_pointer_cast<FieldDoc>(td->scoreDocs[i]);
    float expected = n * d->score;
    float actual = (any_cast<optional<double>>(d->fields[0])).value();
    assertEquals(expected, actual,
                 CheckHits::explainToleranceDelta(expected, actual));
  }
}

void TestDemoExpressions::testDistanceSort() 
{
  shared_ptr<Expression> distance = JavascriptCompiler::compile(
      L"haversin(40.7143528,-74.0059731,latitude,longitude)");
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"latitude", SortField::Type::DOUBLE));
  bindings->add(make_shared<SortField>(L"longitude", SortField::Type::DOUBLE));
  shared_ptr<Sort> sort =
      make_shared<Sort>(distance->getSortField(bindings, false));
  shared_ptr<TopFieldDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 3, sort);

  shared_ptr<FieldDoc> d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]);
  assertEquals(0.4621, any_cast<optional<double>>(d->fields[0]), 1E-4);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[1]);
  assertEquals(1.055, any_cast<optional<double>>(d->fields[0]), 1E-4);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[2]);
  assertEquals(5.2859, any_cast<optional<double>>(d->fields[0]), 1E-4);
}

void TestDemoExpressions::testStaticExtendedVariableExample() throw(
    runtime_error)
{
  shared_ptr<Expression> popularity =
      JavascriptCompiler::compile(L"doc[\"popularity\"].value");
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"doc['popularity'].value",
                DoubleValuesSource::fromIntField(L"popularity"));
  shared_ptr<Sort> sort =
      make_shared<Sort>(popularity->getSortField(bindings, true));
  shared_ptr<TopFieldDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 3, sort);

  shared_ptr<FieldDoc> d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]);
  assertEquals(20, any_cast<optional<double>>(d->fields[0]), 1E-4);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[1]);
  assertEquals(5, any_cast<optional<double>>(d->fields[0]), 1E-4);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[2]);
  assertEquals(2, any_cast<optional<double>>(d->fields[0]), 1E-4);
}

void TestDemoExpressions::testDynamicExtendedVariableExample() throw(
    runtime_error)
{
  shared_ptr<Expression> popularity = JavascriptCompiler::compile(
      L"doc['popularity'].value + magicarray[0] + fourtytwo");

  // The following is an example of how to write bindings which parse the
  // variable name into pieces. Note, however, that this requires a lot of error
  // checking.  Each "error case" below should be filled in with proper error
  // messages for a real use case.
  shared_ptr<Bindings> bindings =
      make_shared<BindingsAnonymousInnerClass>(shared_from_this());
  shared_ptr<Sort> sort =
      make_shared<Sort>(popularity->getSortField(bindings, false));
  shared_ptr<TopFieldDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 3, sort);

  shared_ptr<FieldDoc> d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]);
  assertEquals(2092, any_cast<optional<double>>(d->fields[0]), 1E-4);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[1]);
  assertEquals(2095, any_cast<optional<double>>(d->fields[0]), 1E-4);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[2]);
  assertEquals(2110, any_cast<optional<double>>(d->fields[0]), 1E-4);
}

TestDemoExpressions::BindingsAnonymousInnerClass::BindingsAnonymousInnerClass(
    shared_ptr<TestDemoExpressions> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DoubleValuesSource>
TestDemoExpressions::BindingsAnonymousInnerClass::getDoubleValuesSource(
    const wstring &name)
{
  std::deque<std::shared_ptr<VariableContext>> var =
      VariableContext::parse(name);
  assert(var[0]->type == MEMBER);
  wstring base = var[0]->text;
  if (base == L"doc") {
    if (var.size() > 1 && var[1]->type == STR_INDEX) {
      wstring field = var[1]->text;
      if (var.size() > 2 && var[2]->type == MEMBER &&
          var[2]->text == L"value") {
        return DoubleValuesSource::fromIntField(field);
      } else {
        fail(L"member: " +
             var[2]->text); // error case, non/missing "value" member access
      }
    } else {
      fail(); // error case, doc should be a str indexed array
    }
  } else if (base == L"magicarray") {
    if (var.size() > 1 && var[1]->type == INT_INDEX) {
      return DoubleValuesSource::constant(2048);
    } else {
      fail(); // error case, magic array isn't an array
    }
  } else if (base == L"fourtytwo") {
    return DoubleValuesSource::constant(42);
  } else {
    fail(); // error case (variable doesn't exist)
  }
  throw invalid_argument(L"Illegal reference '" + name + L"'");
}
} // namespace org::apache::lucene::expressions