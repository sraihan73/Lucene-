using namespace std;

#include "Lucene54DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LegacyBinaryDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RandomAccessInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/PagedBytes.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/DirectMonotonicReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/DirectReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedReader.h"

namespace org::apache::lucene::codecs::lucene54
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using NumberType = org::apache::lucene::codecs::lucene54::
    Lucene54DocValuesConsumer::NumberType;
using namespace org::apache::lucene::index;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongValues = org::apache::lucene::util::LongValues;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using DirectMonotonicReader =
    org::apache::lucene::util::packed::DirectMonotonicReader;
using DirectReader = org::apache::lucene::util::packed::DirectReader;
using MonotonicBlockPackedReader =
    org::apache::lucene::util::packed::MonotonicBlockPackedReader;
using namespace org::apache::lucene::codecs::lucene54;
//    import static org.apache.lucene.codecs.lucene54.Lucene54DocValuesFormat.*;

Lucene54DocValuesProducer::Lucene54DocValuesProducer(
    shared_ptr<Lucene54DocValuesProducer> original) 
    : numFields(original->numFields),
      ramBytesUsed(make_shared<AtomicLong>(original->ramBytesUsed_->get())),
      data(original->data->clone()), maxDoc(original->maxDoc), merging(true)
{
  assert(Thread::holdsLock(original));
  numerics.putAll(original->numerics);
  binaries.putAll(original->binaries);
  sortedSets.putAll(original->sortedSets);
  sortedNumerics.putAll(original->sortedNumerics);
  ords.putAll(original->ords);
  ordIndexes.putAll(original->ordIndexes);

  addressInstances.putAll(original->addressInstances);
  reverseIndexInstances.putAll(original->reverseIndexInstances);
}

Lucene54DocValuesProducer::Lucene54DocValuesProducer(
    shared_ptr<SegmentReadState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : ramBytesUsed(make_shared<AtomicLong>(
          RamUsageEstimator::shallowSizeOfInstance(getClass()))),
      data(state->directory->openInput(dataName, state->context)),
      maxDoc(state->segmentInfo->maxDoc()), merging(false)
{
  wstring metaName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, metaExtension);

  int version = -1;
  int numFields = -1;

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
          in_, metaCodec, Lucene54DocValuesFormat::VERSION_START,
          Lucene54DocValuesFormat::VERSION_CURRENT, state->segmentInfo->getId(),
          state->segmentSuffix);
      numFields = readFields(in_, state->fieldInfos);
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(in_, priorE);
    }
  }

  this->numFields = numFields;
  wstring dataName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, dataExtension);
  bool success = false;
  try {
    constexpr int version2 = CodecUtil::checkIndexHeader(
        data, dataCodec, Lucene54DocValuesFormat::VERSION_START,
        Lucene54DocValuesFormat::VERSION_CURRENT, state->segmentInfo->getId(),
        state->segmentSuffix);
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

void Lucene54DocValuesProducer::readSortedField(
    shared_ptr<FieldInfo> info, shared_ptr<IndexInput> meta) 
{
  // sorted = binary + numeric
  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sorted entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::BINARY) {
    throw make_shared<CorruptIndexException>(
        L"sorted entry for field: " + info->name + L" is corrupt", meta);
  }
  shared_ptr<BinaryEntry> b = readBinaryEntry(info, meta);
  binaries.emplace(info->name, b);

  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sorted entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
    throw make_shared<CorruptIndexException>(
        L"sorted entry for field: " + info->name + L" is corrupt", meta);
  }
  shared_ptr<NumericEntry> n = readNumericEntry(info, meta);
  ords.emplace(info->name, n);
}

void Lucene54DocValuesProducer::readSortedSetFieldWithAddresses(
    shared_ptr<FieldInfo> info, shared_ptr<IndexInput> meta) 
{
  // sortedset = binary + numeric (addresses) + ordIndex
  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::BINARY) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  shared_ptr<BinaryEntry> b = readBinaryEntry(info, meta);
  binaries.emplace(info->name, b);

  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  shared_ptr<NumericEntry> n1 = readNumericEntry(info, meta);
  ords.emplace(info->name, n1);

  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  shared_ptr<NumericEntry> n2 = readNumericEntry(info, meta);
  ordIndexes.emplace(info->name, n2);
}

void Lucene54DocValuesProducer::readSortedSetFieldWithTable(
    shared_ptr<FieldInfo> info, shared_ptr<IndexInput> meta) 
{
  // sortedset table = binary + ordset table + ordset index
  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::BINARY) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }

  shared_ptr<BinaryEntry> b = readBinaryEntry(info, meta);
  binaries.emplace(info->name, b);

  if (meta->readVInt() != info->number) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
    throw make_shared<CorruptIndexException>(
        L"sortedset entry for field: " + info->name + L" is corrupt", meta);
  }
  shared_ptr<NumericEntry> n = readNumericEntry(info, meta);
  ords.emplace(info->name, n);
}

int Lucene54DocValuesProducer::readFields(
    shared_ptr<IndexInput> meta,
    shared_ptr<FieldInfos> infos) 
{
  int numFields = 0;
  int fieldNumber = meta->readVInt();
  while (fieldNumber != -1) {
    numFields++;
    shared_ptr<FieldInfo> info = infos->fieldInfo(fieldNumber);
    if (info == nullptr) {
      // trickier to validate more: because we use multiple entries for
      // "composite" types like sortedset, etc.
      throw make_shared<CorruptIndexException>(
          L"Invalid field number: " + to_wstring(fieldNumber), meta);
    }
    char type = meta->readByte();
    if (type == Lucene54DocValuesFormat::NUMERIC) {
      numerics.emplace(info->name, readNumericEntry(info, meta));
    } else if (type == Lucene54DocValuesFormat::BINARY) {
      shared_ptr<BinaryEntry> b = readBinaryEntry(info, meta);
      binaries.emplace(info->name, b);
    } else if (type == Lucene54DocValuesFormat::SORTED) {
      readSortedField(info, meta);
    } else if (type == Lucene54DocValuesFormat::SORTED_SET) {
      shared_ptr<SortedSetEntry> ss = readSortedSetEntry(meta);
      sortedSets.emplace(info->name, ss);
      if (ss->format == Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES) {
        readSortedSetFieldWithAddresses(info, meta);
      } else if (ss->format == Lucene54DocValuesFormat::SORTED_SET_TABLE) {
        readSortedSetFieldWithTable(info, meta);
      } else if (ss->format == Lucene54DocValuesFormat::SORTED_SINGLE_VALUED) {
        if (meta->readVInt() != fieldNumber) {
          throw make_shared<CorruptIndexException>(
              L"sortedset entry for field: " + info->name + L" is corrupt",
              meta);
        }
        if (meta->readByte() != Lucene54DocValuesFormat::SORTED) {
          throw make_shared<CorruptIndexException>(
              L"sortedset entry for field: " + info->name + L" is corrupt",
              meta);
        }
        readSortedField(info, meta);
      } else {
        throw make_shared<AssertionError>();
      }
    } else if (type == Lucene54DocValuesFormat::SORTED_NUMERIC) {
      shared_ptr<SortedSetEntry> ss = readSortedSetEntry(meta);
      sortedNumerics.emplace(info->name, ss);
      if (ss->format == Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES) {
        if (meta->readVInt() != fieldNumber) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        numerics.emplace(info->name, readNumericEntry(info, meta));
        if (meta->readVInt() != fieldNumber) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        shared_ptr<NumericEntry> ordIndex = readNumericEntry(info, meta);
        ordIndexes.emplace(info->name, ordIndex);
      } else if (ss->format == Lucene54DocValuesFormat::SORTED_SET_TABLE) {
        if (meta->readVInt() != info->number) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        shared_ptr<NumericEntry> n = readNumericEntry(info, meta);
        ords.emplace(info->name, n);
      } else if (ss->format == Lucene54DocValuesFormat::SORTED_SINGLE_VALUED) {
        if (meta->readVInt() != fieldNumber) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        if (meta->readByte() != Lucene54DocValuesFormat::NUMERIC) {
          throw make_shared<CorruptIndexException>(
              L"sortednumeric entry for field: " + info->name + L" is corrupt",
              meta);
        }
        numerics.emplace(info->name, readNumericEntry(info, meta));
      } else {
        throw make_shared<AssertionError>();
      }
    } else {
      throw make_shared<CorruptIndexException>(
          L"invalid type: " + to_wstring(type), meta);
    }
    fieldNumber = meta->readVInt();
  }
  return numFields;
}

