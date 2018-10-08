using namespace std;

#include "ContainsPrefixTreeQuery.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using SentinelIntSet = org::apache::lucene::util::SentinelIntSet;

ContainsPrefixTreeQuery::ContainsPrefixTreeQuery(
    shared_ptr<Shape> queryShape, const wstring &fieldName,
    shared_ptr<SpatialPrefixTree> grid, int detailLevel,
    bool multiOverlappingIndexedShapes)
    : AbstractPrefixTreeQuery(queryShape, fieldName, grid, detailLevel),
      multiOverlappingIndexedShapes(multiOverlappingIndexedShapes)
{
}

bool ContainsPrefixTreeQuery::equals(any o)
{
  if (!AbstractPrefixTreeQuery::equals(o)) {
    return false;
  }
  return multiOverlappingIndexedShapes ==
         (any_cast<std::shared_ptr<ContainsPrefixTreeQuery>>(o))
             .multiOverlappingIndexedShapes;
}

int ContainsPrefixTreeQuery::hashCode()
{
  return AbstractPrefixTreeQuery::hashCode() +
         (multiOverlappingIndexedShapes ? 1 : 0);
}

wstring ContainsPrefixTreeQuery::toString(const wstring &field)
{
  return getClass().getSimpleName() + L"(" + L"fieldName=" + fieldName + L"," +
         L"queryShape=" + queryShape + L"," + L"detailLevel=" +
         to_wstring(detailLevel) + L"," + L"multiOverlappingIndexedShapes=" +
         StringHelper::toString(multiOverlappingIndexedShapes) + L")";
}

shared_ptr<DocIdSet> ContainsPrefixTreeQuery::getDocIdSet(
    shared_ptr<LeafReaderContext> context) 
{
  return (make_shared<ContainsVisitor>(shared_from_this(), context))
      ->visit(grid->getWorldCell(), nullptr);
}

ContainsPrefixTreeQuery::ContainsVisitor::ContainsVisitor(
    shared_ptr<ContainsPrefixTreeQuery> outerInstance,
    shared_ptr<LeafReaderContext> context) 
    : BaseTermsEnumTraverser(outerInstance, context),
      outerInstance(outerInstance)
{
  if (termsEnum != nullptr) {
    nextTerm(); // advance to first
  }
}

shared_ptr<SmallDocSet> ContainsPrefixTreeQuery::ContainsVisitor::visit(
    shared_ptr<Cell> cell, shared_ptr<Bits> acceptContains) 
{

  if (thisTerm == nullptr) // signals all done
  {
    return nullptr;
  }

  // Get the AND of all child results (into combinedSubResults)
  shared_ptr<SmallDocSet> combinedSubResults = nullptr;
  //   Optimization: use null subCellsFilter when we know cell is within the
  //   query shape.
  shared_ptr<Shape> subCellsFilter = outerInstance->queryShape;
  if (cell->getLevel() != 0 &&
      ((cell->getShapeRel() == nullptr ||
        cell->getShapeRel() == SpatialRelation::WITHIN))) {
    subCellsFilter.reset();
    assert(cell->getShape()->relate(outerInstance->queryShape) ==
           SpatialRelation::WITHIN);
  }
  shared_ptr<CellIterator> subCells = cell->getNextLevelCells(subCellsFilter);
  while (subCells->hasNext()) {
    shared_ptr<Cell> subCell = subCells->next();
    if (!seek(subCell)) {
      combinedSubResults.reset();
    } else if (subCell->getLevel() == outerInstance->detailLevel) {
      combinedSubResults = getDocs(subCell, acceptContains);
    } else if (!outerInstance->multiOverlappingIndexedShapes &&
               subCell->getShapeRel() == SpatialRelation::WITHIN) {
      combinedSubResults = getLeafDocs(subCell, acceptContains);
    } else {
      // OR the leaf docs with all child results
      shared_ptr<SmallDocSet> leafDocs = getLeafDocs(subCell, acceptContains);
      shared_ptr<SmallDocSet> subDocs =
          visit(subCell, acceptContains); // recursion
      combinedSubResults = union_(leafDocs, subDocs);
    }

    if (combinedSubResults == nullptr) {
      break;
    }
    acceptContains =
        combinedSubResults; // has the 'AND' effect on next iteration
    subCells++;
  }

  return combinedSubResults;
}

