using namespace std;

#include "MemoryDocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LegacyBinaryDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LegacyNumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Bits.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/PagedBytes.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/BlockPackedReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/PackedInts.h"

namespace org::apache::lucene::codecs::memory
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using namespace org::apache::lucene::index;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using org::apache::lucene::util::fst::BytesRefFSTEnum::InputOutput;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
using BlockPackedReader = org::apache::lucene::util::packed::BlockPackedReader;
using MonotonicBlockPackedReader =
    org::apache::lucene::util::packed::MonotonicBlockPackedReader;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

MemoryDocValuesProducer::MemoryDocValuesProducer(
    shared_ptr<MemoryDocValuesProducer> original) 
    : data(original->data->clone()), numEntries(original->numEntries),
      maxDoc(original->maxDoc),
      ramBytesUsed(make_shared<AtomicLong>(original->ramBytesUsed_->get())),
      version(original->version), merging(true)
{
  assert(Thread::holdsLock(original));
  numerics.putAll(original->numerics);
  binaries.putAll(original->binaries);
  fsts.putAll(original->fsts);
  sortedSets.putAll(original->sortedSets);
  sortedNumerics.putAll(original->sortedNumerics);

  numericInstances.putAll(original->numericInstances);
  pagedBytesInstances.putAll(original->pagedBytesInstances);
  fstInstances.putAll(original->fstInstances);
  docsWithFieldInstances.putAll(original->docsWithFieldInstances);
  addresses.putAll(original->addresses);

  numericInfo.putAll(original->numericInfo);
}

MemoryDocValuesProducer::MemoryDocValuesProducer(
    shared_ptr<SegmentReadState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : data(state->directory->openInput(dataName, state->context)),
      maxDoc(state->segmentInfo->maxDoc()), merging(false)
{
  wstring metaName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, metaExtension);
  // read in the entries from the metadata file.
  shared_ptr<ChecksumIndexInput> in_ =
      state->directory->openChecksumInput(metaName, state->context);
  bool success = false;
  try {
    version = org.apache::lucene::codecs::CodecUtil::checkIndexHeader(
        in_, metaCodec, VERSION_START, VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    numEntries = readFields(in_, state->fieldInfos);
    org.apache::lucene::codecs::CodecUtil::checkFooter(in_);
    ramBytesUsed_ = make_shared<AtomicLong>(
        org.apache::lucene::util::RamUsageEstimator::shallowSizeOfInstance(
            getClass()));
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      org.apache::lucene::util::IOUtils::close({in_});
    } else {
      org.apache::lucene::util::IOUtils::closeWhileHandlingException({in_});
    }
  }

  wstring dataName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, dataExtension);
  success = false;
  try {
    constexpr int version2 =
        org.apache::lucene::codecs::CodecUtil::checkIndexHeader(
            data, dataCodec, VERSION_START, VERSION_CURRENT,
            state->segmentInfo->getId(), state->segmentSuffix);
    if (version != version2) {
      throw make_shared<CorruptIndexException>(
          L"Format versions mismatch: meta=" + to_wstring(version) +
              L", data=" + to_wstring(version2),
          data);
    }

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    org.apache::lucene::codecs::CodecUtil::retrieveChecksum(data);

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      org.apache::lucene::util::IOUtils::closeWhileHandlingException(
          {this->data});
    }
  }
}

shared_ptr<NumericEntry> MemoryDocValuesProducer::readNumericEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<NumericEntry> entry = make_shared<NumericEntry>();
  entry->offset = meta->readLong();
  entry->missingOffset = meta->readLong();
  if (entry->missingOffset != -1) {
    entry->missingBytes = meta->readLong();
  } else {
    entry->missingBytes = 0;
  }
  entry->format = meta->readByte();
  switch (entry->format) {
  case DELTA_COMPRESSED:
  case TABLE_COMPRESSED:
  case BLOCK_COMPRESSED:
  case GCD_COMPRESSED:
    break;
  default:
    throw make_shared<CorruptIndexException>(
        L"Unknown format: " + to_wstring(entry->format), meta);
  }
  entry->packedIntsVersion = meta->readVInt();
  entry->count = meta->readLong();
  return entry;
}

