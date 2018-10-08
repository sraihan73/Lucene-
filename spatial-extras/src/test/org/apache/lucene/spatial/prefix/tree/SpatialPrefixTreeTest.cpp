using namespace std;

#include "SpatialPrefixTreeTest.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using TextField = org::apache::lucene::document::TextField;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpatialTestCase = org::apache::lucene::spatial::SpatialTestCase;
using TermQueryPrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::TermQueryPrefixTreeStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void SpatialPrefixTreeTest::setUp() 
{
  SpatialTestCase::setUp();
  ctx = SpatialContext::GEO;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCellTraverse()
void SpatialPrefixTreeTest::testCellTraverse()
{
  trie = make_shared<GeohashPrefixTree>(ctx, 4);

  shared_ptr<Cell> prevC = nullptr;
  shared_ptr<Cell> c = trie->getWorldCell();
  assertEquals(0, c->getLevel());
  assertEquals(ctx->getWorldBounds(), c->getShape());
  while (c->getLevel() < trie->getMaxLevels()) {
    prevC = c;
    deque<std::shared_ptr<Cell>> subCells = deque<std::shared_ptr<Cell>>();
    shared_ptr<CellIterator> subCellsIter = c->getNextLevelCells(nullptr);
    while (subCellsIter->hasNext()) {
      subCells.push_back(subCellsIter->next());
      subCellsIter++;
    }
    c = subCells[random()->nextInt(subCells.size() - 1)];

    assertEquals(prevC->getLevel() + 1, c->getLevel());
    shared_ptr<Rectangle> prevNShape =
        std::static_pointer_cast<Rectangle>(prevC->getShape());
    shared_ptr<Shape> s = c->getShape();
    shared_ptr<Rectangle> sbox = s->getBoundingBox();
    assertTrue(prevNShape->getWidth() > sbox->getWidth());
    assertTrue(prevNShape->getHeight() > sbox->getHeight());
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBadPrefixTreePrune() throws Exception
void SpatialPrefixTreeTest::testBadPrefixTreePrune() 
{

  trie = make_shared<QuadPrefixTree>(ctx, 12);
  shared_ptr<TermQueryPrefixTreeStrategy> strategy =
      make_shared<TermQueryPrefixTreeStrategy>(trie, L"geo");
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"id", L"1", Field::Store::YES));

  shared_ptr<Shape> area = ctx->makeRectangle(-122.82, -122.78, 48.54, 48.56);

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
} // namespace org::apache::lucene::spatial::prefix::tree