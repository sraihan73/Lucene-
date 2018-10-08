using namespace std;

#include "PackedQuadPrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using org::locationtech::spatial4j::shape::impl::RectangleImpl;
std::deque<char> const PackedQuadPrefixTree::QUAD =
    std::deque<char>{0x00, 0x01, 0x02, 0x03};

shared_ptr<SpatialPrefixTree> PackedQuadPrefixTree::Factory::newSPT()
{
  return make_shared<PackedQuadPrefixTree>(outerInstance->ctx,
                                           outerInstance->maxLevels != nullptr
                                               ? outerInstance->maxLevels
                                               : MAX_LEVELS_POSSIBLE);
}

PackedQuadPrefixTree::PackedQuadPrefixTree(shared_ptr<SpatialContext> ctx,
                                           int maxLevels)
    : QuadPrefixTree(ctx, maxLevels)
{
  if (maxLevels > MAX_LEVELS_POSSIBLE) {
    throw invalid_argument(L"maxLevels of " + to_wstring(maxLevels) +
                           L" exceeds limit of " +
                           to_wstring(MAX_LEVELS_POSSIBLE));
  }
}

wstring PackedQuadPrefixTree::toString()
{
  return getClass().getSimpleName() + L"(maxLevels:" + to_wstring(maxLevels) +
         L",ctx:" + ctx + L",prune:" + StringHelper::toString(leafyPrune) +
         L")";
}

shared_ptr<Cell> PackedQuadPrefixTree::getWorldCell()
{
  return make_shared<PackedQuadCell>(shared_from_this(), 0x0LL);
}

shared_ptr<Cell> PackedQuadPrefixTree::getCell(shared_ptr<Point> p, int level)
{
  deque<std::shared_ptr<Cell>> cells = deque<std::shared_ptr<Cell>>(1);
  build(xmid, ymid, 0, cells, 0x0LL, ctx->makePoint(p->getX(), p->getY()),
        level);
  return cells[0]; // note cells could be longer if p on edge
}

void PackedQuadPrefixTree::build(double x, double y, int level,
                                 deque<std::shared_ptr<Cell>> &matches,
                                 int64_t term, shared_ptr<Shape> shape,
                                 int maxLevel)
{
  double w = levelW[level] / 2;
  double h = levelH[level] / 2;

  // Z-Order
  // http://en.wikipedia.org/wiki/Z-order_%28curve%29
  checkBattenberg(QUAD[0], x - w, y + h, level, matches, term, shape, maxLevel);
  checkBattenberg(QUAD[1], x + w, y + h, level, matches, term, shape, maxLevel);
  checkBattenberg(QUAD[2], x - w, y - h, level, matches, term, shape, maxLevel);
  checkBattenberg(QUAD[3], x + w, y - h, level, matches, term, shape, maxLevel);
}

void PackedQuadPrefixTree::checkBattenberg(
    char quad, double cx, double cy, int level,
    deque<std::shared_ptr<Cell>> &matches, int64_t term,
    shared_ptr<Shape> shape, int maxLevel)
{
  // short-circuit if we find a match for the point (no need to continue
  // recursion)
  if (std::dynamic_pointer_cast<Point>(shape) != nullptr && !matches.empty()) {
    return;
  }
  double w = levelW[level] / 2;
  double h = levelH[level] / 2;

  shared_ptr<SpatialRelation> v =
      shape->relate(ctx->makeRectangle(cx - w, cx + w, cy - h, cy + h));

  if (SpatialRelation->DISJOINT == v) {
    return;
  }

  // set bits for next level
  term |= ((static_cast<int64_t>(quad)) << (64 - (++level << 1)));
  // increment level
  term =
      ((static_cast<int64_t>(static_cast<uint64_t>(term) >> 1)) + 1)
      << 1;

  if (SpatialRelation->CONTAINS == v || (level >= maxLevel)) {
    matches.push_back(
        make_shared<PackedQuadCell>(shared_from_this(), term, v->transpose()));
  } else { // SpatialRelation.WITHIN, SpatialRelation.INTERSECTS
    build(cx, cy, level, matches, term, shape, maxLevel);
  }
}

shared_ptr<Cell> PackedQuadPrefixTree::readCell(shared_ptr<BytesRef> term,
                                                shared_ptr<Cell> scratch)
{
  shared_ptr<PackedQuadCell> cell =
      std::static_pointer_cast<PackedQuadCell>(scratch);
  if (cell == nullptr) {
    cell = std::static_pointer_cast<PackedQuadCell>(getWorldCell());
  }
  cell->readCell(term);
  return cell;
}