shared_ptr<BinaryEntry> MemoryDocValuesProducer::readBinaryEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<BinaryEntry> entry = make_shared<BinaryEntry>();
  entry->offset = meta->readLong();
  entry->numBytes = meta->readLong();
  entry->missingOffset = meta->readLong();
  if (entry->missingOffset != -1) {
    entry->missingBytes = meta->readLong();
  } else {
    entry->missingBytes = 0;
  }
  entry->minLength = meta->readVInt();
  entry->maxLength = meta->readVInt();
  if (entry->minLength != entry->maxLength) {
    entry->packedIntsVersion = meta->readVInt();
    entry->blockSize = meta->readVInt();
  }
  return entry;
}

shared_ptr<FSTEntry> MemoryDocValuesProducer::readFSTEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<FSTEntry> entry = make_shared<FSTEntry>();
  entry->offset = meta->readLong();
  entry->numOrds = meta->readVLong();
  return entry;
}

int MemoryDocValuesProducer::readFields(
    shared_ptr<IndexInput> meta,
    shared_ptr<FieldInfos> infos) 
{
  int numEntries = 0;
  int fieldNumber = meta->readVInt();
  while (fieldNumber != -1) {
    numEntries++;
    shared_ptr<FieldInfo> info = infos->fieldInfo(fieldNumber);
    if (info == nullptr) {
      throw make_shared<CorruptIndexException>(
          L"invalid field number: " + to_wstring(fieldNumber), meta);
    }
    int fieldType = meta->readByte();
    if (fieldType == NUMBER) {
      numerics.emplace(info->name, readNumericEntry(meta));
    } else if (fieldType == BYTES) {
      binaries.emplace(info->name, readBinaryEntry(meta));
    } else if (fieldType == org.apache::lucene::util::fst::FST) {
      fsts.emplace(info->name, readFSTEntry(meta));
    } else if (fieldType == SORTED_SET) {
      shared_ptr<SortedSetEntry> entry = make_shared<SortedSetEntry>();
      entry->singleton = false;
      sortedSets.emplace(info->name, entry);
    } else if (fieldType == SORTED_SET_SINGLETON) {
      shared_ptr<SortedSetEntry> entry = make_shared<SortedSetEntry>();
      entry->singleton = true;
      sortedSets.emplace(info->name, entry);
    } else if (fieldType == SORTED_NUMERIC) {
      shared_ptr<SortedNumericEntry> entry = make_shared<SortedNumericEntry>();
      entry->singleton = false;
      entry->packedIntsVersion = meta->readVInt();
      entry->blockSize = meta->readVInt();
      entry->addressOffset = meta->readLong();
      entry->valueCount = meta->readLong();
      sortedNumerics.emplace(info->name, entry);
    } else if (fieldType == SORTED_NUMERIC_SINGLETON) {
      shared_ptr<SortedNumericEntry> entry = make_shared<SortedNumericEntry>();
      entry->singleton = true;
      sortedNumerics.emplace(info->name, entry);
    } else {
      throw make_shared<CorruptIndexException>(L"invalid entry type: " +
                                                   to_wstring(fieldType) +
                                                   L", fieldName=" + info->name,
                                               meta);
    }
    fieldNumber = meta->readVInt();
  }
  return numEntries;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<NumericDocValues> MemoryDocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<LegacyNumericDocValues> instance = numericInstances[field->name];
  if (instance == nullptr) {
    instance = loadNumeric(field);
    if (!merging) {
      numericInstances.emplace(field->name, instance);
    }
  }
  shared_ptr<NumericEntry> ne = numerics[field->name];
  return make_shared<LegacyNumericDocValuesWrapper>(
      getMissingBits(field, ne->missingOffset, ne->missingBytes), instance);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<LegacyNumericDocValues>
MemoryDocValuesProducer::getNumericNonIterator(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<LegacyNumericDocValues> instance = numericInstances[field->name];
  if (instance == nullptr) {
    instance = loadNumeric(field);
    if (!merging) {
      numericInstances.emplace(field->name, instance);
    }
  }
  return instance;
}

int64_t MemoryDocValuesProducer::ramBytesUsed()
{
  return ramBytesUsed_->get();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
MemoryDocValuesProducer::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(org.apache::lucene::util::Accountables::namedAccountables(
      L"numeric field", numericInfo));
  resources.addAll(org.apache::lucene::util::Accountables::namedAccountables(
      L"pagedbytes field", pagedBytesInstances));
  resources.addAll(org.apache::lucene::util::Accountables::namedAccountables(
      L"term dict field", fstInstances));
  resources.addAll(org.apache::lucene::util::Accountables::namedAccountables(
      L"missing bitset field", docsWithFieldInstances));
  resources.addAll(org.apache::lucene::util::Accountables::namedAccountables(
      L"addresses field", addresses));
  return Collections::unmodifiableList(resources);
}

void MemoryDocValuesProducer::checkIntegrity() 
{
  org.apache::lucene::codecs::CodecUtil::checksumEntireFile(data->clone());
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocValuesProducer>
MemoryDocValuesProducer::getMergeInstance() 
{
  return make_shared<MemoryDocValuesProducer>(shared_from_this());
}

wstring MemoryDocValuesProducer::toString()
{
  return getClass().getSimpleName() + L"(entries=" + to_wstring(numEntries) +
         L")";
}

shared_ptr<LegacyNumericDocValues> MemoryDocValuesProducer::loadNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<NumericEntry> entry = numerics[field->name];
  shared_ptr<IndexInput> data = this->data->clone();
  data->seek(entry->offset + entry->missingBytes);
  switch (entry->format) {
  case TABLE_COMPRESSED: {
    int size = data->readVInt();
    if (size > 256) {
      throw make_shared<CorruptIndexException>(
          L"TABLE_COMPRESSED cannot have more than 256 distinct values, got=" +
              to_wstring(size),
          data);
    }
    const std::deque<int64_t> decode = std::deque<int64_t>(size);
    for (int i = 0; i < decode.size(); i++) {
      decode[i] = data->readLong();
    }
    constexpr int formatID = data->readVInt();
    constexpr int bitsPerValue = data->readVInt();
    shared_ptr<PackedInts::Reader> *const ordsReader =
        org.apache::lucene::util::packed::PackedInts::getReaderNoHeader(
            data,
            org.apache::lucene::util::packed::PackedInts::Format::byId(
                formatID),
            entry->packedIntsVersion, static_cast<int>(entry->count),
            bitsPerValue);
    if (!merging) {
      ramBytesUsed_->addAndGet(
          org.apache::lucene::util::RamUsageEstimator::sizeOf(decode) +
          ordsReader->ramBytesUsed());
      numericInfo.emplace(
          field->name, org.apache::lucene::util::Accountables::namedAccountable(
                           L"table compressed", ordsReader));
    }
    return make_shared<LegacyNumericDocValuesAnonymousInnerClass>(
        shared_from_this(), decode, ordsReader);
  }
  case DELTA_COMPRESSED: {
    constexpr int64_t minDelta = data->readLong();
    constexpr int formatIDDelta = data->readVInt();
    constexpr int bitsPerValueDelta = data->readVInt();
    shared_ptr<PackedInts::Reader> *const deltaReader =
        org.apache::lucene::util::packed::PackedInts::getReaderNoHeader(
            data,
            org.apache::lucene::util::packed::PackedInts::Format::byId(
                formatIDDelta),
            entry->packedIntsVersion, static_cast<int>(entry->count),
            bitsPerValueDelta);
    if (!merging) {
      ramBytesUsed_->addAndGet(deltaReader->ramBytesUsed());
      numericInfo.emplace(
          field->name, org.apache::lucene::util::Accountables::namedAccountable(
                           L"delta compressed", deltaReader));
    }
    return make_shared<LegacyNumericDocValuesAnonymousInnerClass2>(
        shared_from_this(), minDelta, deltaReader);
  }
  case BLOCK_COMPRESSED: {
    constexpr int blockSize = data->readVInt();
    shared_ptr<BlockPackedReader> *const reader =
        make_shared<org.apache::lucene::util::packed::BlockPackedReader>(
            data, entry->packedIntsVersion, blockSize, entry->count, false);
    if (!merging) {
      ramBytesUsed_->addAndGet(reader->ramBytesUsed());
      numericInfo.emplace(
          field->name, org.apache::lucene::util::Accountables::namedAccountable(
                           L"block compressed", reader));
    }
    return make_shared<LegacyNumericDocValuesAnonymousInnerClass3>(
        shared_from_this(), reader);
  }
  case GCD_COMPRESSED: {
    constexpr int64_t min = data->readLong();
    constexpr int64_t mult = data->readLong();
    constexpr int formatIDGCD = data->readVInt();
    constexpr int bitsPerValueGCD = data->readVInt();
    shared_ptr<PackedInts::Reader> *const quotientReader =
        org.apache::lucene::util::packed::PackedInts::getReaderNoHeader(
            data,
            org.apache::lucene::util::packed::PackedInts::Format::byId(
                formatIDGCD),
            entry->packedIntsVersion, static_cast<int>(entry->count),
            bitsPerValueGCD);
    if (!merging) {
      ramBytesUsed_->addAndGet(quotientReader->ramBytesUsed());
      numericInfo.emplace(
          field->name, org.apache::lucene::util::Accountables::namedAccountable(
                           L"gcd compressed", quotientReader));
    }
    return make_shared<LegacyNumericDocValuesAnonymousInnerClass4>(
        shared_from_this(), min, mult, quotientReader);
  }
  default:
    throw make_shared<AssertionError>();
  }
}

MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass::
    LegacyNumericDocValuesAnonymousInnerClass(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        deque<int64_t> &decode, shared_ptr<PackedInts::Reader> ordsReader)
{
  this->outerInstance = outerInstance;
  this->decode = decode;
  this->ordsReader = ordsReader;
}

int64_t
MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass::get(
    int docID)
{
  return decode[static_cast<int>(ordsReader->get(docID))];
}

MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass2::
    LegacyNumericDocValuesAnonymousInnerClass2(
        shared_ptr<MemoryDocValuesProducer> outerInstance, int64_t minDelta,
        shared_ptr<PackedInts::Reader> deltaReader)
{
  this->outerInstance = outerInstance;
  this->minDelta = minDelta;
  this->deltaReader = deltaReader;
}

int64_t
MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass2::get(
    int docID)
{
  return minDelta + deltaReader->get(docID);
}

MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass3::
    LegacyNumericDocValuesAnonymousInnerClass3(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        shared_ptr<BlockPackedReader> reader)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
}

