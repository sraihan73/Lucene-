using namespace std;

#include "NumberRangePrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using org::locationtech::spatial4j::shape::impl::RectangleImpl;
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;
const shared_ptr<org::locationtech::spatial4j::context::SpatialContext>
    NumberRangePrefixTree::DUMMY_CTX;

NumberRangePrefixTree::StaticConstructor::StaticConstructor()
{
  shared_ptr<SpatialContextFactory> factory =
      make_shared<SpatialContextFactory>();
  factory->geo = false;
  factory->worldBounds = make_shared<RectangleImpl>(
      -numeric_limits<double>::infinity(), numeric_limits<double>::infinity(),
      0LL, 0LL, nullptr);
  DUMMY_CTX = factory->newSpatialContext();
}

NumberRangePrefixTree::StaticConstructor
    NumberRangePrefixTree::staticConstructor;

shared_ptr<NRShape>
NumberRangePrefixTree::toRangeShape(shared_ptr<UnitNRShape> startUnit,
                                    shared_ptr<UnitNRShape> endUnit)
{
  // note: this normalization/optimization process is actually REQUIRED based on
  // assumptions elsewhere. Normalize start & end
  startUnit = startUnit->getShapeAtLevel(
      truncateStartVals(startUnit, 0)); // chops off trailing min-vals (zeroes)
  endUnit = endUnit->getShapeAtLevel(
      truncateEndVals(endUnit, 0)); // chops off trailing max-vals
  // Optimize to just start or end if it's equivalent, e.g. April to April 1st
  // is April 1st.
  int cmp = comparePrefix(startUnit, endUnit);
  if (cmp > 0) {
    throw invalid_argument(L"Wrong order: " + startUnit + L" TO " + endUnit);
  }
  if (cmp == 0) { // one is a prefix of the other
    if (startUnit->getLevel() == endUnit->getLevel()) {
      // same
      return startUnit;
    } else if (endUnit->getLevel() > startUnit->getLevel()) {
      // e.g. April to April 1st
      if (truncateStartVals(endUnit, startUnit->getLevel()) ==
          startUnit->getLevel()) {
        return endUnit;
      }
    } else { // minLV level > maxLV level
      // e.g. April 30 to April
      if (truncateEndVals(startUnit, endUnit->getLevel()) ==
          endUnit->getLevel()) {
        return startUnit;
      }
    }
  }
  return make_shared<SpanUnitsNRShape>(shared_from_this(), startUnit, endUnit);
}

int NumberRangePrefixTree::truncateStartVals(shared_ptr<UnitNRShape> lv,
                                             int endLevel)
{
  for (int level = lv->getLevel(); level > endLevel; level--) {
    if (lv->getValAtLevel(level) != 0) {
      return level;
    }
  }
  return endLevel;
}

int NumberRangePrefixTree::truncateEndVals(shared_ptr<UnitNRShape> lv,
                                           int endLevel)
{
  for (int level = lv->getLevel(); level > endLevel; level--) {
    int max = getNumSubCells(lv->getShapeAtLevel(level - 1)) - 1;
    if (lv->getValAtLevel(level) != max) {
      return level;
    }
  }
  return endLevel;
}

wstring NumberRangePrefixTree::toStringUnitRaw(shared_ptr<UnitNRShape> lv)
{
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>(100);
  buf->append(L'[');
  for (int level = 1; level <= lv->getLevel(); level++) {
    buf->append(lv->getValAtLevel(level))->append(L',');
  }
  buf->setLength(buf->length() - 1); // chop off ','
  buf->append(L']');
  return buf->toString();
}

shared_ptr<NRShape>
NumberRangePrefixTree::parseShape(const wstring &str) 
{
  if (str == L"" || str.isEmpty()) {
    throw invalid_argument(L"str is null or blank");
  }
  if (str[0] == L'[') {
    if (str[str.length() - 1] != L']') {
      throw make_shared<ParseException>(
          L"If starts with [ must end with ]; got " + str, str.length() - 1);
    }
    int middle = (int)str.find(L" TO ");
    if (middle < 0) {
      throw make_shared<ParseException>(
          L"If starts with [ must contain ' TO '; got " + str, -1);
    }
    wstring leftStr = str.substr(1, middle - 1);
    wstring rightStr =
        str.substr(middle + (wstring(L" TO ")).length(),
                   (str.length() - 1) - (middle + (wstring(L" TO ")).length()));
    return toRangeShape(parseUnitShape(leftStr), parseUnitShape(rightStr));
  } else if (str[0] == L'{') {
    throw make_shared<ParseException>(
        L"Exclusive ranges not supported; got " + str, 0);
  } else {
    return parseUnitShape(str);
  }
}

