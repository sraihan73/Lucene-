using namespace std;

#include "Lucene70DocValuesProducer.h"

namespace org::apache::lucene::codecs::lucene70
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DocValues = org::apache::lucene::index::DocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using DirectMonotonicReader =
    org::apache::lucene::util::packed::DirectMonotonicReader;
using DirectReader = org::apache::lucene::util::packed::DirectReader;

Lucene70DocValuesProducer::Lucene70DocValuesProducer(
    shared_ptr<SegmentReadState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : data(state->directory->openInput(dataName, state->context)),
      maxDoc(state->segmentInfo->maxDoc())
{
  wstring metaName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, metaExtension);
  ramBytesUsed_ = RamUsageEstimator::shallowSizeOfInstance(getClass());

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
          in_, metaCodec, Lucene70DocValuesFormat::VERSION_START,
          Lucene70DocValuesFormat::VERSION_CURRENT, state->segmentInfo->getId(),
          state->segmentSuffix);
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
        data, dataCodec, Lucene70DocValuesFormat::VERSION_START,
        Lucene70DocValuesFormat::VERSION_CURRENT, state->segmentInfo->getId(),
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

void Lucene70DocValuesProducer::readFields(
    shared_ptr<ChecksumIndexInput> meta,
    shared_ptr<FieldInfos> infos) 
{
  for (int fieldNumber = meta->readInt(); fieldNumber != -1;
       fieldNumber = meta->readInt()) {
    shared_ptr<FieldInfo> info = infos->fieldInfo(fieldNumber);
    if (info == nullptr) {
      throw make_shared<CorruptIndexException>(
          L"Invalid field number: " + to_wstring(fieldNumber), meta);
    }
    char type = meta->readByte();
    if (type == Lucene70DocValuesFormat::NUMERIC) {
      numerics.emplace(info->name, readNumeric(meta));
    } else if (type == Lucene70DocValuesFormat::BINARY) {
      binaries.emplace(info->name, readBinary(meta));
    } else if (type == Lucene70DocValuesFormat::SORTED) {
      sorted.emplace(info->name, readSorted(meta));
    } else if (type == Lucene70DocValuesFormat::SORTED_SET) {
      sortedSets.emplace(info->name, readSortedSet(meta));
    } else if (type == Lucene70DocValuesFormat::SORTED_NUMERIC) {
      sortedNumerics.emplace(info->name, readSortedNumeric(meta));
    } else {
      throw make_shared<CorruptIndexException>(
          L"invalid type: " + to_wstring(type), meta);
    }
  }
}

shared_ptr<NumericEntry> Lucene70DocValuesProducer::readNumeric(
    shared_ptr<ChecksumIndexInput> meta) 
{
  shared_ptr<NumericEntry> entry = make_shared<NumericEntry>();
  readNumeric(meta, entry);
  return entry;
}

void Lucene70DocValuesProducer::readNumeric(
    shared_ptr<ChecksumIndexInput> meta,
    shared_ptr<NumericEntry> entry) 
{
  entry->docsWithFieldOffset = meta->readLong();
  entry->docsWithFieldLength = meta->readLong();
  entry->numValues = meta->readLong();
  int tableSize = meta->readInt();
  if (tableSize > 256) {
    throw make_shared<CorruptIndexException>(
        L"invalid table size: " + to_wstring(tableSize), meta);
  }
  if (tableSize >= 0) {
    entry->table = std::deque<int64_t>(tableSize);
    ramBytesUsed_ += RamUsageEstimator::sizeOf(entry->table);
    for (int i = 0; i < tableSize; ++i) {
      entry->table[i] = meta->readLong();
    }
  }
  if (tableSize < -1) {
    entry->blockShift = -2 - tableSize;
  } else {
    entry->blockShift = -1;
  }
  entry->bitsPerValue = meta->readByte();
  entry->minValue = meta->readLong();
  entry->gcd = meta->readLong();
  entry->valuesOffset = meta->readLong();
  entry->valuesLength = meta->readLong();
}

shared_ptr<BinaryEntry> Lucene70DocValuesProducer::readBinary(
    shared_ptr<ChecksumIndexInput> meta) 
{
  shared_ptr<BinaryEntry> entry = make_shared<BinaryEntry>();
  entry->dataOffset = meta->readLong();
  entry->dataLength = meta->readLong();
  entry->docsWithFieldOffset = meta->readLong();
  entry->docsWithFieldLength = meta->readLong();
  entry->numDocsWithField = meta->readInt();
  entry->minLength = meta->readInt();
  entry->maxLength = meta->readInt();
  if (entry->minLength < entry->maxLength) {
    entry->addressesOffset = meta->readLong();
    constexpr int blockShift = meta->readVInt();
    entry->addressesMeta = DirectMonotonicReader::loadMeta(
        meta, entry->numDocsWithField + 1LL, blockShift);
    ramBytesUsed_ += entry->addressesMeta->ramBytesUsed();
    entry->addressesLength = meta->readLong();
  }
  return entry;
}

shared_ptr<SortedEntry> Lucene70DocValuesProducer::readSorted(
    shared_ptr<ChecksumIndexInput> meta) 
{
  shared_ptr<SortedEntry> entry = make_shared<SortedEntry>();
  entry->docsWithFieldOffset = meta->readLong();
  entry->docsWithFieldLength = meta->readLong();
  entry->numDocsWithField = meta->readInt();
  entry->bitsPerValue = meta->readByte();
  entry->ordsOffset = meta->readLong();
  entry->ordsLength = meta->readLong();
  readTermDict(meta, entry);
  return entry;
}

shared_ptr<SortedSetEntry> Lucene70DocValuesProducer::readSortedSet(
    shared_ptr<ChecksumIndexInput> meta) 
{
  shared_ptr<SortedSetEntry> entry = make_shared<SortedSetEntry>();
  char multiValued = meta->readByte();
  switch (multiValued) {
  case 0: // singlevalued
    entry->singleValueEntry = readSorted(meta);
    return entry;
  case 1: // multivalued
    break;
  default:
    throw make_shared<CorruptIndexException>(
        L"Invalid multiValued flag: " + to_wstring(multiValued), meta);
  }
  entry->docsWithFieldOffset = meta->readLong();
  entry->docsWithFieldLength = meta->readLong();
  entry->bitsPerValue = meta->readByte();
  entry->ordsOffset = meta->readLong();
  entry->ordsLength = meta->readLong();
  entry->numDocsWithField = meta->readInt();
  entry->addressesOffset = meta->readLong();
  constexpr int blockShift = meta->readVInt();
  entry->addressesMeta = DirectMonotonicReader::loadMeta(
      meta, entry->numDocsWithField + 1, blockShift);
  ramBytesUsed_ += entry->addressesMeta->ramBytesUsed();
  entry->addressesLength = meta->readLong();
  readTermDict(meta, entry);
  return entry;
}

