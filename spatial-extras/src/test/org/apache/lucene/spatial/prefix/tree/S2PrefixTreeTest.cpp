using namespace std;

#include "S2PrefixTreeTest.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using com::google::common::geometry::S2CellId;
using com::google::common::geometry::S2Projections;
using Geo3dSpatialContextFactory =
    org::apache::lucene::spatial::spatial4j::Geo3dSpatialContextFactory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testCells()
void S2PrefixTreeTest::testCells()
{
  int face = random()->nextInt(6);
  shared_ptr<S2CellId> id = S2CellId::fromFacePosLevel(face, 0, 0);
  int arity = random()->nextInt(3) + 1;
  int level = random()->nextInt(S2PrefixTree::getMaxLevels(arity));
  level = level * arity;
  for (int i = 0; i < level; i++) {
    int pos = random()->nextInt(4);
    id = id->childBegin();
    if (pos == 0) {
      continue;
    }
    id = id->next();
    if (pos == 1) {
      continue;
    }
    id = id->next();
    if (pos == 2) {
      continue;
    }
    id = id->next();
  }
  shared_ptr<S2PrefixTree> tree = make_shared<S2PrefixTree>(
      (make_shared<Geo3dSpatialContextFactory>())->newSpatialContext(),
      S2PrefixTree::getMaxLevels(arity), arity);
  shared_ptr<S2PrefixTreeCell> cell = make_shared<S2PrefixTreeCell>(tree, id);
  shared_ptr<BytesRef> ref = cell->getTokenBytesWithLeaf(nullptr);
  if (random()->nextBoolean()) {
    int newOffset = random()->nextInt(10) + 1;
    std::deque<char> newBytes(ref->bytes.size() + newOffset);
    for (int i = 0; i < ref->bytes.size(); i++) {
      newBytes[i + newOffset] = ref->bytes[i];
    }
    ref->bytes = newBytes;
    ref->offset = ref->offset + newOffset;
  }
  shared_ptr<S2PrefixTreeCell> cell2 =
      make_shared<S2PrefixTreeCell>(tree, nullptr);
  cell2->readCell(tree, ref);
  assertEquals(cell, cell2);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testDistanceAndLevels()
void S2PrefixTreeTest::testDistanceAndLevels()
{
  shared_ptr<S2PrefixTree> tree = make_shared<S2PrefixTree>(
      (make_shared<Geo3dSpatialContextFactory>())->newSpatialContext(),
      S2PrefixTree::getMaxLevels(1), 1);

  double randomDist = random()->nextDouble() * 5;
  int levelDistance = tree->getLevelForDistance(randomDist);
  double distanceLevel = tree->getDistanceForLevel(levelDistance);
  assertTrue(randomDist > distanceLevel);

  tree = make_shared<S2PrefixTree>(
      (make_shared<Geo3dSpatialContextFactory>())->newSpatialContext(),
      S2PrefixTree::getMaxLevels(2), 2);

  levelDistance = tree->getLevelForDistance(randomDist);
  distanceLevel = tree->getDistanceForLevel(levelDistance);
  assertTrue(randomDist > distanceLevel);

  tree = make_shared<S2PrefixTree>(
      (make_shared<Geo3dSpatialContextFactory>())->newSpatialContext(),
      S2PrefixTree::getMaxLevels(3), 3);

  levelDistance = tree->getLevelForDistance(randomDist);
  distanceLevel = tree->getDistanceForLevel(levelDistance);
  assertTrue(randomDist > distanceLevel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testPrecision()
void S2PrefixTreeTest::testPrecision()
{
  int arity = random()->nextInt(3) + 1;
  shared_ptr<SpatialContext> context =
      (make_shared<Geo3dSpatialContextFactory>())->newSpatialContext();
  shared_ptr<S2PrefixTree> tree = make_shared<S2PrefixTree>(
      context, S2PrefixTree::getMaxLevels(arity), arity);
  double precision = random()->nextDouble();
  int level = tree->getLevelForDistance(precision);
  shared_ptr<Point> point = context->getShapeFactory().pointXY(0, 0);
  shared_ptr<CellIterator> iterator = tree->getTreeCellIterator(point, level);
  shared_ptr<S2PrefixTreeCell> cell = nullptr;
  while (iterator->hasNext()) {
    cell = std::static_pointer_cast<S2PrefixTreeCell>(iterator->next());
    iterator++;
  }
  assertTrue(cell->getLevel() == level);
  double precisionCell =
      S2Projections::MAX_WIDTH::getValue(cell->cellId->level());
  assertTrue(precision > precisionCell);
}
} // namespace org::apache::lucene::spatial::prefix::tree