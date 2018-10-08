using namespace std;

#include "StoredFieldsWriter.h"
#include "../analysis/Analyzer.h"
#include "../analysis/TokenStream.h"
#include "../document/StoredField.h"
#include "../index/FieldInfo.h"
#include "../index/FieldInfos.h"
#include "../index/IndexableFieldType.h"
#include "../index/MergeState.h"
#include "../util/BytesRef.h"
#include "StoredFieldsReader.h"

namespace org::apache::lucene::codecs
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StoredField = org::apache::lucene::document::StoredField;
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using MergeState = org::apache::lucene::index::MergeState;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using BytesRef = org::apache::lucene::util::BytesRef;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

StoredFieldsWriter::StoredFieldsWriter() {}

void StoredFieldsWriter::finishDocument()  {}

StoredFieldsWriter::StoredFieldsMergeSub::StoredFieldsMergeSub(
    shared_ptr<MergeVisitor> visitor, shared_ptr<MergeState::DocMap> docMap,
    shared_ptr<StoredFieldsReader> reader, int maxDoc)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), reader(reader),
      maxDoc(maxDoc), visitor(visitor)
{
}

int StoredFieldsWriter::StoredFieldsMergeSub::nextDoc()
{
  docID++;
  if (docID == maxDoc) {
    return NO_MORE_DOCS;
  } else {
    return docID;
  }
}

int StoredFieldsWriter::merge(shared_ptr<MergeState> mergeState) throw(
    IOException)
{
  deque<std::shared_ptr<StoredFieldsMergeSub>> subs =
      deque<std::shared_ptr<StoredFieldsMergeSub>>();
  for (int i = 0; i < mergeState->storedFieldsReaders.size(); i++) {
    shared_ptr<StoredFieldsReader> storedFieldsReader =
        mergeState->storedFieldsReaders[i];
    storedFieldsReader->checkIntegrity();
    subs.push_back(make_shared<StoredFieldsMergeSub>(
        make_shared<MergeVisitor>(shared_from_this(), mergeState, i),
        mergeState->docMaps[i], storedFieldsReader, mergeState->maxDocs[i]));
  }

  shared_ptr<DocIDMerger<std::shared_ptr<StoredFieldsMergeSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  int docCount = 0;
  while (true) {
    shared_ptr<StoredFieldsMergeSub> sub = docIDMerger->next();
    if (sub == nullptr) {
      break;
    }
    assert(sub->mappedDocID == docCount);
    startDocument();
    sub->reader.visitDocument(sub->docID, sub->visitor);
    finishDocument();
    docCount++;
  }
  finish(mergeState->mergeFieldInfos, docCount);
  return docCount;
}

StoredFieldsWriter::MergeVisitor::MergeVisitor(
    shared_ptr<StoredFieldsWriter> outerInstance,
    shared_ptr<MergeState> mergeState, int readerIndex)
    : outerInstance(outerInstance)
{
  // if field numbers are aligned, we can save hash lookups
  // on every field access. Otherwise, we need to lookup
  // fieldname each time, and remap to a new number.
  for (auto fi : mergeState->fieldInfos[readerIndex]) {
    shared_ptr<FieldInfo> other =
        mergeState->mergeFieldInfos->fieldInfo(fi->number);
    if (other == nullptr || other->name != fi->name) {
      remapper = mergeState->mergeFieldInfos;
      break;
    }
  }
}

void StoredFieldsWriter::MergeVisitor::binaryField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  reset(fieldInfo);
  // TODO: can we avoid new BR here?
  binaryValue_ = make_shared<BytesRef>(value);
  write();
}

void StoredFieldsWriter::MergeVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  reset(fieldInfo);
  // TODO: can we avoid new std::wstring here?
  stringValue_ = wstring(value, StandardCharsets::UTF_8);
  write();
}

void StoredFieldsWriter::MergeVisitor::intField(shared_ptr<FieldInfo> fieldInfo,
                                                int value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

void StoredFieldsWriter::MergeVisitor::longField(
    shared_ptr<FieldInfo> fieldInfo, int64_t value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

void StoredFieldsWriter::MergeVisitor::floatField(
    shared_ptr<FieldInfo> fieldInfo, float value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

void StoredFieldsWriter::MergeVisitor::doubleField(
    shared_ptr<FieldInfo> fieldInfo, double value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

Status StoredFieldsWriter::MergeVisitor::needsField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  return Status::YES;
}

wstring StoredFieldsWriter::MergeVisitor::name() { return currentField->name; }

shared_ptr<IndexableFieldType> StoredFieldsWriter::MergeVisitor::fieldType()
{
  return StoredField::TYPE;
}

shared_ptr<BytesRef> StoredFieldsWriter::MergeVisitor::binaryValue()
{
  return binaryValue_;
}

wstring StoredFieldsWriter::MergeVisitor::stringValue() { return stringValue_; }

shared_ptr<Number> StoredFieldsWriter::MergeVisitor::numericValue()
{
  return numericValue_;
}

shared_ptr<Reader> StoredFieldsWriter::MergeVisitor::readerValue()
{
  return nullptr;
}

shared_ptr<TokenStream>
StoredFieldsWriter::MergeVisitor::tokenStream(shared_ptr<Analyzer> analyzer,
                                              shared_ptr<TokenStream> reuse)
{
  return nullptr;
}

void StoredFieldsWriter::MergeVisitor::reset(shared_ptr<FieldInfo> field)
{
  if (remapper->size() > 0) {
    // field numbers are not aligned, we need to remap to the new field number
    currentField = remapper->fieldInfo(field->name);
  } else {
    currentField = field;
  }
  binaryValue_.reset();
  stringValue_ = L"";
  numericValue_.reset();
}

void StoredFieldsWriter::MergeVisitor::write() 
{
  outerInstance->writeField(currentField, shared_from_this());
}
} // namespace org::apache::lucene::codecs