shared_ptr<NumericEntry> Lucene54DocValuesProducer::readNumericEntry(
    shared_ptr<FieldInfo> info, shared_ptr<IndexInput> meta) 
{
  shared_ptr<NumericEntry> entry = make_shared<NumericEntry>();
  entry->format = meta->readVInt();
  entry->missingOffset = meta->readLong();
  if (entry->format == Lucene54DocValuesFormat::SPARSE_COMPRESSED) {
    // sparse bits need a bit more metadata
    entry->numDocsWithValue = meta->readVLong();
    constexpr int blockShift = meta->readVInt();
    entry->monotonicMeta = DirectMonotonicReader::loadMeta(
        meta, entry->numDocsWithValue, blockShift);
    ramBytesUsed_->addAndGet(entry->monotonicMeta->ramBytesUsed());
    directAddressesMeta.emplace(info->name, entry->monotonicMeta);
  }
  entry->offset = meta->readLong();
  entry->count = meta->readVLong();
  switch (entry->format) {
  case Lucene54DocValuesFormat::CONST_COMPRESSED:
    entry->minValue = meta->readLong();
    if (entry->count > numeric_limits<int>::max()) {
      // currently just a limitation e.g. of bits interface and so on.
      throw make_shared<CorruptIndexException>(
          L"illegal CONST_COMPRESSED count: " + to_wstring(entry->count), meta);
    }
    break;
  case Lucene54DocValuesFormat::GCD_COMPRESSED:
    entry->minValue = meta->readLong();
    entry->gcd = meta->readLong();
    entry->bitsPerValue = meta->readVInt();
    break;
  case Lucene54DocValuesFormat::TABLE_COMPRESSED: {
    constexpr int uniqueValues = meta->readVInt();
    if (uniqueValues > 256) {
      throw make_shared<CorruptIndexException>(
          L"TABLE_COMPRESSED cannot have more than 256 distinct values, got=" +
              to_wstring(uniqueValues),
          meta);
    }
    entry->table = std::deque<int64_t>(uniqueValues);
    for (int i = 0; i < uniqueValues; ++i) {
      entry->table[i] = meta->readLong();
    }
    ramBytesUsed_->addAndGet(RamUsageEstimator::sizeOf(entry->table));
    entry->bitsPerValue = meta->readVInt();
    break;
  }
  case Lucene54DocValuesFormat::DELTA_COMPRESSED:
    entry->minValue = meta->readLong();
    entry->bitsPerValue = meta->readVInt();
    break;
  case Lucene54DocValuesFormat::MONOTONIC_COMPRESSED: {
    constexpr int blockShift = meta->readVInt();
    entry->monotonicMeta =
        DirectMonotonicReader::loadMeta(meta, maxDoc + 1, blockShift);
    ramBytesUsed_->addAndGet(entry->monotonicMeta->ramBytesUsed());
    directAddressesMeta.emplace(info->name, entry->monotonicMeta);
    break;
  }
  case Lucene54DocValuesFormat::SPARSE_COMPRESSED: {
    constexpr char numberType = meta->readByte();
    switch (numberType) {
    case 0:
      entry->numberType = NumberType::VALUE;
      break;
    case 1:
      entry->numberType = NumberType::ORDINAL;
      break;
    default:
      throw make_shared<CorruptIndexException>(
          L"Number type can only be 0 or 1, got=" + to_wstring(numberType),
          meta);
    }

    // now read the numeric entry for non-missing values
    constexpr int fieldNumber = meta->readVInt();
    if (fieldNumber != info->number) {
      throw make_shared<CorruptIndexException>(
          L"Field numbers mistmatch: " + to_wstring(fieldNumber) + L" != " +
              to_wstring(info->number),
          meta);
    }
    constexpr int dvFormat = meta->readByte();
    if (dvFormat != Lucene54DocValuesFormat::NUMERIC) {
      throw make_shared<CorruptIndexException>(
          L"Formats mistmatch: " + to_wstring(dvFormat) + L" != " +
              to_wstring(Lucene54DocValuesFormat::NUMERIC),
          meta);
    }
    entry->nonMissingValues = readNumericEntry(info, meta);
    break;
  }
  default:
    throw make_shared<CorruptIndexException>(
        L"Unknown format: " + to_wstring(entry->format) + L", input=", meta);
  }
  entry->endOffset = meta->readLong();
  return entry;
}

shared_ptr<BinaryEntry> Lucene54DocValuesProducer::readBinaryEntry(
    shared_ptr<FieldInfo> info, shared_ptr<IndexInput> meta) 
{
  shared_ptr<BinaryEntry> entry = make_shared<BinaryEntry>();
  entry->format = meta->readVInt();
  entry->missingOffset = meta->readLong();
  entry->minLength = meta->readVInt();
  entry->maxLength = meta->readVInt();
  entry->count = meta->readVLong();
  entry->offset = meta->readLong();
  switch (entry->format) {
  case Lucene54DocValuesFormat::BINARY_FIXED_UNCOMPRESSED:
    break;
  case Lucene54DocValuesFormat::BINARY_PREFIX_COMPRESSED:
    entry->addressesOffset = meta->readLong();
    entry->packedIntsVersion = meta->readVInt();
    entry->blockSize = meta->readVInt();
    entry->reverseIndexOffset = meta->readLong();
    break;
  case Lucene54DocValuesFormat::BINARY_VARIABLE_UNCOMPRESSED: {
    entry->addressesOffset = meta->readLong();
    constexpr int blockShift = meta->readVInt();
    entry->addressesMeta =
        DirectMonotonicReader::loadMeta(meta, entry->count + 1, blockShift);
    ramBytesUsed_->addAndGet(entry->addressesMeta->ramBytesUsed());
    directAddressesMeta.emplace(info->name, entry->addressesMeta);
    entry->addressesEndOffset = meta->readLong();
    break;
  }
  default:
    throw make_shared<CorruptIndexException>(
        L"Unknown format: " + to_wstring(entry->format), meta);
  }
  return entry;
}

shared_ptr<SortedSetEntry> Lucene54DocValuesProducer::readSortedSetEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<SortedSetEntry> entry = make_shared<SortedSetEntry>();
  entry->format = meta->readVInt();
  if (entry->format == Lucene54DocValuesFormat::SORTED_SET_TABLE) {
    constexpr int totalTableLength = meta->readInt();
    if (totalTableLength > 256) {
      throw make_shared<CorruptIndexException>(
          L"SORTED_SET_TABLE cannot have more than 256 values in its "
          L"dictionary, got=" +
              to_wstring(totalTableLength),
          meta);
    }
    entry->table = std::deque<int64_t>(totalTableLength);
    for (int i = 0; i < totalTableLength; ++i) {
      entry->table[i] = meta->readLong();
    }
    ramBytesUsed_->addAndGet(RamUsageEstimator::sizeOf(entry->table));
    constexpr int tableSize = meta->readInt();
    if (tableSize > totalTableLength + 1) { // +1 because of the empty set
      throw make_shared<CorruptIndexException>(
          L"SORTED_SET_TABLE cannot have more set ids than ords in its "
          L"dictionary, got " +
              to_wstring(totalTableLength) + L" ords and " +
              to_wstring(tableSize) + L" sets",
          meta);
    }
    entry->tableOffsets = std::deque<int>(tableSize + 1);
    for (int i = 1; i < entry->tableOffsets.size(); ++i) {
      entry->tableOffsets[i] = entry->tableOffsets[i - 1] + meta->readInt();
    }
    ramBytesUsed_->addAndGet(RamUsageEstimator::sizeOf(entry->tableOffsets));
  } else if (entry->format != Lucene54DocValuesFormat::SORTED_SINGLE_VALUED &&
             entry->format != Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES) {
    throw make_shared<CorruptIndexException>(
        L"Unknown format: " + to_wstring(entry->format), meta);
  }
  return entry;
}

