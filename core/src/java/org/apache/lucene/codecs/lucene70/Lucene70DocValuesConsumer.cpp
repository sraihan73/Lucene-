using namespace std;

#include "Lucene70DocValuesConsumer.h"

namespace org::apache::lucene::codecs::lucene70
{
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70DocValuesFormat.DIRECT_MONOTONIC_BLOCK_SHIFT;
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70DocValuesFormat.NUMERIC_BLOCK_SHIFT;
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70DocValuesFormat.NUMERIC_BLOCK_SIZE;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using EmptyDocValuesProducer =
    org::apache::lucene::index::EmptyDocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using MathUtil = org::apache::lucene::util::MathUtil;
using StringHelper = org::apache::lucene::util::StringHelper;
using DirectMonotonicWriter =
    org::apache::lucene::util::packed::DirectMonotonicWriter;
using DirectWriter = org::apache::lucene::util::packed::DirectWriter;

Lucene70DocValuesConsumer::Lucene70DocValuesConsumer(
    shared_ptr<SegmentWriteState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
{
  bool success = false;
  try {
    wstring dataName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, dataExtension);
    data = state->directory->createOutput(dataName, state->context);
    CodecUtil::writeIndexHeader(
        data, dataCodec, Lucene70DocValuesFormat::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    wstring metaName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, metaExtension);
    meta = state->directory->createOutput(metaName, state->context);
    CodecUtil::writeIndexHeader(
        meta, metaCodec, Lucene70DocValuesFormat::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    maxDoc = state->segmentInfo->maxDoc();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({shared_from_this()});
    }
  }
}

Lucene70DocValuesConsumer::~Lucene70DocValuesConsumer()
{
  bool success = false;
  try {
    if (meta != nullptr) {
      meta->writeInt(-1);           // write EOF marker
      CodecUtil::writeFooter(meta); // write checksum
    }
    if (data != nullptr) {
      CodecUtil::writeFooter(data); // write checksum
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({data, meta});
    } else {
      IOUtils::closeWhileHandlingException({data, meta});
    }
    meta = data = nullptr;
  }
}

void Lucene70DocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeInt(field->number);
  meta->writeByte(Lucene70DocValuesFormat::NUMERIC);

  writeValues(field, make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                         shared_from_this(), field, valuesProducer));
}

Lucene70DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> valuesProducer)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->valuesProducer = valuesProducer;
}

shared_ptr<SortedNumericDocValues>
Lucene70DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    getSortedNumeric(shared_ptr<FieldInfo> field) 
{
  return DocValues::singleton(valuesProducer->getNumeric(field));
}

Lucene70DocValuesConsumer::MinMaxTracker::MinMaxTracker()
{
  reset();
  spaceInBits = 0;
}

void Lucene70DocValuesConsumer::MinMaxTracker::reset()
{
  min = numeric_limits<int64_t>::max();
  max = numeric_limits<int64_t>::min();
  numValues = 0;
}

void Lucene70DocValuesConsumer::MinMaxTracker::update(int64_t v)
{
  min = min(min, v);
  max = max(max, v);
  ++numValues;
}

void Lucene70DocValuesConsumer::MinMaxTracker::finish()
{
  if (max > min) {
    spaceInBits += DirectWriter::unsignedBitsRequired(max - min) * numValues;
  }
}

void Lucene70DocValuesConsumer::MinMaxTracker::nextBlock()
{
  finish();
  reset();
}

