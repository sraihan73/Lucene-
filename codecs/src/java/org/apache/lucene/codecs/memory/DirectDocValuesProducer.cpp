using namespace std;

#include "DirectDocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LegacyBinaryDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LegacyNumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Bits.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"

namespace org::apache::lucene::codecs::memory
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using namespace org::apache::lucene::index;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

DirectDocValuesProducer::DirectDocValuesProducer(
    shared_ptr<DirectDocValuesProducer> original) 
    : data(original->data->clone()), numEntries(original->numEntries),
      maxDoc(original->maxDoc),
      ramBytesUsed(make_shared<AtomicLong>(original->ramBytesUsed_->get())),
      version(original->version), merging(true)
{
  assert(Thread::holdsLock(original));
  numerics.putAll(original->numerics);
  binaries.putAll(original->binaries);
  sorteds.putAll(original->sorteds);
  sortedSets.putAll(original->sortedSets);
  sortedNumerics.putAll(original->sortedNumerics);

  numericInstances.putAll(original->numericInstances);
  binaryInstances.putAll(original->binaryInstances);
  sortedInstances.putAll(original->sortedInstances);
  sortedSetInstances.putAll(original->sortedSetInstances);
  sortedNumericInstances.putAll(original->sortedNumericInstances);
  docsWithFieldInstances.putAll(original->docsWithFieldInstances);
}

DirectDocValuesProducer::DirectDocValuesProducer(
    shared_ptr<SegmentReadState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : data(state->directory->openInput(dataName, state->context)),
      maxDoc(state->segmentInfo->maxDoc()),
      ramBytesUsed(make_shared<AtomicLong>(
          RamUsageEstimator::shallowSizeOfInstance(getClass()))),
      merging(false)
{
  wstring metaName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, metaExtension);
  // read in the entries from the metadata file.
  shared_ptr<ChecksumIndexInput> in_ =
      state->directory->openChecksumInput(metaName, state->context);
  bool success = false;
  try {
    version = CodecUtil::checkIndexHeader(
        in_, metaCodec, VERSION_START, VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    numEntries = readFields(in_, state->fieldInfos);

    CodecUtil::checkFooter(in_);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({in_});
    } else {
      IOUtils::closeWhileHandlingException({in_});
    }
  }

  wstring dataName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, dataExtension);
  success = false;
  try {
    constexpr int version2 = CodecUtil::checkIndexHeader(
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

shared_ptr<NumericEntry> DirectDocValuesProducer::readNumericEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<NumericEntry> entry = make_shared<NumericEntry>();
  entry->offset = meta->readLong();
  entry->count = meta->readInt();
  entry->missingOffset = meta->readLong();
  if (entry->missingOffset != -1) {
    entry->missingBytes = meta->readLong();
  } else {
    entry->missingBytes = 0;
  }
  entry->byteWidth = meta->readByte();

  return entry;
}

shared_ptr<BinaryEntry> DirectDocValuesProducer::readBinaryEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<BinaryEntry> entry = make_shared<BinaryEntry>();
  entry->offset = meta->readLong();
  entry->numBytes = meta->readInt();
  entry->count = meta->readInt();
  entry->missingOffset = meta->readLong();
  if (entry->missingOffset != -1) {
    entry->missingBytes = meta->readLong();
  } else {
    entry->missingBytes = 0;
  }

  return entry;
}

shared_ptr<SortedEntry> DirectDocValuesProducer::readSortedEntry(
    shared_ptr<IndexInput> meta) 
{
  shared_ptr<SortedEntry> entry = make_shared<SortedEntry>();
  entry->docToOrd = readNumericEntry(meta);
  entry->values = readBinaryEntry(meta);
  return entry;
}

shared_ptr<SortedSetEntry>
DirectDocValuesProducer::readSortedSetEntry(shared_ptr<IndexInput> meta,
                                            bool singleton) 
{
  shared_ptr<SortedSetEntry> entry = make_shared<SortedSetEntry>();
  if (singleton == false) {
    entry->docToOrdAddress = readNumericEntry(meta);
  }
  entry->ords = readNumericEntry(meta);
  entry->values = readBinaryEntry(meta);
  return entry;
}

