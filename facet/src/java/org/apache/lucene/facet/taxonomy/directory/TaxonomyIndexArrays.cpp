using namespace std;

#include "TaxonomyIndexArrays.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using ParallelTaxonomyArrays =
    org::apache::lucene::facet::taxonomy::ParallelTaxonomyArrays;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

TaxonomyIndexArrays::TaxonomyIndexArrays(std::deque<int> &parents)
    : parents(parents)
{
}

TaxonomyIndexArrays::TaxonomyIndexArrays(shared_ptr<IndexReader> reader) throw(
    IOException)
    : parents(std::deque<int>(reader->maxDoc()))
{
  if (parents_.size() > 0) {
    initParents(reader, 0);
    // Starting Lucene 2.9, following the change LUCENE-1542, we can
    // no longer reliably read the parent "-1" (see comment in
    // LuceneTaxonomyWriter.SinglePositionTokenStream). We have no way
    // to fix this in indexing without breaking backward-compatibility
    // with existing indexes, so what we'll do instead is just
    // hard-code the parent of ordinal 0 to be -1, and assume (as is
    // indeed the case) that no other parent can be -1.
    parents_[0] = TaxonomyReader::INVALID_ORDINAL;
  }
}

TaxonomyIndexArrays::TaxonomyIndexArrays(
    shared_ptr<IndexReader> reader,
    shared_ptr<TaxonomyIndexArrays> copyFrom) 
    : parents(std::deque<int>(reader->maxDoc()))
{
  assert(copyFrom != nullptr);

  // note that copyParents.length may be equal to reader.maxDoc(). this is not a
  // bug it may be caused if e.g. the taxonomy segments were merged, and so an
  // updated NRT reader was obtained, even though nothing was changed. this is
  // not very likely to happen.
  std::deque<int> copyParents = copyFrom->parents();
  System::arraycopy(copyParents, 0, parents_, 0, copyParents.size());
  initParents(reader, copyParents.size());

  if (copyFrom->initializedChildren) {
    initChildrenSiblings(copyFrom);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void TaxonomyIndexArrays::initChildrenSiblings(
    shared_ptr<TaxonomyIndexArrays> copyFrom)
{
  if (!initializedChildren) { // must do this check !
    children_ = std::deque<int>(parents_.size());
    siblings_ = std::deque<int>(parents_.size());
    if (copyFrom != nullptr) {
      // called from the ctor, after we know copyFrom has initialized
      // children/siblings
      System::arraycopy(copyFrom->children(), 0, children_, 0,
                        copyFrom->children().size());
      System::arraycopy(copyFrom->siblings(), 0, siblings_, 0,
                        copyFrom->siblings().size());
      computeChildrenSiblings(copyFrom->parents_.size());
    } else {
      computeChildrenSiblings(0);
    }
    initializedChildren = true;
  }
}

void TaxonomyIndexArrays::computeChildrenSiblings(int first)
{
  // reset the youngest child of all ordinals. while this should be done only
  // for the leaves, we don't know up front which are the leaves, so we reset
  // all of them.
  for (int i = first; i < parents_.size(); i++) {
    children_[i] = TaxonomyReader::INVALID_ORDINAL;
  }

  // the root category has no parent, and therefore no siblings
  if (first == 0) {
    first = 1;
    siblings_[0] = TaxonomyReader::INVALID_ORDINAL;
  }

  for (int i = first; i < parents_.size(); i++) {
    // note that parents[i] is always < i, so the right-hand-side of
    // the following line is already set when we get here
    siblings_[i] = children_[parents_[i]];
    children_[parents_[i]] = i;
  }
}

void TaxonomyIndexArrays::initParents(shared_ptr<IndexReader> reader,
                                      int first) 
{
  if (reader->maxDoc() == first) {
    return;
  }

  // it's ok to use MultiFields because we only iterate on one posting deque.
  // breaking it to loop over the leaves() only complicates code for no
  // apparent gain.
  shared_ptr<PostingsEnum> positions = MultiFields::getTermPositionsEnum(
      reader, Consts::FIELD_PAYLOADS, Consts::PAYLOAD_PARENT_BYTES_REF,
      PostingsEnum::PAYLOADS);

  // shouldn't really happen, if it does, something's wrong
  if (positions == nullptr ||
      positions->advance(first) == DocIdSetIterator::NO_MORE_DOCS) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<CorruptIndexException>(
        L"Missing parent data for category " + to_wstring(first),
        reader->toString());
  }

  int num = reader->maxDoc();
  for (int i = first; i < num; i++) {
    if (positions->docID() == i) {
      if (positions->freq() == 0) { // shouldn't happen
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw make_shared<CorruptIndexException>(
            L"Missing parent data for category " + to_wstring(i),
            reader->toString());
      }

      parents_[i] = positions->nextPosition();

      if (positions->nextDoc() == DocIdSetIterator::NO_MORE_DOCS) {
        if (i + 1 < num) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw make_shared<CorruptIndexException>(
              L"Missing parent data for category " + to_wstring(i + 1),
              reader->toString());
        }
        break;
      }
    } else { // this shouldn't happen
             // C++ TODO: There is no native C++ equivalent to 'toString':
      throw make_shared<CorruptIndexException>(
          L"Missing parent data for category " + to_wstring(i),
          reader->toString());
    }
  }
}

shared_ptr<TaxonomyIndexArrays> TaxonomyIndexArrays::add(int ordinal,
                                                         int parentOrdinal)
{
  if (ordinal >= parents_.size()) {
    std::deque<int> newarray = ArrayUtil::grow(parents_, ordinal + 1);
    newarray[ordinal] = parentOrdinal;
    return make_shared<TaxonomyIndexArrays>(newarray);
  }
  parents_[ordinal] = parentOrdinal;
  return shared_from_this();
}

std::deque<int> TaxonomyIndexArrays::parents() { return parents_; }

std::deque<int> TaxonomyIndexArrays::children()
{
  if (!initializedChildren) {
    initChildrenSiblings(nullptr);
  }

  // the array is guaranteed to be populated
  return children_;
}

std::deque<int> TaxonomyIndexArrays::siblings()
{
  if (!initializedChildren) {
    initChildrenSiblings(nullptr);
  }

  // the array is guaranteed to be populated
  return siblings_;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t TaxonomyIndexArrays::ramBytesUsed()
{
  int64_t ramBytesUsed = RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
                           3 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
                           RamUsageEstimator::NUM_BYTES_BOOLEAN;
  ramBytesUsed += RamUsageEstimator::shallowSizeOf(parents_);
  if (children_.size() > 0) {
    ramBytesUsed += RamUsageEstimator::shallowSizeOf(children_);
  }
  if (siblings_.size() > 0) {
    ramBytesUsed += RamUsageEstimator::shallowSizeOf(siblings_);
  }
  return ramBytesUsed;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
TaxonomyIndexArrays::getChildResources()
{
  const deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.push_back(Accountables::namedAccountable(
      L"parents", RamUsageEstimator::shallowSizeOf(parents_)));
  if (children_.size() > 0) {
    resources.push_back(Accountables::namedAccountable(
        L"children", RamUsageEstimator::shallowSizeOf(children_)));
  }
  if (siblings_.size() > 0) {
    resources.push_back(Accountables::namedAccountable(
        L"siblings", RamUsageEstimator::shallowSizeOf(siblings_)));
  }
  return Collections::unmodifiableList(resources);
}
} // namespace org::apache::lucene::facet::taxonomy::directory