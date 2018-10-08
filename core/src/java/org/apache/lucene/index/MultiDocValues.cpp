using namespace std;

#include "MultiDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

MultiDocValues::MultiDocValues() {}

shared_ptr<NumericDocValues>
MultiDocValues::getNormValues(shared_ptr<IndexReader> r,
                              const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  constexpr int size = leaves.size();
  if (size == 0) {
    return nullptr;
  } else if (size == 1) {
    return leaves[0]->reader().getNormValues(field);
  }
  shared_ptr<FieldInfo> fi =
      MultiFields::getMergedFieldInfos(r)->fieldInfo(field);
  if (fi == nullptr || fi->hasNorms() == false) {
    return nullptr;
  }

  return make_shared<NumericDocValuesAnonymousInnerClass>(field, leaves);
}

MultiDocValues::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        const wstring &field,
        deque<std::shared_ptr<LeafReaderContext>> &leaves)
{
  this->field = field;
  this->leaves = leaves;
}

int MultiDocValues::NumericDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  while (true) {
    if (currentValues == nullptr) {
      if (nextLeaf == leaves.size()) {
        docID = NO_MORE_DOCS;
        return docID;
      }
      currentLeaf = leaves[nextLeaf];
      currentValues = currentLeaf::reader().getNormValues(field);
      nextLeaf++;
      continue;
    }

    int newDocID = currentValues::nextDoc();

    if (newDocID == NO_MORE_DOCS) {
      currentValues = nullptr;
      continue;
    } else {
      docID = currentLeaf::docBase + newDocID;
      return docID;
    }
  }
}

int MultiDocValues::NumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int MultiDocValues::NumericDocValuesAnonymousInnerClass::advance(
    int targetDocID) 
{
  if (targetDocID <= docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      currentValues = nullptr;
      docID = NO_MORE_DOCS;
      return docID;
    }
    currentLeaf = leaves[readerIndex];
    currentValues = currentLeaf::reader().getNormValues(field);
    if (currentValues == nullptr) {
      return nextDoc();
    }
    nextLeaf = readerIndex + 1;
  }
  int newDocID = currentValues::advance(targetDocID - currentLeaf::docBase);
  if (newDocID == NO_MORE_DOCS) {
    currentValues = nullptr;
    return nextDoc();
  } else {
    docID = currentLeaf::docBase + newDocID;
    return docID;
  }
}

bool MultiDocValues::NumericDocValuesAnonymousInnerClass::advanceExact(
    int targetDocID) 
{
  if (targetDocID < docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      throw invalid_argument(L"Out of range: " + to_wstring(targetDocID));
    }
    currentLeaf = leaves[readerIndex];
    currentValues = currentLeaf::reader().getNormValues(field);
    nextLeaf = readerIndex + 1;
  }
  docID = targetDocID;
  if (currentValues == nullptr) {
    return false;
  }
  return currentValues::advanceExact(targetDocID - currentLeaf::docBase);
}

int64_t
MultiDocValues::NumericDocValuesAnonymousInnerClass::longValue() throw(
    IOException)
{
  return currentValues::longValue();
}

int64_t MultiDocValues::NumericDocValuesAnonymousInnerClass::cost()
{
  // TODO
  return 0;
}

shared_ptr<NumericDocValues>
MultiDocValues::getNumericValues(shared_ptr<IndexReader> r,
                                 const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  constexpr int size = leaves.size();
  if (size == 0) {
    return nullptr;
  } else if (size == 1) {
    return leaves[0]->reader().getNumericDocValues(field);
  }

  bool anyReal = false;
  for (auto leaf : leaves) {
    shared_ptr<FieldInfo> fieldInfo =
        leaf->reader()->getFieldInfos()->fieldInfo(field);
    if (fieldInfo != nullptr) {
      DocValuesType dvType = fieldInfo->getDocValuesType();
      if (dvType == DocValuesType::NUMERIC) {
        anyReal = true;
        break;
      }
    }
  }

  if (anyReal == false) {
    return nullptr;
  }

  return make_shared<NumericDocValuesAnonymousInnerClass2>(field, leaves);
}