int64_t
MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass3::get(
    int docID)
{
  return reader->get(docID);
}

MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass4::
    LegacyNumericDocValuesAnonymousInnerClass4(
        shared_ptr<MemoryDocValuesProducer> outerInstance, int64_t min,
        int64_t mult, shared_ptr<PackedInts::Reader> quotientReader)
{
  this->outerInstance = outerInstance;
  this->min = min;
  this->mult = mult;
  this->quotientReader = quotientReader;
}

int64_t
MemoryDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass4::get(
    int docID)
{
  return min + mult * quotientReader->get(docID);
}

shared_ptr<LegacyBinaryDocValues> MemoryDocValuesProducer::getLegacyBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryEntry> entry = binaries[field->name];

  shared_ptr<BytesAndAddresses> instance;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    instance = pagedBytesInstances[field->name];
    if (instance == nullptr) {
      instance = loadBinary(field);
      if (!merging) {
        pagedBytesInstances.emplace(field->name, instance);
      }
    }
  }
  shared_ptr<PagedBytes::Reader> *const bytesReader = instance->reader;
  shared_ptr<MonotonicBlockPackedReader> *const addresses = instance->addresses;

  if (addresses == nullptr) {
    assert(entry->minLength == entry->maxLength);
    constexpr int fixedLength = entry->minLength;
    return make_shared<LegacyBinaryDocValuesAnonymousInnerClass>(
        shared_from_this(), bytesReader, fixedLength);
  } else {
    return make_shared<LegacyBinaryDocValuesAnonymousInnerClass2>(
        shared_from_this(), bytesReader, addresses);
  }
}

