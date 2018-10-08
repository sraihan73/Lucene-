using namespace std;

#include "BlockJoinSelector.h"

namespace org::apache::lucene::search::join
{
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using Bits = org::apache::lucene::util::Bits;

BlockJoinSelector::BlockJoinSelector() {}

shared_ptr<Bits> BlockJoinSelector::wrap(shared_ptr<Bits> docsWithValue,
                                         shared_ptr<BitSet> parents,
                                         shared_ptr<BitSet> children)
{
  return make_shared<BitsAnonymousInnerClass>(docsWithValue, parents, children);
}

BlockJoinSelector::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    shared_ptr<Bits> docsWithValue, shared_ptr<BitSet> parents,
    shared_ptr<BitSet> children)
{
  this->docsWithValue = docsWithValue;
  this->parents = parents;
  this->children = children;
}

bool BlockJoinSelector::BitsAnonymousInnerClass::get(int docID)
{
  assert((parents->get(docID),
          L"this selector may only be used on parent documents"));

  if (docID == 0) {
    // no children
    return false;
  }

  constexpr int firstChild = parents->prevSetBit(docID - 1) + 1;
  for (int child = children->nextSetBit(firstChild); child < docID;
       child = children->nextSetBit(child + 1)) {
    if (docsWithValue->get(child)) {
      return true;
    }
  }
  return false;
}

int BlockJoinSelector::BitsAnonymousInnerClass::length()
{
  return docsWithValue->length();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.index.SortedDocValues
// wrap(org.apache.lucene.index.SortedSetDocValues sortedSet, Type selection,
// org.apache.lucene.util.BitSet parents, org.apache.lucene.util.BitSet children)
shared_ptr<SortedDocValues>
BlockJoinSelector::wrap(shared_ptr<SortedSetDocValues> sortedSet,
                        Type selection, shared_ptr<BitSet> parents,
                        shared_ptr<BitSet> children)
{
  return wrap(sortedSet, selection, parents, toIter(children));
}

shared_ptr<SortedDocValues>
BlockJoinSelector::wrap(shared_ptr<SortedSetDocValues> sortedSet,
                        Type selection, shared_ptr<BitSet> parents,
                        shared_ptr<DocIdSetIterator> children)
{
  shared_ptr<SortedDocValues> values;
  switch (selection) {
  case org::apache::lucene::search::join::BlockJoinSelector::Type::MIN:
    values = SortedSetSelector::wrap(sortedSet, SortedSetSelector::Type::MIN);
    break;
  case org::apache::lucene::search::join::BlockJoinSelector::Type::MAX:
    values = SortedSetSelector::wrap(sortedSet, SortedSetSelector::Type::MAX);
    break;
  default:
    throw make_shared<AssertionError>();
  }
  return wrap(values, selection, parents, children);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.index.SortedDocValues wrap(final
// org.apache.lucene.index.SortedDocValues values, Type selection,
// org.apache.lucene.util.BitSet parents, org.apache.lucene.util.BitSet children)
shared_ptr<SortedDocValues>
BlockJoinSelector::wrap(shared_ptr<SortedDocValues> values, Type selection,
                        shared_ptr<BitSet> parents, shared_ptr<BitSet> children)
{
  return wrap(values, selection, parents, toIter(children));
}

shared_ptr<SortedDocValues>
BlockJoinSelector::wrap(shared_ptr<SortedDocValues> values, Type selection,
                        shared_ptr<BitSet> parents,
                        shared_ptr<DocIdSetIterator> children)
{
  if (values->docID() != -1) {
    throw invalid_argument(
        L"values iterator was already consumed: values.docID=" +
        to_wstring(values->docID()));
  }
  return ToParentDocValues::wrap(values, selection, parents, children);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.index.NumericDocValues
// wrap(org.apache.lucene.index.SortedNumericDocValues sortedNumerics, Type
// selection, org.apache.lucene.util.BitSet parents,
// org.apache.lucene.util.BitSet children)
shared_ptr<NumericDocValues>
BlockJoinSelector::wrap(shared_ptr<SortedNumericDocValues> sortedNumerics,
                        Type selection, shared_ptr<BitSet> parents,
                        shared_ptr<BitSet> children)
{
  return wrap(sortedNumerics, selection, parents, toIter(children));
}

shared_ptr<BitSetIterator>
BlockJoinSelector::toIter(shared_ptr<BitSet> children)
{
  return make_shared<BitSetIterator>(children, 0);
}

shared_ptr<NumericDocValues>
BlockJoinSelector::wrap(shared_ptr<SortedNumericDocValues> sortedNumerics,
                        Type selection, shared_ptr<BitSet> parents,
                        shared_ptr<DocIdSetIterator> children)
{
  shared_ptr<NumericDocValues> values;
  switch (selection) {
  case org::apache::lucene::search::join::BlockJoinSelector::Type::MIN:
    values = SortedNumericSelector::wrap(sortedNumerics,
                                         SortedNumericSelector::Type::MIN,
                                         SortField::Type::LONG);
    break;
  case org::apache::lucene::search::join::BlockJoinSelector::Type::MAX:
    values = SortedNumericSelector::wrap(sortedNumerics,
                                         SortedNumericSelector::Type::MAX,
                                         SortField::Type::LONG);
    break;
  default:
    throw make_shared<AssertionError>();
  }
  return wrap(values, selection, parents, children);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.index.NumericDocValues wrap(final
// org.apache.lucene.index.NumericDocValues values, Type selection,
// org.apache.lucene.util.BitSet parents, org.apache.lucene.util.BitSet children)
shared_ptr<NumericDocValues>
BlockJoinSelector::wrap(shared_ptr<NumericDocValues> values, Type selection,
                        shared_ptr<BitSet> parents, shared_ptr<BitSet> children)
{
  return wrap(values, selection, parents, toIter(children));
}

shared_ptr<NumericDocValues>
BlockJoinSelector::wrap(shared_ptr<NumericDocValues> values, Type selection,
                        shared_ptr<BitSet> parents,
                        shared_ptr<DocIdSetIterator> children)
{
  if (values->docID() != -1) {
    throw invalid_argument(
        L"values iterator was already consumed: values.docID=" +
        to_wstring(values->docID()));
  }
  return ToParentDocValues::wrap(values, selection, parents, children);
}
} // namespace org::apache::lucene::search::join