void Lucene70DocValuesProducer::readTermDict(
    shared_ptr<ChecksumIndexInput> meta,
    shared_ptr<TermsDictEntry> entry) 
{
  entry->termsDictSize = meta->readVLong();
  entry->termsDictBlockShift = meta->readInt();
  constexpr int blockShift = meta->readInt();
  constexpr int64_t addressesSize = static_cast<int64_t>(
      static_cast<uint64_t>(
          (entry->termsDictSize + (1LL << entry->termsDictBlockShift) - 1)) >>
      entry->termsDictBlockShift);
  entry->termsAddressesMeta =
      DirectMonotonicReader::loadMeta(meta, addressesSize, blockShift);
  entry->maxTermLength = meta->readInt();
  entry->termsDataOffset = meta->readLong();
  entry->termsDataLength = meta->readLong();
  entry->termsAddressesOffset = meta->readLong();
  entry->termsAddressesLength = meta->readLong();
  entry->termsDictIndexShift = meta->readInt();
  constexpr int64_t indexSize = static_cast<int64_t>(
      static_cast<uint64_t>(
          (entry->termsDictSize + (1LL << entry->termsDictIndexShift) - 1)) >>
      entry->termsDictIndexShift);
  entry->termsIndexAddressesMeta =
      DirectMonotonicReader::loadMeta(meta, 1 + indexSize, blockShift);
  entry->termsIndexOffset = meta->readLong();
  entry->termsIndexLength = meta->readLong();
  entry->termsIndexAddressesOffset = meta->readLong();
  entry->termsIndexAddressesLength = meta->readLong();
}

shared_ptr<SortedNumericEntry> Lucene70DocValuesProducer::readSortedNumeric(
    shared_ptr<ChecksumIndexInput> meta) 
{
  shared_ptr<SortedNumericEntry> entry = make_shared<SortedNumericEntry>();
  readNumeric(meta, entry);
  entry->numDocsWithField = meta->readInt();
  if (entry->numDocsWithField != entry->numValues) {
    entry->addressesOffset = meta->readLong();
    constexpr int blockShift = meta->readVInt();
    entry->addressesMeta = DirectMonotonicReader::loadMeta(
        meta, entry->numDocsWithField + 1, blockShift);
    ramBytesUsed_ += entry->addressesMeta->ramBytesUsed();
    entry->addressesLength = meta->readLong();
  }
  return entry;
}

Lucene70DocValuesProducer::~Lucene70DocValuesProducer() { delete data; }

int64_t Lucene70DocValuesProducer::ramBytesUsed() { return ramBytesUsed_; }

shared_ptr<NumericDocValues> Lucene70DocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<NumericEntry> entry = numerics[field->name];
  return getNumeric(entry);
}

Lucene70DocValuesProducer::DenseNumericDocValues::DenseNumericDocValues(
    int maxDoc)
    : maxDoc(maxDoc)
{
}

int Lucene70DocValuesProducer::DenseNumericDocValues::docID() { return doc; }

int Lucene70DocValuesProducer::DenseNumericDocValues::nextDoc() throw(
    IOException)
{
  return advance(doc + 1);
}

int Lucene70DocValuesProducer::DenseNumericDocValues::advance(int target) throw(
    IOException)
{
  if (target >= maxDoc) {
    return doc = NO_MORE_DOCS;
  }
  return doc = target;
}

bool Lucene70DocValuesProducer::DenseNumericDocValues::advanceExact(int target)
{
  doc = target;
  return true;
}

int64_t Lucene70DocValuesProducer::DenseNumericDocValues::cost()
{
  return maxDoc;
}

Lucene70DocValuesProducer::SparseNumericDocValues::SparseNumericDocValues(
    shared_ptr<IndexedDISI> disi)
    : disi(disi)
{
}

int Lucene70DocValuesProducer::SparseNumericDocValues::advance(
    int target) 
{
  return disi->advance(target);
}

bool Lucene70DocValuesProducer::SparseNumericDocValues::advanceExact(
    int target) 
{
  return disi->advanceExact(target);
}

int Lucene70DocValuesProducer::SparseNumericDocValues::nextDoc() throw(
    IOException)
{
  return disi->nextDoc();
}

int Lucene70DocValuesProducer::SparseNumericDocValues::docID()
{
  return disi->docID();
}

int64_t Lucene70DocValuesProducer::SparseNumericDocValues::cost()
{
  return disi->cost();
}

