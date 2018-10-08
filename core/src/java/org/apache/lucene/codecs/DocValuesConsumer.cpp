using namespace std;

#include "DocValuesConsumer.h"
#include "../index/DocValues.h"
#include "../index/DocValuesType.h"
#include "../index/FieldInfo.h"
#include "../index/MergeState.h"
#include "../index/OrdinalMap.h"
#include "../index/TermsEnum.h"
#include "../util/Bits.h"
#include "../util/BytesRef.h"
#include "../util/LongBitSet.h"
#include "../util/LongValues.h"
#include "../util/packed/PackedInts.h"
#include "DocValuesProducer.h"

namespace org::apache::lucene::codecs
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using EmptyDocValuesProducer =
    org::apache::lucene::index::EmptyDocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

DocValuesConsumer::DocValuesConsumer() {}

void DocValuesConsumer::merge(shared_ptr<MergeState> mergeState) throw(
    IOException)
{
  for (auto docValuesProducer : mergeState->docValuesProducers) {
    if (docValuesProducer != nullptr) {
      docValuesProducer->checkIntegrity();
    }
  }

  for (auto mergeFieldInfo : mergeState->mergeFieldInfos) {
    DocValuesType type = mergeFieldInfo->getDocValuesType();
    if (type != DocValuesType::NONE) {
      if (type == DocValuesType::NUMERIC) {
        mergeNumericField(mergeFieldInfo, mergeState);
      } else if (type == DocValuesType::BINARY) {
        mergeBinaryField(mergeFieldInfo, mergeState);
      } else if (type == DocValuesType::SORTED) {
        mergeSortedField(mergeFieldInfo, mergeState);
      } else if (type == DocValuesType::SORTED_SET) {
        mergeSortedSetField(mergeFieldInfo, mergeState);
      } else if (type == DocValuesType::SORTED_NUMERIC) {
        mergeSortedNumericField(mergeFieldInfo, mergeState);
      } else {
        throw make_shared<AssertionError>(L"type=" + type);
      }
    }
  }
}

DocValuesConsumer::NumericDocValuesSub::NumericDocValuesSub(
    shared_ptr<MergeState::DocMap> docMap, shared_ptr<NumericDocValues> values)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), values(values)
{
  assert(values->docID() == -1);
}

int DocValuesConsumer::NumericDocValuesSub::nextDoc() 
{
  return values->nextDoc();
}

void DocValuesConsumer::mergeNumericField(
    shared_ptr<FieldInfo> mergeFieldInfo,
    shared_ptr<MergeState> mergeState) 
{
  addNumericField(mergeFieldInfo,
                  make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                      shared_from_this(), mergeFieldInfo, mergeState));
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> mergeFieldInfo, shared_ptr<MergeState> mergeState)
{
  this->outerInstance = outerInstance;
  this->mergeFieldInfo = mergeFieldInfo;
  this->mergeState = mergeState;
}

