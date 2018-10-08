using namespace std;

#include "DocumentValueSourceDictionary.h"

namespace org::apache::lucene::search::suggest
{
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;

DocumentValueSourceDictionary::DocumentValueSourceDictionary(
    shared_ptr<IndexReader> reader, const wstring &field,
    shared_ptr<LongValuesSource> weightsValueSource, const wstring &payload,
    const wstring &contexts)
    : DocumentDictionary(reader, field, nullptr, payload, contexts),
      weightsValueSource(weightsValueSource)
{
}

DocumentValueSourceDictionary::DocumentValueSourceDictionary(
    shared_ptr<IndexReader> reader, const wstring &field,
    shared_ptr<LongValuesSource> weightsValueSource, const wstring &payload)
    : DocumentDictionary(reader, field, nullptr, payload),
      weightsValueSource(weightsValueSource)
{
}

DocumentValueSourceDictionary::DocumentValueSourceDictionary(
    shared_ptr<IndexReader> reader, const wstring &field,
    shared_ptr<LongValuesSource> weightsValueSource)
    : DocumentDictionary(reader, field, nullptr, nullptr),
      weightsValueSource(weightsValueSource)
{
}

shared_ptr<InputIterator>
DocumentValueSourceDictionary::getEntryIterator() 
{
  return make_shared<DocumentValueSourceInputIterator>(
      shared_from_this(), payloadField != L"", contextsField != L"");
}

DocumentValueSourceDictionary::DocumentValueSourceInputIterator::
    DocumentValueSourceInputIterator(
        shared_ptr<DocumentValueSourceDictionary> outerInstance,
        bool hasPayloads, bool hasContexts) 
    : DocumentDictionary::DocumentInputIterator(outerInstance, hasPayloads,
                                                hasContexts),
      leaves(outerInstance->reader->leaves()),
      starts(std::deque<int>(leaves.size() + 1)), outerInstance(outerInstance)
{
  for (int i = 0; i < leaves.size(); i++) {
    starts[i] = leaves[i]->docBase;
  }
  starts[leaves.size()] = outerInstance->reader->maxDoc();
  currentWeightValues = (leaves.size() > 0)
                            ? outerInstance->weightsValueSource->getValues(
                                  leaves[currentLeafIndex], nullptr)
                            : nullptr;
}

int64_t
DocumentValueSourceDictionary::DocumentValueSourceInputIterator::getWeight(
    shared_ptr<Document> doc, int docId) 
{
  if (currentWeightValues == nullptr) {
    return 0;
  }
  int subIndex = ReaderUtil::subIndex(docId, starts);
  if (subIndex != currentLeafIndex) {
    currentLeafIndex = subIndex;
    currentWeightValues = outerInstance->weightsValueSource->getValues(
        leaves[currentLeafIndex], nullptr);
  }
  if (currentWeightValues->advanceExact(docId - starts[subIndex])) {
    return currentWeightValues->longValue();
  } else {
    return 0;
  }
}
} // namespace org::apache::lucene::search::suggest