int NumberRangePrefixTree::comparePrefix(shared_ptr<UnitNRShape> a,
                                         shared_ptr<UnitNRShape> b)
{
  int minLevel = min(a->getLevel(), b->getLevel());
  for (int level = 1; level <= minLevel; level++) {
    int diff = a->getValAtLevel(level) - b->getValAtLevel(level);
    if (diff != 0) {
      return diff;
    }
  }
  return 0;
}

NumberRangePrefixTree::SpanUnitsNRShape::SpanUnitsNRShape(
    shared_ptr<NumberRangePrefixTree> outerInstance,
    shared_ptr<UnitNRShape> minLV, shared_ptr<UnitNRShape> maxLV)
    : minLV(minLV), maxLV(maxLV), lastLevelInCommon(level - 1),
      outerInstance(outerInstance)
{

  // calc lastLevelInCommon
  int level = 1;
  for (; level <= minLV->getLevel() && level <= maxLV->getLevel(); level++) {
    if (minLV->getValAtLevel(level) != maxLV->getValAtLevel(level)) {
      break;
    }
  }
}

shared_ptr<SpatialContext> NumberRangePrefixTree::SpanUnitsNRShape::getContext()
{
  return DUMMY_CTX;
}

shared_ptr<UnitNRShape> NumberRangePrefixTree::SpanUnitsNRShape::getMinUnit()
{
  return minLV;
}

shared_ptr<UnitNRShape> NumberRangePrefixTree::SpanUnitsNRShape::getMaxUnit()
{
  return maxLV;
}

int NumberRangePrefixTree::SpanUnitsNRShape::getLevelsInCommon()
{
  return lastLevelInCommon;
}

shared_ptr<NRShape>
NumberRangePrefixTree::SpanUnitsNRShape::roundToLevel(int targetLevel)
{
  return outerInstance->toRangeShape(minLV->roundToLevel(targetLevel),
                                     maxLV->roundToLevel(targetLevel));
}

shared_ptr<SpatialRelation>
NumberRangePrefixTree::SpanUnitsNRShape::relate(shared_ptr<Shape> shape)
{
  //      if (shape instanceof UnitNRShape)
  //        return relate((UnitNRShape)shape);
  if (std::dynamic_pointer_cast<SpanUnitsNRShape>(shape) != nullptr) {
    return relate(std::static_pointer_cast<SpanUnitsNRShape>(shape));
  }
  return shape->relate(shared_from_this()).transpose(); // probably a
                                                        // UnitNRShape
}

shared_ptr<SpatialRelation> NumberRangePrefixTree::SpanUnitsNRShape::relate(
    shared_ptr<SpanUnitsNRShape> ext)
{
  // This logic somewhat mirrors RectangleImpl.relate_range()
  int extMin_intMax = comparePrefix(ext->getMinUnit(), getMaxUnit());
  if (extMin_intMax > 0) {
    return SpatialRelation::DISJOINT;
  }
  int extMax_intMin = comparePrefix(ext->getMaxUnit(), getMinUnit());
  if (extMax_intMin < 0) {
    return SpatialRelation::DISJOINT;
  }
  int extMin_intMin = comparePrefix(ext->getMinUnit(), getMinUnit());
  int extMax_intMax = comparePrefix(ext->getMaxUnit(), getMaxUnit());
  if ((extMin_intMin > 0 ||
       extMin_intMin == 0 &&
           ext->getMinUnit()->getLevel() >= getMinUnit()->getLevel()) &&
      (extMax_intMax < 0 ||
       extMax_intMax == 0 &&
           ext->getMaxUnit()->getLevel() >= getMaxUnit()->getLevel())) {
    return SpatialRelation::CONTAINS;
  }
  if ((extMin_intMin < 0 ||
       extMin_intMin == 0 &&
           ext->getMinUnit()->getLevel() <= getMinUnit()->getLevel()) &&
      (extMax_intMax > 0 ||
       extMax_intMax == 0 &&
           ext->getMaxUnit()->getLevel() <= getMaxUnit()->getLevel())) {
    return SpatialRelation::WITHIN;
  }
  return SpatialRelation::INTERSECTS;
}