shared_ptr<NumericDocValues>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::getNumeric(
    shared_ptr<FieldInfo> fieldInfo) 
{
  if (fieldInfo != mergeFieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }

  deque<std::shared_ptr<NumericDocValuesSub>> subs =
      deque<std::shared_ptr<NumericDocValuesSub>>();
  assert(mergeState->docMaps.size() == mergeState->docValuesProducers.size());
  int64_t cost = 0;
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<NumericDocValues> values = nullptr;
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(mergeFieldInfo->name);
      if (readerFieldInfo != nullptr &&
          readerFieldInfo->getDocValuesType() == DocValuesType::NUMERIC) {
        values = docValuesProducer->getNumeric(readerFieldInfo);
      }
    }
    if (values != nullptr) {
      cost += values->cost();
      subs.push_back(
          make_shared<NumericDocValuesSub>(mergeState->docMaps[i], values));
    }
  }

  shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  constexpr int64_t finalCost = cost;

  return make_shared<NumericDocValuesAnonymousInnerClass>(
      shared_from_this(), docIDMerger, finalCost);
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::NumericDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
        shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
            docIDMerger,
        int64_t finalCost)
{
  this->outerInstance = outerInstance;
  this->docIDMerger = docIDMerger;
  this->finalCost = finalCost;
  docID = -1;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::nextDoc() 
{
  current = docIDMerger->next();
  if (current == nullptr) {
    docID = NO_MORE_DOCS;
  } else {
    docID = current::mappedDocID;
  }
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::advance(int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

bool DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::cost()
{
  return finalCost;
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::longValue() 
{
  return current::values::longValue();
}

DocValuesConsumer::BinaryDocValuesSub::BinaryDocValuesSub(
    shared_ptr<MergeState::DocMap> docMap, shared_ptr<BinaryDocValues> values)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), values(values)
{
  assert(values->docID() == -1);
}

int DocValuesConsumer::BinaryDocValuesSub::nextDoc() 
{
  return values->nextDoc();
}

void DocValuesConsumer::mergeBinaryField(
    shared_ptr<FieldInfo> mergeFieldInfo,
    shared_ptr<MergeState> mergeState) 
{
  addBinaryField(mergeFieldInfo,
                 make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                     shared_from_this(), mergeFieldInfo, mergeState));
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> mergeFieldInfo, shared_ptr<MergeState> mergeState)
    : outerInstance(outerInstance)
{
  this->mergeFieldInfo = mergeFieldInfo;
  this->mergeState = mergeState;
}

shared_ptr<BinaryDocValues>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::getBinary(
    shared_ptr<FieldInfo> fieldInfo) 
{
  if (fieldInfo != mergeFieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }

  deque<std::shared_ptr<BinaryDocValuesSub>> subs =
      deque<std::shared_ptr<BinaryDocValuesSub>>();

  int64_t cost = 0;
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<BinaryDocValues> values = nullptr;
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(mergeFieldInfo->name);
      if (readerFieldInfo != nullptr &&
          readerFieldInfo->getDocValuesType() == DocValuesType::BINARY) {
        values = docValuesProducer->getBinary(readerFieldInfo);
      }
    }
    if (values != nullptr) {
      cost += values->cost();
      subs.push_back(
          make_shared<BinaryDocValuesSub>(mergeState->docMaps[i], values));
    }
  }

  shared_ptr<DocIDMerger<std::shared_ptr<BinaryDocValuesSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);
  constexpr int64_t finalCost = cost;

  return make_shared<BinaryDocValuesAnonymousInnerClass>(
      shared_from_this(), docIDMerger, finalCost);
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::BinaryDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
        shared_ptr<DocIDMerger<std::shared_ptr<BinaryDocValuesSub>>>
            docIDMerger,
        int64_t finalCost)
{
  this->outerInstance = outerInstance;
  this->docIDMerger = docIDMerger;
  this->finalCost = finalCost;
  docID = -1;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::nextDoc() 
{
  current = docIDMerger->next();
  if (current == nullptr) {
    docID = NO_MORE_DOCS;
  } else {
    docID = current::mappedDocID;
  }
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::advance(int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

bool DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::cost()
{
  return finalCost;
}

shared_ptr<BytesRef>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::binaryValue() 
{
  return current::values::binaryValue();
}

DocValuesConsumer::SortedNumericDocValuesSub::SortedNumericDocValuesSub(
    shared_ptr<MergeState::DocMap> docMap,
    shared_ptr<SortedNumericDocValues> values)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), values(values)
{
  assert(values->docID() == -1);
}

int DocValuesConsumer::SortedNumericDocValuesSub::nextDoc() 
{
  return values->nextDoc();
}

void DocValuesConsumer::mergeSortedNumericField(
    shared_ptr<FieldInfo> mergeFieldInfo,
    shared_ptr<MergeState> mergeState) 
{

  addSortedNumericField(mergeFieldInfo,
                        make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                            shared_from_this(), mergeFieldInfo, mergeState));
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> mergeFieldInfo, shared_ptr<MergeState> mergeState)
    : outerInstance(outerInstance)
{
  this->mergeFieldInfo = mergeFieldInfo;
  this->mergeState = mergeState;
}

shared_ptr<SortedNumericDocValues>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::getSortedNumeric(
    shared_ptr<FieldInfo> fieldInfo) 
{
  if (fieldInfo != mergeFieldInfo) {
    throw invalid_argument(L"wrong FieldInfo");
  }

  // We must make new iterators + DocIDMerger for each iterator:
  deque<std::shared_ptr<SortedNumericDocValuesSub>> subs =
      deque<std::shared_ptr<SortedNumericDocValuesSub>>();
  int64_t cost = 0;
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    shared_ptr<SortedNumericDocValues> values = nullptr;
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(mergeFieldInfo->name);
      if (readerFieldInfo != nullptr && readerFieldInfo->getDocValuesType() ==
                                            DocValuesType::SORTED_NUMERIC) {
        values = docValuesProducer->getSortedNumeric(readerFieldInfo);
      }
    }
    if (values == nullptr) {
      values = DocValues::emptySortedNumeric(mergeState->maxDocs[i]);
    }
    cost += values->cost();
    subs.push_back(
        make_shared<SortedNumericDocValuesSub>(mergeState->docMaps[i], values));
  }

  constexpr int64_t finalCost = cost;

  shared_ptr<DocIDMerger<std::shared_ptr<SortedNumericDocValuesSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  return make_shared<SortedNumericDocValuesAnonymousInnerClass>(
      shared_from_this(), finalCost, docIDMerger);
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::
        SortedNumericDocValuesAnonymousInnerClass(
            shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
            int64_t finalCost,
            shared_ptr<DocIDMerger<std::shared_ptr<SortedNumericDocValuesSub>>>
                docIDMerger)
{
  this->outerInstance = outerInstance;
  this->finalCost = finalCost;
  this->docIDMerger = docIDMerger;
  docID = -1;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::nextDoc() 
{
  currentSub = docIDMerger->next();
  if (currentSub == nullptr) {
    docID = NO_MORE_DOCS;
  } else {
    docID = currentSub::mappedDocID;
  }

  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::advance(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

bool DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::docValueCount()
{
  return currentSub::values::docValueCount();
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return finalCost;
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass::nextValue() 
{
  return currentSub::values::nextValue();
}

DocValuesConsumer::SortedDocValuesSub::SortedDocValuesSub(
    shared_ptr<MergeState::DocMap> docMap, shared_ptr<SortedDocValues> values,
    shared_ptr<LongValues> map_obj)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), values(values),
      map_obj(map_obj)
{
  assert(values->docID() == -1);
}

int DocValuesConsumer::SortedDocValuesSub::nextDoc() 
{
  return values->nextDoc();
}

void DocValuesConsumer::mergeSortedField(
    shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<MergeState> mergeState) 
{
  deque<std::shared_ptr<SortedDocValues>> toMerge =
      deque<std::shared_ptr<SortedDocValues>>();
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<SortedDocValues> values = nullptr;
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(fieldInfo->name);
      if (readerFieldInfo != nullptr &&
          readerFieldInfo->getDocValuesType() == DocValuesType::SORTED) {
        values = docValuesProducer->getSorted(fieldInfo);
      }
    }
    if (values == nullptr) {
      values = DocValues::emptySorted();
    }
    toMerge.push_back(values);
  }

  constexpr int numReaders = toMerge.size();
  std::deque<std::shared_ptr<SortedDocValues>> dvs = toMerge.toArray(
      std::deque<std::shared_ptr<SortedDocValues>>(numReaders));

  // step 1: iterate thru each sub and mark terms still in use
  std::deque<std::shared_ptr<TermsEnum>> liveTerms(dvs.size());
  std::deque<int64_t> weights(liveTerms.size());
  for (int sub = 0; sub < numReaders; sub++) {
    shared_ptr<SortedDocValues> dv = dvs[sub];
    shared_ptr<Bits> liveDocs = mergeState->liveDocs[sub];
    if (liveDocs == nullptr) {
      liveTerms[sub] = dv->termsEnum();
      weights[sub] = dv->getValueCount();
    } else {
      shared_ptr<LongBitSet> bitset =
          make_shared<LongBitSet>(dv->getValueCount());
      int docID;
      while ((docID = dv->nextDoc()) != NO_MORE_DOCS) {
        if (liveDocs->get(docID)) {
          int ord = dv->ordValue();
          if (ord >= 0) {
            bitset->set(ord);
          }
        }
      }
      liveTerms[sub] =
          make_shared<BitsFilteredTermsEnum>(dv->termsEnum(), bitset);
      weights[sub] = bitset->cardinality();
    }
  }

  // step 2: create ordinal map_obj (this conceptually does the "merging")
  shared_ptr<OrdinalMap> *const map_obj =
      OrdinalMap::build(nullptr, liveTerms, weights, PackedInts::COMPACT);

  // step 3: add field
  addSortedField(fieldInfo,
                 make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                     shared_from_this(), fieldInfo, mergeState, dvs, map_obj));
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> fieldInfo, shared_ptr<MergeState> mergeState,
        deque<std::shared_ptr<SortedDocValues>> &dvs,
        shared_ptr<OrdinalMap> map_obj)
    : outerInstance(outerInstance)
{
  this->fieldInfo = fieldInfo;
  this->mergeState = mergeState;
  this->dvs = dvs;
  this->map_obj = map_obj;
}