MemoryDocValuesProducer::LegacyBinaryDocValuesAnonymousInnerClass::
    LegacyBinaryDocValuesAnonymousInnerClass(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        shared_ptr<PagedBytes::Reader> bytesReader, int fixedLength)
{
  this->outerInstance = outerInstance;
  this->bytesReader = bytesReader;
  this->fixedLength = fixedLength;
  term = make_shared<org.apache::lucene::util::BytesRef>();
}

shared_ptr<BytesRef>
MemoryDocValuesProducer::LegacyBinaryDocValuesAnonymousInnerClass::get(
    int docID)
{
  bytesReader->fillSlice(term, fixedLength * static_cast<int64_t>(docID),
                         fixedLength);
  return term;
}

MemoryDocValuesProducer::LegacyBinaryDocValuesAnonymousInnerClass2::
    LegacyBinaryDocValuesAnonymousInnerClass2(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        shared_ptr<PagedBytes::Reader> bytesReader,
        shared_ptr<MonotonicBlockPackedReader> addresses)
{
  this->outerInstance = outerInstance;
  this->bytesReader = bytesReader;
  this->addresses = addresses;
  term = make_shared<org.apache::lucene::util::BytesRef>();
}

shared_ptr<BytesRef>
MemoryDocValuesProducer::LegacyBinaryDocValuesAnonymousInnerClass2::get(
    int docID)
{
  int64_t startAddress = docID == 0 ? 0 : addresses->get(docID - 1);
  int64_t endAddress = addresses->get(docID);
  bytesReader->fillSlice(term, startAddress,
                         static_cast<int>(endAddress - startAddress));
  return term;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<BinaryDocValues> MemoryDocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryEntry> be = binaries[field->name];
  return make_shared<LegacyBinaryDocValuesWrapper>(
      getMissingBits(field, be->missingOffset, be->missingBytes),
      getLegacyBinary(field));
}