shared_ptr<Rectangle> NumberRangePrefixTree::SpanUnitsNRShape::getBoundingBox()
{
  throw make_shared<UnsupportedOperationException>();
}

bool NumberRangePrefixTree::SpanUnitsNRShape::hasArea() { return true; }

double NumberRangePrefixTree::SpanUnitsNRShape::getArea(
    shared_ptr<SpatialContext> spatialContext)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Point> NumberRangePrefixTree::SpanUnitsNRShape::getCenter()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Shape> NumberRangePrefixTree::SpanUnitsNRShape::getBuffered(
    double v, shared_ptr<SpatialContext> spatialContext)
{
  throw make_shared<UnsupportedOperationException>();
}

bool NumberRangePrefixTree::SpanUnitsNRShape::isEmpty() { return false; }

shared_ptr<SpanUnitsNRShape> NumberRangePrefixTree::SpanUnitsNRShape::clone()
{
  return make_shared<SpanUnitsNRShape>(outerInstance, minLV->clone(),
                                       maxLV->clone());
}

wstring NumberRangePrefixTree::SpanUnitsNRShape::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"[" + outerInstance->toString(minLV) +
         L" TO "
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + outerInstance->toString(maxLV) + L"]";
}

bool NumberRangePrefixTree::SpanUnitsNRShape::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<SpanUnitsNRShape> spanShape =
      any_cast<std::shared_ptr<SpanUnitsNRShape>>(o);

  if (!maxLV->equals(spanShape->maxLV)) {
    return false;
  }
  if (!minLV->equals(spanShape->minLV)) {
    return false;
  }

  return true;
}