std::deque<int64_t> Lucene70DocValuesConsumer::writeValues(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<SortedNumericDocValues> values =
      valuesProducer->getSortedNumeric(field);
  int numDocsWithValue = 0;
  shared_ptr<MinMaxTracker> minMax = make_shared<MinMaxTracker>();
  shared_ptr<MinMaxTracker> blockMinMax = make_shared<MinMaxTracker>();
  int64_t gcd = 0;
  shared_ptr<Set<int64_t>> uniqueValues = unordered_set<int64_t>();
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    for (int i = 0, count = values->docValueCount(); i < count; ++i) {
      int64_t v = values->nextValue();

      if (gcd != 1) {
        if (v < numeric_limits<int64_t>::min() / 2 ||
            v > numeric_limits<int64_t>::max() / 2) {
          // in that case v - minValue might overflow and make the GCD
          // computation return wrong results. Since these extreme values are
          // unlikely, we just discard GCD computation for them
          gcd = 1;
        } else if (minMax->numValues != 0) { // minValue needs to be set first
          gcd = MathUtil::gcd(gcd, v - minMax->min);
        }
      }

      minMax->update(v);
      blockMinMax->update(v);
      if (blockMinMax->numValues == NUMERIC_BLOCK_SIZE) {
        blockMinMax->nextBlock();
      }

      if (uniqueValues != nullptr && uniqueValues->add(v) &&
          uniqueValues->size() > 256) {
        uniqueValues.reset();
      }
    }

    numDocsWithValue++;
  }

  minMax->finish();
  blockMinMax->finish();

  constexpr int64_t numValues = minMax->numValues;
  int64_t min = minMax->min;
  constexpr int64_t max = minMax->max;
  assert(blockMinMax->spaceInBits <= minMax->spaceInBits);

  if (numDocsWithValue == 0) {
    meta->writeLong(-2);
    meta->writeLong(0LL);
  } else if (numDocsWithValue == maxDoc) {
    meta->writeLong(-1);
    meta->writeLong(0LL);
  } else {
    int64_t offset = data->getFilePointer();
    meta->writeLong(offset);
    values = valuesProducer->getSortedNumeric(field);
    IndexedDISI::writeBitSet(values, data);
    meta->writeLong(data->getFilePointer() - offset);
  }

  meta->writeLong(numValues);
  constexpr int numBitsPerValue;
  bool doBlocks = false;
  unordered_map<int64_t, int> encode;
  if (min >= max) {
    numBitsPerValue = 0;
    meta->writeInt(-1);
  } else {
    if (uniqueValues != nullptr && uniqueValues->size() > 1 &&
        DirectWriter::unsignedBitsRequired(uniqueValues->size() - 1) <
            DirectWriter::unsignedBitsRequired((max - min) / gcd)) {
      numBitsPerValue =
          DirectWriter::unsignedBitsRequired(uniqueValues->size() - 1);
      const std::deque<optional<int64_t>> sortedUniqueValues =
          uniqueValues->toArray(std::deque<optional<int64_t>>(0));
      Arrays::sort(sortedUniqueValues);
      meta->writeInt(sortedUniqueValues.size());
      for (shared_ptr<> : : optional<int64_t> v : sortedUniqueValues) {
        meta->writeLong(v);
      }
      encode = unordered_map<>();
      for (int i = 0; i < sortedUniqueValues.size(); ++i) {
        encode.emplace(sortedUniqueValues[i], i);
      }
      min = 0;
      gcd = 1;
    } else {
      uniqueValues.reset();
      // we do blocks if that appears to save 10+% storage
      doBlocks =
          minMax->spaceInBits > 0 &&
          static_cast<double>(blockMinMax->spaceInBits) / minMax->spaceInBits <=
              0.9;
      if (doBlocks) {
        numBitsPerValue = 0xFF;
        meta->writeInt(-2 - NUMERIC_BLOCK_SHIFT);
      } else {
        numBitsPerValue = DirectWriter::unsignedBitsRequired((max - min) / gcd);
        if (gcd == 1 && min > 0 &&
            DirectWriter::unsignedBitsRequired(max) ==
                DirectWriter::unsignedBitsRequired(max - min)) {
          min = 0;
        }
        meta->writeInt(-1);
      }
    }
  }

  meta->writeByte(static_cast<char>(numBitsPerValue));
  meta->writeLong(min);
  meta->writeLong(gcd);
  int64_t startOffset = data->getFilePointer();
  meta->writeLong(startOffset);
  if (doBlocks) {
    writeValuesMultipleBlocks(valuesProducer->getSortedNumeric(field), gcd);
  } else if (numBitsPerValue != 0) {
    writeValuesSingleBlock(valuesProducer->getSortedNumeric(field), numValues,
                           numBitsPerValue, min, gcd, encode);
  }
  meta->writeLong(data->getFilePointer() - startOffset);

  return std::deque<int64_t>{numDocsWithValue, numValues};
}