MultiDocValues::NumericDocValuesAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass2(
        const wstring &field,
        deque<std::shared_ptr<LeafReaderContext>> &leaves)
{
  this->field = field;
  this->leaves = leaves;
}

int MultiDocValues::NumericDocValuesAnonymousInnerClass2::docID()
{
  return docID;
}

int MultiDocValues::NumericDocValuesAnonymousInnerClass2::nextDoc() throw(
    IOException)
{
  while (true) {
    while (currentValues == nullptr) {
      if (nextLeaf == leaves.size()) {
        docID = NO_MORE_DOCS;
        return docID;
      }
      currentLeaf = leaves[nextLeaf];
      currentValues = currentLeaf::reader().getNumericDocValues(field);
      nextLeaf++;
    }

    int newDocID = currentValues::nextDoc();

    if (newDocID == NO_MORE_DOCS) {
      currentValues = nullptr;
      continue;
    } else {
      docID = currentLeaf::docBase + newDocID;
      return docID;
    }
  }
}

int MultiDocValues::NumericDocValuesAnonymousInnerClass2::advance(
    int targetDocID) 
{
  if (targetDocID <= docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      currentValues = nullptr;
      docID = NO_MORE_DOCS;
      return docID;
    }
    currentLeaf = leaves[readerIndex];
    currentValues = currentLeaf::reader().getNumericDocValues(field);
    nextLeaf = readerIndex + 1;
    if (currentValues == nullptr) {
      return nextDoc();
    }
  }
  int newDocID = currentValues::advance(targetDocID - currentLeaf::docBase);
  if (newDocID == NO_MORE_DOCS) {
    currentValues = nullptr;
    return nextDoc();
  } else {
    docID = currentLeaf::docBase + newDocID;
    return docID;
  }
}

bool MultiDocValues::NumericDocValuesAnonymousInnerClass2::advanceExact(
    int targetDocID) 
{
  if (targetDocID < docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      throw invalid_argument(L"Out of range: " + to_wstring(targetDocID));
    }
    currentLeaf = leaves[readerIndex];
    currentValues = currentLeaf::reader().getNumericDocValues(field);
    nextLeaf = readerIndex + 1;
  }
  docID = targetDocID;
  if (currentValues == nullptr) {
    return false;
  }
  return currentValues::advanceExact(targetDocID - currentLeaf::docBase);
}

int64_t
MultiDocValues::NumericDocValuesAnonymousInnerClass2::longValue() throw(
    IOException)
{
  return currentValues::longValue();
}

int64_t MultiDocValues::NumericDocValuesAnonymousInnerClass2::cost()
{
  // TODO
  return 0;
}

shared_ptr<BinaryDocValues>
MultiDocValues::getBinaryValues(shared_ptr<IndexReader> r,
                                const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  constexpr int size = leaves.size();
  if (size == 0) {
    return nullptr;
  } else if (size == 1) {
    return leaves[0]->reader().getBinaryDocValues(field);
  }

  bool anyReal = false;
  for (auto leaf : leaves) {
    shared_ptr<FieldInfo> fieldInfo =
        leaf->reader()->getFieldInfos()->fieldInfo(field);
    if (fieldInfo != nullptr) {
      DocValuesType dvType = fieldInfo->getDocValuesType();
      if (dvType == DocValuesType::BINARY) {
        anyReal = true;
        break;
      }
    }
  }

  if (anyReal == false) {
    return nullptr;
  }

  return make_shared<BinaryDocValuesAnonymousInnerClass>(field, leaves);
}

MultiDocValues::BinaryDocValuesAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass(
        const wstring &field,
        deque<std::shared_ptr<LeafReaderContext>> &leaves)
{
  this->field = field;
  this->leaves = leaves;
}

