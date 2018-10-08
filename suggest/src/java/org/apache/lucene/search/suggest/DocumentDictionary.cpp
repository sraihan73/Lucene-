using namespace std;

#include "DocumentDictionary.h"

namespace org::apache::lucene::search::suggest
{
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using MultiFields = org::apache::lucene::index::MultiFields;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

DocumentDictionary::DocumentDictionary(shared_ptr<IndexReader> reader,
                                       const wstring &field,
                                       const wstring &weightField)
    : DocumentDictionary(reader, field, weightField, nullptr)
{
}

DocumentDictionary::DocumentDictionary(shared_ptr<IndexReader> reader,
                                       const wstring &field,
                                       const wstring &weightField,
                                       const wstring &payloadField)
    : DocumentDictionary(reader, field, weightField, payloadField, nullptr)
{
}

DocumentDictionary::DocumentDictionary(shared_ptr<IndexReader> reader,
                                       const wstring &field,
                                       const wstring &weightField,
                                       const wstring &payloadField,
                                       const wstring &contextsField)
    : reader(reader), payloadField(payloadField), contextsField(contextsField),
      field(field), weightField(weightField)
{
}

shared_ptr<InputIterator>
DocumentDictionary::getEntryIterator() 
{
  return make_shared<DocumentInputIterator>(
      shared_from_this(), payloadField != L"", contextsField != L"");
}

DocumentDictionary::DocumentInputIterator::DocumentInputIterator(
    shared_ptr<DocumentDictionary> outerInstance, bool hasPayloads,
    bool hasContexts) 
    : docCount(outerInstance->reader->maxDoc() - 1),
      relevantFields(getRelevantFields(std::deque<wstring>{
          outerInstance->field, outerInstance->weightField,
          outerInstance->payloadField, outerInstance->contextsField})),
      hasPayloads(hasPayloads), hasContexts(hasContexts),
      liveDocs((outerInstance->reader->leaves().size() > 0)
                   ? MultiFields::getLiveDocs(outerInstance->reader)
                   : nullptr),
      weightValues((outerInstance->weightField != L"")
                       ? MultiDocValues::getNumericValues(
                             outerInstance->reader, outerInstance->weightField)
                       : nullptr),
      outerInstance(outerInstance)
{
}

int64_t DocumentDictionary::DocumentInputIterator::weight()
{
  return currentWeight;
}

shared_ptr<BytesRef>
DocumentDictionary::DocumentInputIterator::next() 
{
  while (true) {
    if (nextFieldsPosition < currentDocFields.size()) {
      // Still values left from the document
      shared_ptr<IndexableField> fieldValue =
          currentDocFields[nextFieldsPosition++];
      if (fieldValue->binaryValue() != nullptr) {
        return fieldValue->binaryValue();
      } else if (fieldValue->stringValue() != L"") {
        return make_shared<BytesRef>(fieldValue->stringValue());
      } else {
        continue;
      }
    }

    if (currentDocId == docCount) {
      // Iterated over all the documents.
      break;
    }

    currentDocId++;
    if (liveDocs != nullptr && !liveDocs->get(currentDocId)) {
      continue;
    }

    shared_ptr<Document> doc =
        outerInstance->reader->document(currentDocId, relevantFields);

    shared_ptr<BytesRef> tempPayload = nullptr;
    if (hasPayloads_) {
      shared_ptr<IndexableField> payload =
          doc->getField(outerInstance->payloadField);
      if (payload != nullptr) {
        if (payload->binaryValue() != nullptr) {
          tempPayload = payload->binaryValue();
        } else if (payload->stringValue() != L"") {
          tempPayload = make_shared<BytesRef>(payload->stringValue());
        }
      }
      // in case that the iterator has payloads configured, use empty values
      // instead of null for payload
      if (tempPayload == nullptr) {
        tempPayload = make_shared<BytesRef>();
      }
    }

    shared_ptr<Set<std::shared_ptr<BytesRef>>> tempContexts;
    if (hasContexts_) {
      tempContexts = unordered_set<>();
      std::deque<std::shared_ptr<IndexableField>> contextFields =
          doc->getFields(outerInstance->contextsField);
      for (auto contextField : contextFields) {
        if (contextField->binaryValue() != nullptr) {
          tempContexts->add(contextField->binaryValue());
        } else if (contextField->stringValue() != L"") {
          tempContexts->add(make_shared<BytesRef>(contextField->stringValue()));
        } else {
          continue;
        }
      }
    } else {
      tempContexts = Collections::emptySet();
    }

    currentDocFields = doc->getFields(outerInstance->field);
    nextFieldsPosition = 0;
    if (currentDocFields.empty()) { // no values in this document
      continue;
    }
    shared_ptr<IndexableField> fieldValue =
        currentDocFields[nextFieldsPosition++];
    shared_ptr<BytesRef> tempTerm;
    if (fieldValue->binaryValue() != nullptr) {
      tempTerm = fieldValue->binaryValue();
    } else if (fieldValue->stringValue() != L"") {
      tempTerm = make_shared<BytesRef>(fieldValue->stringValue());
    } else {
      continue;
    }

    currentPayload = tempPayload;
    currentContexts = tempContexts;
    currentWeight = getWeight(doc, currentDocId);

    return tempTerm;
  }

  return nullptr;
}

shared_ptr<BytesRef> DocumentDictionary::DocumentInputIterator::payload()
{
  return currentPayload;
}

bool DocumentDictionary::DocumentInputIterator::hasPayloads()
{
  return hasPayloads_;
}

int64_t DocumentDictionary::DocumentInputIterator::getWeight(
    shared_ptr<Document> doc, int docId) 
{
  shared_ptr<IndexableField> weight = doc->getField(outerInstance->weightField);
  if (weight != nullptr) { // found weight as stored
    return (weight->numericValue() != nullptr)
               ? weight->numericValue()->longValue()
               : 0;
  } else if (weightValues != nullptr) { // found weight as NumericDocValue
    if (weightValues->docID() < docId) {
      weightValues->advance(docId);
    }
    if (weightValues->docID() == docId) {
      return weightValues->longValue();
    } else {
      // missing
      return 0;
    }
  } else { // fall back
    return 0;
  }
}

shared_ptr<Set<wstring>>
DocumentDictionary::DocumentInputIterator::getRelevantFields(
    deque<wstring> &fields)
{
  shared_ptr<Set<wstring>> relevantFields = unordered_set<wstring>();
  for (wstring relevantField : fields) {
    if (relevantField != L"") {
      relevantFields->add(relevantField);
    }
  }
  return relevantFields;
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
DocumentDictionary::DocumentInputIterator::contexts()
{
  if (hasContexts_) {
    return currentContexts;
  }
  return nullptr;
}

bool DocumentDictionary::DocumentInputIterator::hasContexts()
{
  return hasContexts_;
}
} // namespace org::apache::lucene::search::suggest