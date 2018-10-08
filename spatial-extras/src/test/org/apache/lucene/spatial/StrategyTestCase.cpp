using namespace std;

#include "StrategyTestCase.h"

namespace org::apache::lucene::spatial
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;
const wstring StrategyTestCase::DATA_SIMPLE_BBOX = L"simple-bbox.txt";
const wstring StrategyTestCase::DATA_STATES_POLY = L"states-poly.txt";
const wstring StrategyTestCase::DATA_STATES_BBOX = L"states-bbox.txt";
const wstring StrategyTestCase::DATA_COUNTRIES_POLY = L"countries-poly.txt";
const wstring StrategyTestCase::DATA_COUNTRIES_BBOX = L"countries-bbox.txt";
const wstring StrategyTestCase::DATA_WORLD_CITIES_POINTS =
    L"world-cities-points.txt";
const wstring StrategyTestCase::QTEST_States_IsWithin_BBox =
    L"states-IsWithin-BBox.txt";
const wstring StrategyTestCase::QTEST_States_Intersects_BBox =
    L"states-Intersects-BBox.txt";
const wstring StrategyTestCase::QTEST_Cities_Intersects_BBox =
    L"cities-Intersects-BBox.txt";
const wstring StrategyTestCase::QTEST_Simple_Queries_BBox =
    L"simple-Queries-BBox.txt";

void StrategyTestCase::executeQueries(
    shared_ptr<SpatialMatchConcern> concern,
    deque<wstring> &testQueryFile) 
{
  log->info(L"testing queried for strategy " + strategy);
  for (wstring path : testQueryFile) {
    Iterator<std::shared_ptr<SpatialTestQuery>> testQueryIterator =
        getTestQueries(path, ctx);
    runTestQueries(testQueryIterator, concern);
  }
}

void StrategyTestCase::getAddAndVerifyIndexedDocuments(
    const wstring &testDataFile) 
{
  deque<std::shared_ptr<Document>> testDocuments = getDocuments(testDataFile);
  addDocumentsAndCommit(testDocuments);
  verifyDocumentsIndexed(testDocuments.size());
}

deque<std::shared_ptr<Document>>
StrategyTestCase::getDocuments(const wstring &testDataFile) 
{
  return getDocuments(getSampleData(testDataFile));
}

deque<std::shared_ptr<Document>> StrategyTestCase::getDocuments(
    shared_ptr<Iterator<std::shared_ptr<SpatialTestData>>> sampleData)
{
  deque<std::shared_ptr<Document>> documents =
      deque<std::shared_ptr<Document>>();
  while (sampleData->hasNext()) {
    shared_ptr<SpatialTestData> data = sampleData->next();
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(
        make_shared<StringField>(L"id", data->id, Field::Store::YES));
    document->push_back(
        make_shared<StringField>(L"name", data->name, Field::Store::YES));
    shared_ptr<Shape> shape = data->shape;
    shape = convertShapeFromGetDocuments(shape);
    if (shape != nullptr) {
      for (auto f : strategy->createIndexableFields(shape)) {
        document->push_back(f);
      }
      if (storeShape) // just for diagnostics
      {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        document->push_back(make_shared<StoredField>(strategy->getFieldName(),
                                                     shape->toString()));
      }
    }

    documents.push_back(document);
    sampleData++;
  }
  return documents;
}

shared_ptr<Shape>
StrategyTestCase::convertShapeFromGetDocuments(shared_ptr<Shape> shape)
{
  return shape;
}

shared_ptr<Iterator<std::shared_ptr<SpatialTestData>>>
StrategyTestCase::getSampleData(const wstring &testDataFile) 
{
  wstring path = L"data/" + testDataFile;
  shared_ptr<InputStream> stream =
      getClass().getClassLoader().getResourceAsStream(path);
  if (stream == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(L"classpath resource not found: " +
                                        path);
  }
  return SpatialTestData::getTestData(stream, ctx); // closes the InputStream
}

shared_ptr<Iterator<std::shared_ptr<SpatialTestQuery>>>
StrategyTestCase::getTestQueries(
    const wstring &testQueryFile,
    shared_ptr<SpatialContext> ctx) 
{
  shared_ptr<InputStream> in_ =
      getClass().getClassLoader().getResourceAsStream(testQueryFile);
  return SpatialTestQuery::getTestQueries(argsParser, ctx, testQueryFile,
                                          in_); // closes the InputStream
}

void StrategyTestCase::runTestQueries(
    shared_ptr<Iterator<std::shared_ptr<SpatialTestQuery>>> queries,
    shared_ptr<SpatialMatchConcern> concern)
{
  while (queries->hasNext()) {
    shared_ptr<SpatialTestQuery> q = queries->next();
    runTestQuery(concern, q);
    queries++;
  }
}

