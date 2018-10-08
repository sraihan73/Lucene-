using namespace std;

#include "LegacyCell.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;

LegacyCell::LegacyCell(std::deque<char> &bytes, int off, int len)
{
  this->bytes = bytes;
  this->b_off = off;
  this->b_len = len;
  readLeafAdjust();
}

void LegacyCell::readCell(shared_ptr<BytesRef> bytes)
{
  shapeRel.reset();
  shape.reset();
  this->bytes = bytes->bytes;
  this->b_off = bytes->offset;
  this->b_len = static_cast<short>(bytes->length);
  readLeafAdjust();
}

void LegacyCell::readLeafAdjust()
{
  isLeaf_ = (b_len > 0 && bytes[b_off + b_len - 1] == LEAF_BYTE);
  if (isLeaf_) {
    b_len--;
  }
  if (getLevel() == getMaxLevels()) {
    isLeaf_ = true;
  }
}

shared_ptr<SpatialRelation> LegacyCell::getShapeRel() { return shapeRel; }

void LegacyCell::setShapeRel(shared_ptr<SpatialRelation> rel)
{
  this->shapeRel = rel;
}

bool LegacyCell::isLeaf() { return isLeaf_; }

void LegacyCell::setLeaf() { isLeaf_ = true; }

shared_ptr<BytesRef>
LegacyCell::getTokenBytesWithLeaf(shared_ptr<BytesRef> result)
{
  result = getTokenBytesNoLeaf(result);
  if (!isLeaf_ || getLevel() == getMaxLevels()) {
    return result;
  }
  if (result->bytes.size() < result->offset + result->length + 1) {
    assert((false, L"Not supposed to happen; performance bug"));
    std::deque<char> copy(result->length + 1);
    System::arraycopy(result->bytes, result->offset, copy, 0,
                      result->length - 1);
    result->bytes = copy;
    result->offset = 0;
  }
  result->bytes[result->offset + result->length++] = LEAF_BYTE;
  return result;
}

shared_ptr<BytesRef>
LegacyCell::getTokenBytesNoLeaf(shared_ptr<BytesRef> result)
{
  if (result == nullptr) {
    return make_shared<BytesRef>(bytes, b_off, b_len);
  }
  result->bytes = bytes;
  result->offset = b_off;
  result->length = b_len;
  return result;
}

int LegacyCell::getLevel() { return b_len; }

shared_ptr<CellIterator>
LegacyCell::getNextLevelCells(shared_ptr<Shape> shapeFilter)
{
  assert(getLevel() < getGrid()->getMaxLevels());
  if (std::dynamic_pointer_cast<Point>(shapeFilter) != nullptr) {
    shared_ptr<LegacyCell> cell =
        getSubCell(std::static_pointer_cast<Point>(shapeFilter));
    cell->shapeRel = SpatialRelation::CONTAINS;
    return make_shared<SingletonCellIterator>(cell);
  } else {
    return make_shared<FilterCellIterator>(getSubCells()->begin(), shapeFilter);
  }
}

bool LegacyCell::isPrefixOf(shared_ptr<Cell> c)
{
  // Note: this only works when each level uses a whole number of bytes.
  shared_ptr<LegacyCell> cell = std::static_pointer_cast<LegacyCell>(c);
  bool result =
      sliceEquals(cell->bytes, cell->b_off, cell->b_len, bytes, b_off, b_len);
  assert((result == StringHelper::startsWith(c->getTokenBytesNoLeaf(nullptr),
                                             getTokenBytesNoLeaf(nullptr))));
  return result;
}

bool LegacyCell::sliceEquals(std::deque<char> &sliceToTest_bytes,
                             int sliceToTest_offset, int sliceToTest_length,
                             std::deque<char> &other_bytes, int other_offset,
                             int other_length)
{
  if (sliceToTest_length < other_length) {
    return false;
  }
  int i = sliceToTest_offset;
  int j = other_offset;
  constexpr int k = other_offset + other_length;

  while (j < k) {
    if (sliceToTest_bytes[i++] != other_bytes[j++]) {
      return false;
    }
  }

  return true;
}

int LegacyCell::compareToNoLeaf(shared_ptr<Cell> fromCell)
{
  shared_ptr<LegacyCell> b = std::static_pointer_cast<LegacyCell>(fromCell);
  return compare(bytes, b_off, b_len, b->bytes, b->b_off, b->b_len);
}

int LegacyCell::compare(std::deque<char> &aBytes, int aUpto, int a_length,
                        std::deque<char> &bBytes, int bUpto, int b_length)
{
  constexpr int aStop = aUpto + min(a_length, b_length);
  while (aUpto < aStop) {
    int aByte = aBytes[aUpto++] & 0xff;
    int bByte = bBytes[bUpto++] & 0xff;

    int diff = aByte - bByte;
    if (diff != 0) {
      return diff;
    }
  }

  // One is a prefix of the other, or, they are equal:
  return a_length - b_length;
}

bool LegacyCell::equals(any obj)
{
  // this method isn't "normally" called; just in asserts/tests
  if (std::dynamic_pointer_cast<Cell>(obj) != nullptr) {
    shared_ptr<Cell> cell = any_cast<std::shared_ptr<Cell>>(obj);
    return getTokenBytesWithLeaf(nullptr)->equals(
        cell->getTokenBytesWithLeaf(nullptr));
  } else {
    return false;
  }
}

int LegacyCell::hashCode()
{
  return getTokenBytesWithLeaf(nullptr)->hashCode();
}

wstring LegacyCell::toString()
{
  // this method isn't "normally" called; just in asserts/tests
  return getTokenBytesWithLeaf(nullptr)->utf8ToString();
}
} // namespace org::apache::lucene::spatial::prefix::tree