shared_ptr<NumericDocValues> Lucene70DocValuesProducer::getNumeric(
    shared_ptr<NumericEntry> entry) 
{
  if (entry->docsWithFieldOffset == -2) {
    // empty
    return DocValues::emptyNumeric();
  } else if (entry->docsWithFieldOffset == -1) {
    // dense
    if (entry->bitsPerValue == 0) {
      return make_shared<DenseNumericDocValuesAnonymousInnerClass>(
          shared_from_this(), maxDoc, entry);
    } else {
      shared_ptr<RandomAccessInput> *const slice =
          data->randomAccessSlice(entry->valuesOffset, entry->valuesLength);
      if (entry->blockShift >= 0) {
        // dense but split into blocks of different bits per value
        constexpr int shift = entry->blockShift;
        constexpr int64_t mul = entry->gcd;
        constexpr int mask = (1 << shift) - 1;
        return make_shared<DenseNumericDocValuesAnonymousInnerClass2>(
            shared_from_this(), maxDoc, slice, shift, mul, mask);
      } else {
        shared_ptr<LongValues> *const values =
            DirectReader::getInstance(slice, entry->bitsPerValue);
        if (entry->table.size() > 0) {
          const std::deque<int64_t> table = entry->table;
          return make_shared<DenseNumericDocValuesAnonymousInnerClass3>(
              shared_from_this(), maxDoc, values, table);
        } else {
          constexpr int64_t mul = entry->gcd;
          constexpr int64_t delta = entry->minValue;
          return make_shared<DenseNumericDocValuesAnonymousInnerClass4>(
              shared_from_this(), maxDoc, values, mul, delta);
        }
      }
    }
  } else {
    // sparse
    shared_ptr<IndexedDISI> *const disi =
        make_shared<IndexedDISI>(data, entry->docsWithFieldOffset,
                                 entry->docsWithFieldLength, entry->numValues);
    if (entry->bitsPerValue == 0) {
      return make_shared<SparseNumericDocValuesAnonymousInnerClass>(
          shared_from_this(), entry);
    } else {
      shared_ptr<RandomAccessInput> *const slice =
          data->randomAccessSlice(entry->valuesOffset, entry->valuesLength);
      if (entry->blockShift >= 0) {
        // sparse and split into blocks of different bits per value
        constexpr int shift = entry->blockShift;
        constexpr int64_t mul = entry->gcd;
        constexpr int mask = (1 << shift) - 1;
        return make_shared<SparseNumericDocValuesAnonymousInnerClass2>(
            shared_from_this(), disi, slice, shift, mul, mask);
      } else {
        shared_ptr<LongValues> *const values =
            DirectReader::getInstance(slice, entry->bitsPerValue);
        if (entry->table.size() > 0) {
          const std::deque<int64_t> table = entry->table;
          return make_shared<SparseNumericDocValuesAnonymousInnerClass3>(
              shared_from_this(), disi, values, table);
        } else {
          constexpr int64_t mul = entry->gcd;
          constexpr int64_t delta = entry->minValue;
          return make_shared<SparseNumericDocValuesAnonymousInnerClass4>(
              shared_from_this(), disi, values, mul, delta);
        }
      }
    }
  }
}

Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass::
    DenseNumericDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::NumericEntry>
            entry)
    : DenseNumericDocValues(maxDoc)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
}

int64_t Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass::
    longValue() 
{
  return entry->minValue;
}

Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass2::
    DenseNumericDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice, int shift, int64_t mul, int mask)
    : DenseNumericDocValues(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
  this->shift = shift;
  this->mul = mul;
  this->mask = mask;
  block = -1;
}

int64_t Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass2::
    longValue() 
{
  constexpr int block =
      static_cast<int>(static_cast<unsigned int>(doc) >> shift);
  if (this->block != block) {
    int bitsPerValue;
    do {
      offset = blockEndOffset;
      bitsPerValue = slice->readByte(offset++);
      delta = slice->readLong(offset);
      offset += Long::BYTES;
      if (bitsPerValue == 0) {
        blockEndOffset = offset;
      } else {
        constexpr int length = slice->readInt(offset);
        offset += Integer::BYTES;
        blockEndOffset = offset + length;
      }
      this->block++;
    } while (this->block != block);
    values = bitsPerValue == 0
                 ? LongValues::ZEROES
                 : DirectReader::getInstance(slice, bitsPerValue, offset);
  }
  return mul * values->get(doc & mask) + delta;
}

Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass3::
    DenseNumericDocValuesAnonymousInnerClass3(
        shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        shared_ptr<LongValues> values, deque<int64_t> &table)
    : DenseNumericDocValues(maxDoc)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->table = table;
}

int64_t Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass3::
    longValue() 
{
  return table[static_cast<int>(values->get(doc))];
}

Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass4::
    DenseNumericDocValuesAnonymousInnerClass4(
        shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        shared_ptr<LongValues> values, int64_t mul, int64_t delta)
    : DenseNumericDocValues(maxDoc)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->mul = mul;
  this->delta = delta;
}

int64_t Lucene70DocValuesProducer::DenseNumericDocValuesAnonymousInnerClass4::
    longValue() 
{
  return mul * values->get(doc) + delta;
}

Lucene70DocValuesProducer::SparseNumericDocValuesAnonymousInnerClass::
    SparseNumericDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::NumericEntry>
            entry)
    : SparseNumericDocValues(disi)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
}

int64_t Lucene70DocValuesProducer::SparseNumericDocValuesAnonymousInnerClass::
    longValue() 
{
  return entry->minValue;
}

Lucene70DocValuesProducer::SparseNumericDocValuesAnonymousInnerClass2::
    SparseNumericDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<RandomAccessInput> slice, int shift, int64_t mul, int mask)
    : SparseNumericDocValues(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->slice = slice;
  this->shift = shift;
  this->mul = mul;
  this->mask = mask;
  block = -1;
}

int64_t Lucene70DocValuesProducer::
    SparseNumericDocValuesAnonymousInnerClass2::longValue() 
{
  constexpr int index = disi->index();
  constexpr int block =
      static_cast<int>(static_cast<unsigned int>(index) >> shift);
  if (this->block != block) {
    int bitsPerValue;
    do {
      offset = blockEndOffset;
      bitsPerValue = slice->readByte(offset++);
      delta = slice->readLong(offset);
      offset += Long::BYTES;
      if (bitsPerValue == 0) {
        blockEndOffset = offset;
      } else {
        constexpr int length = slice->readInt(offset);
        offset += Integer::BYTES;
        blockEndOffset = offset + length;
      }
      this->block++;
    } while (this->block != block);
    values = bitsPerValue == 0
                 ? LongValues::ZEROES
                 : DirectReader::getInstance(slice, bitsPerValue, offset);
  }
  return mul * values->get(index & mask) + delta;
}

Lucene70DocValuesProducer::SparseNumericDocValuesAnonymousInnerClass3::
    SparseNumericDocValuesAnonymousInnerClass3(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<LongValues> values, deque<int64_t> &table)
    : SparseNumericDocValues(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->values = values;
  this->table = table;
}

int64_t Lucene70DocValuesProducer::
    SparseNumericDocValuesAnonymousInnerClass3::longValue() 
{
  return table[static_cast<int>(values->get(disi->index()))];
}

Lucene70DocValuesProducer::SparseNumericDocValuesAnonymousInnerClass4::
    SparseNumericDocValuesAnonymousInnerClass4(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<LongValues> values, int64_t mul, int64_t delta)
    : SparseNumericDocValues(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->values = values;
  this->mul = mul;
  this->delta = delta;
}

int64_t Lucene70DocValuesProducer::
    SparseNumericDocValuesAnonymousInnerClass4::longValue() 
{
  return mul * values->get(disi->index()) + delta;
}

