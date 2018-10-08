using namespace std;

#include "MultiSorter.h"

namespace org::apache::lucene::index
{
using DocMap = org::apache::lucene::index::MergeState::DocMap;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Bits = org::apache::lucene::util::Bits;
using LongValues = org::apache::lucene::util::LongValues;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

std::deque<std::shared_ptr<MergeState::DocMap>> MultiSorter::sort(
    shared_ptr<Sort> sort,
    deque<std::shared_ptr<CodecReader>> &readers) 
{

  // TODO: optimize if only 1 reader is incoming, though that's a rare case

  std::deque<std::shared_ptr<SortField>> fields = sort->getSort();
  std::deque<std::deque<std::shared_ptr<ComparableProvider>>> comparables(
      fields.size());
  for (int i = 0; i < fields.size(); i++) {
    comparables[i] = getComparableProviders(readers, fields[i]);
  }

  int leafCount = readers.size();

  shared_ptr<PriorityQueue<std::shared_ptr<LeafAndDocID>>> queue =
      make_shared<PriorityQueueAnonymousInnerClass>(leafCount, comparables);

  std::deque<std::shared_ptr<PackedLongValues::Builder>> builders(leafCount);

  for (int i = 0; i < leafCount; i++) {
    shared_ptr<CodecReader> reader = readers[i];
    shared_ptr<LeafAndDocID> leaf = make_shared<LeafAndDocID>(
        i, reader->getLiveDocs(), reader->maxDoc(), comparables.size());
    for (int j = 0; j < comparables.size(); j++) {
      leaf->values[j] = comparables[j][i]->getComparable(leaf->docID);
      assert(leaf->values[j] != nullptr);
    }
    queue->push_back(leaf);
    builders[i] = PackedLongValues::monotonicBuilder(PackedInts::COMPACT);
  }

  // merge sort:
  int mappedDocID = 0;
  int lastReaderIndex = 0;
  bool isSorted = true;
  while (queue->size() != 0) {
    shared_ptr<LeafAndDocID> top = queue->top();
    if (lastReaderIndex > top->readerIndex) {
      // merge sort is needed
      isSorted = false;
    }
    lastReaderIndex = top->readerIndex;
    builders[top->readerIndex]->add(mappedDocID);
    if (top->liveDocs == nullptr || top->liveDocs->get(top->docID)) {
      mappedDocID++;
    }
    top->docID++;
    if (top->docID < top->maxDoc) {
      for (int j = 0; j < comparables.size(); j++) {
        top->values[j] =
            comparables[j][top->readerIndex]->getComparable(top->docID);
        assert(top->values[j] != nullptr);
      }
      queue->updateTop();
    } else {
      queue->pop();
    }
  }
  if (isSorted) {
    return nullptr;
  }

  std::deque<std::shared_ptr<MergeState::DocMap>> docMaps(leafCount);
  for (int i = 0; i < leafCount; i++) {
    shared_ptr<PackedLongValues> *const remapped = builders[i]->build();
    shared_ptr<Bits> *const liveDocs = readers[i]->getLiveDocs();
    docMaps[i] = make_shared<DocMapAnonymousInnerClass>(remapped, liveDocs);
  }

  return docMaps;
}

MultiSorter::PriorityQueueAnonymousInnerClass::PriorityQueueAnonymousInnerClass(
    int leafCount,
    deque<deque<std::shared_ptr<
        org::apache::lucene::index::MultiSorter::ComparableProvider>>>
        &comparables)
    : org::apache::lucene::util::PriorityQueue<LeafAndDocID>(leafCount)
{
  this->comparables = comparables;
}

bool MultiSorter::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<LeafAndDocID> a, shared_ptr<LeafAndDocID> b)
{
  for (int i = 0; i < comparables.size(); i++) {
    int cmp = a->values[i](b->values[i]);
    if (cmp != 0) {
      return cmp < 0;
    }
  }

  // tie-break by docID natural order:
  if (a->readerIndex != b->readerIndex) {
    return a->readerIndex < b->readerIndex;
  } else {
    return a->docID < b->docID;
  }
}

MultiSorter::DocMapAnonymousInnerClass::DocMapAnonymousInnerClass(
    shared_ptr<PackedLongValues> remapped, shared_ptr<Bits> liveDocs)
{
  this->remapped = remapped;
  this->liveDocs = liveDocs;
}

int MultiSorter::DocMapAnonymousInnerClass::get(int docID)
{
  if (liveDocs == nullptr || liveDocs->get(docID)) {
    return static_cast<int>(remapped->get(docID));
  } else {
    return -1;
  }
}

MultiSorter::LeafAndDocID::LeafAndDocID(int readerIndex,
                                        shared_ptr<Bits> liveDocs, int maxDoc,
                                        int numComparables)
    : readerIndex(readerIndex), liveDocs(liveDocs), maxDoc(maxDoc),
      values(std::deque<Comparable>(numComparables))
{
}

