using namespace std;

#include "GeohashPrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::io::GeohashUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using BytesRef = org::apache::lucene::util::BytesRef;

int GeohashPrefixTree::Factory::getLevelForDistance(double degrees)
{
  shared_ptr<GeohashPrefixTree> grid = make_shared<GeohashPrefixTree>(
      ctx, GeohashPrefixTree::getMaxLevelsPossible());
  return grid->getLevelForDistance(degrees);
}

shared_ptr<SpatialPrefixTree> GeohashPrefixTree::Factory::newSPT()
{
  return make_shared<GeohashPrefixTree>(
      ctx, maxLevels ? maxLevels : GeohashPrefixTree::getMaxLevelsPossible());
}

GeohashPrefixTree::GeohashPrefixTree(shared_ptr<SpatialContext> ctx,
                                     int maxLevels)
    : LegacyPrefixTree(ctx, maxLevels)
{
  shared_ptr<Rectangle> bounds = ctx->getWorldBounds();
  if (bounds->getMinX() != -180) {
    throw invalid_argument(L"Geohash only supports lat-lon world bounds. Got " +
                           bounds);
  }
  int MAXP = getMaxLevelsPossible();
  if (maxLevels <= 0 || maxLevels > MAXP) {
    throw invalid_argument(L"maxLevels must be [1-" + to_wstring(MAXP) +
                           L"] but got " + to_wstring(maxLevels));
  }
}

int GeohashPrefixTree::getMaxLevelsPossible()
{
  return GeohashUtils::MAX_PRECISION;
}

shared_ptr<Cell> GeohashPrefixTree::getWorldCell()
{
  return make_shared<GhCell>(shared_from_this(), BytesRef::EMPTY_BYTES, 0, 0);
}

int GeohashPrefixTree::getLevelForDistance(double dist)
{
  if (dist == 0) {
    return maxLevels; // short circuit
  }
  constexpr int level = GeohashUtils::lookupHashLenForWidthHeight(dist, dist);
  return max(min(level, maxLevels), 1);
}

shared_ptr<Cell> GeohashPrefixTree::getCell(shared_ptr<Point> p, int level)
{
  return make_shared<GhCell>(
      shared_from_this(),
      GeohashUtils::encodeLatLon(p->getY(), p->getX(),
                                 level)); // args are lat,lon (y,x)
}

std::deque<char> GeohashPrefixTree::stringToBytesPlus1(const wstring &token)
{
  // copy ASCII token to byte array with one extra spot for eventual LEAF_BYTE
  // if needed
  std::deque<char> bytes(token.length() + 1);
  for (int i = 0; i < token.length(); i++) {
    bytes[i] = static_cast<char>(token[i]);
  }
  return bytes;
}

GeohashPrefixTree::GhCell::GhCell(shared_ptr<GeohashPrefixTree> outerInstance,
                                  const wstring &geohash)
    : LegacyCell(stringToBytesPlus1(geohash), 0, geohash.length()),
      outerInstance(outerInstance)
{
  this->geohash = geohash;
  if (isLeaf() &&
      getLevel() <
          getMaxLevels()) // we don't have a leaf byte at max levels (an opt)
  {
    this->geohash = geohash.substr(0, geohash.length() - 1);
  }
}

GeohashPrefixTree::GhCell::GhCell(shared_ptr<GeohashPrefixTree> outerInstance,
                                  std::deque<char> &bytes, int off, int len)
    : LegacyCell(bytes, off, len), outerInstance(outerInstance)
{
}

shared_ptr<GeohashPrefixTree> GeohashPrefixTree::GhCell::getGrid()
{
  return outerInstance;
}

int GeohashPrefixTree::GhCell::getMaxLevels()
{
  return outerInstance->maxLevels;
}

void GeohashPrefixTree::GhCell::readCell(shared_ptr<BytesRef> bytesRef)
{
  LegacyCell::readCell(bytesRef);
  geohash = L"";
}

shared_ptr<deque<std::shared_ptr<Cell>>>
GeohashPrefixTree::GhCell::getSubCells()
{
  std::deque<wstring> hashes =
      GeohashUtils::getSubGeohashes(getGeohash()); // sorted
  deque<std::shared_ptr<Cell>> cells =
      deque<std::shared_ptr<Cell>>(hashes.size());
  for (auto hash : hashes) {
    cells.push_back(make_shared<GhCell>(outerInstance, hash));
  }
  return cells;
}

int GeohashPrefixTree::GhCell::getSubCellsSize()
{
  return 32; // 8x4
}

shared_ptr<GhCell> GeohashPrefixTree::GhCell::getSubCell(shared_ptr<Point> p)
{
  return std::static_pointer_cast<GhCell>(
      getGrid()->getCell(p, getLevel() + 1)); // not performant!
}

shared_ptr<Shape> GeohashPrefixTree::GhCell::getShape()
{
  if (shape == nullptr) {
    shape = GeohashUtils::decodeBoundary(getGeohash(),
                                         getGrid()->getSpatialContext());
  }
  return shape;
}

wstring GeohashPrefixTree::GhCell::getGeohash()
{
  if (geohash == L"") {
    geohash = getTokenBytesNoLeaf(nullptr)->utf8ToString();
  }
  return geohash;
}
} // namespace org::apache::lucene::spatial::prefix::tree