shared_ptr<LongValues> Lucene70DocValuesProducer::getNumericValues(
    shared_ptr<NumericEntry> entry) 
{
  if (entry->bitsPerValue == 0) {
    return make_shared<LongValuesAnonymousInnerClass>(shared_from_this(),
                                                      entry);
  } else {
    shared_ptr<RandomAccessInput> *const slice =
        data->randomAccessSlice(entry->valuesOffset, entry->valuesLength);
    if (entry->blockShift >= 0) {
      constexpr int shift = entry->blockShift;
      constexpr int64_t mul = entry->gcd;
      constexpr int64_t mask = (1LL << shift) - 1;
      return make_shared<LongValuesAnonymousInnerClass2>(
          shared_from_this(), slice, shift, mul, mask);
    } else {
      shared_ptr<LongValues> *const values =
          DirectReader::getInstance(slice, entry->bitsPerValue);
      if (entry->table.size() > 0) {
        const std::deque<int64_t> table = entry->table;
        return make_shared<LongValuesAnonymousInnerClass3>(shared_from_this(),
                                                           values, table);
      } else if (entry->gcd != 1) {
        constexpr int64_t gcd = entry->gcd;
        constexpr int64_t minValue = entry->minValue;
        return make_shared<LongValuesAnonymousInnerClass4>(
            shared_from_this(), values, gcd, minValue);
      } else if (entry->minValue != 0) {
        constexpr int64_t minValue = entry->minValue;
        return make_shared<LongValuesAnonymousInnerClass5>(shared_from_this(),
                                                           values, minValue);
      } else {
        return values;
      }
    }
  }
}

Lucene70DocValuesProducer::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::NumericEntry>
            entry)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
}

int64_t
Lucene70DocValuesProducer::LongValuesAnonymousInnerClass::get(int64_t index)
{
  return entry->minValue;
}

Lucene70DocValuesProducer::LongValuesAnonymousInnerClass2::
    LongValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<RandomAccessInput> slice, int shift, int64_t mul,
        int64_t mask)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
  this->shift = shift;
  this->mul = mul;
  this->mask = mask;
  block = -1;
}

int64_t
Lucene70DocValuesProducer::LongValuesAnonymousInnerClass2::get(int64_t index)
{
  constexpr int64_t block =
      static_cast<int64_t>(static_cast<uint64_t>(index) >> shift);
  if (this->block != block) {
    assert((block > this->block, L"Reading backwards is illegal: " +
                                     this->block + L" < " + to_wstring(block)));
    int bitsPerValue;
    do {
      offset = blockEndOffset;
      try {
        bitsPerValue = slice->readByte(offset++);
        delta = slice->readLong(offset);
        offset += Long::BYTES;
        if (bitsPerValue == 0) {
          blockEndOffset = offset;
        } else {
          constexpr int length = slice->readInt(offset);
          offset += Integer::BYTES;
          blockEndOffset = offset + length;
        }
      } catch (const IOException &e) {
        throw runtime_error(e);
      }
      this->block++;
    } while (this->block != block);
    values = bitsPerValue == 0
                 ? LongValues::ZEROES
                 : DirectReader::getInstance(slice, bitsPerValue, offset);
  }
  return mul * values->get(index & mask) + delta;
}

Lucene70DocValuesProducer::LongValuesAnonymousInnerClass3::
    LongValuesAnonymousInnerClass3(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<LongValues> values, deque<int64_t> &table)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->table = table;
}

int64_t
Lucene70DocValuesProducer::LongValuesAnonymousInnerClass3::get(int64_t index)
{
  return table[static_cast<int>(values->get(index))];
}

Lucene70DocValuesProducer::LongValuesAnonymousInnerClass4::
    LongValuesAnonymousInnerClass4(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<LongValues> values, int64_t gcd, int64_t minValue)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->gcd = gcd;
  this->minValue = minValue;
}

int64_t
Lucene70DocValuesProducer::LongValuesAnonymousInnerClass4::get(int64_t index)
{
  return values->get(index) * gcd + minValue;
}

Lucene70DocValuesProducer::LongValuesAnonymousInnerClass5::
    LongValuesAnonymousInnerClass5(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<LongValues> values, int64_t minValue)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->minValue = minValue;
}

int64_t
Lucene70DocValuesProducer::LongValuesAnonymousInnerClass5::get(int64_t index)
{
  return values->get(index) + minValue;
}

Lucene70DocValuesProducer::DenseBinaryDocValues::DenseBinaryDocValues(
    int maxDoc)
    : maxDoc(maxDoc)
{
}

int Lucene70DocValuesProducer::DenseBinaryDocValues::nextDoc() throw(
    IOException)
{
  return advance(doc + 1);
}

int Lucene70DocValuesProducer::DenseBinaryDocValues::docID() { return doc; }

int64_t Lucene70DocValuesProducer::DenseBinaryDocValues::cost()
{
  return maxDoc;
}

int Lucene70DocValuesProducer::DenseBinaryDocValues::advance(int target) throw(
    IOException)
{
  if (target >= maxDoc) {
    return doc = NO_MORE_DOCS;
  }
  return doc = target;
}

bool Lucene70DocValuesProducer::DenseBinaryDocValues::advanceExact(
    int target) 
{
  doc = target;
  return true;
}

Lucene70DocValuesProducer::SparseBinaryDocValues::SparseBinaryDocValues(
    shared_ptr<IndexedDISI> disi)
    : disi(disi)
{
}

int Lucene70DocValuesProducer::SparseBinaryDocValues::nextDoc() throw(
    IOException)
{
  return disi->nextDoc();
}

int Lucene70DocValuesProducer::SparseBinaryDocValues::docID()
{
  return disi->docID();
}

int64_t Lucene70DocValuesProducer::SparseBinaryDocValues::cost()
{
  return disi->cost();
}

int Lucene70DocValuesProducer::SparseBinaryDocValues::advance(int target) throw(
    IOException)
{
  return disi->advance(target);
}

bool Lucene70DocValuesProducer::SparseBinaryDocValues::advanceExact(
    int target) 
{
  return disi->advanceExact(target);
}