shared_ptr<CellIterator>
PackedQuadPrefixTree::getTreeCellIterator(shared_ptr<Shape> shape,
                                          int detailLevel)
{
  if (detailLevel > maxLevels) {
    throw invalid_argument(L"detailLevel:" + to_wstring(detailLevel) +
                           L" exceed max: " + to_wstring(maxLevels));
  }
  return make_shared<PrefixTreeIterator>(shared_from_this(), shape,
                                         static_cast<short>(detailLevel));
}

bool PackedQuadPrefixTree::isPruneLeafyBranches() { return leafyPrune; }

void PackedQuadPrefixTree::setPruneLeafyBranches(bool pruneLeafyBranches)
{
  this->leafyPrune = pruneLeafyBranches;
}

PackedQuadPrefixTree::PackedQuadCell::PackedQuadCell(
    shared_ptr<PackedQuadPrefixTree> outerInstance, int64_t term)
    : QuadCell(outerInstance, nullptr, 0, 0), outerInstance(outerInstance)
{
  this->term = term;
  this->b_off = 0;
  this->bytes = longToByteArray(this->term, std::deque<char>(8));
  this->b_len = 8;
  readLeafAdjust();
}

PackedQuadPrefixTree::PackedQuadCell::PackedQuadCell(
    shared_ptr<PackedQuadPrefixTree> outerInstance, int64_t term,
    shared_ptr<SpatialRelation> shapeRel)
    : PackedQuadCell(outerInstance, term)
{
  this->shapeRel = shapeRel;
}

void PackedQuadPrefixTree::PackedQuadCell::readCell(shared_ptr<BytesRef> bytes)
{
  shapeRel.reset();
  shape.reset();
  this->bytes = bytes->bytes;
  this->b_off = bytes->offset;
  this->b_len = static_cast<short>(bytes->length);
  this->term = longFromByteArray(this->bytes, bytes->offset);
  readLeafAdjust();
}

int PackedQuadPrefixTree::PackedQuadCell::getShiftForLevel(int const level)
{
  return 64 - (level << 1);
}

bool PackedQuadPrefixTree::PackedQuadCell::isEnd(int const level,
                                                 int const shift)
{
  return (term != 0x0LL &&
          ((((0x1LL << (level << 1)) - 1) -
            (static_cast<int64_t>(static_cast<uint64_t>(term) >>
                                    shift))) == 0x0LL));
}

shared_ptr<PackedQuadCell>
PackedQuadPrefixTree::PackedQuadCell::nextCell(bool descend)
{
  constexpr int level = getLevel();
  constexpr int shift = getShiftForLevel(level);
  // base case: can't go further
  if ((!descend && isEnd(level, shift)) ||
      isEnd(outerInstance->maxLevels,
            getShiftForLevel(outerInstance->maxLevels))) {
    return nullptr;
  }
  int64_t newTerm;
  constexpr bool isLeaf = (term & 0x1LL) == 0x1LL;
  // if descend requested && we're not at the maxLevel
  if ((descend && !isLeaf && (level != outerInstance->maxLevels)) ||
      level == 0) {
    // simple case: increment level bits (next level)
    newTerm =
        ((static_cast<int64_t>(static_cast<uint64_t>(term) >> 1)) +
         0x1LL)
        << 1;
  } else { // we're not descending or we can't descend
    newTerm = term + (0x1LL << shift);
    // we're at the last sibling...force descend
    if (((static_cast<int64_t>(static_cast<uint64_t>(term) >>
                                 shift)) &
         0x3LL) == 0x3LL) {
      // adjust level for number popping up
      newTerm =
          ((static_cast<int64_t>(static_cast<uint64_t>(newTerm) >>
                                   1)) -
           (static_cast<int>(
               static_cast<unsigned int>(
                   Long::numberOfTrailingZeros(static_cast<int64_t>(
                       static_cast<uint64_t>(newTerm) >> shift))) >>
               1)))
          << 1;
    }
  }
  return make_shared<PackedQuadCell>(outerInstance, newTerm);
}

void PackedQuadPrefixTree::PackedQuadCell::readLeafAdjust()
{
  isLeaf_ = ((0x1LL) & term) == 0x1LL;
  if (getLevel() == getMaxLevels()) {
    isLeaf_ = true;
  }
}

shared_ptr<BytesRef>
PackedQuadPrefixTree::PackedQuadCell::getTokenBytesWithLeaf(
    shared_ptr<BytesRef> result)
{
  result = getTokenBytesNoLeaf(result);
  if (isLeaf()) {
    result->bytes[8 - 1] |= 0x1LL; // set leaf
  }
  return result;
}

shared_ptr<BytesRef> PackedQuadPrefixTree::PackedQuadCell::getTokenBytesNoLeaf(
    shared_ptr<BytesRef> result)
{
  if (result == nullptr) {
    result = make_shared<BytesRef>(8);
  } else if (result->bytes.size() < 8) {
    result->bytes = std::deque<char>(8);
  }
  result->bytes = longToByteArray(this->term, result->bytes);
  result->offset = 0;
  result->length = 8;
  // no leaf
  result->bytes[8 - 1] &= ~1; // clear last bit (leaf bit)
  return result;
}

