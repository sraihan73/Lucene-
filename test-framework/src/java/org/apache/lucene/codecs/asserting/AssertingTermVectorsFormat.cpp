using namespace std;

#include "AssertingTermVectorsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using AssertingLeafReader = org::apache::lucene::index::AssertingLeafReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<TermVectorsReader> AssertingTermVectorsFormat::vectorsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<IOContext> context) 
{
  return make_shared<AssertingTermVectorsReader>(
      in_->vectorsReader(directory, segmentInfo, fieldInfos, context));
}

shared_ptr<TermVectorsWriter> AssertingTermVectorsFormat::vectorsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<IOContext> context) 
{
  return make_shared<AssertingTermVectorsWriter>(
      in_->vectorsWriter(directory, segmentInfo, context));
}

AssertingTermVectorsFormat::AssertingTermVectorsReader::
    AssertingTermVectorsReader(shared_ptr<TermVectorsReader> in_)
    : in_(in_)
{
  // do a few simple checks on init
  assert(toString() != L"");
  assert(ramBytesUsed() >= 0);
  assert(getChildResources() != nullptr);
}

AssertingTermVectorsFormat::AssertingTermVectorsReader::
    ~AssertingTermVectorsReader()
{
  delete in_;
  delete in_; // close again
}

shared_ptr<Fields> AssertingTermVectorsFormat::AssertingTermVectorsReader::get(
    int doc) 
{
  shared_ptr<Fields> fields = in_->get(doc);
  return fields->empty()
             ? nullptr
             : make_shared<AssertingLeafReader::AssertingFields>(fields);
}

shared_ptr<TermVectorsReader>
AssertingTermVectorsFormat::AssertingTermVectorsReader::clone()
{
  return make_shared<AssertingTermVectorsReader>(in_->clone());
}

int64_t AssertingTermVectorsFormat::AssertingTermVectorsReader::ramBytesUsed()
{
  int64_t v = in_->ramBytesUsed();
  assert(v >= 0);
  return v;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AssertingTermVectorsFormat::AssertingTermVectorsReader::getChildResources()
{
  shared_ptr<deque<std::shared_ptr<Accountable>>> res =
      in_->getChildResources();
  TestUtil::checkReadOnly(res);
  return res;
}

void AssertingTermVectorsFormat::AssertingTermVectorsReader::
    checkIntegrity() 
{
  in_->checkIntegrity();
}

shared_ptr<TermVectorsReader> AssertingTermVectorsFormat::
    AssertingTermVectorsReader::getMergeInstance() 
{
  return make_shared<AssertingTermVectorsReader>(in_->getMergeInstance());
}

wstring AssertingTermVectorsFormat::AssertingTermVectorsReader::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}

AssertingTermVectorsFormat::AssertingTermVectorsWriter::
    AssertingTermVectorsWriter(shared_ptr<TermVectorsWriter> in_)
    : in_(in_)
{
  docStatus = Status::UNDEFINED;
  fieldStatus = Status::UNDEFINED;
  termStatus = Status::UNDEFINED;
  fieldCount = termCount = positionCount = 0;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::startDocument(
    int numVectorFields) 
{
  assert(fieldCount == 0);
  assert(docStatus != Status::STARTED);
  in_->startDocument(numVectorFields);
  docStatus = Status::STARTED;
  fieldCount = numVectorFields;
  docCount++;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::
    finishDocument() 
{
  assert(fieldCount == 0);
  assert(docStatus == Status::STARTED);
  in_->finishDocument();
  docStatus = Status::FINISHED;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::startField(
    shared_ptr<FieldInfo> info, int numTerms, bool positions, bool offsets,
    bool payloads) 
{
  assert(termCount == 0);
  assert(docStatus == Status::STARTED);
  assert(fieldStatus != Status::STARTED);
  in_->startField(info, numTerms, positions, offsets, payloads);
  fieldStatus = Status::STARTED;
  termCount = numTerms;
  hasPositions = positions || offsets || payloads;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::
    finishField() 
{
  assert(termCount == 0);
  assert(fieldStatus == Status::STARTED);
  in_->finishField();
  fieldStatus = Status::FINISHED;
  --fieldCount;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::startTerm(
    shared_ptr<BytesRef> term, int freq) 
{
  assert(docStatus == Status::STARTED);
  assert(fieldStatus == Status::STARTED);
  assert(termStatus != Status::STARTED);
  in_->startTerm(term, freq);
  termStatus = Status::STARTED;
  positionCount = hasPositions ? freq : 0;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::finishTerm() throw(
    IOException)
{
  assert(positionCount == 0);
  assert(docStatus == Status::STARTED);
  assert(fieldStatus == Status::STARTED);
  assert(termStatus == Status::STARTED);
  in_->finishTerm();
  termStatus = Status::FINISHED;
  --termCount;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::addPosition(
    int position, int startOffset, int endOffset,
    shared_ptr<BytesRef> payload) 
{
  assert(docStatus == Status::STARTED);
  assert(fieldStatus == Status::STARTED);
  assert(termStatus == Status::STARTED);
  in_->addPosition(position, startOffset, endOffset, payload);
  --positionCount;
}

void AssertingTermVectorsFormat::AssertingTermVectorsWriter::finish(
    shared_ptr<FieldInfos> fis, int numDocs) 
{
  assert(docCount == numDocs);
      assert((docStatus == (numDocs > 0 ? Status::FINISHED, Status::UNDEFINED)));
      assert(fieldStatus != Status::STARTED);
      assert(termStatus != Status::STARTED);
      in_->finish(fis, numDocs);
}

AssertingTermVectorsFormat::AssertingTermVectorsWriter::
    ~AssertingTermVectorsWriter()
{
  delete in_;
  delete in_; // close again
}
} // namespace org::apache::lucene::codecs::asserting