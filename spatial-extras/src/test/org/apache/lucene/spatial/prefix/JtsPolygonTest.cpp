using namespace std;

#include "JtsPolygonTest.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using TextField = org::apache::lucene::document::TextField;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::junit::Test;

JtsPolygonTest::JtsPolygonTest()
{
  try {
    unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
    args.emplace(
        L"spatialContextFactory",
        L"org.locationtech.spatial4j.context.jts.JtsSpatialContextFactory");
    ctx = SpatialContextFactory::makeSpatialContext(
        args, getClass().getClassLoader());
  } catch (const NoClassDefFoundError &e) {
    assumeTrue(L"This test requires JTS jar: " + e, false);
  }

  shared_ptr<GeohashPrefixTree> grid =
      make_shared<GeohashPrefixTree>(ctx, 11); //< 1 meter == 11 maxLevels
  this->strategy = make_shared<RecursivePrefixTreeStrategy>(
      grid, getClass().getSimpleName());
  (std::static_pointer_cast<RecursivePrefixTreeStrategy>(this->strategy))
      ->setDistErrPct(LUCENE_4464_distErrPct); // 1% radius (small!)
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCloseButNoMatch() throws Exception
void JtsPolygonTest::testCloseButNoMatch() 
{
  getAddAndVerifyIndexedDocuments(L"LUCENE-4464.txt");
  shared_ptr<SpatialArgs> args =
      q(wstring(L"POLYGON((-93.18100824442227 45.25676372469945,") +
            L"-93.23182001200654 45.21421290799412," +
            L"-93.16315546122038 45.23742639412364," +
            L"-93.18100824442227 45.25676372469945))",
        LUCENE_4464_distErrPct);
  shared_ptr<SearchResults> got = executeQuery(strategy->makeQuery(args), 100);
  assertEquals(1, got->numFound);
  assertEquals(L"poly2", got->results[0]->document[L"id"]);
  // did not find poly 1 !
}

shared_ptr<SpatialArgs>
JtsPolygonTest::q(const wstring &shapeStr,
                  double distErrPct) 
{
  shared_ptr<Shape> shape = ctx->readShapeFromWkt(shapeStr);
  shared_ptr<SpatialArgs> args =
      make_shared<SpatialArgs>(SpatialOperation::Intersects, shape);
  args->setDistErrPct(distErrPct);
  return args;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBadPrefixTreePrune() throws Exception
void JtsPolygonTest::testBadPrefixTreePrune() 
{

  shared_ptr<Shape> area = ctx->readShapeFromWkt(
      L"POLYGON((-122.83 48.57, -122.77 48.56, -122.79 48.53, -122.83 48.57))");

  shared_ptr<SpatialPrefixTree> trie = make_shared<QuadPrefixTree>(ctx, 12);
  shared_ptr<TermQueryPrefixTreeStrategy> strategy =
      make_shared<TermQueryPrefixTreeStrategy>(trie, L"geo");
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"id", L"1", Field::Store::YES));

  std::deque<std::shared_ptr<Field>> fields =
      strategy->createIndexableFields(area, 0.025);
  for (auto field : fields) {
    doc->push_back(field);
  }
  addDocument(doc);

  shared_ptr<Point> upperleft = ctx->makePoint(-122.88, 48.54);
  shared_ptr<Point> lowerright = ctx->makePoint(-122.82, 48.62);

  shared_ptr<Query> query = strategy->makeQuery(make_shared<SpatialArgs>(
      SpatialOperation::Intersects, ctx->makeRectangle(upperleft, lowerright)));
  commit();

  shared_ptr<TopDocs> search = indexSearcher->search(query, 10);
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs = search->scoreDocs;
  for (auto scoreDoc : scoreDocs) {
    wcout << indexSearcher->doc(scoreDoc->doc) << endl;
  }

  assertEquals(1, search->totalHits);
}
} // namespace org::apache::lucene::spatial::prefix