shared_ptr<NumericDocValues> Lucene54DocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<NumericEntry> entry = numerics[field->name];
  shared_ptr<Bits> docsWithField;

  if (entry->format == Lucene54DocValuesFormat::SPARSE_COMPRESSED) {
    return getSparseNumericDocValues(entry);
  } else {
    if (entry->missingOffset == Lucene54DocValuesFormat::ALL_MISSING) {
      return DocValues::emptyNumeric();
    } else if (entry->missingOffset == Lucene54DocValuesFormat::ALL_LIVE) {
      shared_ptr<LongValues> values = getNumeric(entry);
      return make_shared<NumericDocValuesAnonymousInnerClass>(
          shared_from_this(), values);
    } else {
      docsWithField = getLiveBits(entry->missingOffset, maxDoc);
    }
  }
  shared_ptr<LongValues> *const values = getNumeric(entry);
  return make_shared<NumericDocValuesAnonymousInnerClass2>(
      shared_from_this(), docsWithField, values);
}

Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<LongValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  docID = -1;
}

int Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::nextDoc()
{
  docID++;
  if (docID == outerInstance->maxDoc) {
    docID = NO_MORE_DOCS;
  }
  return docID;
}

int Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::advance(
    int target)
{
  if (target >= outerInstance->maxDoc) {
    docID = NO_MORE_DOCS;
  } else {
    docID = target;
  }
  return docID;
}

bool Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  docID = target;
  return true;
}

int64_t Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::cost()
{
  // TODO
  return 0;
}

int64_t
Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass::longValue()
{
  return values->get(docID);
}

Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::
    NumericDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<Bits> docsWithField, shared_ptr<LongValues> values)
{
  this->outerInstance = outerInstance;
  this->docsWithField = docsWithField;
  this->values = values;
  doc = -1;
}

int64_t Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::
    longValue() 
{
  return value;
}

int Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::docID()
{
  return doc;
}

int Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::
    nextDoc() 
{
  return advance(doc + 1);
}

int Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::advance(
    int target) 
{
  for (int doc = target; doc < outerInstance->maxDoc; ++doc) {
    value = values->get(doc);
    if (value != 0 || docsWithField->get(doc)) {
      return this->doc = doc;
    }
  }
  return doc = NO_MORE_DOCS;
}

bool Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  doc = target;
  value = values->get(doc);
  return value != 0 || docsWithField->get(doc);
}

int64_t
Lucene54DocValuesProducer::NumericDocValuesAnonymousInnerClass2::cost()
{
  return outerInstance->maxDoc;
}

int64_t Lucene54DocValuesProducer::ramBytesUsed()
{
  return ramBytesUsed_->get();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
Lucene54DocValuesProducer::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(
      Accountables::namedAccountables(L"addresses field", addressInstances));
  resources.addAll(Accountables::namedAccountables(L"reverse index field",
                                                   reverseIndexInstances));
  resources.addAll(Accountables::namedAccountables(
      L"direct addresses meta field", directAddressesMeta));
  return Collections::unmodifiableList(resources);
}

void Lucene54DocValuesProducer::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(data);
}

wstring Lucene54DocValuesProducer::toString()
{
  return getClass().getSimpleName() + L"(fields=" + to_wstring(numFields) +
         L")";
}

shared_ptr<LongValues> Lucene54DocValuesProducer::getNumeric(
    shared_ptr<NumericEntry> entry) 
{
  switch (entry->format) {
  case Lucene54DocValuesFormat::CONST_COMPRESSED: {
    constexpr int64_t constant = entry->minValue;
    shared_ptr<Bits> *const live =
        getLiveBits(entry->missingOffset, static_cast<int>(entry->count));
    return make_shared<LongValuesAnonymousInnerClass>(shared_from_this(),
                                                      constant, live);
  }
  case Lucene54DocValuesFormat::DELTA_COMPRESSED: {
    shared_ptr<RandomAccessInput> slice = this->data->randomAccessSlice(
        entry->offset, entry->endOffset - entry->offset);
    constexpr int64_t delta = entry->minValue;
    shared_ptr<LongValues> *const values =
        DirectReader::getInstance(slice, entry->bitsPerValue, 0);
    return make_shared<LongValuesAnonymousInnerClass2>(shared_from_this(),
                                                       delta, values);
  }
  case Lucene54DocValuesFormat::GCD_COMPRESSED: {
    shared_ptr<RandomAccessInput> slice = this->data->randomAccessSlice(
        entry->offset, entry->endOffset - entry->offset);
    constexpr int64_t min = entry->minValue;
    constexpr int64_t mult = entry->gcd;
    shared_ptr<LongValues> *const quotientReader =
        DirectReader::getInstance(slice, entry->bitsPerValue, 0);
    return make_shared<LongValuesAnonymousInnerClass3>(shared_from_this(), min,
                                                       mult, quotientReader);
  }
  case Lucene54DocValuesFormat::TABLE_COMPRESSED: {
    shared_ptr<RandomAccessInput> slice = this->data->randomAccessSlice(
        entry->offset, entry->endOffset - entry->offset);
    const std::deque<int64_t> table = entry->table;
    shared_ptr<LongValues> *const ords =
        DirectReader::getInstance(slice, entry->bitsPerValue, 0);
    return make_shared<LongValuesAnonymousInnerClass4>(shared_from_this(),
                                                       table, ords);
  }
  case Lucene54DocValuesFormat::SPARSE_COMPRESSED: {
    shared_ptr<SparseNumericDocValues> *const values =
        getSparseNumericDocValues(entry);
    constexpr int64_t missingValue;
    switch (entry->numberType) {
    case NumberType::ORDINAL:
      missingValue = -1LL;
      break;
    case NumberType::VALUE:
      missingValue = 0LL;
      break;
    default:
      throw make_shared<AssertionError>();
    }
    return make_shared<SparseNumericDocValuesRandomAccessWrapper>(values,
                                                                  missingValue);
  }
  default:
    throw make_shared<AssertionError>();
  }
}

Lucene54DocValuesProducer::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesProducer> outerInstance, int64_t constant,
        shared_ptr<Bits> live)
{
  this->outerInstance = outerInstance;
  this->constant = constant;
  this->live = live;
}

int64_t
Lucene54DocValuesProducer::LongValuesAnonymousInnerClass::get(int64_t index)
{
  return live->get(static_cast<int>(index)) ? constant : 0;
}

Lucene54DocValuesProducer::LongValuesAnonymousInnerClass2::
    LongValuesAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesProducer> outerInstance, int64_t delta,
        shared_ptr<LongValues> values)
{
  this->outerInstance = outerInstance;
  this->delta = delta;
  this->values = values;
}

int64_t
Lucene54DocValuesProducer::LongValuesAnonymousInnerClass2::get(int64_t id)
{
  return delta + values->get(id);
}

Lucene54DocValuesProducer::LongValuesAnonymousInnerClass3::
    LongValuesAnonymousInnerClass3(
        shared_ptr<Lucene54DocValuesProducer> outerInstance, int64_t min,
        int64_t mult, shared_ptr<LongValues> quotientReader)
{
  this->outerInstance = outerInstance;
  this->min = min;
  this->mult = mult;
  this->quotientReader = quotientReader;
}

