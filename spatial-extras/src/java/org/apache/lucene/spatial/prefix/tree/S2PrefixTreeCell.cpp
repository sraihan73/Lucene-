using namespace std;

#include "S2PrefixTreeCell.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using com::google::common::geometry::S2CellId;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
std::deque<std::shared_ptr<com::google::common::geometry::S2CellId>>
    S2PrefixTreeCell::FACES(6);

S2PrefixTreeCell::StaticConstructor::StaticConstructor()
{
  FACES[0] = S2CellId::fromFacePosLevel(0, 0, 0);
  FACES[1] = S2CellId::fromFacePosLevel(1, 0, 0);
  FACES[2] = S2CellId::fromFacePosLevel(2, 0, 0);
  FACES[3] = S2CellId::fromFacePosLevel(3, 0, 0);
  FACES[4] = S2CellId::fromFacePosLevel(4, 0, 0);
  FACES[5] = S2CellId::fromFacePosLevel(5, 0, 0);
  PIXELS = unordered_map<>(TOKENS.size());
  for (int i = 0; i < TOKENS.size(); i++) {
    PIXELS.emplace(TOKENS[i], i);
  }
}

S2PrefixTreeCell::StaticConstructor S2PrefixTreeCell::staticConstructor;
std::deque<char> const S2PrefixTreeCell::TOKENS = {
    L'.', L'/', L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8',
    L'9', L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J',
    L'K', L'L', L'M', L'N', L'O', L'P', L'Q', L'R', L'S', L'T', L'U',
    L'V', L'W', L'X', L'Y', L'Z', L'a', L'b', L'c', L'd', L'e', L'f',
    L'g', L'h', L'i', L'j', L'k', L'l', L'm', L'n', L'o', L'p', L'q',
    L'r', L's', L't', L'u', L'v', L'w', L'x', L'y', L'z'};
const unordered_map<char, int> S2PrefixTreeCell::PIXELS;

S2PrefixTreeCell::S2PrefixTreeCell(shared_ptr<S2PrefixTree> tree,
                                   shared_ptr<S2CellId> cellId)
{
  this->cellId = cellId;
  this->tree = tree;
  setLevel();
  if (getLevel() == tree->getMaxLevels()) {
    setLeaf();
  }
}

void S2PrefixTreeCell::readCell(shared_ptr<S2PrefixTree> tree,
                                shared_ptr<BytesRef> ref)
{
  isLeaf_ = false;
  shape.reset();
  shapeRel.reset();
  this->tree = tree;
  cellId = getS2CellIdFromBytesRef(ref);
  setLevel();
  if (isLeaf(ref) || getLevel() == tree->getMaxLevels()) {
    setLeaf();
  }
}

shared_ptr<SpatialRelation> S2PrefixTreeCell::getShapeRel() { return shapeRel; }

void S2PrefixTreeCell::setShapeRel(shared_ptr<SpatialRelation> rel)
{
  shapeRel = rel;
}

bool S2PrefixTreeCell::isLeaf() { return isLeaf_; }

void S2PrefixTreeCell::setLeaf() { isLeaf_ = true; }

shared_ptr<BytesRef>
S2PrefixTreeCell::getTokenBytesWithLeaf(shared_ptr<BytesRef> result)
{
  result = getTokenBytesNoLeaf(result);
  // max levels do not have leaf
  if (isLeaf() && !(getLevel() == tree->getMaxLevels())) {
    // Add leaf byte
    result->bytes[result->offset + result->length] = LEAF;
    result->length++;
  }
  return result;
}

shared_ptr<BytesRef>
S2PrefixTreeCell::getTokenBytesNoLeaf(shared_ptr<BytesRef> result)
{
  if (result == nullptr) {
    result = make_shared<BytesRef>();
  }
  getBytesRefFromS2CellId(cellId, result);
  return result;
}

int S2PrefixTreeCell::getLevel() { return this->level; }

void S2PrefixTreeCell::setLevel()
{
  if (this->cellId == nullptr) {
    this->level = 0;
  } else {
    assert(cellId->level() % tree->arity == 0);
    this->level = (this->cellId->level() / tree->arity) + 1;
  }
}