void Lucene70DocValuesConsumer::writeValuesSingleBlock(
    shared_ptr<SortedNumericDocValues> values, int64_t numValues,
    int numBitsPerValue, int64_t min, int64_t gcd,
    unordered_map<int64_t, int> &encode) 
{
  shared_ptr<DirectWriter> writer =
      DirectWriter::getInstance(data, numValues, numBitsPerValue);
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    for (int i = 0, count = values->docValueCount(); i < count; ++i) {
      int64_t v = values->nextValue();
      if (encode.empty()) {
        writer->add((v - min) / gcd);
      } else {
        writer->add(encode[v]);
      }
    }
  }
  writer->finish();
}

void Lucene70DocValuesConsumer::writeValuesMultipleBlocks(
    shared_ptr<SortedNumericDocValues> values, int64_t gcd) 
{
  const std::deque<int64_t> buffer =
      std::deque<int64_t>(NUMERIC_BLOCK_SIZE);
  shared_ptr<GrowableByteArrayDataOutput> *const encodeBuffer =
      make_shared<GrowableByteArrayDataOutput>(NUMERIC_BLOCK_SIZE);
  int upTo = 0;
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    for (int i = 0, count = values->docValueCount(); i < count; ++i) {
      buffer[upTo++] = values->nextValue();
      if (upTo == NUMERIC_BLOCK_SIZE) {
        writeBlock(buffer, NUMERIC_BLOCK_SIZE, gcd, encodeBuffer);
        upTo = 0;
      }
    }
  }
  if (upTo > 0) {
    writeBlock(buffer, upTo, gcd, encodeBuffer);
  }
}

void Lucene70DocValuesConsumer::writeBlock(
    std::deque<int64_t> &values, int length, int64_t gcd,
    shared_ptr<GrowableByteArrayDataOutput> buffer) 
{
  assert(length > 0);
  int64_t min = values[0];
  int64_t max = values[0];
  for (int i = 1; i < length; ++i) {
    constexpr int64_t v = values[i];
    assert((Math::floorMod(values[i] - min, gcd) == 0));
    min = min(min, v);
    max = max(max, v);
  }
  if (min == max) {
    data->writeByte(static_cast<char>(0));
    data->writeLong(min);
  } else {
    constexpr int bitsPerValue = DirectWriter::unsignedBitsRequired(max - min);
    buffer->reset();
    assert(buffer->getPosition() == 0);
    shared_ptr<DirectWriter> *const w =
        DirectWriter::getInstance(buffer, length, bitsPerValue);
    for (int i = 0; i < length; ++i) {
      w->add((values[i] - min) / gcd);
    }
    w->finish();
    data->writeByte(static_cast<char>(bitsPerValue));
    data->writeLong(min);
    data->writeInt(buffer->getPosition());
    data->writeBytes(buffer->getBytes(), buffer->getPosition());
  }
}

void Lucene70DocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeInt(field->number);
  meta->writeByte(Lucene70DocValuesFormat::BINARY);

  shared_ptr<BinaryDocValues> values = valuesProducer->getBinary(field);
  int64_t start = data->getFilePointer();
  meta->writeLong(start);
  int numDocsWithField = 0;
  int minLength = numeric_limits<int>::max();
  int maxLength = 0;
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    numDocsWithField++;
    shared_ptr<BytesRef> v = values->binaryValue();
    int length = v->length;
    data->writeBytes(v->bytes, v->offset, v->length);
    minLength = min(length, minLength);
    maxLength = max(length, maxLength);
  }
  assert(numDocsWithField <= maxDoc);
  meta->writeLong(data->getFilePointer() - start);

  if (numDocsWithField == 0) {
    meta->writeLong(-2);
    meta->writeLong(0LL);
  } else if (numDocsWithField == maxDoc) {
    meta->writeLong(-1);
    meta->writeLong(0LL);
  } else {
    int64_t offset = data->getFilePointer();
    meta->writeLong(offset);
    values = valuesProducer->getBinary(field);
    IndexedDISI::writeBitSet(values, data);
    meta->writeLong(data->getFilePointer() - offset);
  }

  meta->writeInt(numDocsWithField);
  meta->writeInt(minLength);
  meta->writeInt(maxLength);
  if (maxLength > minLength) {
    start = data->getFilePointer();
    meta->writeLong(start);
    meta->writeVInt(DIRECT_MONOTONIC_BLOCK_SHIFT);

    shared_ptr<DirectMonotonicWriter> *const writer =
        DirectMonotonicWriter::getInstance(meta, data, numDocsWithField + 1,
                                           DIRECT_MONOTONIC_BLOCK_SHIFT);
    int64_t addr = 0;
    writer->add(addr);
    values = valuesProducer->getBinary(field);
    for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = values->nextDoc()) {
      addr += values->binaryValue()->length;
      writer->add(addr);
    }
    writer->finish();
    meta->writeLong(data->getFilePointer() - start);
  }
}