int PackedQuadPrefixTree::PackedQuadCell::compareToNoLeaf(
    shared_ptr<Cell> fromCell)
{
  shared_ptr<PackedQuadCell> b =
      std::static_pointer_cast<PackedQuadCell>(fromCell);
  // TODO clear last bit without the condition
  constexpr int64_t thisTerm = (((0x1LL) & term) == 0x1LL) ? term - 1 : term;
  constexpr int64_t fromTerm =
      (((0x1LL) & b->term) == 0x1LL) ? b->term - 1 : b->term;
  constexpr int result = Long::compareUnsigned(thisTerm, fromTerm);
  assert(
      (Math::signum(result) ==
       Math::signum(compare(longToByteArray(thisTerm, std::deque<char>(8)), 0,
                            8, longToByteArray(fromTerm, std::deque<char>(8)),
                            0, 8)))); // TODO remove
  return result;
}

int PackedQuadPrefixTree::PackedQuadCell::getLevel()
{
  int l = static_cast<int>(
      (static_cast<int64_t>(static_cast<uint64_t>(term) >> 1)) &
      0x1FLL);
  return l;
}

shared_ptr<deque<std::shared_ptr<Cell>>>
PackedQuadPrefixTree::PackedQuadCell::getSubCells()
{
  deque<std::shared_ptr<Cell>> cells = deque<std::shared_ptr<Cell>>(4);
  shared_ptr<PackedQuadCell> pqc =
      (make_shared<PackedQuadCell>(
           outerInstance, ((term & 0x1) == 0x1) ? this->term - 1 : this->term))
          ->nextCell(true);
  cells.push_back(pqc);
  cells.push_back((pqc = pqc->nextCell(false)));
  cells.push_back((pqc = pqc->nextCell(false)));
  cells.push_back(pqc->nextCell(false));
  return cells;
}

shared_ptr<QuadCell>
PackedQuadPrefixTree::PackedQuadCell::getSubCell(shared_ptr<Point> p)
{
  return std::static_pointer_cast<PackedQuadCell>(
      outerInstance->getCell(p, getLevel() + 1)); // not performant!
}

bool PackedQuadPrefixTree::PackedQuadCell::isPrefixOf(shared_ptr<Cell> c)
{
  shared_ptr<PackedQuadCell> cell = std::static_pointer_cast<PackedQuadCell>(c);
  return (this->term == 0x0LL) || isInternalPrefix(cell);
}

bool PackedQuadPrefixTree::PackedQuadCell::isInternalPrefix(
    shared_ptr<PackedQuadCell> c)
{
  constexpr int shift = 64 - (getLevel() << 1);
  return (static_cast<static_cast<int64_t>(
              static_cast<uint64_t>(term) >> shift) *>(
             -(static_cast<int64_t>(
                 static_cast<uint64_t>(c->term) >> shift)))) == 0x0LL;
}

int64_t PackedQuadPrefixTree::PackedQuadCell::concat(char postfix)
{
  // extra leaf bit
  return this->term | ((static_cast<int64_t>(postfix))
                       << ((getMaxLevels() - getLevel() << 1) + 6));
}

shared_ptr<Rectangle> PackedQuadPrefixTree::PackedQuadCell::makeShape()
{
  double xmin = outerInstance->xmin;
  double ymin = outerInstance->ymin;
  int level = getLevel();

  char b;
  for (short l = 0, i = 1; l < level; ++l, ++i) {
    b = static_cast<char>(
        (static_cast<int64_t>(static_cast<uint64_t>(term) >>
                                (64 - (i << 1)))) &
        0x3LL);

    switch (b) {
    case 0x00:
      ymin += outerInstance->levelH[l];
      break;
    case 0x01:
      xmin += outerInstance->levelW[l];
      ymin += outerInstance->levelH[l];
      break;
    case 0x02:
      break; // nothing really
    case 0x03:
      xmin += outerInstance->levelW[l];
      break;
    default:
      throw runtime_error(L"unexpected quadrant");
    }
  }

  double width, height;
  if (level > 0) {
    width = outerInstance->levelW[level - 1];
    height = outerInstance->levelH[level - 1];
  } else {
    width = outerInstance->gridW;
    height = outerInstance->gridH;
  }
  return make_shared<RectangleImpl>(xmin, xmin + width, ymin, ymin + height,
                                    outerInstance->ctx);
}