shared_ptr<BytesAndAddresses> MemoryDocValuesProducer::loadBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BytesAndAddresses> bytesAndAddresses =
      make_shared<BytesAndAddresses>();
  shared_ptr<BinaryEntry> entry = binaries[field->name];
  shared_ptr<IndexInput> data = this->data->clone();
  data->seek(entry->offset);
  shared_ptr<PagedBytes> bytes =
      make_shared<org.apache::lucene::util::PagedBytes>(16);
  bytes->copy(data, entry->numBytes);
  bytesAndAddresses->reader = bytes->freeze(true);
  if (!merging) {
    ramBytesUsed_->addAndGet(bytesAndAddresses->reader->ramBytesUsed());
  }
  if (entry->minLength != entry->maxLength) {
    data->seek(data->getFilePointer() + entry->missingBytes);
    bytesAndAddresses->addresses =
        org.apache::lucene::util::packed::MonotonicBlockPackedReader::of(
            data, entry->packedIntsVersion, entry->blockSize, maxDoc, false);
    if (!merging) {
      ramBytesUsed_->addAndGet(bytesAndAddresses->addresses->ramBytesUsed());
    }
  }
  return bytesAndAddresses;
}

shared_ptr<SortedDocValues> MemoryDocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  return make_shared<LegacySortedDocValuesWrapper>(getSortedNonIterator(field),
                                                   maxDoc);
}

shared_ptr<LegacySortedDocValues> MemoryDocValuesProducer::getSortedNonIterator(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<FSTEntry> *const entry = fsts[field->name];
  if (entry->numOrds == 0) {
    return DocValues::emptyLegacySorted();
  }
  shared_ptr<FST<int64_t>> instance;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    instance = fstInstances[field->name];
    if (instance == nullptr) {
      shared_ptr<IndexInput> data = this->data->clone();
      data->seek(entry->offset);
      instance = make_shared<org.apache::lucene::util::fst::FST<int64_t>>(
          data,
          org.apache::lucene::util::fst::PositiveIntOutputs::getSingleton());
      if (!merging) {
        ramBytesUsed_->addAndGet(instance->ramBytesUsed());
        fstInstances.emplace(field->name, instance);
      }
    }
  }
  shared_ptr<LegacyNumericDocValues> *const docToOrd =
      getNumericNonIterator(field);
  shared_ptr<FST<int64_t>> *const fst = instance;

  // per-thread resources
  shared_ptr<BytesReader> *const in_ = fst->getBytesReader();
  shared_ptr<Arc<int64_t>> *const firstArc =
      make_shared<org.apache::lucene::util::fst::FST::Arc<int64_t>>();
  shared_ptr<Arc<int64_t>> *const scratchArc =
      make_shared<org.apache::lucene::util::fst::FST::Arc<int64_t>>();
  shared_ptr<IntsRefBuilder> *const scratchInts =
      make_shared<org.apache::lucene::util::IntsRefBuilder>();
  shared_ptr<BytesRefFSTEnum<int64_t>> *const fstEnum =
      make_shared<org.apache::lucene::util::fst::BytesRefFSTEnum<int64_t>>(
          fst);

  return make_shared<LegacySortedDocValuesAnonymousInnerClass>(
      shared_from_this(), entry, docToOrd, fst, in_, firstArc, scratchArc,
      scratchInts, fstEnum);
}

MemoryDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::
    LegacySortedDocValuesAnonymousInnerClass(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::memory::
                       MemoryDocValuesProducer::FSTEntry>
            entry,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> docToOrd,
        shared_ptr<FST<int64_t>> fst, shared_ptr<BytesReader> in_,
        shared_ptr<Arc<int64_t>> firstArc,
        shared_ptr<Arc<int64_t>> scratchArc,
        shared_ptr<IntsRefBuilder> scratchInts,
        shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
  this->docToOrd = docToOrd;
  this->fst = fst;
  this->in_ = in_;
  this->firstArc = firstArc;
  this->scratchArc = scratchArc;
  this->scratchInts = scratchInts;
  this->fstEnum = fstEnum;
  term = make_shared<org.apache::lucene::util::BytesRefBuilder>();
}

int MemoryDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::getOrd(
    int docID)
{
  return static_cast<int>(docToOrd->get(docID));
}

shared_ptr<BytesRef>
MemoryDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::lookupOrd(
    int ord)
{
  try {
    in_->setPosition(0);
    fst->getFirstArc(firstArc);
    shared_ptr<IntsRef> output =
        org.apache::lucene::util::fst::Util::getByOutput(
            fst, ord, in_, firstArc, scratchArc, scratchInts);
    return org.apache::lucene::util::fst::Util::toBytesRef(output, term);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

int MemoryDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::
    lookupTerm(shared_ptr<BytesRef> key)
{
  try {
    shared_ptr<InputOutput<int64_t>> o = fstEnum->seekCeil(key);
    if (o == nullptr) {
      return -getValueCount() - 1;
    } else if (o->input->equals(key)) {
      return o->output.intValue();
    } else {
      return (int)-o->output - 1;
    }
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

int MemoryDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::
    getValueCount()
{
  return static_cast<int>(entry->numOrds);
}

shared_ptr<TermsEnum>
MemoryDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::termsEnum()
{
  return make_shared<FSTTermsEnum>(fst);
}

shared_ptr<SortedNumericDocValues> MemoryDocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedNumericEntry> entry = sortedNumerics[field->name];
  if (entry->singleton) {
    shared_ptr<LegacyNumericDocValues> values = getNumericNonIterator(field);
    shared_ptr<NumericEntry> ne = numerics[field->name];
    shared_ptr<Bits> docsWithField =
        getMissingBits(field, ne->missingOffset, ne->missingBytes);
    return DocValues::singleton(
        make_shared<LegacyNumericDocValuesWrapper>(docsWithField, values));
  } else {
    shared_ptr<LegacyNumericDocValues> *const values =
        getNumericNonIterator(field);
    shared_ptr<MonotonicBlockPackedReader> *const addr;
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      shared_ptr<MonotonicBlockPackedReader> res = addresses[field->name];
      if (res == nullptr) {
        shared_ptr<IndexInput> data = this->data->clone();
        data->seek(entry->addressOffset);
        res = org.apache::lucene::util::packed::MonotonicBlockPackedReader::of(
            data, entry->packedIntsVersion, entry->blockSize, entry->valueCount,
            false);
        if (!merging) {
          addresses.emplace(field->name, res);
          ramBytesUsed_->addAndGet(res->ramBytesUsed());
        }
      }
      addr = res;
    }
    return make_shared<LegacySortedNumericDocValuesWrapper>(
        make_shared<LegacySortedNumericDocValuesAnonymousInnerClass>(
            shared_from_this(), values, addr),
        maxDoc);
  }
}

MemoryDocValuesProducer::LegacySortedNumericDocValuesAnonymousInnerClass::
    LegacySortedNumericDocValuesAnonymousInnerClass(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> values,
        shared_ptr<MonotonicBlockPackedReader> addr)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->addr = addr;
}

void MemoryDocValuesProducer::LegacySortedNumericDocValuesAnonymousInnerClass::
    setDocument(int doc)
{
  startOffset = static_cast<int>(addr->get(doc));
  endOffset = static_cast<int>(addr->get(doc + 1));
}

int64_t MemoryDocValuesProducer::
    LegacySortedNumericDocValuesAnonymousInnerClass::valueAt(int index)
{
  return values->get(startOffset + index);
}

int MemoryDocValuesProducer::LegacySortedNumericDocValuesAnonymousInnerClass::
    count()
{
  return (endOffset - startOffset);
}

shared_ptr<SortedSetDocValues> MemoryDocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedSetEntry> sortedSetEntry = sortedSets[field->name];
  if (sortedSetEntry->singleton) {
    return DocValues::singleton(getSorted(field));
  }

  shared_ptr<FSTEntry> *const entry = fsts[field->name];
  if (entry->numOrds == 0) {
    return DocValues::emptySortedSet(); // empty FST!
  }
  shared_ptr<FST<int64_t>> instance;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    instance = fstInstances[field->name];
    if (instance == nullptr) {
      shared_ptr<IndexInput> data = this->data->clone();
      data->seek(entry->offset);
      instance = make_shared<org.apache::lucene::util::fst::FST<int64_t>>(
          data,
          org.apache::lucene::util::fst::PositiveIntOutputs::getSingleton());
      if (!merging) {
        ramBytesUsed_->addAndGet(instance->ramBytesUsed());
        fstInstances.emplace(field->name, instance);
      }
    }
  }
  shared_ptr<LegacyBinaryDocValues> *const docToOrds = getLegacyBinary(field);
  shared_ptr<FST<int64_t>> *const fst = instance;

  // per-thread resources
  shared_ptr<BytesReader> *const in_ = fst->getBytesReader();
  shared_ptr<Arc<int64_t>> *const firstArc =
      make_shared<org.apache::lucene::util::fst::FST::Arc<int64_t>>();
  shared_ptr<Arc<int64_t>> *const scratchArc =
      make_shared<org.apache::lucene::util::fst::FST::Arc<int64_t>>();
  shared_ptr<IntsRefBuilder> *const scratchInts =
      make_shared<org.apache::lucene::util::IntsRefBuilder>();
  shared_ptr<BytesRefFSTEnum<int64_t>> *const fstEnum =
      make_shared<org.apache::lucene::util::fst::BytesRefFSTEnum<int64_t>>(
          fst);
  shared_ptr<ByteArrayDataInput> *const input =
      make_shared<org.apache::lucene::store::ByteArrayDataInput>();
  return make_shared<LegacySortedSetDocValuesWrapper>(
      make_shared<LegacySortedSetDocValuesAnonymousInnerClass>(
          shared_from_this(), entry, docToOrds, fst, in_, firstArc, scratchArc,
          scratchInts, fstEnum, input),
      maxDoc);
}

MemoryDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    LegacySortedSetDocValuesAnonymousInnerClass(
        shared_ptr<MemoryDocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::memory::
                       MemoryDocValuesProducer::FSTEntry>
            entry,
        shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> docToOrds,
        shared_ptr<FST<int64_t>> fst, shared_ptr<BytesReader> in_,
        shared_ptr<Arc<int64_t>> firstArc,
        shared_ptr<Arc<int64_t>> scratchArc,
        shared_ptr<IntsRefBuilder> scratchInts,
        shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum,
        shared_ptr<ByteArrayDataInput> input)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
  this->docToOrds = docToOrds;
  this->fst = fst;
  this->in_ = in_;
  this->firstArc = firstArc;
  this->scratchArc = scratchArc;
  this->scratchInts = scratchInts;
  this->fstEnum = fstEnum;
  this->input = input;
  term = make_shared<org.apache::lucene::util::BytesRefBuilder>();
}

int64_t
MemoryDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::nextOrd()
{
  if (input->eof()) {
    return NO_MORE_ORDS;
  } else {
    currentOrd += input->readVLong();
    return currentOrd;
  }
}

void MemoryDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    setDocument(int docID)
{
  ref = docToOrds->get(docID);
  input->reset(ref::bytes, ref::offset, ref->length);
  currentOrd = 0;
}