shared_ptr<CellIterator>
S2PrefixTreeCell::getNextLevelCells(shared_ptr<Shape> shapeFilter)
{
  std::deque<std::shared_ptr<S2CellId>> children;
  if (cellId == nullptr) { // this is the world cell
    children = FACES;
  } else {
    int nChildren = static_cast<int>(pow(4, tree->arity));
    children = std::deque<std::shared_ptr<S2CellId>>(nChildren);
    children[0] = cellId->childBegin(cellId->level() + tree->arity);
    for (int i = 1; i < nChildren; i++) {
      children[i] = children[i - 1]->next();
    }
  }
  deque<std::shared_ptr<Cell>> cells =
      deque<std::shared_ptr<Cell>>(children.size());
  for (auto pixel : children) {
    cells.push_back(make_shared<S2PrefixTreeCell>(tree, pixel));
  }
  return make_shared<FilterCellIterator>(cells.begin(), shapeFilter);
}

shared_ptr<Shape> S2PrefixTreeCell::getShape()
{
  if (shape == nullptr) {
    if (cellId == nullptr) { // World cell
      shape = tree->getSpatialContext().getWorldBounds();
    } else {
      shape = tree->s2ShapeFactory.getS2CellShape(cellId);
    }
  }
  return shape;
}

bool S2PrefixTreeCell::isPrefixOf(shared_ptr<Cell> c)
{
  if (cellId == nullptr) {
    return true;
  }
  shared_ptr<S2PrefixTreeCell> cell =
      std::static_pointer_cast<S2PrefixTreeCell>(c);
  return cellId->contains(cell->cellId);
}

int S2PrefixTreeCell::compareToNoLeaf(shared_ptr<Cell> fromCell)
{
  if (cellId == nullptr) {
    return 1;
  }
  shared_ptr<S2PrefixTreeCell> cell =
      std::static_pointer_cast<S2PrefixTreeCell>(fromCell);
  return cellId->compareTo(cell->cellId);
}

bool S2PrefixTreeCell::isLeaf(shared_ptr<BytesRef> ref)
{
  return (ref->bytes[ref->offset + ref->length - 1] == LEAF);
}

shared_ptr<S2CellId>
S2PrefixTreeCell::getS2CellIdFromBytesRef(shared_ptr<BytesRef> ref)
{
  int length = ref->length;
  if (isLeaf(ref)) {
    length--;
  }
  if (length == 0) {
    return nullptr; // world cell
  }
  int face = PIXELS[ref->bytes[ref->offset]];
  shared_ptr<S2CellId> cellId = FACES[face];
  int64_t id = cellId->id();
  for (int i = ref->offset + 1; i < ref->offset + length; i++) {
    int thisLevel = i - ref->offset;
    int pos = PIXELS[ref->bytes[i]];
    // first child at level
    id = id - (id & -id) +
         (1LL << (2 * (S2CellId::MAX_LEVEL - thisLevel * tree->arity)));
    // next until pos
    id = id + pos * ((id & -id) << 1);
  }
  return make_shared<S2CellId>(id);
}

void S2PrefixTreeCell::getBytesRefFromS2CellId(shared_ptr<S2CellId> cellId,
                                               shared_ptr<BytesRef> bref)
{
  if (cellId == nullptr) { // world cell
    bref->length = 0;
    return;
  }
  int length = getLevel() + 1;
  std::deque<char> b =
      bref->bytes.size() >= length ? bref->bytes : std::deque<char>(length);
  b[0] = TOKENS[cellId->face()];
  for (int i = 1; i < getLevel(); i++) {
    int offset = 0;
    int level = tree->arity * i;
    for (int j = 1; j < tree->arity; j++) {
      offset = 4 * offset + cellId->childPosition(level - tree->arity + j);
    }
    b[i] = TOKENS[4 * offset + cellId->childPosition(level)];
  }
  bref->bytes = b;
  bref->length = getLevel();
  bref->offset = 0;
}

int S2PrefixTreeCell::getSubCellsSize()
{
  if (cellId == nullptr) { // root node
    return 6;
  }
  return static_cast<int>(pow(4, tree->arity));
}

int S2PrefixTreeCell::hashCode()
{
  if (cellId == nullptr) {
    return __super::hashCode();
  }
  return this->cellId->hashCode();
}

bool S2PrefixTreeCell::equals(any o)
{
  shared_ptr<S2PrefixTreeCell> cell =
      any_cast<std::shared_ptr<S2PrefixTreeCell>>(o);
  return Objects::equals(cellId, cell->cellId);
}

wstring S2PrefixTreeCell::toString()
{
  if (cellId == nullptr) {
    return L"0";
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return cellId->toString();
}
} // namespace org::apache::lucene::spatial::prefix::tree