void Lucene70DocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeInt(field->number);
  meta->writeByte(Lucene70DocValuesFormat::SORTED);
  doAddSortedField(field, valuesProducer);
}

void Lucene70DocValuesConsumer::doAddSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<SortedDocValues> values = valuesProducer->getSorted(field);
  int numDocsWithField = 0;
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    numDocsWithField++;
  }

  if (numDocsWithField == 0) {
    meta->writeLong(-2);
    meta->writeLong(0LL);
  } else if (numDocsWithField == maxDoc) {
    meta->writeLong(-1);
    meta->writeLong(0LL);
  } else {
    int64_t offset = data->getFilePointer();
    meta->writeLong(offset);
    values = valuesProducer->getSorted(field);
    IndexedDISI::writeBitSet(values, data);
    meta->writeLong(data->getFilePointer() - offset);
  }

  meta->writeInt(numDocsWithField);
  if (values->getValueCount() <= 1) {
    meta->writeByte(static_cast<char>(0));
    meta->writeLong(0LL);
    meta->writeLong(0LL);
  } else {
    int numberOfBitsPerOrd =
        DirectWriter::unsignedBitsRequired(values->getValueCount() - 1);
    meta->writeByte(static_cast<char>(numberOfBitsPerOrd));
    int64_t start = data->getFilePointer();
    meta->writeLong(start);
    shared_ptr<DirectWriter> writer =
        DirectWriter::getInstance(data, numDocsWithField, numberOfBitsPerOrd);
    values = valuesProducer->getSorted(field);
    for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = values->nextDoc()) {
      writer->add(values->ordValue());
    }
    writer->finish();
    meta->writeLong(data->getFilePointer() - start);
  }

  addTermsDict(DocValues::singleton(valuesProducer->getSorted(field)));
}

void Lucene70DocValuesConsumer::addTermsDict(
    shared_ptr<SortedSetDocValues> values) 
{
  constexpr int64_t size = values->getValueCount();
  meta->writeVLong(size);
  meta->writeInt(Lucene70DocValuesFormat::TERMS_DICT_BLOCK_SHIFT);

  shared_ptr<RAMOutputStream> addressBuffer = make_shared<RAMOutputStream>();
  meta->writeInt(DIRECT_MONOTONIC_BLOCK_SHIFT);
  int64_t numBlocks = static_cast<int64_t>(
      static_cast<uint64_t>(
          (size + Lucene70DocValuesFormat::TERMS_DICT_BLOCK_MASK)) >>
      Lucene70DocValuesFormat::TERMS_DICT_BLOCK_SHIFT);
  shared_ptr<DirectMonotonicWriter> writer = DirectMonotonicWriter::getInstance(
      meta, addressBuffer, numBlocks, DIRECT_MONOTONIC_BLOCK_SHIFT);

  shared_ptr<BytesRefBuilder> previous = make_shared<BytesRefBuilder>();
  int64_t ord = 0;
  int64_t start = data->getFilePointer();
  int maxLength = 0;
  shared_ptr<TermsEnum> iterator = values->termsEnum();
  for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
       term = iterator->next()) {
    if ((ord & Lucene70DocValuesFormat::TERMS_DICT_BLOCK_MASK) == 0) {
      writer->add(data->getFilePointer() - start);
      data->writeVInt(term->length);
      data->writeBytes(term->bytes, term->offset, term->length);
    } else {
      constexpr int prefixLength =
          StringHelper::bytesDifference(previous->get(), term);
      constexpr int suffixLength = term->length - prefixLength;
      assert(suffixLength > 0); // terms are unique

      data->writeByte(static_cast<char>(min(prefixLength, 15) |
                                        (min(15, suffixLength - 1) << 4)));
      if (prefixLength >= 15) {
        data->writeVInt(prefixLength - 15);
      }
      if (suffixLength >= 16) {
        data->writeVInt(suffixLength - 16);
      }
      data->writeBytes(term->bytes, term->offset + prefixLength,
                       term->length - prefixLength);
    }
    maxLength = max(maxLength, term->length);
    previous->copyBytes(term);
    ++ord;
  }
  writer->finish();
  meta->writeInt(maxLength);
  meta->writeLong(start);
  meta->writeLong(data->getFilePointer() - start);
  start = data->getFilePointer();
  addressBuffer->writeTo(data);
  meta->writeLong(start);
  meta->writeLong(data->getFilePointer() - start);

  // Now write the reverse terms index
  writeTermsIndex(values);
}