bool ContainsPrefixTreeQuery::ContainsVisitor::seek(
    shared_ptr<Cell> cell) 
{
  if (thisTerm == nullptr) {
    return false;
  }
  constexpr int compare = indexedCell->compareToNoLeaf(cell);
  if (compare > 0) {
    return false; // leap-frog effect
  } else if (compare == 0) {
    return true; // already there!
  } else {       // compare > 0
    // seek!
    seekTerm = cell->getTokenBytesNoLeaf(seekTerm);
    constexpr TermsEnum::SeekStatus seekStatus = termsEnum->seekCeil(seekTerm);
    if (seekStatus == TermsEnum::SeekStatus::END) {
      thisTerm.reset(); // all done
      return false;
    }
    thisTerm = termsEnum->term();
    indexedCell = outerInstance->grid->readCell(thisTerm, indexedCell);
    if (seekStatus == TermsEnum::SeekStatus::FOUND) {
      return true;
    }
    return indexedCell->isLeaf() && indexedCell->compareToNoLeaf(cell) == 0;
  }
}

shared_ptr<SmallDocSet> ContainsPrefixTreeQuery::ContainsVisitor::getDocs(
    shared_ptr<Cell> cell, shared_ptr<Bits> acceptContains) 
{
  assert(indexedCell->compareToNoLeaf(cell) == 0);
  // called when we've reached detailLevel.
  if (indexedCell->isLeaf()) { // only a leaf
    shared_ptr<SmallDocSet> result = collectDocs(acceptContains);
    nextTerm();
    return result;
  } else {
    shared_ptr<SmallDocSet> docsAtPrefix = collectDocs(acceptContains);
    if (!nextTerm()) {
      return docsAtPrefix;
    }
    // collect leaf too
    if (indexedCell->isLeaf() && indexedCell->compareToNoLeaf(cell) == 0) {
      shared_ptr<SmallDocSet> docsAtLeaf = collectDocs(acceptContains);
      nextTerm();
      return union_(docsAtPrefix, docsAtLeaf);
    } else {
      return docsAtPrefix;
    }
  }
}

shared_ptr<SmallDocSet> ContainsPrefixTreeQuery::ContainsVisitor::getLeafDocs(
    shared_ptr<Cell> cell, shared_ptr<Bits> acceptContains) 
{
  assert(indexedCell->compareToNoLeaf(cell) == 0);
  // Advance past prefix if we're at a prefix; return null if no leaf
  if (!indexedCell->isLeaf()) {
    if (!nextTerm() || !indexedCell->isLeaf() ||
        indexedCell->getLevel() != cell->getLevel()) {
      return nullptr;
    }
  }
  shared_ptr<SmallDocSet> result = collectDocs(acceptContains);
  nextTerm();
  return result;
}

bool ContainsPrefixTreeQuery::ContainsVisitor::nextTerm() 
{
  if ((thisTerm = termsEnum->next()) == nullptr) {
    return false;
  }
  indexedCell = outerInstance->grid->readCell(thisTerm, indexedCell);
  return true;
}

shared_ptr<SmallDocSet>
ContainsPrefixTreeQuery::ContainsVisitor::union_(shared_ptr<SmallDocSet> aSet,
                                                 shared_ptr<SmallDocSet> bSet)
{
  if (bSet != nullptr) {
    if (aSet == nullptr) {
      return bSet;
    }
    return aSet->union_(bSet); // union is 'or'
  }
  return aSet;
}