shared_ptr<BinaryDocValues> Lucene70DocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryEntry> entry = binaries[field->name];
  if (entry->docsWithFieldOffset == -2) {
    return DocValues::emptyBinary();
  }

  shared_ptr<IndexInput> *const bytesSlice =
      data->slice(L"fixed-binary", entry->dataOffset, entry->dataLength);

  if (entry->docsWithFieldOffset == -1) {
    // dense
    if (entry->minLength == entry->maxLength) {
      // fixed length
      constexpr int length = entry->maxLength;
      return make_shared<DenseBinaryDocValuesAnonymousInnerClass>(
          shared_from_this(), maxDoc, bytesSlice, length);
    } else {
      // variable length
      shared_ptr<RandomAccessInput> *const addressesData =
          this->data->randomAccessSlice(entry->addressesOffset,
                                        entry->addressesLength);
      shared_ptr<LongValues> *const addresses =
          DirectMonotonicReader::getInstance(entry->addressesMeta,
                                             addressesData);
      return make_shared<DenseBinaryDocValuesAnonymousInnerClass2>(
          shared_from_this(), maxDoc, entry, bytesSlice, addresses);
    }
  } else {
    // sparse
    shared_ptr<IndexedDISI> *const disi = make_shared<IndexedDISI>(
        data, entry->docsWithFieldOffset, entry->docsWithFieldLength,
        entry->numDocsWithField);
    if (entry->minLength == entry->maxLength) {
      // fixed length
      constexpr int length = entry->maxLength;
      return make_shared<SparseBinaryDocValuesAnonymousInnerClass>(
          shared_from_this(), bytesSlice, disi, length);
    } else {
      // variable length
      shared_ptr<RandomAccessInput> *const addressesData =
          this->data->randomAccessSlice(entry->addressesOffset,
                                        entry->addressesLength);
      shared_ptr<LongValues> *const addresses =
          DirectMonotonicReader::getInstance(entry->addressesMeta,
                                             addressesData);
      return make_shared<SparseBinaryDocValuesAnonymousInnerClass2>(
          shared_from_this(), entry, bytesSlice, disi, addresses);
    }
  }
}

Lucene70DocValuesProducer::DenseBinaryDocValuesAnonymousInnerClass::
    DenseBinaryDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        shared_ptr<IndexInput> bytesSlice, int length)
    : DenseBinaryDocValues(maxDoc)
{
  this->outerInstance = outerInstance;
  this->bytesSlice = bytesSlice;
  this->length = length;
  bytes = make_shared<BytesRef>(std::deque<char>(length), 0, length);
}

shared_ptr<BytesRef> Lucene70DocValuesProducer::
    DenseBinaryDocValuesAnonymousInnerClass::binaryValue() 
{
  bytesSlice->seek(static_cast<int64_t>(doc) * length);
  bytesSlice->readBytes(bytes::bytes, 0, length);
  return bytes;
}

Lucene70DocValuesProducer::DenseBinaryDocValuesAnonymousInnerClass2::
    DenseBinaryDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::BinaryEntry>
            entry,
        shared_ptr<IndexInput> bytesSlice, shared_ptr<LongValues> addresses)
    : DenseBinaryDocValues(maxDoc)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
  this->bytesSlice = bytesSlice;
  this->addresses = addresses;
  bytes = make_shared<BytesRef>(std::deque<char>(entry->maxLength), 0,
                                entry->maxLength);
}

shared_ptr<BytesRef> Lucene70DocValuesProducer::
    DenseBinaryDocValuesAnonymousInnerClass2::binaryValue() 
{
  int64_t startOffset = addresses->get(doc);
  bytes->length = static_cast<int>(addresses->get(doc + 1LL) - startOffset);
  bytesSlice->seek(startOffset);
  bytesSlice->readBytes(bytes::bytes, 0, bytes->length);
  return bytes;
}

Lucene70DocValuesProducer::SparseBinaryDocValuesAnonymousInnerClass::
    SparseBinaryDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<IndexInput> bytesSlice,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        int length)
    : SparseBinaryDocValues(disi)
{
  this->outerInstance = outerInstance;
  this->bytesSlice = bytesSlice;
  this->disi = disi;
  this->length = length;
  bytes = make_shared<BytesRef>(std::deque<char>(length), 0, length);
}

shared_ptr<BytesRef> Lucene70DocValuesProducer::
    SparseBinaryDocValuesAnonymousInnerClass::binaryValue() 
{
  bytesSlice->seek(static_cast<int64_t>(disi->index()) * length);
  bytesSlice->readBytes(bytes::bytes, 0, length);
  return bytes;
}

Lucene70DocValuesProducer::SparseBinaryDocValuesAnonymousInnerClass2::
    SparseBinaryDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::BinaryEntry>
            entry,
        shared_ptr<IndexInput> bytesSlice,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<LongValues> addresses)
    : SparseBinaryDocValues(disi)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
  this->bytesSlice = bytesSlice;
  this->disi = disi;
  this->addresses = addresses;
  bytes = make_shared<BytesRef>(std::deque<char>(entry->maxLength), 0,
                                entry->maxLength);
}

shared_ptr<BytesRef> Lucene70DocValuesProducer::
    SparseBinaryDocValuesAnonymousInnerClass2::binaryValue() 
{
  constexpr int index = disi->index();
  int64_t startOffset = addresses->get(index);
  bytes->length = static_cast<int>(addresses->get(index + 1LL) - startOffset);
  bytesSlice->seek(startOffset);
  bytesSlice->readBytes(bytes::bytes, 0, bytes->length);
  return bytes;
}

shared_ptr<SortedDocValues> Lucene70DocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedEntry> entry = sorted[field->name];
  return getSorted(entry);
}

shared_ptr<SortedDocValues> Lucene70DocValuesProducer::getSorted(
    shared_ptr<SortedEntry> entry) 
{
  if (entry->docsWithFieldOffset == -2) {
    return DocValues::emptySorted();
  }

  shared_ptr<LongValues> *const ords;
  if (entry->bitsPerValue == 0) {
    ords = make_shared<LongValuesAnonymousInnerClass>(shared_from_this());
  } else {
    shared_ptr<RandomAccessInput> *const slice =
        data->randomAccessSlice(entry->ordsOffset, entry->ordsLength);
    ords = DirectReader::getInstance(slice, entry->bitsPerValue);
  }

  if (entry->docsWithFieldOffset == -1) {
    // dense
    return make_shared<BaseSortedDocValuesAnonymousInnerClass>(
        shared_from_this(), entry, data, ords);
  } else {
    // sparse
    shared_ptr<IndexedDISI> *const disi = make_shared<IndexedDISI>(
        data, entry->docsWithFieldOffset, entry->docsWithFieldLength,
        entry->numDocsWithField);
    return make_shared<BaseSortedDocValuesAnonymousInnerClass2>(
        shared_from_this(), entry, data, ords, disi);
  }
}

