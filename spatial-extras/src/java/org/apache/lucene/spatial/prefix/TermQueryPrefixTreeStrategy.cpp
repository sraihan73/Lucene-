using namespace std;

#include "TermQueryPrefixTreeStrategy.h"

namespace org::apache::lucene::spatial::prefix
{
using Query = org::apache::lucene::search::Query;
using TermInSetQuery = org::apache::lucene::search::TermInSetQuery;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using UnsupportedSpatialOperation =
    org::apache::lucene::spatial::query::UnsupportedSpatialOperation;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

TermQueryPrefixTreeStrategy::TermQueryPrefixTreeStrategy(
    shared_ptr<SpatialPrefixTree> grid, const wstring &fieldName)
    : PrefixTreeStrategy(grid, fieldName)
{
}

shared_ptr<CellToBytesRefIterator>
TermQueryPrefixTreeStrategy::newCellToBytesRefIterator()
{
  // Ensure we don't have leaves, as this strategy doesn't handle them.
  return make_shared<CellToBytesRefIteratorAnonymousInnerClass>(
      shared_from_this());
}

TermQueryPrefixTreeStrategy::CellToBytesRefIteratorAnonymousInnerClass::
    CellToBytesRefIteratorAnonymousInnerClass(
        shared_ptr<TermQueryPrefixTreeStrategy> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<BytesRef>
TermQueryPrefixTreeStrategy::CellToBytesRefIteratorAnonymousInnerClass::next()
{
  if (!cellIter->hasNext()) {
    return nullptr;
  }
  return cellIter->next().getTokenBytesNoLeaf(bytesRef);
}

shared_ptr<Query>
TermQueryPrefixTreeStrategy::makeQuery(shared_ptr<SpatialArgs> args)
{
  shared_ptr<SpatialOperation> *const op = args->getOperation();
  if (op != SpatialOperation::Intersects) {
    throw make_shared<UnsupportedSpatialOperation>(op);
  }

  shared_ptr<Shape> shape = args->getShape();
  int detailLevel =
      grid->getLevelForDistance(args->resolveDistErr(ctx, distErrPct));

  //--get a List of BytesRef for each term we want (no parents, no leaf bytes))
  constexpr int GUESS_NUM_TERMS;
  if (std::dynamic_pointer_cast<Point>(shape) != nullptr) {
    GUESS_NUM_TERMS = detailLevel; // perfect guess
  } else {
    GUESS_NUM_TERMS = 4096; // should this be a method on SpatialPrefixTree?
  }

  shared_ptr<BytesRefBuilder> masterBytes =
      make_shared<BytesRefBuilder>(); // shared byte array for all terms
  deque<std::shared_ptr<BytesRef>> terms =
      deque<std::shared_ptr<BytesRef>>(GUESS_NUM_TERMS);

  shared_ptr<CellIterator> cells =
      grid->getTreeCellIterator(shape, detailLevel);
  while (cells->hasNext()) {
    shared_ptr<Cell> cell = cells->next();
    if (!cell->isLeaf()) {
      continue;
    }
    shared_ptr<BytesRef> term = cell->getTokenBytesNoLeaf(
        nullptr); // null because we want a new BytesRef
    // We copy out the bytes because it may be re-used across the iteration.
    // This also gives us the opportunity
    // to use one contiguous block of memory for the bytes of all terms we need.
    masterBytes->grow(masterBytes->length() + term->length);
    masterBytes->append(term);
    term->bytes.clear(); // don't need; will reset later
    term->offset = masterBytes->length() - term->length;
    terms.push_back(term);
    cells++;
  }
  // doing this now because if we did earlier, it's possible the bytes needed to
  // grow()
  for (auto byteRef : terms) {
    byteRef->bytes = masterBytes->bytes();
  }
  // unfortunately TermsQuery will needlessly sort & dedupe
  // TODO an automatonQuery might be faster?
  return make_shared<TermInSetQuery>(getFieldName(), terms);
}
} // namespace org::apache::lucene::spatial::prefix