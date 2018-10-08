using namespace std;

#include "TestTermQueryPrefixGridStrategy.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using SpatialTestCase = org::apache::lucene::spatial::SpatialTestCase;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNGramPrefixGridLosAngeles() throws
// java.io.IOException
void TestTermQueryPrefixGridStrategy::testNGramPrefixGridLosAngeles() throw(
    IOException)
{
  shared_ptr<SpatialContext> ctx = SpatialContext::GEO;
  shared_ptr<TermQueryPrefixTreeStrategy> prefixGridStrategy =
      make_shared<TermQueryPrefixTreeStrategy>(make_shared<QuadPrefixTree>(ctx),
                                               L"geo");

  shared_ptr<Shape> point = ctx->makePoint(-118.243680, 34.052230);

  shared_ptr<Document> losAngeles = make_shared<Document>();
  losAngeles->push_back(
      make_shared<StringField>(L"name", L"Los Angeles", Field::Store::YES));
  for (auto field : prefixGridStrategy->createIndexableFields(point)) {
    losAngeles->push_back(field);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  losAngeles->push_back(
      make_shared<StoredField>(prefixGridStrategy->getFieldName(),
                               point->toString())); // just for diagnostics

  addDocumentsAndCommit(Arrays::asList(losAngeles));

  // This won't work with simple spatial context...
  shared_ptr<SpatialArgsParser> spatialArgsParser =
      make_shared<SpatialArgsParser>();
  // TODO... use a non polygon query
  //    SpatialArgs spatialArgs = spatialArgsParser.parse(
  //        "Intersects(POLYGON((-127.00390625
  //        39.8125,-112.765625 39.98828125,-111.53515625
  //        31.375,-125.94921875 30.14453125,-127.00390625 39.8125)))", new
  //        SimpleSpatialContext());

  //    Query query = prefixGridStrategy.makeQuery(spatialArgs, fieldInfo);
  //    SearchResults searchResults = executeQuery(query, 1);
  //    assertEquals(1, searchResults.numFound);
}
} // namespace org::apache::lucene::spatial::prefix