int64_t
Lucene54DocValuesProducer::LongValuesAnonymousInnerClass3::get(int64_t id)
{
  return min + mult * quotientReader->get(id);
}

Lucene54DocValuesProducer::LongValuesAnonymousInnerClass4::
    LongValuesAnonymousInnerClass4(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        deque<int64_t> &table, shared_ptr<LongValues> ords)
{
  this->outerInstance = outerInstance;
  this->table = table;
  this->ords = ords;
}

int64_t
Lucene54DocValuesProducer::LongValuesAnonymousInnerClass4::get(int64_t id)
{
  return table[static_cast<int>(ords->get(id))];
}

Lucene54DocValuesProducer::SparseNumericDocValues::SparseNumericDocValues(
    int docIDsLength, shared_ptr<LongValues> docIDs,
    shared_ptr<LongValues> values)
    : docIDsLength(docIDsLength), docIds(docIDs), values(values)
{
  reset();
}

void Lucene54DocValuesProducer::SparseNumericDocValues::reset()
{
  index = -1;
  doc = -1;
}

int Lucene54DocValuesProducer::SparseNumericDocValues::docID() { return doc; }

int Lucene54DocValuesProducer::SparseNumericDocValues::nextDoc() throw(
    IOException)
{
  if (index >= docIDsLength - 1) {
    index = docIDsLength;
    return doc = NO_MORE_DOCS;
  }
  return doc = static_cast<int>(docIds->get(++index));
}

int Lucene54DocValuesProducer::SparseNumericDocValues::advance(
    int target) 
{
  int64_t loIndex = index;
  int64_t step = 1;
  int64_t hiIndex;
  int hiDoc;

  // gallop forward by exponentially growing the interval
  // in order to find an interval so that the target doc
  // is in ]lo, hi]. Compared to a regular binary search,
  // this optimizes the case that the caller performs many
  // advance calls by small deltas
  do {
    hiIndex = index + step;
    if (hiIndex >= docIDsLength) {
      hiIndex = docIDsLength;
      hiDoc = NO_MORE_DOCS;
      break;
    }
    hiDoc = static_cast<int>(docIds->get(hiIndex));
    if (hiDoc >= target) {
      break;
    }
    step <<= 1;
  } while (true);

  // now binary search
  while (loIndex + 1 < hiIndex) {
    constexpr int64_t midIndex = static_cast<int64_t>(
        static_cast<uint64_t>((loIndex + 1 + hiIndex)) >> 1);
    constexpr int midDoc = static_cast<int>(docIds->get(midIndex));
    if (midDoc >= target) {
      hiIndex = midIndex;
      hiDoc = midDoc;
    } else {
      loIndex = midIndex;
    }
  }

  index = static_cast<int>(hiIndex);
  return doc = hiDoc;
}

bool Lucene54DocValuesProducer::SparseNumericDocValues::advanceExact(
    int target) 
{
  if (advance(target) == target) {
    return true;
  }
  --index;
  doc = target;
  return index >= 0 && docIds->get(index) == target;
}

int64_t Lucene54DocValuesProducer::SparseNumericDocValues::longValue()
{
  assert(index >= 0);
  assert(index < docIDsLength);
  return values->get(index);
}

int64_t Lucene54DocValuesProducer::SparseNumericDocValues::cost()
{
  return docIDsLength;
}

Lucene54DocValuesProducer::SparseNumericDocValuesRandomAccessWrapper::
    SparseNumericDocValuesRandomAccessWrapper(
        shared_ptr<SparseNumericDocValues> values, int64_t missingValue)
    : values(values), missingValue(missingValue)
{
}

int64_t
Lucene54DocValuesProducer::SparseNumericDocValuesRandomAccessWrapper::get(
    int64_t longIndex)
{
  constexpr int index = Math::toIntExact(longIndex);
  int doc = values->docID();
  if (doc >= index) {
    values->reset();
  }
  assert(values->docID() < index);
  try {
    doc = values->advance(index);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  if (doc == index) {
    return values->longValue();
  } else {
    return missingValue;
  }
}

shared_ptr<LegacyBinaryDocValues> Lucene54DocValuesProducer::getLegacyBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryEntry> bytes = binaries[field->name];
  switch (bytes->format) {
  case Lucene54DocValuesFormat::BINARY_FIXED_UNCOMPRESSED:
    return getFixedBinary(field, bytes);
  case Lucene54DocValuesFormat::BINARY_VARIABLE_UNCOMPRESSED:
    return getVariableBinary(field, bytes);
  case Lucene54DocValuesFormat::BINARY_PREFIX_COMPRESSED:
    return getCompressedBinary(field, bytes);
  default:
    throw make_shared<AssertionError>();
  }
}

shared_ptr<BinaryDocValues> Lucene54DocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryEntry> be = binaries[field->name];
  return make_shared<LegacyBinaryDocValuesWrapper>(
      getLiveBits(be->missingOffset, maxDoc), getLegacyBinary(field));
}

shared_ptr<LegacyBinaryDocValues> Lucene54DocValuesProducer::getFixedBinary(
    shared_ptr<FieldInfo> field,
    shared_ptr<BinaryEntry> bytes) 
{
  shared_ptr<IndexInput> *const data = this->data->slice(
      L"fixed-binary", bytes->offset, bytes->count * bytes->maxLength);

  shared_ptr<BytesRef> *const term = make_shared<BytesRef>(bytes->maxLength);
  const std::deque<char> buffer = term->bytes;
  constexpr int length = term->length = bytes->maxLength;

  return make_shared<LongBinaryDocValuesAnonymousInnerClass>(
      shared_from_this(), data, term, buffer, length);
}

Lucene54DocValuesProducer::LongBinaryDocValuesAnonymousInnerClass::
    LongBinaryDocValuesAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<IndexInput> data, shared_ptr<BytesRef> term,
        deque<char> &buffer, int length)
{
  this->outerInstance = outerInstance;
  this->data = data;
  this->term = term;
  this->buffer = buffer;
  this->length = length;
}

