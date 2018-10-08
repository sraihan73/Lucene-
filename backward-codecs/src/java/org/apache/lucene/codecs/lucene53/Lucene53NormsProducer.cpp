using namespace std;

#include "Lucene53NormsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RandomAccessInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::codecs::lucene53
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using IOUtils = org::apache::lucene::util::IOUtils;
//    import static
//    org.apache.lucene.codecs.lucene53.Lucene53NormsFormat.VERSION_CURRENT;
//    import static
//    org.apache.lucene.codecs.lucene53.Lucene53NormsFormat.VERSION_START;

Lucene53NormsProducer::Lucene53NormsProducer(
    shared_ptr<SegmentReadState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : data(state->directory->openInput(dataName, state->context)),
      maxDoc(state->segmentInfo->maxDoc())
{
  wstring metaName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, metaExtension);
  int version = -1;

  // read in the entries from the metadata file.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // in = state.directory.openChecksumInput(metaName, state.context))
  {
    org::apache::lucene::store::ChecksumIndexInput in_ =
        state->directory->openChecksumInput(metaName, state->context);
    runtime_error priorE = nullptr;
    try {
      version = CodecUtil::checkIndexHeader(
          in_, metaCodec, VERSION_START, VERSION_CURRENT,
          state->segmentInfo->getId(), state->segmentSuffix);
      readFields(in_, state->fieldInfos);
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(in_, priorE);
    }
  }

  wstring dataName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, dataExtension);
  bool success = false;
  try {
    constexpr int version2 = CodecUtil::checkIndexHeader(
        data, dataCodec, VERSION_START, VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    if (version != version2) {
      throw make_shared<CorruptIndexException>(
          L"Format versions mismatch: meta=" + to_wstring(version) + L",data=" +
              to_wstring(version2),
          data);
    }

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(data);

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({this->data});
    }
  }
}

void Lucene53NormsProducer::readFields(
    shared_ptr<IndexInput> meta,
    shared_ptr<FieldInfos> infos) 
{
  int fieldNumber = meta->readVInt();
  while (fieldNumber != -1) {
    shared_ptr<FieldInfo> info = infos->fieldInfo(fieldNumber);
    if (info == nullptr) {
      throw make_shared<CorruptIndexException>(
          L"Invalid field number: " + to_wstring(fieldNumber), meta);
    } else if (!info->hasNorms()) {
      throw make_shared<CorruptIndexException>(L"Invalid field: " + info->name,
                                               meta);
    }
    shared_ptr<NormsEntry> entry = make_shared<NormsEntry>();
    entry->bytesPerValue = meta->readByte();
    switch (entry->bytesPerValue) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 8:
      break;
    default:
      throw make_shared<CorruptIndexException>(
          L"Invalid bytesPerValue: " + to_wstring(entry->bytesPerValue) +
              L", field: " + info->name,
          meta);
    }
    entry->offset = meta->readLong();
    norms.emplace(info->number, entry);
    fieldNumber = meta->readVInt();
  }
}

shared_ptr<NumericDocValues>
Lucene53NormsProducer::getNorms(shared_ptr<FieldInfo> field) 
{
  shared_ptr<NormsEntry> *const entry = norms[field->number];

  if (entry->bytesPerValue == 0) {
    constexpr int64_t value = entry->offset;
    return make_shared<NormsIteratorAnonymousInnerClass>(shared_from_this(),
                                                         maxDoc, value);
  } else {
    shared_ptr<RandomAccessInput> slice;
    {
      lock_guard<mutex> lock(data);
      switch (entry->bytesPerValue) {
      case 1:
        slice = data->randomAccessSlice(entry->offset, maxDoc);
        return make_shared<NormsIteratorAnonymousInnerClass2>(
            shared_from_this(), maxDoc, slice);
      case 2:
        slice = data->randomAccessSlice(entry->offset, maxDoc * 2LL);
        return make_shared<NormsIteratorAnonymousInnerClass3>(
            shared_from_this(), maxDoc, slice);
      case 4:
        slice = data->randomAccessSlice(entry->offset, maxDoc * 4LL);
        return make_shared<NormsIteratorAnonymousInnerClass4>(
            shared_from_this(), maxDoc, slice);
      case 8:
        slice = data->randomAccessSlice(entry->offset, maxDoc * 8LL);
        return make_shared<NormsIteratorAnonymousInnerClass5>(
            shared_from_this(), maxDoc, slice);
      default:
        throw make_shared<AssertionError>();
      }
    }
  }
}

Lucene53NormsProducer::NormsIteratorAnonymousInnerClass::
    NormsIteratorAnonymousInnerClass(
        shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        int64_t value)
    : NormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->value = value;
}

int64_t Lucene53NormsProducer::NormsIteratorAnonymousInnerClass::longValue()
{
  return value;
}

Lucene53NormsProducer::NormsIteratorAnonymousInnerClass2::
    NormsIteratorAnonymousInnerClass2(
        shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : NormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t
Lucene53NormsProducer::NormsIteratorAnonymousInnerClass2::longValue() throw(
    IOException)
{
  return slice->readByte(docID);
}

Lucene53NormsProducer::NormsIteratorAnonymousInnerClass3::
    NormsIteratorAnonymousInnerClass3(
        shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : NormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t
Lucene53NormsProducer::NormsIteratorAnonymousInnerClass3::longValue() throw(
    IOException)
{
  return slice->readShort((static_cast<int64_t>(docID)) << 1LL);
}

Lucene53NormsProducer::NormsIteratorAnonymousInnerClass4::
    NormsIteratorAnonymousInnerClass4(
        shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : NormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t
Lucene53NormsProducer::NormsIteratorAnonymousInnerClass4::longValue() throw(
    IOException)
{
  return slice->readInt((static_cast<int64_t>(docID)) << 2LL);
}

Lucene53NormsProducer::NormsIteratorAnonymousInnerClass5::
    NormsIteratorAnonymousInnerClass5(
        shared_ptr<Lucene53NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : NormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t
Lucene53NormsProducer::NormsIteratorAnonymousInnerClass5::longValue() throw(
    IOException)
{
  return slice->readLong((static_cast<int64_t>(docID)) << 3LL);
}

Lucene53NormsProducer::~Lucene53NormsProducer() { delete data; }

int64_t Lucene53NormsProducer::ramBytesUsed()
{
  return 64LL * norms.size(); // good enough
}

void Lucene53NormsProducer::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(data);
}

wstring Lucene53NormsProducer::toString()
{
  return getClass().getSimpleName() + L"(fields=" + norms.size() + L")";
}

Lucene53NormsProducer::NormsIterator::NormsIterator(int maxDoc) : maxDoc(maxDoc)
{
}

int Lucene53NormsProducer::NormsIterator::docID() { return docID_; }

int Lucene53NormsProducer::NormsIterator::nextDoc()
{
  docID_++;
  if (docID_ == maxDoc) {
    docID_ = NO_MORE_DOCS;
  }
  return docID_;
}

int Lucene53NormsProducer::NormsIterator::advance(int target)
{
  docID_ = target;
  if (docID_ >= maxDoc) {
    docID_ = NO_MORE_DOCS;
  }
  return docID_;
}

bool Lucene53NormsProducer::NormsIterator::advanceExact(int target) throw(
    IOException)
{
  docID_ = target;
  return true;
}

int64_t Lucene53NormsProducer::NormsIterator::cost()
{
  // TODO
  return 0;
}
} // namespace org::apache::lucene::codecs::lucene53