void Lucene70DocValuesConsumer::writeTermsIndex(
    shared_ptr<SortedSetDocValues> values) 
{
  constexpr int64_t size = values->getValueCount();
  meta->writeInt(Lucene70DocValuesFormat::TERMS_DICT_REVERSE_INDEX_SHIFT);
  int64_t start = data->getFilePointer();

  int64_t numBlocks =
      1LL +
      (static_cast<int64_t>(
          static_cast<uint64_t>((
              size + Lucene70DocValuesFormat::TERMS_DICT_REVERSE_INDEX_MASK)) >>
          Lucene70DocValuesFormat::TERMS_DICT_REVERSE_INDEX_SHIFT));
  shared_ptr<RAMOutputStream> addressBuffer = make_shared<RAMOutputStream>();
  shared_ptr<DirectMonotonicWriter> writer = DirectMonotonicWriter::getInstance(
      meta, addressBuffer, numBlocks, DIRECT_MONOTONIC_BLOCK_SHIFT);

  shared_ptr<TermsEnum> iterator = values->termsEnum();
  shared_ptr<BytesRefBuilder> previous = make_shared<BytesRefBuilder>();
  int64_t offset = 0;
  int64_t ord = 0;
  for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
       term = iterator->next()) {
    if ((ord & Lucene70DocValuesFormat::TERMS_DICT_REVERSE_INDEX_MASK) == 0) {
      writer->add(offset);
      constexpr int sortKeyLength;
      if (ord == 0) {
        // no previous term: no bytes to write
        sortKeyLength = 0;
      } else {
        sortKeyLength = StringHelper::sortKeyLength(previous->get(), term);
      }
      offset += sortKeyLength;
      data->writeBytes(term->bytes, term->offset, sortKeyLength);
    } else if ((ord & Lucene70DocValuesFormat::TERMS_DICT_REVERSE_INDEX_MASK) ==
               Lucene70DocValuesFormat::TERMS_DICT_REVERSE_INDEX_MASK) {
      previous->copyBytes(term);
    }
    ++ord;
  }
  writer->add(offset);
  writer->finish();
  meta->writeLong(start);
  meta->writeLong(data->getFilePointer() - start);
  start = data->getFilePointer();
  addressBuffer->writeTo(data);
  meta->writeLong(start);
  meta->writeLong(data->getFilePointer() - start);
}