shared_ptr<SmallDocSet> ContainsPrefixTreeQuery::ContainsVisitor::collectDocs(
    shared_ptr<Bits> acceptContains) 
{
  shared_ptr<SmallDocSet> set = nullptr;

  postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);
  int docid;
  while ((docid = postingsEnum->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    if (acceptContains != nullptr && acceptContains->get(docid) == false) {
      continue;
    }
    if (set == nullptr) {
      int size = termsEnum->docFreq();
      if (size <= 0) {
        size = 16;
      }
      set = make_shared<SmallDocSet>(size);
    }
    set->set(docid);
  }
  return set;
}

ContainsPrefixTreeQuery::SmallDocSet::SmallDocSet(int size)
    : intSet(make_shared<SentinelIntSet>(size, -1))
{
}

bool ContainsPrefixTreeQuery::SmallDocSet::get(int index)
{
  return intSet->exists(index);
}

void ContainsPrefixTreeQuery::SmallDocSet::set(int index)
{
  intSet->put(index);
  if (index > maxInt) {
    maxInt = index;
  }
}

int ContainsPrefixTreeQuery::SmallDocSet::length() { return maxInt; }

int ContainsPrefixTreeQuery::SmallDocSet::size() { return intSet->size(); }

shared_ptr<SmallDocSet>
ContainsPrefixTreeQuery::SmallDocSet::union_(shared_ptr<SmallDocSet> other)
{
  shared_ptr<SmallDocSet> bigger;
  shared_ptr<SmallDocSet> smaller;
  if (other->intSet->size() > this->intSet->size()) {
    bigger = other;
    smaller = shared_from_this();
  } else {
    bigger = shared_from_this();
    smaller = other;
  }
  // modify bigger
  for (auto v : smaller->intSet->keys) {
    if (v == smaller->intSet->emptyVal) {
      continue;
    }
    bigger->set(v);
  }
  return bigger;
}

shared_ptr<Bits> ContainsPrefixTreeQuery::SmallDocSet::bits() 
{
  // if the # of docids is super small, return null since iteration is going
  // to be faster
  return size() > 4 ? shared_from_this() : nullptr;
}

shared_ptr<DocIdSetIterator>
ContainsPrefixTreeQuery::SmallDocSet::iterator() 
{
  if (size() == 0) {
    return nullptr;
  }
  // copy the unsorted values to a new array then sort them
  int d = 0;
  const std::deque<int> docs = std::deque<int>(intSet->size());
  for (auto v : intSet->keys) {
    if (v == intSet->emptyVal) {
      continue;
    }
    docs[d++] = v;
  }
  assert(d == intSet->size());
  constexpr int size = d;

  // sort them
  Arrays::sort(docs, 0, size);

  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this(),
                                                          docs, size);
}

ContainsPrefixTreeQuery::SmallDocSet::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(shared_ptr<SmallDocSet> outerInstance,
                                        deque<int> &docs, int size)
{
  this->outerInstance = outerInstance;
  this->docs = docs;
  this->size = size;
  idx = -1;
}

int ContainsPrefixTreeQuery::SmallDocSet::DocIdSetIteratorAnonymousInnerClass::
    docID()
{
  if (idx < 0) {
    return -1;
  } else if (idx < size) {
    return docs[idx];
  } else {
    return DocIdSetIterator::NO_MORE_DOCS;
  }
}

int ContainsPrefixTreeQuery::SmallDocSet::DocIdSetIteratorAnonymousInnerClass::
    nextDoc() 
{
  if (++idx < size) {
    return docs[idx];
  }
  return DocIdSetIterator::NO_MORE_DOCS;
}

int ContainsPrefixTreeQuery::SmallDocSet::DocIdSetIteratorAnonymousInnerClass::
    advance(int target) 
{
  // for this small set this is likely faster vs. a binary search
  // into the sorted array
  return slowAdvance(target);
}

int64_t ContainsPrefixTreeQuery::SmallDocSet::
    DocIdSetIteratorAnonymousInnerClass::cost()
{
  return size;
}

int64_t ContainsPrefixTreeQuery::SmallDocSet::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_REF + Integer::BYTES) +
         intSet->ramBytesUsed();
}
} // namespace org::apache::lucene::spatial::prefix