int MultiDocValues::BinaryDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  while (true) {
    while (currentValues == nullptr) {
      if (nextLeaf == leaves.size()) {
        docID = NO_MORE_DOCS;
        return docID;
      }
      currentLeaf = leaves[nextLeaf];
      currentValues = currentLeaf::reader().getBinaryDocValues(field);
      nextLeaf++;
    }

    int newDocID = currentValues::nextDoc();

    if (newDocID == NO_MORE_DOCS) {
      currentValues = nullptr;
      continue;
    } else {
      docID = currentLeaf::docBase + newDocID;
      return docID;
    }
  }
}

int MultiDocValues::BinaryDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int MultiDocValues::BinaryDocValuesAnonymousInnerClass::advance(
    int targetDocID) 
{
  if (targetDocID <= docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      currentValues = nullptr;
      docID = NO_MORE_DOCS;
      return docID;
    }
    currentLeaf = leaves[readerIndex];
    currentValues = currentLeaf::reader().getBinaryDocValues(field);
    nextLeaf = readerIndex + 1;
    if (currentValues == nullptr) {
      return nextDoc();
    }
  }
  int newDocID = currentValues::advance(targetDocID - currentLeaf::docBase);
  if (newDocID == NO_MORE_DOCS) {
    currentValues = nullptr;
    return nextDoc();
  } else {
    docID = currentLeaf::docBase + newDocID;
    return docID;
  }
}

bool MultiDocValues::BinaryDocValuesAnonymousInnerClass::advanceExact(
    int targetDocID) 
{
  if (targetDocID < docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      throw invalid_argument(L"Out of range: " + to_wstring(targetDocID));
    }
    currentLeaf = leaves[readerIndex];
    currentValues = currentLeaf::reader().getBinaryDocValues(field);
    nextLeaf = readerIndex + 1;
  }
  docID = targetDocID;
  if (currentValues == nullptr) {
    return false;
  }
  return currentValues::advanceExact(targetDocID - currentLeaf::docBase);
}

shared_ptr<BytesRef>
MultiDocValues::BinaryDocValuesAnonymousInnerClass::binaryValue() throw(
    IOException)
{
  return currentValues::binaryValue();
}

int64_t MultiDocValues::BinaryDocValuesAnonymousInnerClass::cost()
{
  // TODO
  return 0;
}

shared_ptr<SortedNumericDocValues>
MultiDocValues::getSortedNumericValues(shared_ptr<IndexReader> r,
                                       const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  constexpr int size = leaves.size();
  if (size == 0) {
    return nullptr;
  } else if (size == 1) {
    return leaves[0]->reader().getSortedNumericDocValues(field);
  }

  bool anyReal = false;
  std::deque<std::shared_ptr<SortedNumericDocValues>> values(size);
  const std::deque<int> starts = std::deque<int>(size + 1);
  int64_t totalCost = 0;
  for (int i = 0; i < size; i++) {
    shared_ptr<LeafReaderContext> context = leaves[i];
    shared_ptr<SortedNumericDocValues> v =
        context->reader()->getSortedNumericDocValues(field);
    if (v == nullptr) {
      v = DocValues::emptySortedNumeric(context->reader()->maxDoc());
    } else {
      anyReal = true;
    }
    values[i] = v;
    starts[i] = context->docBase;
    totalCost += v->cost();
  }
  starts[size] = r->maxDoc();

  if (anyReal == false) {
    return nullptr;
  }

  constexpr int64_t finalTotalCost = totalCost;

  return make_shared<SortedNumericDocValuesAnonymousInnerClass>(leaves, values,
                                                                finalTotalCost);
}

MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass(
        deque<std::shared_ptr<LeafReaderContext>> &leaves,
        deque<
            std::shared_ptr<org::apache::lucene::index::SortedNumericDocValues>>
            &values,
        int64_t finalTotalCost)
{
  this->leaves = leaves;
  this->values = values;
  this->finalTotalCost = finalTotalCost;
}

int MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  while (true) {
    if (currentValues == nullptr) {
      if (nextLeaf == leaves.size()) {
        docID = NO_MORE_DOCS;
        return docID;
      }
      currentLeaf = leaves[nextLeaf];
      currentValues = values[nextLeaf];
      nextLeaf++;
    }

    int newDocID = currentValues::nextDoc();

    if (newDocID == NO_MORE_DOCS) {
      currentValues = nullptr;
      continue;
    } else {
      docID = currentLeaf::docBase + newDocID;
      return docID;
    }
  }
}

int MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::advance(
    int targetDocID) 
{
  if (targetDocID <= docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      currentValues = nullptr;
      docID = NO_MORE_DOCS;
      return docID;
    }
    currentLeaf = leaves[readerIndex];
    currentValues = values[readerIndex];
    nextLeaf = readerIndex + 1;
  }
  int newDocID = currentValues::advance(targetDocID - currentLeaf::docBase);
  if (newDocID == NO_MORE_DOCS) {
    currentValues = nullptr;
    return nextDoc();
  } else {
    docID = currentLeaf::docBase + newDocID;
    return docID;
  }
}

bool MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::advanceExact(
    int targetDocID) 
{
  if (targetDocID < docID) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" + docID +
        L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, leaves);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == leaves.size()) {
      throw invalid_argument(L"Out of range: " + to_wstring(targetDocID));
    }
    currentLeaf = leaves[readerIndex];
    currentValues = values[readerIndex];
    nextLeaf = readerIndex + 1;
  }
  docID = targetDocID;
  if (currentValues == nullptr) {
    return false;
  }
  return currentValues::advanceExact(targetDocID - currentLeaf::docBase);
}

int64_t MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return finalTotalCost;
}

int MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::docValueCount()
{
  return currentValues::docValueCount();
}

int64_t
MultiDocValues::SortedNumericDocValuesAnonymousInnerClass::nextValue() throw(
    IOException)
{
  return currentValues::nextValue();
}

shared_ptr<SortedDocValues>
MultiDocValues::getSortedValues(shared_ptr<IndexReader> r,
                                const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  constexpr int size = leaves.size();

  if (size == 0) {
    return nullptr;
  } else if (size == 1) {
    return leaves[0]->reader().getSortedDocValues(field);
  }

  bool anyReal = false;
  std::deque<std::shared_ptr<SortedDocValues>> values(size);
  const std::deque<int> starts = std::deque<int>(size + 1);
  int64_t totalCost = 0;
  for (int i = 0; i < size; i++) {
    shared_ptr<LeafReaderContext> context = leaves[i];
    shared_ptr<SortedDocValues> v =
        context->reader()->getSortedDocValues(field);
    if (v == nullptr) {
      v = DocValues::emptySorted();
    } else {
      anyReal = true;
      totalCost += v->cost();
    }
    values[i] = v;
    starts[i] = context->docBase;
  }
  starts[size] = r->maxDoc();

  if (anyReal == false) {
    return nullptr;
  } else {
    shared_ptr<IndexReader::CacheHelper> cacheHelper =
        r->getReaderCacheHelper();
    shared_ptr<IndexReader::CacheKey> owner =
        cacheHelper == nullptr ? nullptr : cacheHelper->getKey();
    shared_ptr<OrdinalMap> mapping =
        OrdinalMap::build(owner, values, PackedInts::DEFAULT);
    return make_shared<MultiSortedDocValues>(values, starts, mapping,
                                             totalCost);
  }
}