shared_ptr<SortedNumericEntry> DirectDocValuesProducer::readSortedNumericEntry(
    shared_ptr<IndexInput> meta, bool singleton) 
{
  shared_ptr<SortedNumericEntry> entry = make_shared<SortedNumericEntry>();
  if (singleton == false) {
    entry->docToAddress = readNumericEntry(meta);
  }
  entry->values = readNumericEntry(meta);
  return entry;
}

int DirectDocValuesProducer::readFields(
    shared_ptr<IndexInput> meta,
    shared_ptr<FieldInfos> infos) 
{
  int numEntries = 0;
  int fieldNumber = meta->readVInt();
  while (fieldNumber != -1) {
    numEntries++;
    shared_ptr<FieldInfo> info = infos->fieldInfo(fieldNumber);
    int fieldType = meta->readByte();
    if (fieldType == NUMBER) {
      numerics.emplace(info->name, readNumericEntry(meta));
    } else if (fieldType == BYTES) {
      binaries.emplace(info->name, readBinaryEntry(meta));
    } else if (fieldType == SORTED) {
      shared_ptr<SortedEntry> entry = readSortedEntry(meta);
      sorteds.emplace(info->name, entry);
      binaries.emplace(info->name, entry->values);
    } else if (fieldType == SORTED_SET) {
      shared_ptr<SortedSetEntry> entry = readSortedSetEntry(meta, false);
      sortedSets.emplace(info->name, entry);
      binaries.emplace(info->name, entry->values);
    } else if (fieldType == SORTED_SET_SINGLETON) {
      shared_ptr<SortedSetEntry> entry = readSortedSetEntry(meta, true);
      sortedSets.emplace(info->name, entry);
      binaries.emplace(info->name, entry->values);
    } else if (fieldType == SORTED_NUMERIC) {
      shared_ptr<SortedNumericEntry> entry =
          readSortedNumericEntry(meta, false);
      sortedNumerics.emplace(info->name, entry);
    } else if (fieldType == SORTED_NUMERIC_SINGLETON) {
      shared_ptr<SortedNumericEntry> entry = readSortedNumericEntry(meta, true);
      sortedNumerics.emplace(info->name, entry);
    } else {
      throw make_shared<CorruptIndexException>(L"invalid entry type: " +
                                                   to_wstring(fieldType) +
                                                   L", field= " + info->name,
                                               meta);
    }
    fieldNumber = meta->readVInt();
  }
  return numEntries;
}

int64_t DirectDocValuesProducer::ramBytesUsed()
{
  return ramBytesUsed_->get();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectDocValuesProducer::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(
      Accountables::namedAccountables(L"numeric field", numericInstances));
  resources.addAll(
      Accountables::namedAccountables(L"binary field", binaryInstances));
  resources.addAll(
      Accountables::namedAccountables(L"sorted field", sortedInstances));
  resources.addAll(
      Accountables::namedAccountables(L"sorted set field", sortedSetInstances));
  resources.addAll(Accountables::namedAccountables(L"sorted numeric field",
                                                   sortedNumericInstances));
  resources.addAll(Accountables::namedAccountables(L"missing bitset field",
                                                   docsWithFieldInstances));
  return Collections::unmodifiableList(resources);
}

wstring DirectDocValuesProducer::toString()
{
  return getClass().getSimpleName() + L"(entries=" + to_wstring(numEntries) +
         L")";
}

void DirectDocValuesProducer::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(data->clone());
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<NumericDocValues> DirectDocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<NumericRawValues> instance = numericInstances[field->name];
  shared_ptr<NumericEntry> ne = numerics[field->name];
  if (instance == nullptr) {
    // Lazy load
    instance = loadNumeric(ne);
    if (!merging) {
      numericInstances.emplace(field->name, instance);
      ramBytesUsed_->addAndGet(instance->ramBytesUsed());
    }
  }
  return make_shared<LegacyNumericDocValuesWrapper>(
      getMissingBits(field, ne->missingOffset, ne->missingBytes),
      instance->numerics);
}

shared_ptr<NumericRawValues> DirectDocValuesProducer::loadNumeric(
    shared_ptr<NumericEntry> entry) 
{
  shared_ptr<NumericRawValues> ret = make_shared<NumericRawValues>();
  shared_ptr<IndexInput> data = this->data->clone();
  data->seek(entry->offset + entry->missingBytes);
  switch (entry->byteWidth) {
  case 1: {
    const std::deque<char> values = std::deque<char>(entry->count);
    data->readBytes(values, 0, entry->count);
    ret->bytesUsed = RamUsageEstimator::sizeOf(values);
    ret->numerics = make_shared<LegacyNumericDocValuesAnonymousInnerClass>(
        shared_from_this(), values);
    return ret;
  }

  case 2: {
    const std::deque<short> values = std::deque<short>(entry->count);
    for (int i = 0; i < entry->count; i++) {
      values[i] = data->readShort();
    }
    ret->bytesUsed = RamUsageEstimator::sizeOf(values);
    ret->numerics = make_shared<LegacyNumericDocValuesAnonymousInnerClass2>(
        shared_from_this(), values);
    return ret;
  }

  case 4: {
    const std::deque<int> values = std::deque<int>(entry->count);
    for (int i = 0; i < entry->count; i++) {
      values[i] = data->readInt();
    }
    ret->bytesUsed = RamUsageEstimator::sizeOf(values);
    ret->numerics = make_shared<LegacyNumericDocValuesAnonymousInnerClass3>(
        shared_from_this(), values);
    return ret;
  }

  case 8: {
    const std::deque<int64_t> values = std::deque<int64_t>(entry->count);
    for (int i = 0; i < entry->count; i++) {
      values[i] = data->readLong();
    }
    ret->bytesUsed = RamUsageEstimator::sizeOf(values);
    ret->numerics = make_shared<LegacyNumericDocValuesAnonymousInnerClass4>(
        shared_from_this(), values);
    return ret;
  }

  default:
    throw make_shared<AssertionError>();
  }
}

DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass::
    LegacyNumericDocValuesAnonymousInnerClass(
        shared_ptr<DirectDocValuesProducer> outerInstance, deque<char> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

int64_t
DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass::get(int idx)
{
  return values[idx];
}

DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass2::
    LegacyNumericDocValuesAnonymousInnerClass2(
        shared_ptr<DirectDocValuesProducer> outerInstance,
        deque<short> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

int64_t
DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass2::get(
    int idx)
{
  return values[idx];
}

DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass3::
    LegacyNumericDocValuesAnonymousInnerClass3(
        shared_ptr<DirectDocValuesProducer> outerInstance, deque<int> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

int64_t
DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass3::get(
    int idx)
{
  return values[idx];
}

DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass4::
    LegacyNumericDocValuesAnonymousInnerClass4(
        shared_ptr<DirectDocValuesProducer> outerInstance,
        deque<int64_t> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

int64_t
DirectDocValuesProducer::LegacyNumericDocValuesAnonymousInnerClass4::get(
    int idx)
{
  return values[idx];
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<LegacyBinaryDocValues> DirectDocValuesProducer::getLegacyBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryRawValues> instance = binaryInstances[field->name];
  if (instance == nullptr) {
    // Lazy load
    instance = loadBinary(binaries[field->name]);
    if (!merging) {
      binaryInstances.emplace(field->name, instance);
      ramBytesUsed_->addAndGet(instance->ramBytesUsed());
    }
  }
  const std::deque<char> bytes = instance->bytes;
  const std::deque<int> address = instance->address;

  return make_shared<LegacyBinaryDocValuesAnonymousInnerClass>(
      shared_from_this(), bytes, address);
}

DirectDocValuesProducer::LegacyBinaryDocValuesAnonymousInnerClass::
    LegacyBinaryDocValuesAnonymousInnerClass(
        shared_ptr<DirectDocValuesProducer> outerInstance, deque<char> &bytes,
        deque<int> &address)
{
  this->outerInstance = outerInstance;
  this->bytes = bytes;
  this->address = address;
  term = make_shared<BytesRef>();
}

shared_ptr<BytesRef>
DirectDocValuesProducer::LegacyBinaryDocValuesAnonymousInnerClass::get(
    int docID)
{
  term->bytes = bytes;
  term->offset = address[docID];
  term->length = address[docID + 1] - term::offset;
  return term;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<BinaryDocValues> DirectDocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryEntry> be = binaries[field->name];
  return make_shared<LegacyBinaryDocValuesWrapper>(
      getMissingBits(field, be->missingOffset, be->missingBytes),
      getLegacyBinary(field));
}

shared_ptr<BinaryRawValues> DirectDocValuesProducer::loadBinary(
    shared_ptr<BinaryEntry> entry) 
{
  shared_ptr<IndexInput> data = this->data->clone();
  data->seek(entry->offset);
  const std::deque<char> bytes = std::deque<char>(entry->numBytes);
  data->readBytes(bytes, 0, entry->numBytes);
  data->seek(entry->offset + entry->numBytes + entry->missingBytes);

  const std::deque<int> address = std::deque<int>(entry->count + 1);
  for (int i = 0; i < entry->count; i++) {
    address[i] = data->readInt();
  }
  address[entry->count] = data->readInt();

  shared_ptr<BinaryRawValues> values = make_shared<BinaryRawValues>();
  values->bytes = bytes;
  values->address = address;
  return values;
}

shared_ptr<SortedDocValues> DirectDocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedEntry> *const entry = sorteds[field->name];
  shared_ptr<SortedRawValues> instance;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    instance = sortedInstances[field->name];
    if (instance == nullptr) {
      // Lazy load
      instance = loadSorted(field);
      if (!merging) {
        sortedInstances.emplace(field->name, instance);
        ramBytesUsed_->addAndGet(instance->ramBytesUsed());
      }
    }
  }
  return make_shared<LegacySortedDocValuesWrapper>(
      newSortedInstance(instance->docToOrd->numerics, getLegacyBinary(field),
                        entry->values->count),
      maxDoc);
}

shared_ptr<LegacySortedDocValues> DirectDocValuesProducer::newSortedInstance(
    shared_ptr<LegacyNumericDocValues> docToOrd,
    shared_ptr<LegacyBinaryDocValues> values, int const count)
{
  return make_shared<LegacySortedDocValuesAnonymousInnerClass>(
      shared_from_this(), docToOrd, values, count);
}

DirectDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::
    LegacySortedDocValuesAnonymousInnerClass(
        shared_ptr<DirectDocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> docToOrd,
        shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> values,
        int count)
{
  this->outerInstance = outerInstance;
  this->docToOrd = docToOrd;
  this->values = values;
  this->count = count;
}

int DirectDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::getOrd(
    int docID)
{
  return static_cast<int>(docToOrd->get(docID));
}

shared_ptr<BytesRef>
DirectDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::lookupOrd(
    int ord)
{
  return values->get(ord);
}

int DirectDocValuesProducer::LegacySortedDocValuesAnonymousInnerClass::
    getValueCount()
{
  return count;
}

shared_ptr<SortedRawValues> DirectDocValuesProducer::loadSorted(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedEntry> *const entry = sorteds[field->name];
  shared_ptr<NumericRawValues> *const docToOrd = loadNumeric(entry->docToOrd);
  shared_ptr<SortedRawValues> *const values = make_shared<SortedRawValues>();
  values->docToOrd = docToOrd;
  return values;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SortedNumericDocValues> DirectDocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedNumericRawValues> instance =
      sortedNumericInstances[field->name];
  shared_ptr<SortedNumericEntry> *const entry = sortedNumerics[field->name];
  if (instance == nullptr) {
    // Lazy load
    instance = loadSortedNumeric(entry);
    if (!merging) {
      sortedNumericInstances.emplace(field->name, instance);
      ramBytesUsed_->addAndGet(instance->ramBytesUsed());
    }
  }

  if (entry->docToAddress == nullptr) {
    shared_ptr<LegacyNumericDocValues> *const single =
        instance->values->numerics;
    shared_ptr<Bits> *const docsWithField = getMissingBits(
        field, entry->values->missingOffset, entry->values->missingBytes);
    return DocValues::singleton(
        make_shared<LegacyNumericDocValuesWrapper>(docsWithField, single));
  } else {
    shared_ptr<LegacyNumericDocValues> *const docToAddress =
        instance->docToAddress->numerics;
    shared_ptr<LegacyNumericDocValues> *const values =
        instance->values->numerics;

    return make_shared<LegacySortedNumericDocValuesWrapper>(
        make_shared<LegacySortedNumericDocValuesAnonymousInnerClass>(
            shared_from_this(), docToAddress, values),
        maxDoc);
  }
}

DirectDocValuesProducer::LegacySortedNumericDocValuesAnonymousInnerClass::
    LegacySortedNumericDocValuesAnonymousInnerClass(
        shared_ptr<DirectDocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            docToAddress,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->docToAddress = docToAddress;
  this->values = values;
}

void DirectDocValuesProducer::LegacySortedNumericDocValuesAnonymousInnerClass::
    setDocument(int doc)
{
  valueStart = static_cast<int>(docToAddress->get(doc));
  valueLimit = static_cast<int>(docToAddress->get(doc + 1));
}

int64_t DirectDocValuesProducer::
    LegacySortedNumericDocValuesAnonymousInnerClass::valueAt(int index)
{
  return values->get(valueStart + index);
}

int DirectDocValuesProducer::LegacySortedNumericDocValuesAnonymousInnerClass::
    count()
{
  return valueLimit - valueStart;
}

shared_ptr<SortedNumericRawValues> DirectDocValuesProducer::loadSortedNumeric(
    shared_ptr<SortedNumericEntry> entry) 
{
  shared_ptr<SortedNumericRawValues> instance =
      make_shared<SortedNumericRawValues>();
  if (entry->docToAddress != nullptr) {
    instance->docToAddress = loadNumeric(entry->docToAddress);
  }
  instance->values = loadNumeric(entry->values);
  return instance;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SortedSetDocValues> DirectDocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedSetRawValues> instance = sortedSetInstances[field->name];
  shared_ptr<SortedSetEntry> *const entry = sortedSets[field->name];
  if (instance == nullptr) {
    // Lazy load
    instance = loadSortedSet(entry);
    if (!merging) {
      sortedSetInstances.emplace(field->name, instance);
      ramBytesUsed_->addAndGet(instance->ramBytesUsed());
    }
  }

  if (instance->docToOrdAddress == nullptr) {
    shared_ptr<LegacySortedDocValues> sorted = newSortedInstance(
        instance->ords->numerics, getLegacyBinary(field), entry->values->count);
    return DocValues::singleton(
        make_shared<LegacySortedDocValuesWrapper>(sorted, maxDoc));
  } else {
    shared_ptr<LegacyNumericDocValues> *const docToOrdAddress =
        instance->docToOrdAddress->numerics;
    shared_ptr<LegacyNumericDocValues> *const ords = instance->ords->numerics;
    shared_ptr<LegacyBinaryDocValues> *const values = getLegacyBinary(field);

    // Must make a new instance since the iterator has state:
    return make_shared<LegacySortedSetDocValuesWrapper>(
        make_shared<LegacySortedSetDocValuesAnonymousInnerClass>(
            shared_from_this(), entry, docToOrdAddress, ords, values),
        maxDoc);
  }
}

DirectDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    LegacySortedSetDocValuesAnonymousInnerClass(
        shared_ptr<DirectDocValuesProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::memory::
                       DirectDocValuesProducer::SortedSetEntry>
            entry,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            docToOrdAddress,
        shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> ords,
        shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> values)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
  this->docToOrdAddress = docToOrdAddress;
  this->ords = ords;
  this->values = values;
}

int64_t
DirectDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::nextOrd()
{
  if (ordUpto == ordLimit) {
    return NO_MORE_ORDS;
  } else {
    return ords->get(ordUpto++);
  }
}

void DirectDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    setDocument(int docID)
{
  ordStart = ordUpto = static_cast<int>(docToOrdAddress->get(docID));
  ordLimit = static_cast<int>(docToOrdAddress->get(docID + 1));
}

shared_ptr<BytesRef>
DirectDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::lookupOrd(
    int64_t ord)
{
  return values->get(static_cast<int>(ord));
}

int64_t DirectDocValuesProducer::LegacySortedSetDocValuesAnonymousInnerClass::
    getValueCount()
{
  return entry->values->count;
}

shared_ptr<SortedSetRawValues> DirectDocValuesProducer::loadSortedSet(
    shared_ptr<SortedSetEntry> entry) 
{
  shared_ptr<SortedSetRawValues> instance = make_shared<SortedSetRawValues>();
  if (entry->docToOrdAddress != nullptr) {
    instance->docToOrdAddress = loadNumeric(entry->docToOrdAddress);
  }
  instance->ords = loadNumeric(entry->ords);
  return instance;
}

shared_ptr<Bits> DirectDocValuesProducer::getMissingBits(
    shared_ptr<FieldInfo> field, int64_t const offset,
    int64_t const length) 
{
  if (offset == -1) {
    return make_shared<Bits::MatchAllBits>(maxDoc);
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
        instance = make_shared<FixedBitSet>(bits, maxDoc);
        if (!merging) {
          docsWithFieldInstances.emplace(field->name, instance);
          ramBytesUsed_->addAndGet(instance->ramBytesUsed());
        }
      }
    }
    return instance;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocValuesProducer>
DirectDocValuesProducer::getMergeInstance() 
{
  return make_shared<DirectDocValuesProducer>(shared_from_this());
}

DirectDocValuesProducer::~DirectDocValuesProducer() { delete data; }

int64_t DirectDocValuesProducer::BinaryRawValues::ramBytesUsed()
{
  int64_t bytesUsed = RamUsageEstimator::sizeOf(bytes);
  if (address.size() > 0) {
    bytesUsed += RamUsageEstimator::sizeOf(address);
  }
  return bytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectDocValuesProducer::BinaryRawValues::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (address.size() > 0) {
    resources.push_back(Accountables::namedAccountable(
        L"addresses", RamUsageEstimator::sizeOf(address)));
  }
  resources.push_back(Accountables::namedAccountable(
      L"bytes", RamUsageEstimator::sizeOf(bytes)));
  return Collections::unmodifiableList(resources);
}

wstring DirectDocValuesProducer::BinaryRawValues::toString()
{
  return getClass().getSimpleName();
}

int64_t DirectDocValuesProducer::NumericRawValues::ramBytesUsed()
{
  return bytesUsed;
}

wstring DirectDocValuesProducer::NumericRawValues::toString()
{
  return getClass().getSimpleName();
}

int64_t DirectDocValuesProducer::SortedRawValues::ramBytesUsed()
{
  return docToOrd->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectDocValuesProducer::SortedRawValues::getChildResources()
{
  return docToOrd->getChildResources();
}

wstring DirectDocValuesProducer::SortedRawValues::toString()
{
  return getClass().getSimpleName();
}

int64_t DirectDocValuesProducer::SortedNumericRawValues::ramBytesUsed()
{
  int64_t bytesUsed = values->ramBytesUsed();
  if (docToAddress != nullptr) {
    bytesUsed += docToAddress->ramBytesUsed();
  }
  return bytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectDocValuesProducer::SortedNumericRawValues::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (docToAddress != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"addresses", docToAddress));
  }
  resources.push_back(Accountables::namedAccountable(L"values", values));
  return Collections::unmodifiableList(resources);
}

wstring DirectDocValuesProducer::SortedNumericRawValues::toString()
{
  return getClass().getSimpleName();
}

int64_t DirectDocValuesProducer::SortedSetRawValues::ramBytesUsed()
{
  int64_t bytesUsed = ords->ramBytesUsed();
  if (docToOrdAddress != nullptr) {
    bytesUsed += docToOrdAddress->ramBytesUsed();
  }
  return bytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectDocValuesProducer::SortedSetRawValues::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (docToOrdAddress != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"addresses", docToOrdAddress));
  }
  resources.push_back(Accountables::namedAccountable(L"ordinals", ords));
  return Collections::unmodifiableList(resources);
}

wstring DirectDocValuesProducer::SortedSetRawValues::toString()
{
  return getClass().getSimpleName();
}
} // namespace org::apache::lucene::codecs::memory