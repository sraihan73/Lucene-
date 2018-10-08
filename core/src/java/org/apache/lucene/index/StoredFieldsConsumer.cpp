using namespace std;

#include "StoredFieldsConsumer.h"

namespace org::apache::lucene::index
{
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using IOContext = org::apache::lucene::store::IOContext;
using IOUtils = org::apache::lucene::util::IOUtils;

StoredFieldsConsumer::StoredFieldsConsumer(
    shared_ptr<DocumentsWriterPerThread> docWriter)
    : docWriter(docWriter)
{
  this->lastDoc = -1;
}

void StoredFieldsConsumer::initStoredFieldsWriter() 
{
  if (writer == nullptr) {
    this->writer = docWriter->codec->storedFieldsFormat()->fieldsWriter(
        docWriter->directory, docWriter->getSegmentInfo(), IOContext::DEFAULT);
  }
}

void StoredFieldsConsumer::startDocument(int docID) 
{
  assert(lastDoc < docID);
  initStoredFieldsWriter();
  while (++lastDoc < docID) {
    writer->startDocument();
    writer->finishDocument();
  }
  writer->startDocument();
}

void StoredFieldsConsumer::writeField(
    shared_ptr<FieldInfo> info,
    shared_ptr<IndexableField> field) 
{
  writer->writeField(info, field);
}

void StoredFieldsConsumer::finishDocument() 
{
  writer->finishDocument();
}

void StoredFieldsConsumer::finish(int maxDoc) 
{
  while (lastDoc < maxDoc - 1) {
    startDocument(lastDoc);
    finishDocument();
    ++lastDoc;
  }
}

void StoredFieldsConsumer::flush(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  try {
    writer->finish(state->fieldInfos, state->segmentInfo->maxDoc());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({writer});
    writer.reset();
  }
}

void StoredFieldsConsumer::abort()
{
  if (writer != nullptr) {
    IOUtils::closeWhileHandlingException({writer});
    writer.reset();
  }
}
} // namespace org::apache::lucene::index