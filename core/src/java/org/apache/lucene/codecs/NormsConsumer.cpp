using namespace std;

#include "NormsConsumer.h"
#include "../index/FieldInfo.h"
#include "../index/MergeState.h"

namespace org::apache::lucene::codecs
{
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

NormsConsumer::NormsConsumer() {}

void NormsConsumer::merge(shared_ptr<MergeState> mergeState) 
{
  for (auto normsProducer : mergeState->normsProducers) {
    if (normsProducer != nullptr) {
      normsProducer->checkIntegrity();
    }
  }
  for (auto mergeFieldInfo : mergeState->mergeFieldInfos) {
    if (mergeFieldInfo->hasNorms()) {
      mergeNormsField(mergeFieldInfo, mergeState);
    }
  }
}

NormsConsumer::NumericDocValuesSub::NumericDocValuesSub(
    shared_ptr<MergeState::DocMap> docMap, shared_ptr<NumericDocValues> values)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), values(values)
{
  assert(values->docID() == -1);
}

int NormsConsumer::NumericDocValuesSub::nextDoc() 
{
  return values->nextDoc();
}

void NormsConsumer::mergeNormsField(
    shared_ptr<FieldInfo> mergeFieldInfo,
    shared_ptr<MergeState> mergeState) 
{

  // TODO: try to share code with default merge of DVConsumer by passing
  // MatchAllBits ?
  addNormsField(mergeFieldInfo,
                make_shared<NormsProducerAnonymousInnerClass>(
                    shared_from_this(), mergeFieldInfo, mergeState));
}

NormsConsumer::NormsProducerAnonymousInnerClass::
    NormsProducerAnonymousInnerClass(shared_ptr<NormsConsumer> outerInstance,
                                     shared_ptr<FieldInfo> mergeFieldInfo,
                                     shared_ptr<MergeState> mergeState)
{
  this->outerInstance = outerInstance;
  this->mergeFieldInfo = mergeFieldInfo;
  this->mergeState = mergeState;
}

shared_ptr<NumericDocValues>
NormsConsumer::NormsProducerAnonymousInnerClass::getNorms(
    shared_ptr<FieldInfo> fieldInfo) 
{
  if (fieldInfo != mergeFieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }

  deque<std::shared_ptr<NumericDocValuesSub>> subs =
      deque<std::shared_ptr<NumericDocValuesSub>>();
  assert(mergeState->docMaps.size() == mergeState->docValuesProducers.size());
  for (int i = 0; i < mergeState->docValuesProducers.size(); i++) {
    shared_ptr<NumericDocValues> norms = nullptr;
    shared_ptr<NormsProducer> normsProducer = mergeState->normsProducers[i];
    if (normsProducer != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(mergeFieldInfo->name);
      if (readerFieldInfo != nullptr && readerFieldInfo->hasNorms()) {
        norms = normsProducer->getNorms(readerFieldInfo);
      }
    }

    if (norms != nullptr) {
      subs.push_back(
          make_shared<NumericDocValuesSub>(mergeState->docMaps[i], norms));
    }
  }

  shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  return make_shared<NumericDocValuesAnonymousInnerClass>(shared_from_this(),
                                                          docIDMerger);
}

NormsConsumer::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::NumericDocValuesAnonymousInnerClass(
        shared_ptr<NormsProducerAnonymousInnerClass> outerInstance,
        shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
            docIDMerger)
{
  this->outerInstance = outerInstance;
  this->docIDMerger = docIDMerger;
  docID = -1;
}

int NormsConsumer::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int NormsConsumer::NormsProducerAnonymousInnerClass::
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

int NormsConsumer::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::advance(int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

bool NormsConsumer::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t NormsConsumer::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::cost()
{
  return 0;
}

int64_t NormsConsumer::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::longValue() 
{
  return current::values::longValue();
}

void NormsConsumer::NormsProducerAnonymousInnerClass::checkIntegrity() {}

NormsConsumer::NormsProducerAnonymousInnerClass::
    ~NormsProducerAnonymousInnerClass()
{
}

int64_t NormsConsumer::NormsProducerAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}
} // namespace org::apache::lucene::codecs