void StrategyTestCase::runTestQuery(shared_ptr<SpatialMatchConcern> concern,
                                    shared_ptr<SpatialTestQuery> q)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring msg = q->toString(); //"Query: " + q.args.toString(ctx);
  shared_ptr<SearchResults> got =
      executeQuery(makeQuery(q), max(100, q->ids.size() + 1));
  if (storeShape && got->numFound > 0) {
    // check stored value is there
    assertNotNull(got->results[0]->document[strategy->getFieldName()]);
  }
  if (concern->orderIsImportant) {
    deque<wstring>::const_iterator ids = q->ids.begin();
    for (auto r : got->results) {
      wstring id = r->document[L"id"];
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      if (!ids.hasNext()) {
        fail(msg + L" :: Did not get enough results.  Expect" + q->ids +
             L", got: " + got->toDebugString());
      }
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      assertEquals(L"out of order: " + msg, ids.next(), id);
    }

    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    if (ids.hasNext()) {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      fail(msg + L" :: expect more results then we got: " + ids.next());
    }
  } else {
    // We are looking at how the results overlap
    if (concern->resultsAreSuperset) {
      shared_ptr<Set<wstring>> found = unordered_set<wstring>();
      for (auto r : got->results) {
        found->add(r->document[L"id"]);
      }
      for (auto s : q->ids) {
        if (!found->contains(s)) {
          fail(L"Results are mising id: " + s + L" :: " + found);
        }
      }
    } else {
      deque<wstring> found = deque<wstring>();
      for (auto r : got->results) {
        found.push_back(r->document[L"id"]);
      }

      // sort both so that the order is not important
      sort(q->ids.begin(), q->ids.end());
      sort(found.begin(), found.end());
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(msg, q->ids.toString(), found.toString());
    }
  }
}

shared_ptr<Query> StrategyTestCase::makeQuery(shared_ptr<SpatialTestQuery> q)
{
  return strategy->makeQuery(q->args);
}

void StrategyTestCase::adoc(const wstring &id,
                            const wstring &shapeStr) throw(IOException,
                                                           ParseException)
{
  shared_ptr<Shape> shape =
      shapeStr == L"" ? nullptr : ctx->readShapeFromWkt(shapeStr);
  addDocument(newDoc(id, shape));
}

void StrategyTestCase::adoc(const wstring &id,
                            shared_ptr<Shape> shape) 
{
  addDocument(newDoc(id, shape));
}

shared_ptr<Document> StrategyTestCase::newDoc(const wstring &id,
                                              shared_ptr<Shape> shape)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", id, Field::Store::YES));
  if (shape != nullptr) {
    for (auto f : strategy->createIndexableFields(shape)) {
      doc->push_back(f);
    }
    if (storeShape) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StoredField>(
          strategy->getFieldName(),
          shape->toString())); // not to be parsed; just for debug
    }
  }
  return doc;
}

void StrategyTestCase::deleteDoc(const wstring &id) 
{
  indexWriter->deleteDocuments(
      make_shared<TermQuery>(make_shared<Term>(L"id", id)));
}

void StrategyTestCase::checkValueSource(shared_ptr<DoubleValuesSource> vs,
                                        std::deque<float> &scores,
                                        float delta) 
{

  for (auto ctx : indexSearcher->getTopReaderContext()->leaves()) {
    shared_ptr<DoubleValues> v = vs->getValues(ctx, nullptr);
    int count = ctx->reader()->maxDoc();
    for (int i = 0; i < count; i++) {
      assertTrue(v->advanceExact(i));
      int doc = i + ctx->docBase;
      assertEquals(L"Not equal for doc " + to_wstring(doc), v->doubleValue(),
                   static_cast<double>(scores[doc]), delta);
    }
  }
}

void StrategyTestCase::testOperation(shared_ptr<Shape> indexedShape,
                                     shared_ptr<SpatialOperation> operation,
                                     shared_ptr<Shape> queryShape,
                                     bool match) 
{
  assertTrue(L"Faulty test",
             operation->evaluate(indexedShape, queryShape) == match ||
                 indexedShape->equals(queryShape) &&
                     (operation == SpatialOperation::Contains ||
                      operation == SpatialOperation::IsWithin));
  adoc(L"0", indexedShape);
  commit();
  shared_ptr<Query> query =
      strategy->makeQuery(make_shared<SpatialArgs>(operation, queryShape));
  shared_ptr<SearchResults> got = executeQuery(query, 1);
  assert((got->numFound <= 1, L"unclean test env"));
  if ((got->numFound == 1) != match) {
    fail(operation + L" I:" + indexedShape + L" Q:" + queryShape);
  }
  deleteAll(); // clean up after ourselves
}
} // namespace org::apache::lucene::spatial