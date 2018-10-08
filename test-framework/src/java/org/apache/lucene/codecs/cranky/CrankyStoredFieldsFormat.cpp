using namespace std;

#include "CrankyStoredFieldsFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

CrankyStoredFieldsFormat::CrankyStoredFieldsFormat(
    shared_ptr<StoredFieldsFormat> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

shared_ptr<StoredFieldsReader> CrankyStoredFieldsFormat::fieldsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<FieldInfos> fn, shared_ptr<IOContext> context) 
{
  return delegate_->fieldsReader(directory, si, fn, context);
}

shared_ptr<StoredFieldsWriter> CrankyStoredFieldsFormat::fieldsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsFormat.fieldsWriter()");
  }
  return make_shared<CrankyStoredFieldsWriter>(
      delegate_->fieldsWriter(directory, si, context), random);
}

CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::CrankyStoredFieldsWriter(
    shared_ptr<StoredFieldsWriter> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

void CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::finish(
    shared_ptr<FieldInfos> fis, int numDocs) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsWriter.finish()");
  }
  delegate_->finish(fis, numDocs);
}

int CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsWriter.merge()");
  }
  return StoredFieldsWriter::merge(mergeState);
}

CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::~CrankyStoredFieldsWriter()
{
  delete delegate_;
  if (random->nextInt(1000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsWriter.close()");
  }
}

void CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::startDocument() throw(
    IOException)
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsWriter.startDocument()");
  }
  delegate_->startDocument();
}

void CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::finishDocument() throw(
    IOException)
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsWriter.finishDocument()");
  }
  delegate_->finishDocument();
}

void CrankyStoredFieldsFormat::CrankyStoredFieldsWriter::writeField(
    shared_ptr<FieldInfo> info,
    shared_ptr<IndexableField> field) 
{
  if (random->nextInt(10000) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from StoredFieldsWriter.writeField()");
  }
  delegate_->writeField(info, field);
}
} // namespace org::apache::lucene::codecs::cranky