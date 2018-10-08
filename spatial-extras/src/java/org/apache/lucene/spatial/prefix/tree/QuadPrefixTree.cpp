using namespace std;

#include "QuadPrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using BytesRef = org::apache::lucene::util::BytesRef;

int QuadPrefixTree::Factory::getLevelForDistance(double degrees)
{
  shared_ptr<QuadPrefixTree> grid =
      make_shared<QuadPrefixTree>(ctx, MAX_LEVELS_POSSIBLE);
  return grid->getLevelForDistance(degrees);
}

shared_ptr<SpatialPrefixTree> QuadPrefixTree::Factory::newSPT()
{
  return make_shared<QuadPrefixTree>(ctx, maxLevels ? maxLevels
                                                    : MAX_LEVELS_POSSIBLE);
}

QuadPrefixTree::QuadPrefixTree(shared_ptr<SpatialContext> ctx,
                               shared_ptr<Rectangle> bounds, int maxLevels)
    : LegacyPrefixTree(ctx, maxLevels), xmin(bounds->getMinX()),
      xmax(bounds->getMaxX()), ymin(bounds->getMinY()), ymax(bounds->getMaxY()),
      xmid(xmin + gridW / 2.0), ymid(ymin + gridH / 2.0), gridW(xmax - xmin),
      gridH(ymax - ymin), levelW(std::deque<double>(maxLevels)),
      levelH(std::deque<double>(maxLevels)),
      levelS(std::deque<int>(maxLevels)), levelN(std::deque<int>(maxLevels))
{

  levelW[0] = gridW / 2.0;
  levelH[0] = gridH / 2.0;
  levelS[0] = 2;
  levelN[0] = 4;

  for (int i = 1; i < levelW.size(); i++) {
    levelW[i] = levelW[i - 1] / 2.0;
    levelH[i] = levelH[i - 1] / 2.0;
    levelS[i] = levelS[i - 1] * 2;
    levelN[i] = levelN[i - 1] * 4;
  }
}

QuadPrefixTree::QuadPrefixTree(shared_ptr<SpatialContext> ctx)
    : QuadPrefixTree(ctx, DEFAULT_MAX_LEVELS)
{
}

QuadPrefixTree::QuadPrefixTree(shared_ptr<SpatialContext> ctx, int maxLevels)
    : QuadPrefixTree(ctx, ctx->getWorldBounds(), maxLevels)
{
}

shared_ptr<Cell> QuadPrefixTree::getWorldCell()
{
  return make_shared<QuadCell>(shared_from_this(), BytesRef::EMPTY_BYTES, 0, 0);
}

void QuadPrefixTree::printInfo(shared_ptr<PrintStream> out)
{
  shared_ptr<NumberFormat> nf = NumberFormat::getNumberInstance(Locale::ROOT);
  nf->setMaximumFractionDigits(5);
  nf->setMinimumFractionDigits(5);
  nf->setMinimumIntegerDigits(3);

  for (int i = 0; i < maxLevels; i++) {
    out->println(to_wstring(i) + L"]\t" + nf->format(levelW[i]) + L"\t" +
                 nf->format(levelH[i]) + L"\t" + to_wstring(levelS[i]) + L"\t" +
                 to_wstring(levelS[i] * levelS[i]));
  }
}

int QuadPrefixTree::getLevelForDistance(double dist)
{
  if (dist == 0) // short circuit
  {
    return maxLevels;
  }
  for (int i = 0; i < maxLevels - 1; i++) {
    // note: level[i] is actually a lookup for level i+1
    if (dist > levelW[i] && dist > levelH[i]) {
      return i + 1;
    }
  }
  return maxLevels;
}

shared_ptr<Cell> QuadPrefixTree::getCell(shared_ptr<Point> p, int level)
{
  deque<std::shared_ptr<Cell>> cells = deque<std::shared_ptr<Cell>>(1);
  build(xmid, ymid, 0, cells, make_shared<BytesRef>(maxLevels + 1),
        ctx->makePoint(p->getX(), p->getY()), level);
  return cells[0]; // note cells could be longer if p on edge
}

void QuadPrefixTree::build(double x, double y, int level,
                           deque<std::shared_ptr<Cell>> &matches,
                           shared_ptr<BytesRef> str, shared_ptr<Shape> shape,
                           int maxLevel)
{
  assert(str->length == level);
  double w = levelW[level] / 2;
  double h = levelH[level] / 2;

  // Z-Order
  // http://en.wikipedia.org/wiki/Z-order_%28curve%29
  checkBattenberg(L'A', x - w, y + h, level, matches, str, shape, maxLevel);
  checkBattenberg(L'B', x + w, y + h, level, matches, str, shape, maxLevel);
  checkBattenberg(L'C', x - w, y - h, level, matches, str, shape, maxLevel);
  checkBattenberg(L'D', x + w, y - h, level, matches, str, shape, maxLevel);

  // possibly consider hilbert curve
  // http://en.wikipedia.org/wiki/Hilbert_curve
  // http://blog.notdot.net/2009/11/Damn-Cool-Algorithms-Spatial-indexing-with-Quadtrees-and-Hilbert-Curves
  // if we actually use the range property in the query, this could be useful
}