shared_ptr<SortedSetDocValues>
MultiDocValues::getSortedSetValues(shared_ptr<IndexReader> r,
                                   const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  constexpr int size = leaves.size();

  if (size == 0) {
    return nullptr;
  } else if (size == 1) {
    return leaves[0]->reader().getSortedSetDocValues(field);
  }

  bool anyReal = false;
  std::deque<std::shared_ptr<SortedSetDocValues>> values(size);
  const std::deque<int> starts = std::deque<int>(size + 1);
  int64_t totalCost = 0;
  for (int i = 0; i < size; i++) {
    shared_ptr<LeafReaderContext> context = leaves[i];
    shared_ptr<SortedSetDocValues> v =
        context->reader()->getSortedSetDocValues(field);
    if (v == nullptr) {
      v = DocValues::emptySortedSet();
    } else {
      anyReal = true;
      totalCost += v->cost();
    }
    values[i] = v;
    starts[i] = context->docBase;
  }
  starts[size] = r->maxDoc();

  if (anyReal == false) {
    return nullptr;
  } else {
    shared_ptr<IndexReader::CacheHelper> cacheHelper =
        r->getReaderCacheHelper();
    shared_ptr<IndexReader::CacheKey> owner =
        cacheHelper == nullptr ? nullptr : cacheHelper->getKey();
    shared_ptr<OrdinalMap> mapping =
        OrdinalMap::build(owner, values, PackedInts::DEFAULT);
    return make_shared<MultiSortedSetDocValues>(values, starts, mapping,
                                                totalCost);
  }
}

MultiDocValues::MultiSortedDocValues::MultiSortedDocValues(
    std::deque<std::shared_ptr<SortedDocValues>> &values,
    std::deque<int> &docStarts, shared_ptr<OrdinalMap> mapping,
    int64_t totalCost) 
    : docStarts(docStarts), values(values), mapping(mapping),
      totalCost(totalCost)
{
  assert(docStarts.size() == values.size() + 1);
}

int MultiDocValues::MultiSortedDocValues::docID() { return docID_; }

int MultiDocValues::MultiSortedDocValues::nextDoc() 
{
  while (true) {
    while (currentValues == nullptr) {
      if (nextLeaf == values.size()) {
        docID_ = NO_MORE_DOCS;
        return docID_;
      }
      currentDocStart = docStarts[nextLeaf];
      currentValues = values[nextLeaf];
      nextLeaf++;
    }

    int newDocID = currentValues->nextDoc();

    if (newDocID == NO_MORE_DOCS) {
      currentValues.reset();
      continue;
    } else {
      docID_ = currentDocStart + newDocID;
      return docID_;
    }
  }
}

int MultiDocValues::MultiSortedDocValues::advance(int targetDocID) throw(
    IOException)
{
  if (targetDocID <= docID_) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" +
        to_wstring(docID_) + L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, docStarts);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == values.size()) {
      currentValues.reset();
      docID_ = NO_MORE_DOCS;
      return docID_;
    }
    currentDocStart = docStarts[readerIndex];
    currentValues = values[readerIndex];
    nextLeaf = readerIndex + 1;
  }
  int newDocID = currentValues->advance(targetDocID - currentDocStart);
  if (newDocID == NO_MORE_DOCS) {
    currentValues.reset();
    return nextDoc();
  } else {
    docID_ = currentDocStart + newDocID;
    return docID_;
  }
}

bool MultiDocValues::MultiSortedDocValues::advanceExact(int targetDocID) throw(
    IOException)
{
  if (targetDocID < docID_) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" +
        to_wstring(docID_) + L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, docStarts);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == values.size()) {
      throw invalid_argument(L"Out of range: " + to_wstring(targetDocID));
    }
    currentDocStart = docStarts[readerIndex];
    currentValues = values[readerIndex];
    nextLeaf = readerIndex + 1;
  }
  docID_ = targetDocID;
  if (currentValues == nullptr) {
    return false;
  }
  return currentValues->advanceExact(targetDocID - currentDocStart);
}

int MultiDocValues::MultiSortedDocValues::ordValue() 
{
  return static_cast<int>(
      mapping->getGlobalOrds(nextLeaf - 1)->get(currentValues->ordValue()));
}

shared_ptr<BytesRef>
MultiDocValues::MultiSortedDocValues::lookupOrd(int ord) 
{
  int subIndex = mapping->getFirstSegmentNumber(ord);
  int segmentOrd = static_cast<int>(mapping->getFirstSegmentOrd(ord));
  return values[subIndex]->lookupOrd(segmentOrd);
}

