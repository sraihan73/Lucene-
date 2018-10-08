using namespace std;

#include "AssertingStoredFieldsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<StoredFieldsReader> AssertingStoredFieldsFormat::fieldsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<FieldInfos> fn, shared_ptr<IOContext> context) 
{
  return make_shared<AssertingStoredFieldsReader>(
      in_->fieldsReader(directory, si, fn, context), si->maxDoc());
}

shared_ptr<StoredFieldsWriter> AssertingStoredFieldsFormat::fieldsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  return make_shared<AssertingStoredFieldsWriter>(
      in_->fieldsWriter(directory, si, context));
}

AssertingStoredFieldsFormat::AssertingStoredFieldsReader::
    AssertingStoredFieldsReader(shared_ptr<StoredFieldsReader> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
  // do a few simple checks on init
  assert(toString() != L"");
  assert(ramBytesUsed() >= 0);
  assert(getChildResources() != nullptr);
}

AssertingStoredFieldsFormat::AssertingStoredFieldsReader::
    ~AssertingStoredFieldsReader()
{
  delete in_;
  delete in_; // close again
}

void AssertingStoredFieldsFormat::AssertingStoredFieldsReader::visitDocument(
    int n, shared_ptr<StoredFieldVisitor> visitor) 
{
  assert(n >= 0 && n < maxDoc);
  in_->visitDocument(n, visitor);
}

shared_ptr<StoredFieldsReader>
AssertingStoredFieldsFormat::AssertingStoredFieldsReader::clone()
{
  return make_shared<AssertingStoredFieldsReader>(in_->clone(), maxDoc);
}

int64_t
AssertingStoredFieldsFormat::AssertingStoredFieldsReader::ramBytesUsed()
{
  int64_t v = in_->ramBytesUsed();
  assert(v >= 0);
  return v;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AssertingStoredFieldsFormat::AssertingStoredFieldsReader::getChildResources()
{
  shared_ptr<deque<std::shared_ptr<Accountable>>> res =
      in_->getChildResources();
  TestUtil::checkReadOnly(res);
  return res;
}

void AssertingStoredFieldsFormat::AssertingStoredFieldsReader::
    checkIntegrity() 
{
  in_->checkIntegrity();
}

shared_ptr<StoredFieldsReader> AssertingStoredFieldsFormat::
    AssertingStoredFieldsReader::getMergeInstance() 
{
  return make_shared<AssertingStoredFieldsReader>(in_->getMergeInstance(),
                                                  maxDoc);
}

wstring AssertingStoredFieldsFormat::AssertingStoredFieldsReader::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}

AssertingStoredFieldsFormat::AssertingStoredFieldsWriter::
    AssertingStoredFieldsWriter(shared_ptr<StoredFieldsWriter> in_)
    : in_(in_)
{
  this->docStatus = Status::UNDEFINED;
}

void AssertingStoredFieldsFormat::AssertingStoredFieldsWriter::
    startDocument() 
{
  assert(docStatus != Status::STARTED);
  in_->startDocument();
  numWritten++;
  docStatus = Status::STARTED;
}

void AssertingStoredFieldsFormat::AssertingStoredFieldsWriter::
    finishDocument() 
{
  assert(docStatus == Status::STARTED);
  in_->finishDocument();
  docStatus = Status::FINISHED;
}

void AssertingStoredFieldsFormat::AssertingStoredFieldsWriter::writeField(
    shared_ptr<FieldInfo> info,
    shared_ptr<IndexableField> field) 
{
  assert(docStatus == Status::STARTED);
  in_->writeField(info, field);
}

void AssertingStoredFieldsFormat::AssertingStoredFieldsWriter::finish(
    shared_ptr<FieldInfos> fis, int numDocs) 
{
      assert((docStatus == (numDocs > 0 ? Status::FINISHED, Status::UNDEFINED)));
      in_->finish(fis, numDocs);
      assert(numDocs == numWritten);
}

AssertingStoredFieldsFormat::AssertingStoredFieldsWriter::
    ~AssertingStoredFieldsWriter()
{
  delete in_;
  delete in_; // close again
}
} // namespace org::apache::lucene::codecs::asserting