shared_ptr<BytesRef>
Lucene54DocValuesProducer::LongBinaryDocValuesAnonymousInnerClass::get(
    int64_t id)
{
  try {
    data->seek(id * length);
    data->readBytes(buffer, 0, buffer.size());
    return term;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

shared_ptr<LegacyBinaryDocValues> Lucene54DocValuesProducer::getVariableBinary(
    shared_ptr<FieldInfo> field,
    shared_ptr<BinaryEntry> bytes) 
{
  shared_ptr<RandomAccessInput> *const addressesData =
      this->data->randomAccessSlice(bytes->addressesOffset,
                                    bytes->addressesEndOffset -
                                        bytes->addressesOffset);
  shared_ptr<LongValues> *const addresses =
      DirectMonotonicReader::getInstance(bytes->addressesMeta, addressesData);

  shared_ptr<IndexInput> *const data = this->data->slice(
      L"var-binary", bytes->offset, bytes->addressesOffset - bytes->offset);
  shared_ptr<BytesRef> *const term =
      make_shared<BytesRef>(max(0, bytes->maxLength));
  const std::deque<char> buffer = term->bytes;

  return make_shared<LongBinaryDocValuesAnonymousInnerClass2>(
      shared_from_this(), addresses, data, term, buffer);
}

Lucene54DocValuesProducer::LongBinaryDocValuesAnonymousInnerClass2::
    LongBinaryDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<LongValues> addresses, shared_ptr<IndexInput> data,
        shared_ptr<BytesRef> term, deque<char> &buffer)
{
  this->outerInstance = outerInstance;
  this->addresses = addresses;
  this->data = data;
  this->term = term;
  this->buffer = buffer;
}

shared_ptr<BytesRef>
Lucene54DocValuesProducer::LongBinaryDocValuesAnonymousInnerClass2::get(
    int64_t id)
{
  int64_t startAddress = addresses->get(id);
  int64_t endAddress = addresses->get(id + 1);
  int length = static_cast<int>(endAddress - startAddress);
  try {
    data->seek(startAddress);
    data->readBytes(buffer, 0, length);
    term->length = length;
    return term;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<MonotonicBlockPackedReader>
Lucene54DocValuesProducer::getIntervalInstance(
    shared_ptr<FieldInfo> field,
    shared_ptr<BinaryEntry> bytes) 
{
  shared_ptr<MonotonicBlockPackedReader> addresses =
      addressInstances[field->name];
  if (addresses == nullptr) {
    data->seek(bytes->addressesOffset);
    constexpr int64_t size = static_cast<int64_t>(
        static_cast<uint64_t>(
            (bytes->count + Lucene54DocValuesFormat::INTERVAL_MASK)) >>
        Lucene54DocValuesFormat::INTERVAL_SHIFT);
    addresses = MonotonicBlockPackedReader::of(data, bytes->packedIntsVersion,
                                               bytes->blockSize, size, false);
    if (!merging) {
      addressInstances.emplace(field->name, addresses);
      ramBytesUsed_->addAndGet(addresses->ramBytesUsed() + Integer::BYTES);
    }
  }
  return addresses;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<ReverseTermsIndex>
Lucene54DocValuesProducer::getReverseIndexInstance(
    shared_ptr<FieldInfo> field,
    shared_ptr<BinaryEntry> bytes) 
{
  shared_ptr<ReverseTermsIndex> index = reverseIndexInstances[field->name];
  if (index == nullptr) {
    index = make_shared<ReverseTermsIndex>();
    data->seek(bytes->reverseIndexOffset);
    int64_t size = static_cast<int64_t>(
        static_cast<uint64_t>(
            (bytes->count + Lucene54DocValuesFormat::REVERSE_INTERVAL_MASK)) >>
        Lucene54DocValuesFormat::REVERSE_INTERVAL_SHIFT);
    index->termAddresses = MonotonicBlockPackedReader::of(
        data, bytes->packedIntsVersion, bytes->blockSize, size, false);
    int64_t dataSize = data->readVLong();
    shared_ptr<PagedBytes> pagedBytes = make_shared<PagedBytes>(15);
    pagedBytes->copy(data, dataSize);
    index->terms = pagedBytes->freeze(true);
    if (!merging) {
      reverseIndexInstances.emplace(field->name, index);
      ramBytesUsed_->addAndGet(index->ramBytesUsed());
    }
  }
  return index;
}

shared_ptr<LegacyBinaryDocValues>
Lucene54DocValuesProducer::getCompressedBinary(
    shared_ptr<FieldInfo> field,
    shared_ptr<BinaryEntry> bytes) 
{
  shared_ptr<MonotonicBlockPackedReader> *const addresses =
      getIntervalInstance(field, bytes);
  shared_ptr<ReverseTermsIndex> *const index =
      getReverseIndexInstance(field, bytes);
  assert(addresses->size() > 0); // we don't have to handle empty case
  shared_ptr<IndexInput> slice = data->slice(
      L"terms", bytes->offset, bytes->addressesOffset - bytes->offset);
  return make_shared<CompressedBinaryDocValues>(bytes, addresses, index, slice);
}

shared_ptr<SortedDocValues> Lucene54DocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  constexpr int valueCount = static_cast<int>(binaries[field->name]->count);
  shared_ptr<LegacyBinaryDocValues> *const binary = getLegacyBinary(field);
  shared_ptr<NumericEntry> entry = ords[field->name];
  shared_ptr<LongValues> *const ordinals = getNumeric(entry);
  if (entry->format == Lucene54DocValuesFormat::SPARSE_COMPRESSED) {
    shared_ptr<SparseNumericDocValues> *const sparseValues =
        (std::static_pointer_cast<SparseNumericDocValuesRandomAccessWrapper>(
             ordinals))
            ->values;
    return make_shared<SortedDocValuesAnonymousInnerClass>(
        shared_from_this(), valueCount, binary, sparseValues);
  }
  return make_shared<SortedDocValuesAnonymousInnerClass2>(
      shared_from_this(), valueCount, binary, ordinals);
}

Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesProducer> outerInstance, int valueCount,
        shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> binary,
        shared_ptr<org::apache::lucene::codecs::lucene54::
                       Lucene54DocValuesProducer::SparseNumericDocValues>
            sparseValues)
{
  this->outerInstance = outerInstance;
  this->valueCount = valueCount;
  this->binary = binary;
  this->sparseValues = sparseValues;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::ordValue()
{
  return static_cast<int>(sparseValues->longValue());
}

shared_ptr<BytesRef>
Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::lookupOrd(
    int ord)
{
  return binary->get(ord);
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::
    getValueCount()
{
  return valueCount;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::docID()
{
  return sparseValues->docID();
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return sparseValues->nextDoc();
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::advance(
    int target) 
{
  return sparseValues->advance(target);
}

bool Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  return sparseValues->advanceExact(target);
}

int64_t Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass::cost()
{
  return sparseValues->cost();
}

Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::
    SortedDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesProducer> outerInstance, int valueCount,
        shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> binary,
        shared_ptr<LongValues> ordinals)
{
  this->outerInstance = outerInstance;
  this->valueCount = valueCount;
  this->binary = binary;
  this->ordinals = ordinals;
  docID = -1;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::docID()
{
  return docID;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::
    nextDoc() 
{
  assert(docID != NO_MORE_DOCS);
  while (true) {
    docID++;
    if (docID == outerInstance->maxDoc) {
      docID = NO_MORE_DOCS;
      break;
    }
    ord = static_cast<int>(ordinals->get(docID));
    if (ord != -1) {
      break;
    }
  }
  return docID;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::advance(
    int target) 
{
  if (target >= outerInstance->maxDoc) {
    docID = NO_MORE_DOCS;
    return docID;
  } else {
    docID = target - 1;
    return nextDoc();
  }
}

bool Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  docID = target;
  ord = static_cast<int>(ordinals->get(target));
  return ord != -1;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::ordValue()
{
  return ord;
}

int64_t Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::cost()
{
  // TODO
  return 0;
}

shared_ptr<BytesRef>
Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::lookupOrd(
    int ord)
{
  return binary->get(ord);
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::
    getValueCount()
{
  return valueCount;
}

int Lucene54DocValuesProducer::SortedDocValuesAnonymousInnerClass2::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  if (std::dynamic_pointer_cast<CompressedBinaryDocValues>(binary) != nullptr) {
    return static_cast<int>(
        (std::static_pointer_cast<CompressedBinaryDocValues>(binary))
            ->lookupTerm(key));
  } else {
    return outerInstance->super->lookupTerm(key);
  }
}

shared_ptr<TermsEnum> Lucene54DocValuesProducer::
    SortedDocValuesAnonymousInnerClass2::termsEnum() 
{
  if (std::dynamic_pointer_cast<CompressedBinaryDocValues>(binary) != nullptr) {
    return (std::static_pointer_cast<CompressedBinaryDocValues>(binary))
        ->getTermsEnum();
  } else {
    return outerInstance->super->termsEnum();
  }
}

shared_ptr<LongValues> Lucene54DocValuesProducer::getOrdIndexInstance(
    shared_ptr<FieldInfo> field,
    shared_ptr<NumericEntry> entry) 
{
  shared_ptr<RandomAccessInput> data = this->data->randomAccessSlice(
      entry->offset, entry->endOffset - entry->offset);
  return DirectMonotonicReader::getInstance(entry->monotonicMeta, data);
}

shared_ptr<SortedNumericDocValues> Lucene54DocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedSetEntry> ss = sortedNumerics[field->name];
  if (ss->format == Lucene54DocValuesFormat::SORTED_SINGLE_VALUED) {
    shared_ptr<NumericEntry> numericEntry = numerics[field->name];
    shared_ptr<LongValues> *const values = getNumeric(numericEntry);
    if (numericEntry->format == Lucene54DocValuesFormat::SPARSE_COMPRESSED) {
      shared_ptr<SparseNumericDocValues> sparseValues =
          (std::static_pointer_cast<SparseNumericDocValuesRandomAccessWrapper>(
               values))
              ->values;
      return make_shared<SortedNumericDocValuesAnonymousInnerClass>(
          shared_from_this(), sparseValues);
    }
    shared_ptr<Bits> *const docsWithField =
        getLiveBits(numericEntry->missingOffset, maxDoc);
    return make_shared<SortedNumericDocValuesAnonymousInnerClass2>(
        shared_from_this(), values, docsWithField);
  } else if (ss->format == Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES) {
    shared_ptr<NumericEntry> numericEntry = numerics[field->name];
    shared_ptr<LongValues> *const values = getNumeric(numericEntry);
    shared_ptr<LongValues> *const ordIndex =
        getOrdIndexInstance(field, ordIndexes[field->name]);

    return make_shared<SortedNumericDocValuesAnonymousInnerClass3>(
        shared_from_this(), values, ordIndex);
  } else if (ss->format == Lucene54DocValuesFormat::SORTED_SET_TABLE) {
    shared_ptr<NumericEntry> entry = ords[field->name];
    shared_ptr<LongValues> *const ordinals = getNumeric(entry);

    const std::deque<int64_t> table = ss->table;
    const std::deque<int> offsets = ss->tableOffsets;
    return make_shared<SortedNumericDocValuesAnonymousInnerClass4>(
        shared_from_this(), ordinals, table, offsets);
  } else {
    throw make_shared<AssertionError>();
  }
}

Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene54::
                       Lucene54DocValuesProducer::SparseNumericDocValues>
            sparseValues)
{
  this->outerInstance = outerInstance;
  this->sparseValues = sparseValues;
}

int64_t Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    nextValue() 
{
  return sparseValues->longValue();
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    docValueCount()
{
  return 1;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    docID()
{
  return sparseValues->docID();
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return sparseValues->nextDoc();
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    advance(int target) 
{
  return sparseValues->advance(target);
}

bool Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  return sparseValues->advanceExact(target);
}

int64_t
Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return sparseValues->cost();
}

Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    SortedNumericDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<LongValues> values, shared_ptr<Bits> docsWithField)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->docsWithField = docsWithField;
  docID = -1;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    docID()
{
  return docID;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    nextDoc()
{
  while (true) {
    docID++;
    if (docID == outerInstance->maxDoc) {
      docID = NO_MORE_DOCS;
      break;
    }

    if (docsWithField->get(docID)) {
      // TODO: use .nextSetBit here, at least!!
      break;
    }
  }
  return docID;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    advance(int target)
{
  if (target >= outerInstance->maxDoc) {
    docID = NO_MORE_DOCS;
    return docID;
  } else {
    docID = target - 1;
    return nextDoc();
  }
}

bool Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  docID = target;
  return docsWithField->get(docID);
}

int64_t
Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::cost()
{
  // TODO
  return 0;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    docValueCount()
{
  return 1;
}

int64_t Lucene54DocValuesProducer::
    SortedNumericDocValuesAnonymousInnerClass2::nextValue()
{
  return values->get(docID);
}

Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::
    SortedNumericDocValuesAnonymousInnerClass3(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<LongValues> values, shared_ptr<LongValues> ordIndex)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->ordIndex = ordIndex;
  docID = -1;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::
    docID()
{
  return docID;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::
    nextDoc()
{
  while (true) {
    docID++;
    if (docID == outerInstance->maxDoc) {
      docID = NO_MORE_DOCS;
      return docID;
    }
    startOffset = ordIndex->get(docID);
    endOffset = ordIndex->get(docID + 1LL);
    if (endOffset > startOffset) {
      break;
    }
  }
  upto = startOffset;
  return docID;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::
    advance(int target)
{
  if (target >= outerInstance->maxDoc) {
    docID = NO_MORE_DOCS;
    return docID;
  } else {
    docID = target - 1;
    return nextDoc();
  }
}

bool Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::
    advanceExact(int target) 
{
  docID = target;
  startOffset = ordIndex->get(docID);
  endOffset = ordIndex->get(docID + 1LL);
  upto = startOffset;
  return endOffset > startOffset;
}

int64_t
Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::cost()
{
  // TODO
  return 0;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass3::
    docValueCount()
{
  return static_cast<int>(endOffset - startOffset);
}

int64_t Lucene54DocValuesProducer::
    SortedNumericDocValuesAnonymousInnerClass3::nextValue()
{
  return values->get(upto++);
}

Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::
    SortedNumericDocValuesAnonymousInnerClass4(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        shared_ptr<LongValues> ordinals, deque<int64_t> &table,
        deque<int> &offsets)
{
  this->outerInstance = outerInstance;
  this->ordinals = ordinals;
  this->table = table;
  this->offsets = offsets;
  docID = -1;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::
    docID()
{
  return docID;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::
    nextDoc()
{
  while (true) {
    docID++;
    if (docID == outerInstance->maxDoc) {
      docID = NO_MORE_DOCS;
      return docID;
    }
    int ord = static_cast<int>(ordinals->get(docID));
    startOffset = offsets[ord];
    endOffset = offsets[ord + 1];
    if (endOffset > startOffset) {
      break;
    }
  }
  upto = startOffset;
  return docID;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::
    advance(int target)
{
  if (target >= outerInstance->maxDoc) {
    docID = NO_MORE_DOCS;
    return docID;
  } else {
    docID = target - 1;
    return nextDoc();
  }
}

bool Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::
    advanceExact(int target) 
{
  docID = target;
  int ord = static_cast<int>(ordinals->get(docID));
  startOffset = offsets[ord];
  endOffset = offsets[ord + 1];
  upto = startOffset;
  return endOffset > startOffset;
}

int64_t
Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::cost()
{
  // TODO
  return 0;
}

int Lucene54DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass4::
    docValueCount()
{
  return endOffset - startOffset;
}

int64_t Lucene54DocValuesProducer::
    SortedNumericDocValuesAnonymousInnerClass4::nextValue()
{
  return table[upto++];
}

shared_ptr<SortedSetDocValues> Lucene54DocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedSetEntry> ss = sortedSets[field->name];
  switch (ss->format) {
  case Lucene54DocValuesFormat::SORTED_SINGLE_VALUED:
    return DocValues::singleton(getSorted(field));
  case Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES:
    return getSortedSetWithAddresses(field);
  case Lucene54DocValuesFormat::SORTED_SET_TABLE:
    return getSortedSetTable(field, ss);
  default:
    throw make_shared<AssertionError>();
  }
}

shared_ptr<SortedSetDocValues>
Lucene54DocValuesProducer::getSortedSetWithAddresses(
    shared_ptr<FieldInfo> field) 
{
  constexpr int64_t valueCount = binaries[field->name]->count;
  // we keep the byte[]s and deque of ords on disk, these could be large
  shared_ptr<LongBinaryDocValues> *const binary =
      std::static_pointer_cast<LongBinaryDocValues>(getLegacyBinary(field));
  shared_ptr<LongValues> *const ordinals = getNumeric(ords[field->name]);
  // but the addresses to the ord stream are in RAM
  shared_ptr<LongValues> *const ordIndex =
      getOrdIndexInstance(field, ordIndexes[field->name]);

  return make_shared<LegacySortedSetDocValuesWrapper>(
      make_shared<LegacySortedSetDocValuesAnonymousInnerClass>(
          shared_from_this(), valueCount, binary, ordinals, ordIndex),
      maxDoc);
}

Lucene54DocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    LegacySortedSetDocValuesAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int64_t valueCount,
        shared_ptr<org::apache::lucene::codecs::lucene54::
                       Lucene54DocValuesProducer::LongBinaryDocValues>
            binary,
        shared_ptr<LongValues> ordinals, shared_ptr<LongValues> ordIndex)
{
  this->outerInstance = outerInstance;
  this->valueCount = valueCount;
  this->binary = binary;
  this->ordinals = ordinals;
  this->ordIndex = ordIndex;
}

int64_t Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass::nextOrd()
{
  if (offset == endOffset) {
    return NO_MORE_ORDS;
  } else {
    int64_t ord = ordinals->get(offset);
    offset++;
    return ord;
  }
}

void Lucene54DocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    setDocument(int docID)
{
  startOffset = offset = ordIndex->get(docID);
  endOffset = ordIndex->get(docID + 1LL);
}

shared_ptr<BytesRef> Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass::lookupOrd(int64_t ord)
{
  return binary->get(ord);
}

int64_t Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass::getValueCount()
{
  return valueCount;
}

int64_t
Lucene54DocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    lookupTerm(shared_ptr<BytesRef> key)
{
  if (std::dynamic_pointer_cast<CompressedBinaryDocValues>(binary) != nullptr) {
    return (std::static_pointer_cast<CompressedBinaryDocValues>(binary))
        ->lookupTerm(key);
  } else {
    return outerInstance->super->lookupTerm(key);
  }
}

shared_ptr<TermsEnum> Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass::termsEnum() 
{
  if (std::dynamic_pointer_cast<CompressedBinaryDocValues>(binary) != nullptr) {
    return (std::static_pointer_cast<CompressedBinaryDocValues>(binary))
        ->getTermsEnum();
  } else {
    return outerInstance->super->termsEnum();
  }
}

shared_ptr<SortedSetDocValues> Lucene54DocValuesProducer::getSortedSetTable(
    shared_ptr<FieldInfo> field,
    shared_ptr<SortedSetEntry> ss) 
{
  constexpr int64_t valueCount = binaries[field->name]->count;
  shared_ptr<LongBinaryDocValues> *const binary =
      std::static_pointer_cast<LongBinaryDocValues>(getLegacyBinary(field));
  shared_ptr<NumericEntry> *const ordinalsEntry = ords[field->name];
  shared_ptr<LongValues> *const ordinals = getNumeric(ordinalsEntry);

  const std::deque<int64_t> table = ss->table;
  const std::deque<int> offsets = ss->tableOffsets;

  return make_shared<LegacySortedSetDocValuesWrapper>(
      make_shared<LegacySortedSetDocValuesAnonymousInnerClass2>(
          shared_from_this(), valueCount, binary, ordinals, table, offsets),
      maxDoc);
}

Lucene54DocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass2::
    LegacySortedSetDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int64_t valueCount,
        shared_ptr<org::apache::lucene::codecs::lucene54::
                       Lucene54DocValuesProducer::LongBinaryDocValues>
            binary,
        shared_ptr<LongValues> ordinals, deque<int64_t> &table,
        deque<int> &offsets)
{
  this->outerInstance = outerInstance;
  this->valueCount = valueCount;
  this->binary = binary;
  this->ordinals = ordinals;
  this->table = table;
  this->offsets = offsets;
}

void Lucene54DocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass2::
    setDocument(int docID)
{
  constexpr int ord = static_cast<int>(ordinals->get(docID));
  offset = startOffset = offsets[ord];
  endOffset = offsets[ord + 1];
}

int64_t Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass2::nextOrd()
{
  if (offset == endOffset) {
    return NO_MORE_ORDS;
  } else {
    return table[offset++];
  }
}

shared_ptr<BytesRef> Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass2::lookupOrd(int64_t ord)
{
  return binary->get(ord);
}

int64_t Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass2::getValueCount()
{
  return valueCount;
}

int64_t
Lucene54DocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass2::
    lookupTerm(shared_ptr<BytesRef> key)
{
  if (std::dynamic_pointer_cast<CompressedBinaryDocValues>(binary) != nullptr) {
    return (std::static_pointer_cast<CompressedBinaryDocValues>(binary))
        ->lookupTerm(key);
  } else {
    return outerInstance->super->lookupTerm(key);
  }
}

shared_ptr<TermsEnum> Lucene54DocValuesProducer::
    LegacySortedSetDocValuesAnonymousInnerClass2::termsEnum() 
{
  if (std::dynamic_pointer_cast<CompressedBinaryDocValues>(binary) != nullptr) {
    return (std::static_pointer_cast<CompressedBinaryDocValues>(binary))
        ->getTermsEnum();
  } else {
    return outerInstance->super->termsEnum();
  }
}

shared_ptr<Bits>
Lucene54DocValuesProducer::getLiveBits(int64_t const offset,
                                       int const count) 
{
  if (offset == Lucene54DocValuesFormat::ALL_MISSING) {
    return make_shared<Bits::MatchNoBits>(count);
  } else if (offset == Lucene54DocValuesFormat::ALL_LIVE) {
    return make_shared<Bits::MatchAllBits>(count);
  } else {
    int length = static_cast<int>(static_cast<int64_t>(
        static_cast<uint64_t>((count + 7LL)) >> 3));
    shared_ptr<RandomAccessInput> *const in_ =
        data->randomAccessSlice(offset, length);
    return make_shared<BitsAnonymousInnerClass>(shared_from_this(), count, in_);
  }
}

Lucene54DocValuesProducer::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    shared_ptr<Lucene54DocValuesProducer> outerInstance, int count,
    shared_ptr<RandomAccessInput> in_)
{
  this->outerInstance = outerInstance;
  this->count = count;
  this->in_ = in_;
}

bool Lucene54DocValuesProducer::BitsAnonymousInnerClass::get(int index)
{
  try {
    return (in_->readByte(index >> 3) & (1 << (index & 7))) != 0;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

int Lucene54DocValuesProducer::BitsAnonymousInnerClass::length()
{
  return count;
}

shared_ptr<SparseNumericDocValues>
Lucene54DocValuesProducer::getSparseNumericDocValues(
    shared_ptr<NumericEntry> entry) 
{
  shared_ptr<RandomAccessInput> *const docIdsData =
      this->data->randomAccessSlice(entry->missingOffset,
                                    entry->offset - entry->missingOffset);
  shared_ptr<LongValues> *const docIDs =
      DirectMonotonicReader::getInstance(entry->monotonicMeta, docIdsData);
  shared_ptr<LongValues> *const values =
      getNumeric(entry->nonMissingValues); // cannot be sparse
  return make_shared<SparseNumericDocValues>(
      Math::toIntExact(entry->numDocsWithValue), docIDs, values);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocValuesProducer>
Lucene54DocValuesProducer::getMergeInstance() 
{
  return make_shared<Lucene54DocValuesProducer>(shared_from_this());
}

Lucene54DocValuesProducer::~Lucene54DocValuesProducer() { delete data; }

Lucene54DocValuesProducer::NumericEntry::NumericEntry() {}

Lucene54DocValuesProducer::BinaryEntry::BinaryEntry() {}

Lucene54DocValuesProducer::SortedSetEntry::SortedSetEntry() {}

shared_ptr<BytesRef>
Lucene54DocValuesProducer::LongBinaryDocValues::get(int docID)
{
  return get(static_cast<int64_t>(docID));
}

int64_t Lucene54DocValuesProducer::ReverseTermsIndex::ramBytesUsed()
{
  return termAddresses->ramBytesUsed() + terms->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
Lucene54DocValuesProducer::ReverseTermsIndex::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.push_back(Accountables::namedAccountable(L"term bytes", terms));
  resources.push_back(
      Accountables::namedAccountable(L"term addresses", termAddresses));
  return Collections::unmodifiableList(resources);
}

wstring Lucene54DocValuesProducer::ReverseTermsIndex::toString()
{
  return getClass().getSimpleName() + L"(size=" +
         to_wstring(termAddresses->size()) + L")";
}

Lucene54DocValuesProducer::CompressedBinaryDocValues::CompressedBinaryDocValues(
    shared_ptr<BinaryEntry> bytes,
    shared_ptr<MonotonicBlockPackedReader> addresses,
    shared_ptr<ReverseTermsIndex> index,
    shared_ptr<IndexInput> data) 
    : numValues(bytes->count), numIndexValues(addresses->size()),
      maxTermLength(bytes->maxLength), addresses(addresses), data(data),
      termsEnum(getTermsEnum(data)), reverseTerms(index->terms),
      reverseAddresses(index->termAddresses),
      numReverseIndexValues(reverseAddresses->size())
{
}

shared_ptr<BytesRef>
Lucene54DocValuesProducer::CompressedBinaryDocValues::get(int64_t id)
{
  try {
    termsEnum->seekExact(id);
    return termsEnum->term();
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

int64_t Lucene54DocValuesProducer::CompressedBinaryDocValues::lookupTerm(
    shared_ptr<BytesRef> key)
{
  try {
    switch (termsEnum->seekCeil(key)) {
    case FOUND:
      return termsEnum->ord();
    case NOT_FOUND:
      return -termsEnum->ord() - 1;
    default:
      return -numValues - 1;
    }
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

shared_ptr<TermsEnum>
Lucene54DocValuesProducer::CompressedBinaryDocValues::getTermsEnum() throw(
    IOException)
{
  return getTermsEnum(data->clone());
}

shared_ptr<CompressedBinaryTermsEnum>
Lucene54DocValuesProducer::CompressedBinaryDocValues::getTermsEnum(
    shared_ptr<IndexInput> input) 
{
  return make_shared<CompressedBinaryTermsEnum>(shared_from_this(), input);
}

Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::CompressedBinaryTermsEnum(
        shared_ptr<Lucene54DocValuesProducer::CompressedBinaryDocValues>
            outerInstance,
        shared_ptr<IndexInput> input) 
    : input(input), outerInstance(outerInstance)
{
  input->seek(0);
}

void Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::readHeader() 
{
  firstTerm->length = input->readVInt();
  input->readBytes(firstTerm->bytes, 0, firstTerm->length);
  input->readBytes(buffer, 0, Lucene54DocValuesFormat::INTERVAL_COUNT - 1);
  if (buffer[0] == -1) {
    readShortAddresses();
  } else {
    readByteAddresses();
  }
  currentBlockStart = input->getFilePointer();
}

void Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::readByteAddresses() 
{
  int addr = 0;
  for (int i = 1; i < offsets.size(); i++) {
    addr += 2 + (buffer[i - 1] & 0xFF);
    offsets[i] = addr;
  }
}

void Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::readShortAddresses() 
{
  input->readBytes(buffer, Lucene54DocValuesFormat::INTERVAL_COUNT - 1,
                   Lucene54DocValuesFormat::INTERVAL_COUNT);
  int addr = 0;
  for (int i = 1; i < offsets.size(); i++) {
    int x = i << 1;
    addr += 2 + ((buffer[x - 1] << 8) | (buffer[x] & 0xFF));
    offsets[i] = addr;
  }
}

void Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::readFirstTerm() 
{
  term_->length = firstTerm->length;
  System::arraycopy(firstTerm->bytes, firstTerm->offset, term_->bytes, 0,
                    term_->length);
}

void Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::readTerm(int offset) 
{
  int start = input->readByte() & 0xFF;
  System::arraycopy(firstTerm->bytes, firstTerm->offset, term_->bytes, 0,
                    start);
  int suffix = offsets[offset] - offsets[offset - 1] - 1;
  input->readBytes(term_->bytes, start, suffix);
  term_->length = start + suffix;
}

shared_ptr<BytesRef> Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::next() 
{
  currentOrd++;
  if (currentOrd >= outerInstance->numValues) {
    return nullptr;
  } else {
    int offset =
        static_cast<int>(currentOrd & Lucene54DocValuesFormat::INTERVAL_MASK);
    if (offset == 0) {
      // switch to next block
      readHeader();
      readFirstTerm();
    } else {
      readTerm(offset);
    }
    return term_;
  }
}

int64_t Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::binarySearchIndex(
        shared_ptr<BytesRef> text) 
{
  int64_t low = 0;
  int64_t high = outerInstance->numReverseIndexValues - 1;
  while (low <= high) {
    int64_t mid = static_cast<int64_t>(
        static_cast<uint64_t>((low + high)) >> 1);
    outerInstance->reverseTerms->fill(
        scratch, outerInstance->reverseAddresses->get(mid));
    int cmp = scratch->compareTo(text);

    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      return mid;
    }
  }
  return high;
}

int64_t Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::binarySearchBlock(
        shared_ptr<BytesRef> text, int64_t low,
        int64_t high) 
{
  while (low <= high) {
    int64_t mid = static_cast<int64_t>(
        static_cast<uint64_t>((low + high)) >> 1);
    input->seek(outerInstance->addresses->get(mid));
    term_->length = input->readVInt();
    input->readBytes(term_->bytes, 0, term_->length);
    int cmp = term_->compareTo(text);

    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      return mid;
    }
  }
  return high;
}

SeekStatus Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::seekCeil(shared_ptr<BytesRef> text) throw(
        IOException)
{
  // locate block: narrow to block range with index, then search blocks
  constexpr int64_t block;
  int64_t indexPos = binarySearchIndex(text);
  if (indexPos < 0) {
    block = 0;
  } else {
    int64_t low = indexPos << Lucene54DocValuesFormat::BLOCK_INTERVAL_SHIFT;
    int64_t high = min(outerInstance->numIndexValues - 1,
                         low + Lucene54DocValuesFormat::BLOCK_INTERVAL_MASK);
    block = max(low, binarySearchBlock(text, low, high));
  }

  // position before block, then scan to term.
  input->seek(outerInstance->addresses->get(block));
  currentOrd = (block << Lucene54DocValuesFormat::INTERVAL_SHIFT) - 1;

  while (next() != nullptr) {
    int cmp = term_->compareTo(text);
    if (cmp == 0) {
      return SeekStatus::FOUND;
    } else if (cmp > 0) {
      return SeekStatus::NOT_FOUND;
    }
  }
  return SeekStatus::END;
}

void Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::seekExact(int64_t ord) 
{
  int64_t block =
      static_cast<int64_t>(static_cast<uint64_t>(ord) >>
                             Lucene54DocValuesFormat::INTERVAL_SHIFT);
  if (block !=
      static_cast<int64_t>(static_cast<uint64_t>(currentOrd) >>
                             Lucene54DocValuesFormat::INTERVAL_SHIFT)) {
    // switch to different block
    input->seek(outerInstance->addresses->get(block));
    readHeader();
  }

  currentOrd = ord;

  int offset = static_cast<int>(ord & Lucene54DocValuesFormat::INTERVAL_MASK);
  if (offset == 0) {
    readFirstTerm();
  } else {
    input->seek(currentBlockStart + offsets[offset - 1]);
    readTerm(offset);
  }
}

shared_ptr<BytesRef> Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::term() 
{
  return term_;
}

int64_t Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::ord() 
{
  return currentOrd;
}

int Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::docFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::totalTermFreq() 
{
  return -1;
}

shared_ptr<PostingsEnum> Lucene54DocValuesProducer::CompressedBinaryDocValues::
    CompressedBinaryTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                        int flags) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::lucene54