Lucene70DocValuesProducer::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int64_t
Lucene70DocValuesProducer::LongValuesAnonymousInnerClass::get(int64_t index)
{
  return 0LL;
}

Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::
    BaseSortedDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::SortedEntry>
            entry,
        shared_ptr<IndexInput> data, shared_ptr<LongValues> ords)
    : BaseSortedDocValues(entry, data)
{
  this->outerInstance = outerInstance;
  this->ords = ords;
  doc = -1;
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return advance(doc + 1);
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::docID()
{
  return doc;
}

int64_t
Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::advance(
    int target) 
{
  if (target >= outerInstance->maxDoc) {
    return doc = NO_MORE_DOCS;
  }
  return doc = target;
}

bool Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::
    advanceExact(int target)
{
  doc = target;
  return true;
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass::
    ordValue()
{
  return static_cast<int>(ords->get(doc));
}

Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::
    BaseSortedDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::SortedEntry>
            entry,
        shared_ptr<IndexInput> data, shared_ptr<LongValues> ords,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi)
    : BaseSortedDocValues(entry, data)
{
  this->outerInstance = outerInstance;
  this->ords = ords;
  this->disi = disi;
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::
    nextDoc() 
{
  return disi->nextDoc();
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::docID()
{
  return disi->docID();
}

int64_t
Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::cost()
{
  return disi->cost();
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::advance(
    int target) 
{
  return disi->advance(target);
}

bool Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  return disi->advanceExact(target);
}

int Lucene70DocValuesProducer::BaseSortedDocValuesAnonymousInnerClass2::
    ordValue()
{
  return static_cast<int>(ords->get(disi->index()));
}

Lucene70DocValuesProducer::BaseSortedDocValues::BaseSortedDocValues(
    shared_ptr<SortedEntry> entry,
    shared_ptr<IndexInput> data) 
    : entry(entry), data(data), termsEnum(termsEnum())
{
}

int Lucene70DocValuesProducer::BaseSortedDocValues::getValueCount()
{
  return Math::toIntExact(entry->termsDictSize);
}

shared_ptr<BytesRef> Lucene70DocValuesProducer::BaseSortedDocValues::lookupOrd(
    int ord) 
{
  termsEnum_->seekExact(ord);
  return termsEnum_->term();
}

int Lucene70DocValuesProducer::BaseSortedDocValues::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  TermsEnum::SeekStatus status = termsEnum_->seekCeil(key);
  switch (status) {
  case TermsEnum::SeekStatus::FOUND:
    return Math::toIntExact(termsEnum_->ord());
  default:
    return Math::toIntExact(-1LL - termsEnum_->ord());
  }
}

shared_ptr<TermsEnum>
Lucene70DocValuesProducer::BaseSortedDocValues::termsEnum() 
{
  return make_shared<TermsDict>(entry, data);
}

Lucene70DocValuesProducer::BaseSortedSetDocValues::BaseSortedSetDocValues(
    shared_ptr<SortedSetEntry> entry,
    shared_ptr<IndexInput> data) 
    : entry(entry), data(data), termsEnum(termsEnum())
{
}

int64_t Lucene70DocValuesProducer::BaseSortedSetDocValues::getValueCount()
{
  return entry->termsDictSize;
}

shared_ptr<BytesRef>
Lucene70DocValuesProducer::BaseSortedSetDocValues::lookupOrd(
    int64_t ord) 
{
  termsEnum_->seekExact(ord);
  return termsEnum_->term();
}

int64_t Lucene70DocValuesProducer::BaseSortedSetDocValues::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  TermsEnum::SeekStatus status = termsEnum_->seekCeil(key);
  switch (status) {
  case TermsEnum::SeekStatus::FOUND:
    return termsEnum_->ord();
  default:
    return -1LL - termsEnum_->ord();
  }
}

shared_ptr<TermsEnum>
Lucene70DocValuesProducer::BaseSortedSetDocValues::termsEnum() throw(
    IOException)
{
  return make_shared<TermsDict>(entry, data);
}

Lucene70DocValuesProducer::TermsDict::TermsDict(
    shared_ptr<TermsDictEntry> entry,
    shared_ptr<IndexInput> data) 
    : entry(entry), blockAddresses(DirectMonotonicReader::getInstance(
                        entry->termsAddressesMeta, addressesSlice)),
      bytes(data->slice(L"terms", entry->termsDataOffset,
                        entry->termsDataLength)),
      blockMask((1LL << entry->termsDictBlockShift) - 1),
      indexAddresses(DirectMonotonicReader::getInstance(
          entry->termsIndexAddressesMeta, indexAddressesSlice)),
      indexBytes(data->slice(L"terms-index", entry->termsIndexOffset,
                             entry->termsIndexLength)),
      term(make_shared<BytesRef>(entry->maxTermLength))
{
  shared_ptr<RandomAccessInput> addressesSlice = data->randomAccessSlice(
      entry->termsAddressesOffset, entry->termsAddressesLength);
  shared_ptr<RandomAccessInput> indexAddressesSlice = data->randomAccessSlice(
      entry->termsIndexAddressesOffset, entry->termsIndexAddressesLength);
}

shared_ptr<BytesRef>
Lucene70DocValuesProducer::TermsDict::next() 
{
  if (++ord_ >= entry->termsDictSize) {
    return nullptr;
  }
  if ((ord_ & blockMask) == 0LL) {
    term_->length = bytes->readVInt();
    bytes->readBytes(term_->bytes, 0, term_->length);
  } else {
    constexpr int token = Byte::toUnsignedInt(bytes->readByte());
    int prefixLength = token & 0x0F;
    int suffixLength =
        1 + (static_cast<int>(static_cast<unsigned int>(token) >> 4));
    if (prefixLength == 15) {
      prefixLength += bytes->readVInt();
    }
    if (suffixLength == 16) {
      suffixLength += bytes->readVInt();
    }
    term_->length = prefixLength + suffixLength;
    bytes->readBytes(term_->bytes, prefixLength, suffixLength);
  }
  return term_;
}

void Lucene70DocValuesProducer::TermsDict::seekExact(int64_t ord) throw(
    IOException)
{
  if (ord < 0 || ord >= entry->termsDictSize) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IndexOutOfBoundsException();
    throw out_of_range();
  }
  constexpr int64_t blockIndex = static_cast<int64_t>(
      static_cast<uint64_t>(ord) >> entry->termsDictBlockShift);
  constexpr int64_t blockAddress = blockAddresses->get(blockIndex);
  bytes->seek(blockAddress);
  this->ord_ = (blockIndex << entry->termsDictBlockShift) - 1;
  do {
    next();
  } while (this->ord_ < ord);
}

