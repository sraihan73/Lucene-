using namespace std;

#include "CrankyTermVectorsFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;

CrankyTermVectorsFormat::CrankyTermVectorsFormat(
    shared_ptr<TermVectorsFormat> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

shared_ptr<TermVectorsReader> CrankyTermVectorsFormat::vectorsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<IOContext> context) 
{
  return delegate_->vectorsReader(directory, segmentInfo, fieldInfos, context);
}

shared_ptr<TermVectorsWriter> CrankyTermVectorsFormat::vectorsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<IOContext> context) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsFormat.vectorsWriter()");
  }
  return make_shared<CrankyTermVectorsWriter>(
      delegate_->vectorsWriter(directory, segmentInfo, context), random);
}

CrankyTermVectorsFormat::CrankyTermVectorsWriter::CrankyTermVectorsWriter(
    shared_ptr<TermVectorsWriter> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

int CrankyTermVectorsFormat::CrankyTermVectorsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.merge()");
  }
  return TermVectorsWriter::merge(mergeState);
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::finish(
    shared_ptr<FieldInfos> fis, int numDocs) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.finish()");
  }
  delegate_->finish(fis, numDocs);
}

CrankyTermVectorsFormat::CrankyTermVectorsWriter::~CrankyTermVectorsWriter()
{
  delete delegate_;
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.close()");
  }
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::startDocument(
    int numVectorFields) 
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.startDocument()");
  }
  delegate_->startDocument(numVectorFields);
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::finishDocument() throw(
    IOException)
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.finishDocument()");
  }
  delegate_->finishDocument();
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::startField(
    shared_ptr<FieldInfo> info, int numTerms, bool positions, bool offsets,
    bool payloads) 
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.startField()");
  }
  delegate_->startField(info, numTerms, positions, offsets, payloads);
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::finishField() throw(
    IOException)
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.finishField()");
  }
  delegate_->finishField();
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::startTerm(
    shared_ptr<BytesRef> term, int freq) 
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.startTerm()");
  }
  delegate_->startTerm(term, freq);
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::finishTerm() throw(
    IOException)
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.finishTerm()");
  }
  delegate_->finishTerm();
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::addPosition(
    int position, int startOffset, int endOffset,
    shared_ptr<BytesRef> payload) 
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.addPosition()");
  }
  delegate_->addPosition(position, startOffset, endOffset, payload);
}

void CrankyTermVectorsFormat::CrankyTermVectorsWriter::addProx(
    int numProx, shared_ptr<DataInput> positions,
    shared_ptr<DataInput> offsets) 
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TermVectorsWriter.addProx()");
  }
  TermVectorsWriter::addProx(numProx, positions, offsets);
}
} // namespace org::apache::lucene::codecs::cranky