shared_ptr<BytesRef>
MemoryDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::lookupOrd(
    int64_t ord)
{
  try {
    in_->setPosition(0);
    fst->getFirstArc(firstArc);
    shared_ptr<IntsRef> output =
        org.apache::lucene::util::fst::Util::getByOutput(
            fst, ord, in_, firstArc, scratchArc, scratchInts);
    return org.apache::lucene::util::fst::Util::toBytesRef(output, term);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

int64_t MemoryDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    lookupTerm(shared_ptr<BytesRef> key)
{
  try {
    shared_ptr<InputOutput<int64_t>> o = fstEnum->seekCeil(key);
    if (o == nullptr) {
      return -getValueCount() - 1;
    } else if (o->input->equals(key)) {
      return o->output.intValue();
    } else {
      return -o->output - 1;
    }
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

int64_t MemoryDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    getValueCount()
{
  return entry->numOrds;
}

shared_ptr<TermsEnum> MemoryDocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass::termsEnum()
{
  return make_shared<FSTTermsEnum>(fst);
}

shared_ptr<Bits> MemoryDocValuesProducer::getMissingBits(
    shared_ptr<FieldInfo> field, int64_t const offset,
    int64_t const length) 
{
  if (offset == -1) {
    return make_shared<org.apache::lucene::util::Bits::MatchAllBits>(maxDoc);
  } else {
    shared_ptr<FixedBitSet> instance;
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      instance = docsWithFieldInstances[field->name];
      if (instance == nullptr) {
        shared_ptr<IndexInput> data = this->data->clone();
        data->seek(offset);
        assert(length % 8 == 0);
        std::deque<int64_t> bits(static_cast<int>(length) >> 3);
        for (int i = 0; i < bits.size(); i++) {
          bits[i] = data->readLong();
        }
        instance =
            make_shared<org.apache::lucene::util::FixedBitSet>(bits, maxDoc);
        if (!merging) {
          docsWithFieldInstances.emplace(field->name, instance);
          ramBytesUsed_->addAndGet(instance->ramBytesUsed());
        }
      }
    }
    return instance;
  }
}

MemoryDocValuesProducer::~MemoryDocValuesProducer() { delete data; }

int64_t MemoryDocValuesProducer::BytesAndAddresses::ramBytesUsed()
{
  int64_t bytesUsed = reader->ramBytesUsed();
  if (addresses != nullptr) {
    bytesUsed += addresses->ramBytesUsed();
  }
  return bytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
MemoryDocValuesProducer::BytesAndAddresses::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (addresses != nullptr) {
    resources.push_back(
        org.apache::lucene::util::Accountables::namedAccountable(L"addresses",
                                                                 addresses));
  }
  resources.push_back(org.apache::lucene::util::Accountables::namedAccountable(
      L"term bytes", reader));
  return Collections::unmodifiableList(resources);
}

MemoryDocValuesProducer::FSTTermsEnum::FSTTermsEnum(
    shared_ptr<FST<int64_t>> fst)
    : in_(make_shared<BytesRefFSTEnum<>>(fst)), fst(fst),
      bytesReader(fst->getBytesReader())
{
}

shared_ptr<BytesRef>
MemoryDocValuesProducer::FSTTermsEnum::next() 
{
  shared_ptr<InputOutput<int64_t>> io = in_->next();
  if (io == nullptr) {
    return nullptr;
  } else {
    return io->input;
  }
}

SeekStatus MemoryDocValuesProducer::FSTTermsEnum::seekCeil(
    shared_ptr<BytesRef> text) 
{
  if (in_->seekCeil(text) == nullptr) {
    return SeekStatus::END;
  } else if (term()->equals(text)) {
    // TODO: add SeekStatus to FSTEnum like in
    // https://issues.apache.org/jira/browse/LUCENE-3729 to remove this
    // comparision?
    return SeekStatus::FOUND;
  } else {
    return SeekStatus::NOT_FOUND;
  }
}

bool MemoryDocValuesProducer::FSTTermsEnum::seekExact(
    shared_ptr<BytesRef> text) 
{
  if (in_->seekExact(text) == nullptr) {
    return false;
  } else {
    return true;
  }
}

void MemoryDocValuesProducer::FSTTermsEnum::seekExact(int64_t ord) throw(
    IOException)
{
  // TODO: would be better to make this simpler and faster.
  // but we dont want to introduce a bug that corrupts our enum state!
  bytesReader->setPosition(0);
  fst->getFirstArc(firstArc);
  shared_ptr<IntsRef> output = org.apache::lucene::util::fst::Util::getByOutput(
      fst, ord, bytesReader, firstArc, scratchArc, scratchInts);
  // TODO: we could do this lazily, better to try to push into FSTEnum though?
  in_->seekExact(org.apache::lucene::util::fst::Util::toBytesRef(
      output, make_shared<org.apache::lucene::util::BytesRefBuilder>()));
}

shared_ptr<BytesRef>
MemoryDocValuesProducer::FSTTermsEnum::term() 
{
  return in_->current()->input;
}

int64_t MemoryDocValuesProducer::FSTTermsEnum::ord() 
{
  return in_->current()->output;
}

int MemoryDocValuesProducer::FSTTermsEnum::docFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t
MemoryDocValuesProducer::FSTTermsEnum::totalTermFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum>
MemoryDocValuesProducer::FSTTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                                int flags) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::memory