shared_ptr<BytesRef> Lucene70DocValuesProducer::TermsDict::getTermFromIndex(
    int64_t index) 
{
  assert(index >= 0 &&
         index <= static_cast<int64_t>(static_cast<uint64_t>(
                                             (entry->termsDictSize - 1)) >>
                                         entry->termsDictIndexShift));
  constexpr int64_t start = indexAddresses->get(index);
  term_->length = static_cast<int>(indexAddresses->get(index + 1) - start);
  indexBytes->seek(start);
  indexBytes->readBytes(term_->bytes, 0, term_->length);
  return term_;
}

int64_t Lucene70DocValuesProducer::TermsDict::seekTermsIndex(
    shared_ptr<BytesRef> text) 
{
  int64_t lo = 0LL;
  int64_t hi = static_cast<int64_t>(
      static_cast<uint64_t>((entry->termsDictSize - 1)) >>
      entry->termsDictIndexShift);
  while (lo <= hi) {
    constexpr int64_t mid =
        static_cast<int64_t>(static_cast<uint64_t>((lo + hi)) >> 1);
    getTermFromIndex(mid);
    constexpr int cmp = term_->compareTo(text);
    if (cmp <= 0) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }

  assert(hi < 0 || getTermFromIndex(hi)->compareTo(text) <= 0);
  assert(hi == (static_cast<int64_t>(static_cast<uint64_t>(
                                           (entry->termsDictSize - 1)) >>
                                       entry->termsDictIndexShift)) ||
         getTermFromIndex(hi + 1)->compareTo(text) > 0);

  return hi;
}

shared_ptr<BytesRef>
Lucene70DocValuesProducer::TermsDict::getFirstTermFromBlock(
    int64_t block) 
{
  assert(block >= 0 &&
         block <= static_cast<int64_t>(static_cast<uint64_t>(
                                             (entry->termsDictSize - 1)) >>
                                         entry->termsDictBlockShift));
  constexpr int64_t blockAddress = blockAddresses->get(block);
  bytes->seek(blockAddress);
  term_->length = bytes->readVInt();
  bytes->readBytes(term_->bytes, 0, term_->length);
  return term_;
}

int64_t Lucene70DocValuesProducer::TermsDict::seekBlock(
    shared_ptr<BytesRef> text) 
{
  int64_t index = seekTermsIndex(text);
  if (index == -1LL) {
    return -1LL;
  }

  int64_t ordLo = index << entry->termsDictIndexShift;
  int64_t ordHi =
      min(entry->termsDictSize, ordLo + (1LL << entry->termsDictIndexShift)) -
      1LL;

  int64_t blockLo = static_cast<int64_t>(
      static_cast<uint64_t>(ordLo) >> entry->termsDictBlockShift);
  int64_t blockHi = static_cast<int64_t>(
      static_cast<uint64_t>(ordHi) >> entry->termsDictBlockShift);

  while (blockLo <= blockHi) {
    constexpr int64_t blockMid = static_cast<int64_t>(
        static_cast<uint64_t>((blockLo + blockHi)) >> 1);
    getFirstTermFromBlock(blockMid);
    constexpr int cmp = term_->compareTo(text);
    if (cmp <= 0) {
      blockLo = blockMid + 1;
    } else {
      blockHi = blockMid - 1;
    }
  }

  assert(blockHi < 0 || getFirstTermFromBlock(blockHi)->compareTo(text) <= 0);
  assert(blockHi == (static_cast<int64_t>(static_cast<uint64_t>(
                                                (entry->termsDictSize - 1)) >>
                                            entry->termsDictBlockShift)) ||
         getFirstTermFromBlock(blockHi + 1)->compareTo(text) > 0);

  return blockHi;
}

TermsEnum::SeekStatus Lucene70DocValuesProducer::TermsDict::seekCeil(
    shared_ptr<BytesRef> text) 
{
  constexpr int64_t block = seekBlock(text);
  if (block == -1) {
    // before the first term
    seekExact(0LL);
    return TermsEnum::SeekStatus::NOT_FOUND;
  }
  constexpr int64_t blockAddress = blockAddresses->get(block);
  this->ord_ = block << entry->termsDictBlockShift;
  bytes->seek(blockAddress);
  term_->length = bytes->readVInt();
  bytes->readBytes(term_->bytes, 0, term_->length);
  while (true) {
    int cmp = term_->compareTo(text);
    if (cmp == 0) {
      return TermsEnum::SeekStatus::FOUND;
    } else if (cmp > 0) {
      return TermsEnum::SeekStatus::NOT_FOUND;
    }
    if (next() == nullptr) {
      return TermsEnum::SeekStatus::END;
    }
  }
}

shared_ptr<BytesRef>
Lucene70DocValuesProducer::TermsDict::term() 
{
  return term_;
}

int64_t Lucene70DocValuesProducer::TermsDict::ord() 
{
  return ord_;
}

int64_t
Lucene70DocValuesProducer::TermsDict::totalTermFreq() 
{
  return -1LL;
}

shared_ptr<PostingsEnum>
Lucene70DocValuesProducer::TermsDict::postings(shared_ptr<PostingsEnum> reuse,
                                               int flags) 
{
  throw make_shared<UnsupportedOperationException>();
}