void Lucene70DocValuesConsumer::addSortedNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeInt(field->number);
  meta->writeByte(Lucene70DocValuesFormat::SORTED_NUMERIC);

  std::deque<int64_t> stats = writeValues(field, valuesProducer);
  int numDocsWithField = Math::toIntExact(stats[0]);
  int64_t numValues = stats[1];
  assert(numValues >= numDocsWithField);

  meta->writeInt(numDocsWithField);
  if (numValues > numDocsWithField) {
    int64_t start = data->getFilePointer();
    meta->writeLong(start);
    meta->writeVInt(DIRECT_MONOTONIC_BLOCK_SHIFT);

    shared_ptr<DirectMonotonicWriter> *const addressesWriter =
        DirectMonotonicWriter::getInstance(meta, data, numDocsWithField + 1LL,
                                           DIRECT_MONOTONIC_BLOCK_SHIFT);
    int64_t addr = 0;
    addressesWriter->add(addr);
    shared_ptr<SortedNumericDocValues> values =
        valuesProducer->getSortedNumeric(field);
    for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = values->nextDoc()) {
      addr += values->docValueCount();
      addressesWriter->add(addr);
    }
    addressesWriter->finish();
    meta->writeLong(data->getFilePointer() - start);
  }
}

void Lucene70DocValuesConsumer::addSortedSetField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeInt(field->number);
  meta->writeByte(Lucene70DocValuesFormat::SORTED_SET);

  shared_ptr<SortedSetDocValues> values = valuesProducer->getSortedSet(field);
  int numDocsWithField = 0;
  int64_t numOrds = 0;
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    numDocsWithField++;
    for (int64_t ord = values->nextOrd();
         ord != SortedSetDocValues::NO_MORE_ORDS; ord = values->nextOrd()) {
      numOrds++;
    }
  }

  if (numDocsWithField == numOrds) {
    meta->writeByte(static_cast<char>(0));
    doAddSortedField(field,
                     make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                         shared_from_this(), field, valuesProducer));
    return;
  }
  meta->writeByte(static_cast<char>(1));

  assert(numDocsWithField != 0);
  if (numDocsWithField == maxDoc) {
    meta->writeLong(-1);
    meta->writeLong(0LL);
  } else {
    int64_t offset = data->getFilePointer();
    meta->writeLong(offset);
    values = valuesProducer->getSortedSet(field);
    IndexedDISI::writeBitSet(values, data);
    meta->writeLong(data->getFilePointer() - offset);
  }

  int numberOfBitsPerOrd =
      DirectWriter::unsignedBitsRequired(values->getValueCount() - 1);
  meta->writeByte(static_cast<char>(numberOfBitsPerOrd));
  int64_t start = data->getFilePointer();
  meta->writeLong(start);
  shared_ptr<DirectWriter> writer =
      DirectWriter::getInstance(data, numOrds, numberOfBitsPerOrd);
  values = valuesProducer->getSortedSet(field);
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    for (int64_t ord = values->nextOrd();
         ord != SortedSetDocValues::NO_MORE_ORDS; ord = values->nextOrd()) {
      writer->add(ord);
    }
  }
  writer->finish();
  meta->writeLong(data->getFilePointer() - start);

  meta->writeInt(numDocsWithField);
  start = data->getFilePointer();
  meta->writeLong(start);
  meta->writeVInt(DIRECT_MONOTONIC_BLOCK_SHIFT);

  shared_ptr<DirectMonotonicWriter> *const addressesWriter =
      DirectMonotonicWriter::getInstance(meta, data, numDocsWithField + 1,
                                         DIRECT_MONOTONIC_BLOCK_SHIFT);
  int64_t addr = 0;
  addressesWriter->add(addr);
  values = valuesProducer->getSortedSet(field);
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    values->nextOrd();
    addr++;
    while (values->nextOrd() != SortedSetDocValues::NO_MORE_ORDS) {
      addr++;
    }
    addressesWriter->add(addr);
  }
  addressesWriter->finish();
  meta->writeLong(data->getFilePointer() - start);

  addTermsDict(values);
}

Lucene70DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<Lucene70DocValuesConsumer> outerInstance,
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> valuesProducer)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->valuesProducer = valuesProducer;
}

shared_ptr<SortedDocValues>
Lucene70DocValuesConsumer::EmptyDocValuesProducerAnonymousInnerClass::getSorted(
    shared_ptr<FieldInfo> field) 
{
  return SortedSetSelector::wrap(valuesProducer->getSortedSet(field),
                                 SortedSetSelector::Type::MIN);
}
} // namespace org::apache::lucene::codecs::lucene70