void QuadPrefixTree::checkBattenberg(wchar_t c, double cx, double cy, int level,
                                     deque<std::shared_ptr<Cell>> &matches,
                                     shared_ptr<BytesRef> str,
                                     shared_ptr<Shape> shape, int maxLevel)
{
  assert(str->length == level);
  assert(str->offset == 0);
  double w = levelW[level] / 2;
  double h = levelH[level] / 2;

  int strlen = str->length;
  shared_ptr<Rectangle> rectangle =
      ctx->makeRectangle(cx - w, cx + w, cy - h, cy + h);
  shared_ptr<SpatialRelation> v = shape->relate(rectangle);
  if (SpatialRelation->CONTAINS == v) {
    str->bytes[str->length++] = static_cast<char>(c); // append
    // str.append(SpatialPrefixGrid.COVER);
    matches.push_back(make_shared<QuadCell>(
        shared_from_this(), BytesRef::deepCopyOf(str), v->transpose()));
  } else if (SpatialRelation->DISJOINT == v) {
    // nothing
  } else { // SpatialRelation.WITHIN, SpatialRelation.INTERSECTS
    str->bytes[str->length++] = static_cast<char>(c); // append

    int nextLevel = level + 1;
    if (nextLevel >= maxLevel) {
      // str.append(SpatialPrefixGrid.INTERSECTS);
      matches.push_back(make_shared<QuadCell>(
          shared_from_this(), BytesRef::deepCopyOf(str), v->transpose()));
    } else {
      build(cx, cy, nextLevel, matches, str, shape, maxLevel);
    }
  }
  str->length = strlen;
}

QuadPrefixTree::QuadCell::QuadCell(shared_ptr<QuadPrefixTree> outerInstance,
                                   std::deque<char> &bytes, int off, int len)
    : LegacyCell(bytes, off, len), outerInstance(outerInstance)
{
}

QuadPrefixTree::QuadCell::QuadCell(shared_ptr<QuadPrefixTree> outerInstance,
                                   shared_ptr<BytesRef> str,
                                   shared_ptr<SpatialRelation> shapeRel)
    : QuadCell(outerInstance, str->bytes, str->offset, str->length)
{
  this->shapeRel = shapeRel;
}

shared_ptr<QuadPrefixTree> QuadPrefixTree::QuadCell::getGrid()
{
  return outerInstance;
}

int QuadPrefixTree::QuadCell::getMaxLevels()
{
  return outerInstance->maxLevels;
}

shared_ptr<deque<std::shared_ptr<Cell>>>
QuadPrefixTree::QuadCell::getSubCells()
{
  shared_ptr<BytesRef> source = getTokenBytesNoLeaf(nullptr);

  deque<std::shared_ptr<Cell>> cells = deque<std::shared_ptr<Cell>>(4);
  cells.push_back(make_shared<QuadCell>(
      outerInstance, concat(source, static_cast<char>(L'A')), nullptr));
  cells.push_back(make_shared<QuadCell>(
      outerInstance, concat(source, static_cast<char>(L'B')), nullptr));
  cells.push_back(make_shared<QuadCell>(
      outerInstance, concat(source, static_cast<char>(L'C')), nullptr));
  cells.push_back(make_shared<QuadCell>(
      outerInstance, concat(source, static_cast<char>(L'D')), nullptr));
  return cells;
}

shared_ptr<BytesRef>
QuadPrefixTree::QuadCell::concat(shared_ptr<BytesRef> source, char b)
{
  //+2 for new char + potential leaf
  const std::deque<char> buffer = Arrays::copyOfRange(
      source->bytes, source->offset, source->offset + source->length + 2);
  shared_ptr<BytesRef> target = make_shared<BytesRef>(buffer);
  target->length = source->length;
  target->bytes[target->length++] = b;
  return target;
}

int QuadPrefixTree::QuadCell::getSubCellsSize() { return 4; }

shared_ptr<QuadCell> QuadPrefixTree::QuadCell::getSubCell(shared_ptr<Point> p)
{
  return std::static_pointer_cast<QuadCell>(
      outerInstance->getCell(p, getLevel() + 1)); // not performant!
}

shared_ptr<Shape> QuadPrefixTree::QuadCell::getShape()
{
  if (shape == nullptr) {
    shape = makeShape();
  }
  return shape;
}

shared_ptr<Rectangle> QuadPrefixTree::QuadCell::makeShape()
{
  shared_ptr<BytesRef> token = getTokenBytesNoLeaf(nullptr);
  double xmin = outerInstance->xmin;
  double ymin = outerInstance->ymin;

  for (int i = 0; i < token->length; i++) {
    char c = token->bytes[token->offset + i];
    switch (c) {
    case L'A':
      ymin += outerInstance->levelH[i];
      break;
    case L'B':
      xmin += outerInstance->levelW[i];
      ymin += outerInstance->levelH[i];
      break;
    case L'C':
      break; // nothing really
    case L'D':
      xmin += outerInstance->levelW[i];
      break;
    default:
      throw runtime_error(L"unexpected char: " + to_wstring(c));
    }
  }
  int len = token->length;
  double width, height;
  if (len > 0) {
    width = outerInstance->levelW[len - 1];
    height = outerInstance->levelH[len - 1];
  } else {
    width = outerInstance->gridW;
    height = outerInstance->gridH;
  }
  return outerInstance->ctx->makeRectangle(xmin, xmin + width, ymin,
                                           ymin + height);
}
} // namespace org::apache::lucene::spatial::prefix::tree