std::deque<std::shared_ptr<ComparableProvider>>
MultiSorter::getComparableProviders(
    deque<std::shared_ptr<CodecReader>> &readers,
    shared_ptr<SortField> sortField) 
{

  std::deque<std::shared_ptr<ComparableProvider>> providers(readers.size());
  constexpr int reverseMul = sortField->getReverse() ? -1 : 1;
  constexpr SortField::Type sortType = Sorter::getSortFieldType(sortField);

  switch (sortType) {

  case SortField::Type::STRING: {
    // this uses the efficient segment-local ordinal map_obj:
    std::deque<std::shared_ptr<SortedDocValues>> values(readers.size());
    for (int i = 0; i < readers.size(); i++) {
      shared_ptr<SortedDocValues> *const sorted =
          Sorter::getOrWrapSorted(readers[i], sortField);
      values[i] = sorted;
    }
    shared_ptr<OrdinalMap> ordinalMap =
        OrdinalMap::build(nullptr, values, PackedInts::DEFAULT);
    constexpr int missingOrd;
    if (sortField->getMissingValue() == SortField::STRING_LAST) {
      missingOrd = sortField->getReverse() ? numeric_limits<int>::min()
                                           : numeric_limits<int>::max();
    } else {
      missingOrd = sortField->getReverse() ? numeric_limits<int>::max()
                                           : numeric_limits<int>::min();
    }

    for (int readerIndex = 0; readerIndex < readers.size(); readerIndex++) {
      shared_ptr<SortedDocValues> *const readerValues = values[readerIndex];
      shared_ptr<LongValues> *const globalOrds =
          ordinalMap->getGlobalOrds(readerIndex);
      providers[readerIndex] =
          make_shared<ComparableProviderAnonymousInnerClass>(
              reverseMul, missingOrd, readerValues, globalOrds);
    }
  } break;

  case SortField::Type::LONG: {
    const optional<int64_t> missingValue;
    if (sortField->getMissingValue() != nullptr) {
      missingValue =
          any_cast<optional<int64_t>>(sortField->getMissingValue());
    } else {
      missingValue = 0LL;
    }

    for (int readerIndex = 0; readerIndex < readers.size(); readerIndex++) {
      shared_ptr<NumericDocValues> *const values =
          Sorter::getOrWrapNumeric(readers[readerIndex], sortField);

      providers[readerIndex] =
          make_shared<ComparableProviderAnonymousInnerClass2>(
              reverseMul, missingValue, values);
    }
  } break;

  case SortField::Type::INT: {
    const optional<int> missingValue;
    if (sortField->getMissingValue() != nullptr) {
      missingValue = any_cast<optional<int>>(sortField->getMissingValue());
    } else {
      missingValue = 0;
    }

    for (int readerIndex = 0; readerIndex < readers.size(); readerIndex++) {
      shared_ptr<NumericDocValues> *const values =
          Sorter::getOrWrapNumeric(readers[readerIndex], sortField);

      providers[readerIndex] =
          make_shared<ComparableProviderAnonymousInnerClass3>(
              reverseMul, missingValue, values);
    }
  } break;

  case SortField::Type::DOUBLE: {
    const optional<double> missingValue;
    if (sortField->getMissingValue() != nullptr) {
      missingValue = any_cast<optional<double>>(sortField->getMissingValue());
    } else {
      missingValue = 0.0;
    }

    for (int readerIndex = 0; readerIndex < readers.size(); readerIndex++) {
      shared_ptr<NumericDocValues> *const values =
          Sorter::getOrWrapNumeric(readers[readerIndex], sortField);

      providers[readerIndex] =
          make_shared<ComparableProviderAnonymousInnerClass4>(
              reverseMul, missingValue, values);
    }
  } break;

  case SortField::Type::FLOAT: {
    const optional<float> missingValue;
    if (sortField->getMissingValue() != nullptr) {
      missingValue = any_cast<optional<float>>(sortField->getMissingValue());
    } else {
      missingValue = 0.0f;
    }

    for (int readerIndex = 0; readerIndex < readers.size(); readerIndex++) {
      shared_ptr<NumericDocValues> *const values =
          Sorter::getOrWrapNumeric(readers[readerIndex], sortField);

      providers[readerIndex] =
          make_shared<ComparableProviderAnonymousInnerClass5>(
              reverseMul, missingValue, values);
    }
  } break;

  default:
    throw invalid_argument(L"unhandled SortField.getType()=" +
                           sortField->getType());
  }

  return providers;
}

MultiSorter::ComparableProviderAnonymousInnerClass::
    ComparableProviderAnonymousInnerClass(
        int reverseMul, int missingOrd,
        shared_ptr<org::apache::lucene::index::SortedDocValues> readerValues,
        shared_ptr<LongValues> globalOrds)
{
  this->reverseMul = reverseMul;
  this->missingOrd = missingOrd;
  this->readerValues = readerValues;
  this->globalOrds = globalOrds;
}