int64_t PackedQuadPrefixTree::PackedQuadCell::fromBytes(char b1, char b2,
                                                          char b3, char b4,
                                                          char b5, char b6,
                                                          char b7, char b8)
{
  return (static_cast<int64_t>(b1) & 255LL) << 56 |
         (static_cast<int64_t>(b2) & 255LL) << 48 |
         (static_cast<int64_t>(b3) & 255LL) << 40 |
         (static_cast<int64_t>(b4) & 255LL) << 32 |
         (static_cast<int64_t>(b5) & 255LL) << 24 |
         (static_cast<int64_t>(b6) & 255LL) << 16 |
         (static_cast<int64_t>(b7) & 255LL) << 8 |
         static_cast<int64_t>(b8) & 255LL;
}

std::deque<char>
PackedQuadPrefixTree::PackedQuadCell::longToByteArray(int64_t value,
                                                      std::deque<char> &result)
{
  for (int i = 7; i >= 0; --i) {
    result[i] = static_cast<char>(static_cast<int>(value & 255LL));
    value >>= 8;
  }
  return result;
}

int64_t PackedQuadPrefixTree::PackedQuadCell::longFromByteArray(
    std::deque<char> &bytes, int ofs)
{
  assert(bytes.size() >= 8);
  return fromBytes(bytes[0 + ofs], bytes[1 + ofs], bytes[2 + ofs],
                   bytes[3 + ofs], bytes[4 + ofs], bytes[5 + ofs],
                   bytes[6 + ofs], bytes[7 + ofs]);
}

wstring PackedQuadPrefixTree::PackedQuadCell::toString()
{
  shared_ptr<StringBuilder> s = make_shared<StringBuilder>(64);
  constexpr int numberOfLeadingZeros = Long::numberOfLeadingZeros(term);
  for (int i = 0; i < numberOfLeadingZeros; i++) {
    s->append(L'0');
  }
  if (term != 0) {
    s->append(Long::toBinaryString(term));
  }
  return s->toString();
}

PackedQuadPrefixTree::PrefixTreeIterator::PrefixTreeIterator(
    shared_ptr<PackedQuadPrefixTree> outerInstance, shared_ptr<Shape> shape,
    short detailLevel)
    : detailLevel(detailLevel), outerInstance(outerInstance)
{
  this->shape = shape;
  this->thisCell_ =
      (std::static_pointer_cast<PackedQuadCell>(outerInstance->getWorldCell()))
          ->nextCell(true);
  this->nextCell.reset();
}

bool PackedQuadPrefixTree::PrefixTreeIterator::hasNext()
{
  if (nextCell != nullptr) {
    return true;
  }
  shared_ptr<SpatialRelation> rel;
  // loop until we're at the end of the quad tree or we hit a relation
  while (thisCell_ != nullptr) {
    rel = thisCell_->getShape()->relate(shape);
    if (rel == SpatialRelation::DISJOINT) {
      thisCell_ = thisCell_->nextCell(false);
    } else { // within || intersects || contains
      thisCell_->setShapeRel(rel);
      nextCell = thisCell_;
      if (rel == SpatialRelation::WITHIN) {
        thisCell_->setLeaf();
        thisCell_ = thisCell_->nextCell(false);
      } else { // intersects || contains
        level = static_cast<short>(thisCell_->getLevel());
        if (level == detailLevel || pruned(rel)) {
          thisCell_->setLeaf();
          if (std::dynamic_pointer_cast<Point>(shape) != nullptr) {
            thisCell_->setShapeRel(SpatialRelation::WITHIN);
            thisCell_.reset();
          } else {
            thisCell_ = thisCell_->nextCell(false);
          }
          break;
        }
        thisCell_ = thisCell_->nextCell(true);
      }
      break;
    }
  }
  return nextCell != nullptr;
}

bool PackedQuadPrefixTree::PrefixTreeIterator::pruned(
    shared_ptr<SpatialRelation> rel)
{
  int leaves;
  if (rel == SpatialRelation::INTERSECTS && outerInstance->leafyPrune &&
      level == detailLevel - 1) {
    for (leaves = 0, pruneIter = thisCell_->getNextLevelCells(shape);
         pruneIter->hasNext(); pruneIter->next(), ++leaves) {
      ;
    }
    return leaves == 4;
  }
  return false;
}

shared_ptr<Cell> PackedQuadPrefixTree::PrefixTreeIterator::next()
{
  if (nextCell == nullptr) {
    if (!hasNext()) {
      // C++ TODO: The following line could not be converted:
      throw java.util.NoSuchElementException();
    }
  }
  // overriding since this implementation sets thisCell in hasNext
  shared_ptr<Cell> temp = nextCell;
  nextCell.reset();
  return temp;
}

void PackedQuadPrefixTree::PrefixTreeIterator::remove()
{
  // no-op
}
} // namespace org::apache::lucene::spatial::prefix::tree