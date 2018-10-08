using namespace std;

#include "SpatialExample.h"

namespace org::apache::lucene::spatial
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

void SpatialExample::main(std::deque<wstring> &args) 
{
  (make_shared<SpatialExample>())->test();
}

void SpatialExample::test() 
{
  init();
  indexPoints();
  search();
}

void SpatialExample::init()
{
  // Typical geospatial context
  //  These can also be constructed from SpatialContextFactory
  this->ctx = SpatialContext::GEO;

  int maxLevels = 11; // results in sub-meter precision for geohash
  // TODO demo lookup by detail distance
  //  This can also be constructed from SpatialPrefixTreeFactory
  shared_ptr<SpatialPrefixTree> grid =
      make_shared<GeohashPrefixTree>(ctx, maxLevels);

  this->strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, L"myGeoField");

  this->directory = make_shared<RAMDirectory>();
}

void SpatialExample::indexPoints() 
{
  shared_ptr<IndexWriterConfig> iwConfig =
      make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> indexWriter =
      make_shared<IndexWriter>(directory, iwConfig);

  // Spatial4j is x-y order for arguments
  indexWriter->addDocument(
      newSampleDocument(2, {ctx->makePoint(-80.93, 33.77)}));

  // Spatial4j has a WKT parser which is also "x y" order
  indexWriter->addDocument(newSampleDocument(
      4, {ctx->readShapeFromWkt(L"POINT(60.9289094 -50.7693246)")}));

  indexWriter->addDocument(
      newSampleDocument(20, {ctx->makePoint(0.1, 0.1), ctx->makePoint(0, 0)}));

  delete indexWriter;
}

shared_ptr<Document> SpatialExample::newSampleDocument(int id,
                                                       deque<Shape> &shapes)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StoredField>(L"id", id));
  doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
  // Potentially more than one shape in this field is supported by some
  // strategies; see the javadocs of the SpatialStrategy impl to see.
  for (shared_ptr<Shape> shape : shapes) {
    for (auto f : strategy->createIndexableFields(shape)) {
      doc->push_back(f);
    }
    // store it too; the format is up to you
    //  (assume point in this example)
    shared_ptr<Point> pt = std::static_pointer_cast<Point>(shape);
    doc->push_back(make_shared<StoredField>(strategy->getFieldName(),
                                            pt->getX() + L" " + pt->getY()));
  }

  return doc;
}

void SpatialExample::search() 
{
  shared_ptr<IndexReader> indexReader = DirectoryReader::open(directory);
  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(indexReader);
  shared_ptr<Sort> idSort =
      make_shared<Sort>(make_shared<SortField>(L"id", SortField::Type::INT));

  {
    //--Filter by circle (<= distance from a point)
    // Search with circle
    // note: SpatialArgs can be parsed from a string
    shared_ptr<SpatialArgs> args = make_shared<SpatialArgs>(
        SpatialOperation::Intersects,
        ctx->makeCircle(-80.0, 33.0,
                        DistanceUtils::dist2Degrees(
                            200, DistanceUtils::EARTH_MEAN_RADIUS_KM)));
    shared_ptr<Query> query = strategy->makeQuery(args);
    shared_ptr<TopDocs> docs = indexSearcher->search(query, 10, idSort);
    assertDocMatchedIds(indexSearcher, docs, {2});
    // Now, lets get the distance for the 1st doc via computing from stored
    // point value:
    // (this computation is usually not redundant)
    shared_ptr<Document> doc1 = indexSearcher->doc(docs->scoreDocs[0]->doc);
    wstring doc1Str = doc1->getField(strategy->getFieldName())->stringValue();
    // assume doc1Str is "x y" as written in newSampleDocument()
    int spaceIdx = (int)doc1Str.find(L' ');
    double x = stod(doc1Str.substr(0, spaceIdx));
    double y = stod(doc1Str.substr(spaceIdx + 1));
    double doc1DistDEG = ctx->calcDistance(args->getShape()->getCenter(), x, y);
    assertEquals(121.6,
                 DistanceUtils::degrees2Dist(
                     doc1DistDEG, DistanceUtils::EARTH_MEAN_RADIUS_KM),
                 0.1);
    // or more simply:
    assertEquals(121.6, doc1DistDEG * DistanceUtils::DEG_TO_KM, 0.1);
  }
  {
    //--Match all, order by distance ascending
    shared_ptr<Point> pt = ctx->makePoint(60, -50);
    shared_ptr<DoubleValuesSource> valueSource =
        strategy->makeDistanceValueSource(
            pt, DistanceUtils::DEG_TO_KM); // the distance (in km)
    shared_ptr<Sort> distSort =
        (make_shared<Sort>(valueSource->getSortField(false)))
            ->rewrite(indexSearcher); // false=asc dist
    shared_ptr<TopDocs> docs =
        indexSearcher->search(make_shared<MatchAllDocsQuery>(), 10, distSort);
    assertDocMatchedIds(indexSearcher, docs, {4, 20, 2});
    // To get the distance, we could compute from stored values like earlier.
    // However in this example we sorted on it, and the distance will get
    // computed redundantly.  If the distance is only needed for the top-X
    // search results then that's not a big deal. Alternatively, try wrapping
    // the ValueSource with CachingDoubleValueSource then retrieve the value
    // from the ValueSource now. See LUCENE-4541 for an example.
  }
  {
    // demo arg parsing
    shared_ptr<SpatialArgs> args = make_shared<SpatialArgs>(
        SpatialOperation::Intersects, ctx->makeCircle(-80.0, 33.0, 1));
    shared_ptr<SpatialArgs> args2 =
        (make_shared<SpatialArgsParser>())
            ->parse(L"Intersects(BUFFER(POINT(-80 33),1))", ctx);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(args->toString(), args2->toString());
  }

  delete indexReader;
}

void SpatialExample::assertDocMatchedIds(
    shared_ptr<IndexSearcher> indexSearcher, shared_ptr<TopDocs> docs,
    deque<int> &ids) 
{
  std::deque<int> gotIds(Math::toIntExact(docs->totalHits));
  for (int i = 0; i < gotIds.size(); i++) {
    gotIds[i] = indexSearcher->doc(docs->scoreDocs[i]->doc)
                    ->getField(L"id")
                    ->numericValue()
                    ->intValue();
  }
  assertArrayEquals(ids, gotIds);
}
} // namespace org::apache::lucene::spatial