int NumberRangePrefixTree::SpanUnitsNRShape::hashCode()
{
  int result = minLV->hashCode();
  result = 31 * result + maxLV->hashCode();
  return result;
}

    NumberRangePrefixTree::NumberRangePrefixTree(std::deque<int> &maxSubCellsByLevel) : SpatialPrefixTree(DUMMY_CTX, maxSubCellsByLevel.length), maxSubCellsByLevel(maxSubCellsByLevel), termLenByLevel(std::deque<int>(maxLevels + 1)), levelByTermLen(std::deque<int>(maxTermLen)), maxTermLen(termLenByLevel[maxLevels] + 1) / * + 1 for leaf byte */
    {

      // Fill termLenByLevel
      termLenByLevel[0] = 0;
      constexpr int MAX_STATES = 1 << 15; // 1 bit less than 2 bytes
      for (int level = 1; level <= maxLevels; level++) {
        constexpr int states = maxSubCellsByLevel[level - 1];
        if (states >= MAX_STATES || states <= 1) {
          throw invalid_argument(L"Max states is " + to_wstring(MAX_STATES) +
                                 L", given " + to_wstring(states) +
                                 L" at level " + to_wstring(level));
        }
        bool twoBytes = states >= 256;
        termLenByLevel[level] = termLenByLevel[level - 1] + (twoBytes ? 2 : 1);
      }

      // Fill levelByTermLen
      levelByTermLen[0] = 0;
      for (int level = 1; level < termLenByLevel.size(); level++) {
        int termLen = termLenByLevel[level];
        int prevTermLen = termLenByLevel[level - 1];
        if (termLen - prevTermLen == 2) { // 2 byte delta
          // if the term doesn't completely cover this cell then it must be a
          // leaf of the prior.
          levelByTermLen[termLen - 1] = -1; // won't be used; otherwise
                                            // erroneous
          levelByTermLen[termLen] = level;
        } else { // 1 byte delta
          assert(termLen - prevTermLen == 1);
          levelByTermLen[termLen] = level;
        }
      }
    }

    wstring NumberRangePrefixTree::toString()
    {
      return getClass().getSimpleName();
    }

    int NumberRangePrefixTree::getLevelForDistance(double dist)
    {
      // note: it might be useful to compute which level has a raw width
      // (counted in
      // bottom units, e.g. milliseconds), that covers the provided dist in
      // those units?
      return maxLevels; // thus always use full precision. We don't do
                        // approximations in this tree/strategy.
      // throw new UnsupportedOperationException("Not applicable.");
    }

    double NumberRangePrefixTree::getDistanceForLevel(int level)
    {
      // note: we could compute this... should we?
      throw make_shared<UnsupportedOperationException>(L"Not applicable.");
    }

    shared_ptr<UnitNRShape>
    NumberRangePrefixTree::toShape(std::deque<int> &valStack, int len)
    {
      std::deque<std::shared_ptr<NRCell>> cellStack = newCellStack(len);
      for (int i = 0; i < len; i++) {
        cellStack[i + 1]->resetCellWithCellNum(valStack[i]);
      }
      return cellStack[len];
    }

    shared_ptr<Cell> NumberRangePrefixTree::getWorldCell()
    {
      return newCellStack(maxLevels)[0];
    }

    std::deque<std::shared_ptr<NRCell>>
    NumberRangePrefixTree::newCellStack(int levels)
    {
      std::deque<std::shared_ptr<NRCell>> cellsByLevel(levels + 1);
      shared_ptr<BytesRef> *const term = make_shared<BytesRef>(maxTermLen);
      for (int level = 0; level <= levels; level++) {
        cellsByLevel[level] =
            make_shared<NRCell>(shared_from_this(), cellsByLevel, term, level);
      }
      return cellsByLevel;
    }

    shared_ptr<Cell> NumberRangePrefixTree::readCell(shared_ptr<BytesRef> term,
                                                     shared_ptr<Cell> scratch)
    {
      if (scratch == nullptr) {
        scratch = getWorldCell();
      }

      // We decode level #, leaf bool, and populate bytes by reference. We
      // don't decode the stack.

      // reverse lookup term length to the level and hence the cell
      std::deque<std::shared_ptr<NRCell>> cellsByLevel =
          (std::static_pointer_cast<NRCell>(scratch))->cellsByLevel;
      bool isLeaf = term->bytes[term->offset + term->length - 1] == 0;
      int lenNoLeaf = isLeaf ? term->length - 1 : term->length;

      shared_ptr<NRCell> result = cellsByLevel[levelByTermLen[lenNoLeaf]];
      if (cellsByLevel[0]->termBuf.empty()) {
        cellsByLevel[0]->termBuf =
            result->term->bytes; // a kluge; see cell.ensureOwnTermBytes()
      }
      result->term->bytes = term->bytes;
      result->term->offset = term->offset;
      result->term->length =
          lenNoLeaf; // technically this isn't used but may help debugging
      result->reset();
      if (isLeaf) {
        result->setLeaf();
      }

      result->cellNumber = -1; // lazy decode flag

      return result;
    }

    int NumberRangePrefixTree::getNumSubCells(shared_ptr<UnitNRShape> lv)
    {
      return maxSubCellsByLevel[lv->getLevel()];
    }

    NumberRangePrefixTree::NRCell::NRCell(
        shared_ptr<NumberRangePrefixTree> outerInstance,
        std::deque<std::shared_ptr<NRCell>> &cellsByLevel,
        shared_ptr<BytesRef> term, int cellLevel)
        : cellsByLevel(cellsByLevel), term(term), cellLevel(cellLevel),
          outerInstance(outerInstance)
    {
      this->cellNumber = cellLevel == 0 ? 0 : -1;
      this->cellIsLeaf = false;
      assert(cellsByLevel[cellLevel] == nullptr);
    }

    void NumberRangePrefixTree::NRCell::ensureOwnTermBytes()
    {
      shared_ptr<NRCell> cell0 = cellsByLevel[0];
      if (cell0->termBuf.empty()) {
        return; // we already own the bytes
      }
      System::arraycopy(term->bytes, term->offset, cell0->termBuf, 0,
                        term->length);
      term->bytes = cell0->termBuf;
      term->offset = 0;
      cell0->termBuf.clear();
    }

    void NumberRangePrefixTree::NRCell::reset()
    {
      this->cellIsLeaf = false;
      this->cellShapeRel.reset();
    }

    void NumberRangePrefixTree::NRCell::resetCellWithCellNum(int cellNumber)
    {
      reset();

      // update bytes
      //  note: see lazyInitCellNumsFromBytes() for the reverse
      if (cellNumber >= 0) { // valid
        ensureOwnTermBytes();
        int termLen = outerInstance->termLenByLevel[getLevel()];
        bool twoBytes =
            (termLen - outerInstance->termLenByLevel[getLevel() - 1]) > 1;
        if (twoBytes) {
          // right 7 bits, plus 1 (may overflow to 8th bit which is okay)
          term->bytes[termLen - 2] = static_cast<char>(cellNumber >> 7);
          term->bytes[termLen - 1] = static_cast<char>((cellNumber & 0x7F) + 1);
        } else {
          term->bytes[termLen - 1] = static_cast<char>(cellNumber + 1);
        }
        assert(term->bytes[termLen - 1] != 0);
        term->length = termLen;
      }
      this->cellNumber = cellNumber;
    }

    void NumberRangePrefixTree::NRCell::ensureDecoded()
    {
      if (cellNumber >= 0) {
        return;
      }
      // Decode cell numbers from bytes. This is the inverse of
      // resetCellWithCellNum().
      for (int level = 1; level <= getLevel(); level++) {
        shared_ptr<NRCell> cell = cellsByLevel[level];
        int termLen = outerInstance->termLenByLevel[level];
        bool twoBytes =
            (termLen - outerInstance->termLenByLevel[level - 1]) > 1;
        if (twoBytes) {
          int byteH = (term->bytes[term->offset + termLen - 2] & 0xFF);
          int byteL = (term->bytes[term->offset + termLen - 1] & 0xFF);
          assert(byteL - 1 < (1 << 7));
          cell->cellNumber = (byteH << 7) + (byteL - 1);
          assert(cell->cellNumber < 1 << 15);
        } else {
          cell->cellNumber =
              (term->bytes[term->offset + termLen - 1] & 0xFF) - 1;
          assert(cell->cellNumber < 255);
        }
        cell->assertDecoded();
      }
    }

    void NumberRangePrefixTree::NRCell::assertDecoded()
    {
      assert(
          (cellNumber >= 0, L"Illegal state; ensureDecoded() wasn't called"));
    }

    int NumberRangePrefixTree::NRCell::getLevel() { return cellLevel; }

    shared_ptr<SpatialRelation> NumberRangePrefixTree::NRCell::getShapeRel()
    {
      return cellShapeRel;
    }

    void
    NumberRangePrefixTree::NRCell::setShapeRel(shared_ptr<SpatialRelation> rel)
    {
      cellShapeRel = rel;
    }

    bool NumberRangePrefixTree::NRCell::isLeaf() { return cellIsLeaf; }

    void NumberRangePrefixTree::NRCell::setLeaf() { cellIsLeaf = true; }

    shared_ptr<UnitNRShape> NumberRangePrefixTree::NRCell::getShape()
    {
      ensureDecoded();
      return shared_from_this();
    }

    shared_ptr<BytesRef> NumberRangePrefixTree::NRCell::getTokenBytesNoLeaf(
        shared_ptr<BytesRef> result)
    {
      if (result == nullptr) {
        result = make_shared<BytesRef>();
      }
      result->bytes = term->bytes;
      result->offset = term->offset;
      result->length = outerInstance->termLenByLevel[cellLevel];
      assert(result->length <= term->length);
      return result;
    }

    shared_ptr<BytesRef> NumberRangePrefixTree::NRCell::getTokenBytesWithLeaf(
        shared_ptr<BytesRef> result)
    {
      ensureOwnTermBytes(); // normally shouldn't do anything
      result = getTokenBytesNoLeaf(result);
      if (isLeaf()) {
        result->bytes[result->length++] = 0;
      }
      return result;
    }

    bool NumberRangePrefixTree::NRCell::isPrefixOf(shared_ptr<Cell> c)
    {
      shared_ptr<NRCell> otherCell = std::static_pointer_cast<NRCell>(c);
      assert(term != otherCell->term);
      // trick to re-use bytesref; provided that we re-instate it
      int myLastLen = term->length;
      term->length = outerInstance->termLenByLevel[getLevel()];
      int otherLastLen = otherCell->term->length;
      otherCell->term->length =
          outerInstance->termLenByLevel[otherCell->getLevel()];
      bool answer = StringHelper::startsWith(otherCell->term, term);
      term->length = myLastLen;
      otherCell->term->length = otherLastLen;
      return answer;
    }

    int
    NumberRangePrefixTree::NRCell::compareToNoLeaf(shared_ptr<Cell> fromCell)
    {
      shared_ptr<NRCell> *const nrCell =
          std::static_pointer_cast<NRCell>(fromCell);
      assert(term != nrCell->term);
      // trick to re-use bytesref; provided that we re-instate it
      int myLastLen = term->length;
      int otherLastLen = nrCell->term->length;
      term->length = outerInstance->termLenByLevel[getLevel()];
      nrCell->term->length = outerInstance->termLenByLevel[nrCell->getLevel()];
      int answer = term->compareTo(nrCell->term);
      term->length = myLastLen;
      nrCell->term->length = otherLastLen;
      return answer;
    }

    shared_ptr<CellIterator> NumberRangePrefixTree::NRCell::getNextLevelCells(
        shared_ptr<Shape> shapeFilter)
    {
      ensureDecoded();
      shared_ptr<NRCell> subCell = cellsByLevel[cellLevel + 1];
      subCell->initIter(shapeFilter);
      return subCell;
    }

    void NumberRangePrefixTree::NRCell::initIter(shared_ptr<Shape> filter)
    {
      cellNumber = -1;
      if (std::dynamic_pointer_cast<UnitNRShape>(filter) != nullptr &&
          (std::static_pointer_cast<UnitNRShape>(filter))->getLevel() == 0) {
        filter.reset(); // world means everything -- no filter
      }
      iterFilter = filter;

      shared_ptr<NRCell> parent = getShapeAtLevel(getLevel() - 1);

      // Initialize iter* members.

      // no filter means all subcells
      if (filter == nullptr) {
        iterFirstCellNumber = 0;
        iterFirstIsIntersects = false;
        iterLastCellNumber = outerInstance->getNumSubCells(parent) - 1;
        iterLastIsIntersects = false;
        return;
      }

      shared_ptr<UnitNRShape> *const minLV;
      shared_ptr<UnitNRShape> *const maxLV;
      constexpr int lastLevelInCommon; // between minLV & maxLV
      if (std::dynamic_pointer_cast<SpanUnitsNRShape>(filter) != nullptr) {
        shared_ptr<SpanUnitsNRShape> spanShape =
            std::static_pointer_cast<SpanUnitsNRShape>(iterFilter);
        minLV = spanShape->getMinUnit();
        maxLV = spanShape->getMaxUnit();
        lastLevelInCommon = spanShape->getLevelsInCommon();
      } else {
        minLV = std::static_pointer_cast<UnitNRShape>(iterFilter);
        maxLV = minLV;
        lastLevelInCommon = minLV->getLevel();
      }

      // fast path optimization that is usually true, but never first level
      if (iterFilter == parent->iterFilter &&
          (getLevel() <= lastLevelInCommon ||
           parent->iterFirstCellNumber != parent->iterLastCellNumber)) {
        // TODO benchmark if this optimization pays off. We avoid two
        // comparePrefixLV calls.
        if (parent->iterFirstIsIntersects &&
            parent->cellNumber == parent->iterFirstCellNumber &&
            minLV->getLevel() >= getLevel()) {
          iterFirstCellNumber = minLV->getValAtLevel(getLevel());
          iterFirstIsIntersects = (minLV->getLevel() > getLevel());
        } else {
          iterFirstCellNumber = 0;
          iterFirstIsIntersects = false;
        }
        if (parent->iterLastIsIntersects &&
            parent->cellNumber == parent->iterLastCellNumber &&
            maxLV->getLevel() >= getLevel()) {
          iterLastCellNumber = maxLV->getValAtLevel(getLevel());
          iterLastIsIntersects = (maxLV->getLevel() > getLevel());
        } else {
          iterLastCellNumber = outerInstance->getNumSubCells(parent) - 1;
          iterLastIsIntersects = false;
        }
        if (iterFirstCellNumber == iterLastCellNumber) {
          if (iterLastIsIntersects) {
            iterFirstIsIntersects = true;
          } else if (iterFirstIsIntersects) {
            iterLastIsIntersects = true;
          }
        }
        return;
      }

      // not common to get here, except for level 1 which always happens

      int startCmp = comparePrefix(minLV, parent);
      if (startCmp > 0) { // start comes after this cell
        iterFirstCellNumber = 0;
        iterFirstIsIntersects = false;
        iterLastCellNumber = -1; // so ends early (no cells)
        iterLastIsIntersects = false;
        return;
      }
      int endCmp = comparePrefix(maxLV, parent); // compare to end cell
      if (endCmp < 0) {                          // end comes before this cell
        iterFirstCellNumber = 0;
        iterFirstIsIntersects = false;
        iterLastCellNumber = -1; // so ends early (no cells)
        iterLastIsIntersects = false;
        return;
      }
      if (startCmp < 0 || minLV->getLevel() < getLevel()) {
        // start comes before...
        iterFirstCellNumber = 0;
        iterFirstIsIntersects = false;
      } else {
        iterFirstCellNumber = minLV->getValAtLevel(getLevel());
        iterFirstIsIntersects = (minLV->getLevel() > getLevel());
      }
      if (endCmp > 0 || maxLV->getLevel() < getLevel()) {
        // end comes after...
        iterLastCellNumber = outerInstance->getNumSubCells(parent) - 1;
        iterLastIsIntersects = false;
      } else {
        iterLastCellNumber = maxLV->getValAtLevel(getLevel());
        iterLastIsIntersects = (maxLV->getLevel() > getLevel());
      }
      if (iterFirstCellNumber == iterLastCellNumber) {
        if (iterLastIsIntersects) {
          iterFirstIsIntersects = true;
        } else if (iterFirstIsIntersects) {
          iterLastIsIntersects = true;
        }
      }
    }

    bool NumberRangePrefixTree::NRCell::hasNext()
    {
      thisCell_.reset();
      if (nextCell != nullptr) // calling hasNext twice in a row
      {
        return true;
      }

      if (cellNumber >= iterLastCellNumber) {
        return false;
      }

      resetCellWithCellNum(cellNumber < iterFirstCellNumber
                               ? iterFirstCellNumber
                               : cellNumber + 1);

      bool hasChildren =
          (cellNumber == iterFirstCellNumber && iterFirstIsIntersects) ||
          (cellNumber == iterLastCellNumber && iterLastIsIntersects);

      if (!hasChildren) {
        setLeaf();
        setShapeRel(SpatialRelation::WITHIN);
      } else if (iterFirstCellNumber == iterLastCellNumber) {
        setShapeRel(SpatialRelation::CONTAINS);
      } else {
        setShapeRel(SpatialRelation::INTERSECTS);
      }

      nextCell = shared_from_this();
      return true;
    }

    int NumberRangePrefixTree::NRCell::getValAtLevel(int level)
    {
      constexpr int result = cellsByLevel[level]->cellNumber;
      assert(result >= 0); // initialized (decoded)
      return result;
    }

    shared_ptr<NRCell> NumberRangePrefixTree::NRCell::getShapeAtLevel(int level)
    {
      assert(level <= cellLevel);
      return cellsByLevel[level];
    }

    shared_ptr<UnitNRShape>
    NumberRangePrefixTree::NRCell::roundToLevel(int targetLevel)
    {
      if (getLevel() <= targetLevel) {
        return shared_from_this();
      } else {
        return getShapeAtLevel(targetLevel);
      }
    }

    shared_ptr<SpatialRelation>
    NumberRangePrefixTree::NRCell::relate(shared_ptr<Shape> shape)
    {
      assertDecoded();
      if (shape == iterFilter && cellShapeRel != nullptr) {
        return cellShapeRel;
      }
      if (std::dynamic_pointer_cast<UnitNRShape>(shape) != nullptr) {
        return relate(std::static_pointer_cast<UnitNRShape>(shape));
      }
      if (std::dynamic_pointer_cast<SpanUnitsNRShape>(shape) != nullptr) {
        return relate(std::static_pointer_cast<SpanUnitsNRShape>(shape));
      }
      return shape->relate(shared_from_this()).transpose();
    }

    shared_ptr<SpatialRelation>
    NumberRangePrefixTree::NRCell::relate(shared_ptr<UnitNRShape> lv)
    {
      assertDecoded();
      int cmp = comparePrefix(shared_from_this(), lv);
      if (cmp != 0) {
        return SpatialRelation::DISJOINT;
      }
      if (getLevel() > lv->getLevel()) {
        return SpatialRelation::WITHIN;
      }
      return SpatialRelation::CONTAINS; // or equals
      // no INTERSECTS; that won't happen.
    }

    shared_ptr<SpatialRelation> NumberRangePrefixTree::NRCell::relate(
        shared_ptr<SpanUnitsNRShape> spanShape)
    {
      assertDecoded();
      int startCmp = comparePrefix(spanShape->getMinUnit(), shared_from_this());
      if (startCmp > 0) { // start comes after this cell
        return SpatialRelation::DISJOINT;
      }
      int endCmp = comparePrefix(spanShape->getMaxUnit(), shared_from_this());
      if (endCmp < 0) { // end comes before this cell
        return SpatialRelation::DISJOINT;
      }
      int nrMinLevel = spanShape->getMinUnit()->getLevel();
      int nrMaxLevel = spanShape->getMaxUnit()->getLevel();
      if ((startCmp < 0 || startCmp == 0 && nrMinLevel <= getLevel()) &&
          (endCmp > 0 || endCmp == 0 && nrMaxLevel <= getLevel())) {
        return SpatialRelation::WITHIN; // or equals
      }
      // At this point it's Contains or Within.
      if (startCmp != 0 || endCmp != 0) {
        return SpatialRelation::INTERSECTS;
      }
      // if min or max Level is less, it might be on the equivalent edge.
      for (; nrMinLevel < getLevel(); nrMinLevel++) {
        if (getValAtLevel(nrMinLevel + 1) != 0) {
          return SpatialRelation::INTERSECTS;
        }
      }
      for (; nrMaxLevel < getLevel(); nrMaxLevel++) {
        if (getValAtLevel(nrMaxLevel + 1) !=
            outerInstance->getNumSubCells(getShapeAtLevel(nrMaxLevel)) - 1) {
          return SpatialRelation::INTERSECTS;
        }
      }
      return SpatialRelation::CONTAINS;
    }

    shared_ptr<UnitNRShape> NumberRangePrefixTree::NRCell::clone()
    {
      // no leaf distinction; this is purely based on UnitNRShape
      shared_ptr<NRCell> cell = std::static_pointer_cast<NRCell>(
          outerInstance->readCell(getTokenBytesNoLeaf(nullptr), nullptr));
      cell->ensureOwnTermBytes();
      return cell->getShape();
    }

    int NumberRangePrefixTree::NRCell::compareTo(shared_ptr<UnitNRShape> o)
    {
      assertDecoded();
      // no leaf distinction; this is purely based on UnitNRShape
      int cmp = comparePrefix(shared_from_this(), o);
      if (cmp != 0) {
        return cmp;
      } else {
        return getLevel() - o->getLevel();
      }
    }

    shared_ptr<Rectangle> NumberRangePrefixTree::NRCell::getBoundingBox()
    {
      throw make_shared<UnsupportedOperationException>();
    }

    bool NumberRangePrefixTree::NRCell::hasArea() { return true; }

    double
    NumberRangePrefixTree::NRCell::getArea(shared_ptr<SpatialContext> ctx)
    {
      throw make_shared<UnsupportedOperationException>();
    }

    shared_ptr<Point> NumberRangePrefixTree::NRCell::getCenter()
    {
      throw make_shared<UnsupportedOperationException>();
    }

    shared_ptr<Shape>
    NumberRangePrefixTree::NRCell::getBuffered(double distance,
                                               shared_ptr<SpatialContext> ctx)
    {
      throw make_shared<UnsupportedOperationException>();
    }

    bool NumberRangePrefixTree::NRCell::isEmpty() { return false; }

    bool NumberRangePrefixTree::NRCell::equals(any obj)
    {
      if (!(std::dynamic_pointer_cast<NRCell>(obj) != nullptr)) {
        return false;
      }
      if (shared_from_this() == obj) {
        return true;
      }
      shared_ptr<NRCell> nrCell = any_cast<std::shared_ptr<NRCell>>(obj);
      assert(term != nrCell->term);
      if (getLevel() != nrCell->getLevel()) {
        return false;
      }
      // trick to re-use bytesref; provided that we re-instate it
      int myLastLen = term->length;
      int otherLastLen = nrCell->term->length;
      bool answer = getTokenBytesNoLeaf(term)->equals(
          nrCell->getTokenBytesNoLeaf(nrCell->term));
      term->length = myLastLen;
      nrCell->term->length = otherLastLen;
      return answer;
    }

    shared_ptr<SpatialContext> NumberRangePrefixTree::NRCell::getContext()
    {
      return DUMMY_CTX;
    }

    int NumberRangePrefixTree::NRCell::hashCode()
    {
      // trick to re-use bytesref; provided that we re-instate it
      int myLastLen = term->length;
      int result = getTokenBytesNoLeaf(term)->hashCode();
      term->length = myLastLen;
      return result;
    }

    wstring NumberRangePrefixTree::NRCell::toString()
    {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return outerInstance->toString(getShape());
    }

    wstring NumberRangePrefixTree::NRCell::toStringDebug()
    {
      wstring pretty = toString();
      if (getLevel() == 0) {
        return pretty;
      }
      return toStringUnitRaw(shared_from_this()) + (isLeaf() ? L"•" : L"") +
             L" " + pretty;
    }
    } // namespace org::apache::lucene::spatial::prefix::tree