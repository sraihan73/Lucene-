using namespace std;

#include "SortingStoredFieldsConsumer.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using StoredField = org::apache::lucene::document::StoredField;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;

SortingStoredFieldsConsumer::SortingStoredFieldsConsumer(
    shared_ptr<DocumentsWriterPerThread> docWriter)
    : StoredFieldsConsumer(docWriter)
{
}

void SortingStoredFieldsConsumer::initStoredFieldsWriter() 
{
  if (writer == nullptr) {
    this->tmpDirectory =
        make_shared<TrackingTmpOutputDirectoryWrapper>(docWriter->directory);
    this->writer = docWriter->codec->storedFieldsFormat()->fieldsWriter(
        tmpDirectory, docWriter->getSegmentInfo(), IOContext::DEFAULT);
  }
}

void SortingStoredFieldsConsumer::flush(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  StoredFieldsConsumer::flush(state, sortMap);
  if (sortMap == nullptr) {
    // we're lucky the index is already sorted, just rename the temporary file
    // and return
    for (auto entry : tmpDirectory->getTemporaryFiles()) {
      tmpDirectory->rename(entry.second, entry.first);
    }
    return;
  }
  shared_ptr<StoredFieldsReader> reader =
      docWriter->codec->storedFieldsFormat()->fieldsReader(
          tmpDirectory, state->segmentInfo, state->fieldInfos,
          IOContext::DEFAULT);
  shared_ptr<StoredFieldsReader> mergeReader = reader->getMergeInstance();
  shared_ptr<StoredFieldsWriter> sortWriter =
      docWriter->codec->storedFieldsFormat()->fieldsWriter(
          state->directory, state->segmentInfo, IOContext::DEFAULT);
  try {
    reader->checkIntegrity();
    shared_ptr<CopyVisitor> visitor = make_shared<CopyVisitor>(sortWriter);
    for (int docID = 0; docID < state->segmentInfo->maxDoc(); docID++) {
      sortWriter->startDocument();
      mergeReader->visitDocument(sortMap->newToOld(docID), visitor);
      sortWriter->finishDocument();
    }
    sortWriter->finish(state->fieldInfos, state->segmentInfo->maxDoc());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({reader, sortWriter});
    IOUtils::deleteFiles(tmpDirectory,
                         tmpDirectory->getTemporaryFiles().values());
  }
}

void SortingStoredFieldsConsumer::abort()
{
  try {
    StoredFieldsConsumer::abort();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::deleteFilesIgnoringExceptions(
        {tmpDirectory, tmpDirectory->getTemporaryFiles().values()});
  }
}

SortingStoredFieldsConsumer::CopyVisitor::CopyVisitor(
    shared_ptr<StoredFieldsWriter> writer)
    : writer(writer)
{
}

void SortingStoredFieldsConsumer::CopyVisitor::binaryField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  reset(fieldInfo);
  // TODO: can we avoid new BR here?
  binaryValue_ = make_shared<BytesRef>(value);
  write();
}

void SortingStoredFieldsConsumer::CopyVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
  reset(fieldInfo);
  // TODO: can we avoid new std::wstring here?
  stringValue_ = wstring(value, StandardCharsets::UTF_8);
  write();
}

void SortingStoredFieldsConsumer::CopyVisitor::intField(
    shared_ptr<FieldInfo> fieldInfo, int value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

void SortingStoredFieldsConsumer::CopyVisitor::longField(
    shared_ptr<FieldInfo> fieldInfo, int64_t value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

void SortingStoredFieldsConsumer::CopyVisitor::floatField(
    shared_ptr<FieldInfo> fieldInfo, float value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

void SortingStoredFieldsConsumer::CopyVisitor::doubleField(
    shared_ptr<FieldInfo> fieldInfo, double value) 
{
  reset(fieldInfo);
  numericValue_ = value;
  write();
}

Status SortingStoredFieldsConsumer::CopyVisitor::needsField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  return Status::YES;
}

wstring SortingStoredFieldsConsumer::CopyVisitor::name()
{
  return currentField->name;
}

shared_ptr<IndexableFieldType>
SortingStoredFieldsConsumer::CopyVisitor::fieldType()
{
  return StoredField::TYPE;
}

shared_ptr<BytesRef> SortingStoredFieldsConsumer::CopyVisitor::binaryValue()
{
  return binaryValue_;
}

wstring SortingStoredFieldsConsumer::CopyVisitor::stringValue()
{
  return stringValue_;
}

shared_ptr<Number> SortingStoredFieldsConsumer::CopyVisitor::numericValue()
{
  return numericValue_;
}

shared_ptr<Reader> SortingStoredFieldsConsumer::CopyVisitor::readerValue()
{
  return nullptr;
}

shared_ptr<TokenStream> SortingStoredFieldsConsumer::CopyVisitor::tokenStream(
    shared_ptr<Analyzer> analyzer, shared_ptr<TokenStream> reuse)
{
  return nullptr;
}

void SortingStoredFieldsConsumer::CopyVisitor::reset(
    shared_ptr<FieldInfo> field)
{
  currentField = field;
  binaryValue_.reset();
  stringValue_ = L"";
  numericValue_.reset();
}

void SortingStoredFieldsConsumer::CopyVisitor::write() 
{
  writer->writeField(currentField, shared_from_this());
}
} // namespace org::apache::lucene::index