int MultiDocValues::MultiSortedDocValues::getValueCount()
{
  return static_cast<int>(mapping->getValueCount());
}

int64_t MultiDocValues::MultiSortedDocValues::cost() { return totalCost; }

MultiDocValues::MultiSortedSetDocValues::MultiSortedSetDocValues(
    std::deque<std::shared_ptr<SortedSetDocValues>> &values,
    std::deque<int> &docStarts, shared_ptr<OrdinalMap> mapping,
    int64_t totalCost) 
    : docStarts(docStarts), values(values), mapping(mapping),
      totalCost(totalCost)
{
  assert(docStarts.size() == values.size() + 1);
}

int MultiDocValues::MultiSortedSetDocValues::docID() { return docID_; }

int MultiDocValues::MultiSortedSetDocValues::nextDoc() 
{
  while (true) {
    while (currentValues == nullptr) {
      if (nextLeaf == values.size()) {
        docID_ = NO_MORE_DOCS;
        return docID_;
      }
      currentDocStart = docStarts[nextLeaf];
      currentValues = values[nextLeaf];
      nextLeaf++;
    }

    int newDocID = currentValues->nextDoc();

    if (newDocID == NO_MORE_DOCS) {
      currentValues.reset();
      continue;
    } else {
      docID_ = currentDocStart + newDocID;
      return docID_;
    }
  }
}

int MultiDocValues::MultiSortedSetDocValues::advance(int targetDocID) throw(
    IOException)
{
  if (targetDocID <= docID_) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" +
        to_wstring(docID_) + L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, docStarts);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == values.size()) {
      currentValues.reset();
      docID_ = NO_MORE_DOCS;
      return docID_;
    }
    currentDocStart = docStarts[readerIndex];
    currentValues = values[readerIndex];
    nextLeaf = readerIndex + 1;
  }
  int newDocID = currentValues->advance(targetDocID - currentDocStart);
  if (newDocID == NO_MORE_DOCS) {
    currentValues.reset();
    return nextDoc();
  } else {
    docID_ = currentDocStart + newDocID;
    return docID_;
  }
}

bool MultiDocValues::MultiSortedSetDocValues::advanceExact(
    int targetDocID) 
{
  if (targetDocID < docID_) {
    throw invalid_argument(
        L"can only advance beyond current document: on docID=" +
        to_wstring(docID_) + L" but targetDocID=" + to_wstring(targetDocID));
  }
  int readerIndex = ReaderUtil::subIndex(targetDocID, docStarts);
  if (readerIndex >= nextLeaf) {
    if (readerIndex == values.size()) {
      throw invalid_argument(L"Out of range: " + to_wstring(targetDocID));
    }
    currentDocStart = docStarts[readerIndex];
    currentValues = values[readerIndex];
    nextLeaf = readerIndex + 1;
  }
  docID_ = targetDocID;
  if (currentValues == nullptr) {
    return false;
  }
  return currentValues->advanceExact(targetDocID - currentDocStart);
}

int64_t MultiDocValues::MultiSortedSetDocValues::nextOrd() 
{
  int64_t segmentOrd = currentValues->nextOrd();
  if (segmentOrd == NO_MORE_ORDS) {
    return segmentOrd;
  } else {
    return mapping->getGlobalOrds(nextLeaf - 1)->get(segmentOrd);
  }
}

shared_ptr<BytesRef> MultiDocValues::MultiSortedSetDocValues::lookupOrd(
    int64_t ord) 
{
  int subIndex = mapping->getFirstSegmentNumber(ord);
  int64_t segmentOrd = mapping->getFirstSegmentOrd(ord);
  return values[subIndex]->lookupOrd(segmentOrd);
}

int64_t MultiDocValues::MultiSortedSetDocValues::getValueCount()
{
  return mapping->getValueCount();
}

int64_t MultiDocValues::MultiSortedSetDocValues::cost() { return totalCost; }
} // namespace org::apache::lucene::index