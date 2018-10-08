using namespace std;

#include "TermVectorsConsumer.h"

namespace org::apache::lucene::index
{
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

TermVectorsConsumer::TermVectorsConsumer(
    shared_ptr<DocumentsWriterPerThread> docWriter)
    : TermsHash(docWriter, false, nullptr), docWriter(docWriter)
{
}

void TermVectorsConsumer::flush(
    unordered_map<wstring, std::shared_ptr<TermsHashPerField>> &fieldsToFlush,
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  if (writer != nullptr) {
    int numDocs = state->segmentInfo->maxDoc();
    assert(numDocs > 0);
    // At least one doc in this run had term vectors enabled
    try {
      fill(numDocs);
      assert(state->segmentInfo != nullptr);
      writer->finish(state->fieldInfos, numDocs);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::close({writer});
      writer.reset();
      lastDocID = 0;
      hasVectors = false;
    }
  }
}

void TermVectorsConsumer::fill(int docID) 
{
  while (lastDocID < docID) {
    writer->startDocument(0);
    writer->finishDocument();
    lastDocID++;
  }
}

void TermVectorsConsumer::initTermVectorsWriter() 
{
  if (writer == nullptr) {
    shared_ptr<IOContext> context =
        make_shared<IOContext>(make_shared<FlushInfo>(
            docWriter->getNumDocsInRAM(), docWriter->bytesUsed()));
    writer = docWriter->codec->termVectorsFormat()->vectorsWriter(
        docWriter->directory, docWriter->getSegmentInfo(), context);
    lastDocID = 0;
  }
}

void TermVectorsConsumer::finishDocument() 
{

  if (!hasVectors) {
    return;
  }

  // Fields in term vectors are UTF16 sorted:
  ArrayUtil::introSort(perFields, 0, numVectorFields);

  initTermVectorsWriter();

  fill(docState->docID);

  // Append term vectors to the real outputs:
  writer->startDocument(numVectorFields);
  for (int i = 0; i < numVectorFields; i++) {
    perFields[i]->finishDocument();
  }
  writer->finishDocument();

  assert((lastDocID == docState->docID, L"lastDocID=" + to_wstring(lastDocID) +
                                            L" docState.docID=" +
                                            to_wstring(docState->docID)));

  lastDocID++;

  TermsHash::reset();
  resetFields();
}

void TermVectorsConsumer::abort()
{
  hasVectors = false;
  try {
    TermsHash::abort();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({writer});
    writer.reset();
    lastDocID = 0;
    reset();
  }
}

void TermVectorsConsumer::resetFields()
{
  Arrays::fill(perFields, nullptr); // don't hang onto stuff from previous doc
  numVectorFields = 0;
}

shared_ptr<TermsHashPerField>
TermVectorsConsumer::addField(shared_ptr<FieldInvertState> invertState,
                              shared_ptr<FieldInfo> fieldInfo)
{
  return make_shared<TermVectorsConsumerPerField>(
      invertState, shared_from_this(), fieldInfo);
}

void TermVectorsConsumer::addFieldToFlush(
    shared_ptr<TermVectorsConsumerPerField> fieldToFlush)
{
  if (numVectorFields == perFields.size()) {
    int newSize = ArrayUtil::oversize(numVectorFields + 1,
                                      RamUsageEstimator::NUM_BYTES_OBJECT_REF);
    std::deque<std::shared_ptr<TermVectorsConsumerPerField>> newArray(newSize);
    System::arraycopy(perFields, 0, newArray, 0, numVectorFields);
    perFields = newArray;
  }

  perFields[numVectorFields++] = fieldToFlush;
}

void TermVectorsConsumer::startDocument()
{
  resetFields();
  numVectorFields = 0;
}
} // namespace org::apache::lucene::index