int Lucene70DocValuesProducer::TermsDict::docFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<SortedNumericDocValues> Lucene70DocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedNumericEntry> entry = sortedNumerics[field->name];
  if (entry->numValues == entry->numDocsWithField) {
    return DocValues::singleton(getNumeric(entry));
  }

  shared_ptr<RandomAccessInput> *const addressesInput =
      data->randomAccessSlice(entry->addressesOffset, entry->addressesLength);
  shared_ptr<LongValues> *const addresses =
      DirectMonotonicReader::getInstance(entry->addressesMeta, addressesInput);

  shared_ptr<LongValues> *const values = getNumericValues(entry);

  if (entry->docsWithFieldOffset == -1) {
    // dense
    return make_shared<SortedNumericDocValuesAnonymousInnerClass>(
        shared_from_this(), addresses, values);
  } else {
    // sparse
    shared_ptr<IndexedDISI> *const disi = make_shared<IndexedDISI>(
        data, entry->docsWithFieldOffset, entry->docsWithFieldLength,
        entry->numDocsWithField);
    return make_shared<SortedNumericDocValuesAnonymousInnerClass2>(
        shared_from_this(), addresses, values, disi);
  }
}

Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<LongValues> addresses, shared_ptr<LongValues> values)
{
  this->outerInstance = outerInstance;
  this->addresses = addresses;
  this->values = values;
  doc = -1;
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return advance(doc + 1);
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    docID()
{
  return doc;
}

int64_t
Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    advance(int target) 
{
  if (target >= outerInstance->maxDoc) {
    return doc = NO_MORE_DOCS;
  }
  start = addresses->get(target);
  end = addresses->get(target + 1LL);
  count = static_cast<int>(end - start);
  return doc = target;
}

bool Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  start = addresses->get(target);
  end = addresses->get(target + 1LL);
  count = static_cast<int>(end - start);
  doc = target;
  return true;
}

int64_t Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    nextValue() 
{
  return values->get(start++);
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass::
    docValueCount()
{
  return count;
}

Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    SortedNumericDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<LongValues> addresses, shared_ptr<LongValues> values,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi)
{
  this->outerInstance = outerInstance;
  this->addresses = addresses;
  this->values = values;
  this->disi = disi;
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    nextDoc() 
{
  set = false;
  return disi->nextDoc();
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    docID()
{
  return disi->docID();
}

int64_t
Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::cost()
{
  return disi->cost();
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    advance(int target) 
{
  set = false;
  return disi->advance(target);
}

bool Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  set = false;
  return disi->advanceExact(target);
}

int64_t Lucene70DocValuesProducer::
    SortedNumericDocValuesAnonymousInnerClass2::nextValue() 
{
  set();
  return values->get(start++);
}

int Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    docValueCount()
{
  set();
  return count;
}

void Lucene70DocValuesProducer::SortedNumericDocValuesAnonymousInnerClass2::
    set()
{
  if (set == false) {
    constexpr int index = disi->index();
    start = addresses->get(index);
    end = addresses->get(index + 1LL);
    count = static_cast<int>(end - start);
    set = true;
  }
}

shared_ptr<SortedSetDocValues> Lucene70DocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedSetEntry> entry = sortedSets[field->name];
  if (entry->singleValueEntry != nullptr) {
    return DocValues::singleton(getSorted(entry->singleValueEntry));
  }

  shared_ptr<RandomAccessInput> *const slice =
      data->randomAccessSlice(entry->ordsOffset, entry->ordsLength);
  shared_ptr<LongValues> *const ords =
      DirectReader::getInstance(slice, entry->bitsPerValue);

  shared_ptr<RandomAccessInput> *const addressesInput =
      data->randomAccessSlice(entry->addressesOffset, entry->addressesLength);
  shared_ptr<LongValues> *const addresses =
      DirectMonotonicReader::getInstance(entry->addressesMeta, addressesInput);

  if (entry->docsWithFieldOffset == -1) {
    // dense
    return make_shared<BaseSortedSetDocValuesAnonymousInnerClass>(
        shared_from_this(), entry, data, ords, addresses);
  } else {
    // sparse
    shared_ptr<IndexedDISI> *const disi = make_shared<IndexedDISI>(
        data, entry->docsWithFieldOffset, entry->docsWithFieldLength,
        entry->numDocsWithField);
    return make_shared<BaseSortedSetDocValuesAnonymousInnerClass2>(
        shared_from_this(), entry, data, ords, addresses, disi);
  }
}

Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::
    BaseSortedSetDocValuesAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::SortedSetEntry>
            entry,
        shared_ptr<IndexInput> data, shared_ptr<LongValues> ords,
        shared_ptr<LongValues> addresses)
    : BaseSortedSetDocValues(entry, data)
{
  this->outerInstance = outerInstance;
  this->ords = ords;
  this->addresses = addresses;
  doc = -1;
}

int Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return advance(doc + 1);
}

int Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::
    docID()
{
  return doc;
}

int64_t
Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::
    advance(int target) 
{
  if (target >= outerInstance->maxDoc) {
    return doc = NO_MORE_DOCS;
  }
  start = addresses->get(target);
  end = addresses->get(target + 1LL);
  return doc = target;
}

bool Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  start = addresses->get(target);
  end = addresses->get(target + 1LL);
  doc = target;
  return true;
}

int64_t Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass::
    nextOrd() 
{
  if (start == end) {
    return SortedSetDocValues::NO_MORE_ORDS;
  }
  return ords->get(start++);
}

Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass2::
    BaseSortedSetDocValuesAnonymousInnerClass2(
        shared_ptr<Lucene70DocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70DocValuesProducer::SortedSetEntry>
            entry,
        shared_ptr<IndexInput> data, shared_ptr<LongValues> ords,
        shared_ptr<LongValues> addresses,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi)
    : BaseSortedSetDocValues(entry, data)
{
  this->outerInstance = outerInstance;
  this->ords = ords;
  this->addresses = addresses;
  this->disi = disi;
  end = 0;
}

int Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass2::
    nextDoc() 
{
  set = false;
  return disi->nextDoc();
}

int Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass2::
    docID()
{
  return disi->docID();
}

int64_t
Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass2::cost()
{
  return disi->cost();
}

int Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass2::
    advance(int target) 
{
  set = false;
  return disi->advance(target);
}

bool Lucene70DocValuesProducer::BaseSortedSetDocValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  set = false;
  return disi->advanceExact(target);
}

int64_t Lucene70DocValuesProducer::
    BaseSortedSetDocValuesAnonymousInnerClass2::nextOrd() 
{
  if (set == false) {
    constexpr int index = disi->index();
    constexpr int64_t start = addresses->get(index);
    this->start = start + 1;
    end = addresses->get(index + 1LL);
    set = true;
    return ords->get(start);
  } else if (start == end) {
    return SortedSetDocValues::NO_MORE_ORDS;
  } else {
    return ords->get(start++);
  }
}

void Lucene70DocValuesProducer::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(data);
}
} // namespace org::apache::lucene::codecs::lucene70