bool MultiSorter::ComparableProviderAnonymousInnerClass::docsInOrder(int docID)
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(
        L"docs must be sent in order, but lastDocID=" + lastDocID +
        L" vs docID=" + to_wstring(docID));
  }
  lastDocID = docID;
  return true;
}

Comparable MultiSorter::ComparableProviderAnonymousInnerClass::getComparable(
    int docID) 
{
  assert(docsInOrder(docID));
  int readerDocID = readerValues->docID();
  if (readerDocID < docID) {
    readerDocID = readerValues->advance(docID);
  }
  if (readerDocID == docID) {
    // translate segment's ord to global ord space:
    return reverseMul *
           static_cast<int>(globalOrds->get(readerValues->ordValue()));
  } else {
    return missingOrd;
  }
}

MultiSorter::ComparableProviderAnonymousInnerClass2::
    ComparableProviderAnonymousInnerClass2(
        int reverseMul, optional<int64_t> &missingValue,
        shared_ptr<org::apache::lucene::index::NumericDocValues> values)
{
  this->reverseMul = reverseMul;
  this->missingValue = missingValue;
  this->values = values;
}

bool MultiSorter::ComparableProviderAnonymousInnerClass2::docsInOrder(int docID)
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(
        L"docs must be sent in order, but lastDocID=" + lastDocID +
        L" vs docID=" + to_wstring(docID));
  }
  lastDocID = docID;
  return true;
}

Comparable MultiSorter::ComparableProviderAnonymousInnerClass2::getComparable(
    int docID) 
{
  assert(docsInOrder(docID));
  int readerDocID = values->docID();
  if (readerDocID < docID) {
    readerDocID = values->advance(docID);
  }
  if (readerDocID == docID) {
    return reverseMul * values->longValue();
  } else {
    return reverseMul * missingValue;
  }
}

MultiSorter::ComparableProviderAnonymousInnerClass3::
    ComparableProviderAnonymousInnerClass3(
        int reverseMul, optional<int> &missingValue,
        shared_ptr<org::apache::lucene::index::NumericDocValues> values)
{
  this->reverseMul = reverseMul;
  this->missingValue = missingValue;
  this->values = values;
}

bool MultiSorter::ComparableProviderAnonymousInnerClass3::docsInOrder(int docID)
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(
        L"docs must be sent in order, but lastDocID=" + lastDocID +
        L" vs docID=" + to_wstring(docID));
  }
  lastDocID = docID;
  return true;
}

Comparable MultiSorter::ComparableProviderAnonymousInnerClass3::getComparable(
    int docID) 
{
  assert(docsInOrder(docID));
  int readerDocID = values->docID();
  if (readerDocID < docID) {
    readerDocID = values->advance(docID);
  }
  if (readerDocID == docID) {
    return reverseMul * static_cast<int>(values->longValue());
  } else {
    return reverseMul * missingValue;
  }
}

MultiSorter::ComparableProviderAnonymousInnerClass4::
    ComparableProviderAnonymousInnerClass4(
        int reverseMul, optional<double> &missingValue,
        shared_ptr<org::apache::lucene::index::NumericDocValues> values)
{
  this->reverseMul = reverseMul;
  this->missingValue = missingValue;
  this->values = values;
}

bool MultiSorter::ComparableProviderAnonymousInnerClass4::docsInOrder(int docID)
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(
        L"docs must be sent in order, but lastDocID=" + lastDocID +
        L" vs docID=" + to_wstring(docID));
  }
  lastDocID = docID;
  return true;
}

Comparable MultiSorter::ComparableProviderAnonymousInnerClass4::getComparable(
    int docID) 
{
  assert(docsInOrder(docID));
  int readerDocID = values->docID();
  if (readerDocID < docID) {
    readerDocID = values->advance(docID);
  }
  if (readerDocID == docID) {
    return reverseMul * Double::longBitsToDouble(values->longValue());
  } else {
    return reverseMul * missingValue;
  }
}

MultiSorter::ComparableProviderAnonymousInnerClass5::
    ComparableProviderAnonymousInnerClass5(
        int reverseMul, optional<float> &missingValue,
        shared_ptr<org::apache::lucene::index::NumericDocValues> values)
{
  this->reverseMul = reverseMul;
  this->missingValue = missingValue;
  this->values = values;
}

bool MultiSorter::ComparableProviderAnonymousInnerClass5::docsInOrder(int docID)
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(
        L"docs must be sent in order, but lastDocID=" + lastDocID +
        L" vs docID=" + to_wstring(docID));
  }
  lastDocID = docID;
  return true;
}

Comparable MultiSorter::ComparableProviderAnonymousInnerClass5::getComparable(
    int docID) 
{
  assert(docsInOrder(docID));
  int readerDocID = values->docID();
  if (readerDocID < docID) {
    readerDocID = values->advance(docID);
  }
  if (readerDocID == docID) {
    return reverseMul *
           Float::intBitsToFloat(static_cast<int>(values->longValue()));
  } else {
    return reverseMul * missingValue;
  }
}
} // namespace org::apache::lucene::index