shared_ptr<SortedDocValues>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::getSorted(
    shared_ptr<FieldInfo> fieldInfoIn) 
{
  if (fieldInfoIn != fieldInfo) {
    throw invalid_argument(L"wrong FieldInfo");
  }

  // We must make new iterators + DocIDMerger for each iterator:

  deque<std::shared_ptr<SortedDocValuesSub>> subs =
      deque<std::shared_ptr<SortedDocValuesSub>>();
  int64_t cost = 0;
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<SortedDocValues> values = nullptr;
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(fieldInfo->name);
      if (readerFieldInfo != nullptr &&
          readerFieldInfo->getDocValuesType() == DocValuesType::SORTED) {
        values = docValuesProducer->getSorted(readerFieldInfo);
      }
    }
    if (values == nullptr) {
      values = DocValues::emptySorted();
    }
    cost += values->cost();

    subs.push_back(make_shared<SortedDocValuesSub>(
        mergeState->docMaps[i], values, map_obj->getGlobalOrds(i)));
  }

  constexpr int64_t finalCost = cost;

  shared_ptr<DocIDMerger<std::shared_ptr<SortedDocValuesSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  return make_shared<SortedDocValuesAnonymousInnerClass>(
      shared_from_this(), finalCost, docIDMerger);
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::SortedDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
        int64_t finalCost,
        shared_ptr<DocIDMerger<std::shared_ptr<SortedDocValuesSub>>>
            docIDMerger)
{
  this->outerInstance = outerInstance;
  this->finalCost = finalCost;
  this->docIDMerger = docIDMerger;
  docID = -1;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::nextDoc() 
{
  shared_ptr<SortedDocValuesSub> sub = docIDMerger->next();
  if (sub == nullptr) {
    return docID = NO_MORE_DOCS;
  }
  int subOrd = sub->values->ordValue();
  assert(subOrd != -1);
  ord = static_cast<int>(sub->map_obj->get(subOrd));
  docID = sub->mappedDocID;
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::ordValue()
{
  return ord;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::cost()
{
  return finalCost;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::getValueCount()
{
  return static_cast<int>(outerInstance->map_obj.getValueCount());
}

shared_ptr<BytesRef>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass::lookupOrd(int ord) 
{
  int segmentNumber = outerInstance->map_obj.getFirstSegmentNumber(ord);
  int segmentOrd = static_cast<int>(outerInstance->map_obj.getFirstSegmentOrd(ord));
  return outerInstance->dvs[segmentNumber].lookupOrd(segmentOrd);
}

DocValuesConsumer::SortedSetDocValuesSub::SortedSetDocValuesSub(
    shared_ptr<MergeState::DocMap> docMap,
    shared_ptr<SortedSetDocValues> values, shared_ptr<LongValues> map_obj)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), values(values),
      map_obj(map_obj)
{
  assert(values->docID() == -1);
}

int DocValuesConsumer::SortedSetDocValuesSub::nextDoc() 
{
  return values->nextDoc();
}

wstring DocValuesConsumer::SortedSetDocValuesSub::toString()
{
  return L"SortedSetDocValuesSub(mappedDocID=" + to_wstring(mappedDocID) +
         L" values=" + values + L")";
}

void DocValuesConsumer::mergeSortedSetField(
    shared_ptr<FieldInfo> mergeFieldInfo,
    shared_ptr<MergeState> mergeState) 
{

  deque<std::shared_ptr<SortedSetDocValues>> toMerge =
      deque<std::shared_ptr<SortedSetDocValues>>();
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<SortedSetDocValues> values = nullptr;
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> fieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(mergeFieldInfo->name);
      if (fieldInfo != nullptr &&
          fieldInfo->getDocValuesType() == DocValuesType::SORTED_SET) {
        values = docValuesProducer->getSortedSet(fieldInfo);
      }
    }
    if (values == nullptr) {
      values = DocValues::emptySortedSet();
    }
    toMerge.push_back(values);
  }

  // step 1: iterate thru each sub and mark terms still in use
  std::deque<std::shared_ptr<TermsEnum>> liveTerms(toMerge.size());
  std::deque<int64_t> weights(liveTerms.size());
  for (int sub = 0; sub < liveTerms.size(); sub++) {
    shared_ptr<SortedSetDocValues> dv = toMerge[sub];
    shared_ptr<Bits> liveDocs = mergeState->liveDocs[sub];
    if (liveDocs == nullptr) {
      liveTerms[sub] = dv->termsEnum();
      weights[sub] = dv->getValueCount();
    } else {
      shared_ptr<LongBitSet> bitset =
          make_shared<LongBitSet>(dv->getValueCount());
      int docID;
      while ((docID = dv->nextDoc()) != NO_MORE_DOCS) {
        if (liveDocs->get(docID)) {
          int64_t ord;
          while ((ord = dv->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
            bitset->set(ord);
          }
        }
      }
      liveTerms[sub] =
          make_shared<BitsFilteredTermsEnum>(dv->termsEnum(), bitset);
      weights[sub] = bitset->cardinality();
    }
  }

  // step 2: create ordinal map_obj (this conceptually does the "merging")
  shared_ptr<OrdinalMap> *const map_obj =
      OrdinalMap::build(nullptr, liveTerms, weights, PackedInts::COMPACT);

  // step 3: add field
  addSortedSetField(
      mergeFieldInfo,
      make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
          shared_from_this(), mergeFieldInfo, mergeState, toMerge, map_obj));
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> mergeFieldInfo, shared_ptr<MergeState> mergeState,
        deque<std::shared_ptr<SortedSetDocValues>> &toMerge,
        shared_ptr<OrdinalMap> map_obj)
    : outerInstance(outerInstance)
{
  this->mergeFieldInfo = mergeFieldInfo;
  this->mergeState = mergeState;
  this->toMerge = toMerge;
  this->map_obj = map_obj;
}

shared_ptr<SortedSetDocValues>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::getSortedSet(
    shared_ptr<FieldInfo> fieldInfo) 
{
  if (fieldInfo != mergeFieldInfo) {
    throw invalid_argument(L"wrong FieldInfo");
  }

  // We must make new iterators + DocIDMerger for each iterator:
  deque<std::shared_ptr<SortedSetDocValuesSub>> subs =
      deque<std::shared_ptr<SortedSetDocValuesSub>>();

  int64_t cost = 0;

  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<SortedSetDocValues> values = nullptr;
    shared_ptr<DocValuesProducer> docValuesProducer =
        mergeState->docValuesProducers[i];
    if (docValuesProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(mergeFieldInfo->name);
      if (readerFieldInfo != nullptr &&
          readerFieldInfo->getDocValuesType() == DocValuesType::SORTED_SET) {
        values = docValuesProducer->getSortedSet(readerFieldInfo);
      }
    }
    if (values == nullptr) {
      values = DocValues::emptySortedSet();
    }
    cost += values->cost();
    subs.push_back(make_shared<SortedSetDocValuesSub>(
        mergeState->docMaps[i], values, map_obj->getGlobalOrds(i)));
  }

  shared_ptr<DocIDMerger<std::shared_ptr<SortedSetDocValuesSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  constexpr int64_t finalCost = cost;

  return make_shared<SortedSetDocValuesAnonymousInnerClass>(
      shared_from_this(), docIDMerger, finalCost);
}

DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::
        SortedSetDocValuesAnonymousInnerClass(
            shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
            shared_ptr<DocIDMerger<std::shared_ptr<SortedSetDocValuesSub>>>
                docIDMerger,
            int64_t finalCost)
{
  this->outerInstance = outerInstance;
  this->docIDMerger = docIDMerger;
  this->finalCost = finalCost;
  docID = -1;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::nextDoc() 
{
  currentSub = docIDMerger->next();
  if (currentSub == nullptr) {
    docID = NO_MORE_DOCS;
  } else {
    docID = currentSub::mappedDocID;
  }

  return docID;
}

int DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::advance(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

bool DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::nextOrd() 
{
  int64_t subOrd = currentSub::values::nextOrd();
  if (subOrd == SortedSetDocValues::NO_MORE_ORDS) {
    return SortedSetDocValues::NO_MORE_ORDS;
  }
  return currentSub::map_obj->get(subOrd);
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::cost()
{
  return finalCost;
}

shared_ptr<BytesRef>
DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::lookupOrd(int64_t ord) throw(
        IOException)
{
  int segmentNumber = outerInstance->map_obj.getFirstSegmentNumber(ord);
  int64_t segmentOrd = outerInstance->map_obj.getFirstSegmentOrd(ord);
  return outerInstance->toMerge[segmentNumber].lookupOrd(segmentOrd);
}

int64_t DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass::getValueCount()
{
  return outerInstance->map_obj.getValueCount();
}

DocValuesConsumer::BitsFilteredTermsEnum::BitsFilteredTermsEnum(
    shared_ptr<TermsEnum> in_, shared_ptr<LongBitSet> liveTerms)
    : org::apache::lucene::index::FilteredTermsEnum(in_, false),
      liveTerms(liveTerms)
{
  assert(liveTerms != nullptr);
}

AcceptStatus DocValuesConsumer::BitsFilteredTermsEnum::accept(
    shared_ptr<BytesRef> term) 
{
  if (liveTerms->get(ord())) {
    return AcceptStatus::YES;
  } else {
    return AcceptStatus::NO;
  }
}

bool DocValuesConsumer::isSingleValued(
    deque<std::shared_ptr<Number>> &docToValueCount)
{
  for (auto count : docToValueCount) {
    if (count->longValue() > 1) {
      return false;
    }
  }
  return true;
}

deque<std::shared_ptr<Number>> DocValuesConsumer::singletonView(
    deque<std::shared_ptr<Number>> &docToValueCount,
    deque<std::shared_ptr<Number>> &values, shared_ptr<Number> missingValue)
{
  assert(isSingleValued(docToValueCount));
  return make_shared<IterableAnonymousInnerClass>(docToValueCount, values,
                                                  missingValue);
}

DocValuesConsumer::IterableAnonymousInnerClass::IterableAnonymousInnerClass(
    deque<std::shared_ptr<Number>> &docToValueCount,
    deque<std::shared_ptr<Number>> &values, shared_ptr<Number> missingValue)
{
  this->docToValueCount = docToValueCount;
  this->values = values;
  this->missingValue = missingValue;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
DocValuesConsumer::IterableAnonymousInnerClass::iterator()
{
  constexpr deque<std::shared_ptr<Number>>::const_iterator countIterator =
      docToValueCount.begin();
  constexpr deque<std::shared_ptr<Number>>::const_iterator valuesIterator =
      values.begin();
  return make_shared<IteratorAnonymousInnerClass>(
      shared_from_this(), countIterator, valuesIterator);
}

DocValuesConsumer::IterableAnonymousInnerClass::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass> outerInstance,
        deque<Number>::const_iterator countIterator,
        deque<Number>::const_iterator valuesIterator)
{
  this->outerInstance = outerInstance;
  this->countIterator = countIterator;
  this->valuesIterator = valuesIterator;
}

bool DocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return countIterator.hasNext();
}

shared_ptr<Number> DocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  int count = countIterator.next().intValue();
  if (count == 0) {
    return outerInstance->missingValue;
  } else {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    return valuesIterator.next();
  }
}

void DocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs