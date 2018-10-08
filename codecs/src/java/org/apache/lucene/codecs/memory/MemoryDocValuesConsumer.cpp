using namespace std;

#include "MemoryDocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/LegacyDocValuesIterables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/MathUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/BlockPackedWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/PackedInts.h"
#include "MemoryDocValuesFormat.h"

namespace org::apache::lucene::codecs::memory
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using LegacyDocValuesIterables =
    org::apache::lucene::codecs::LegacyDocValuesIterables;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using MathUtil = org::apache::lucene::util::MathUtil;
using Builder = org::apache::lucene::util::fst::Builder;
using INPUT_TYPE = org::apache::lucene::util::fst::FST::INPUT_TYPE;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
using BlockPackedWriter = org::apache::lucene::util::packed::BlockPackedWriter;
using MonotonicBlockPackedWriter =
    org::apache::lucene::util::packed::MonotonicBlockPackedWriter;
using FormatAndBits =
    org::apache::lucene::util::packed::PackedInts::FormatAndBits;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.BLOCK_COMPRESSED;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.BLOCK_SIZE; import
//    static org.apache.lucene.codecs.memory.MemoryDocValuesProducer.BYTES;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.DELTA_COMPRESSED;
//    import static org.apache.lucene.codecs.memory.MemoryDocValuesProducer.FST;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.GCD_COMPRESSED;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.NUMBER; import
//    static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_NUMERIC;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_NUMERIC_SINGLETON;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_SET; import
//    static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_SET_SINGLETON;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.TABLE_COMPRESSED;
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.VERSION_CURRENT;

MemoryDocValuesConsumer::MemoryDocValuesConsumer(
    shared_ptr<SegmentWriteState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension,
    float acceptableOverheadRatio) 
    : maxDoc(state->segmentInfo->maxDoc()),
      acceptableOverheadRatio(acceptableOverheadRatio)
{
  bool success = false;
  try {
    wstring dataName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, dataExtension);
    data = state->directory->createOutput(dataName, state->context);
    CodecUtil::writeIndexHeader(data, dataCodec, FST::VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    wstring metaName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, metaExtension);
    meta = state->directory->createOutput(metaName, state->context);
    CodecUtil::writeIndexHeader(meta, metaCodec, FST::VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
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

void MemoryDocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  addNumericField(
      field,
      LegacyDocValuesIterables::numericIterable(field, valuesProducer, maxDoc),
      true);
}

void MemoryDocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field, deque<std::shared_ptr<Number>> &values,
    bool optimizeStorage) 
{
  meta->writeVInt(field->number);
  meta->writeByte(NUMBER);
  meta->writeLong(data->getFilePointer());
  int64_t minValue = numeric_limits<int64_t>::max();
  int64_t maxValue = numeric_limits<int64_t>::min();
  int64_t blockSum = 0;
  int64_t gcd = 0;
  bool missing = false;
  // TODO: more efficient?
  unordered_set<int64_t> uniqueValues = nullptr;
  int64_t count = 0;

  if (optimizeStorage) {
    uniqueValues = unordered_set<int64_t>();

    int64_t currentBlockMin = numeric_limits<int64_t>::max();
    int64_t currentBlockMax = numeric_limits<int64_t>::min();
    for (auto nv : values) {
      constexpr int64_t v;
      if (nv == nullptr) {
        v = 0;
        missing = true;
      } else {
        v = nv->longValue();
      }

      if (gcd != 1) {
        if (v < numeric_limits<int64_t>::min() / 2 ||
            v > numeric_limits<int64_t>::max() / 2) {
          // in that case v - minValue might overflow and make the GCD
          // computation return wrong results. Since these extreme values are
          // unlikely, we just discard GCD computation for them
          gcd = 1;
        } else if (count != 0) { // minValue needs to be set first
          gcd = MathUtil::gcd(gcd, v - minValue);
        }
      }

      currentBlockMin = min(minValue, v);
      currentBlockMax = max(maxValue, v);

      minValue = min(minValue, v);
      maxValue = max(maxValue, v);

      if (uniqueValues != nullptr) {
        if (uniqueValues.insert(v)) {
          if (uniqueValues.size() > 256) {
            uniqueValues = nullptr;
          }
        }
      }

      ++count;
      if (count % BLOCK_SIZE == 0) {
        constexpr int64_t blockDelta = currentBlockMax - currentBlockMin;
        constexpr int blockDeltaRequired =
            PackedInts::unsignedBitsRequired(blockDelta);
        constexpr int blockBPV =
            PackedInts::fastestFormatAndBits(BLOCK_SIZE, blockDeltaRequired,
                                             acceptableOverheadRatio)
                ->bitsPerValue;
        blockSum += blockBPV;
        currentBlockMax = numeric_limits<int64_t>::min();
        currentBlockMin = numeric_limits<int64_t>::max();
      }
    }
  } else {
    for (auto nv : values) {
      int64_t v = nv->longValue();
      maxValue = max(v, maxValue);
      minValue = min(v, minValue);
      count++;
    }
  }

  if (missing) {
    int64_t start = data->getFilePointer();
    writeMissingBitset(values);
    meta->writeLong(start);
    meta->writeLong(data->getFilePointer() - start);
  } else {
    meta->writeLong(-1LL);
  }

  constexpr int64_t delta = maxValue - minValue;
  constexpr int deltaRequired =
      delta < 0 ? 64 : PackedInts::bitsRequired(delta);
  shared_ptr<FormatAndBits> *const deltaBPV = PackedInts::fastestFormatAndBits(
      maxDoc, deltaRequired, acceptableOverheadRatio);

  shared_ptr<FormatAndBits> *const tableBPV;
  if (count < numeric_limits<int>::max() && uniqueValues != nullptr) {
    tableBPV = PackedInts::fastestFormatAndBits(
        maxDoc, PackedInts::bitsRequired(uniqueValues.size() - 1),
        acceptableOverheadRatio);
  } else {
    tableBPV.reset();
  }

  shared_ptr<FormatAndBits> *const gcdBPV;
  if (count < numeric_limits<int>::max() && gcd != 0 && gcd != 1) {
    constexpr int64_t gcdDelta = (maxValue - minValue) / gcd;
    constexpr int gcdRequired =
        gcdDelta < 0 ? 64 : PackedInts::bitsRequired(gcdDelta);
    gcdBPV = PackedInts::fastestFormatAndBits(maxDoc, gcdRequired,
                                              acceptableOverheadRatio);
  } else {
    gcdBPV.reset();
  }

  bool doBlock = false;
  if (blockSum != 0) {
    int numBlocks = maxDoc / BLOCK_SIZE;
    float avgBPV = blockSum / static_cast<float>(numBlocks);
    // just a heuristic, with tiny amounts of blocks our estimate is skewed as
    // we ignore the final "incomplete" block. with at least 4 blocks it's
    // pretty accurate. The difference must also be significant (according to
    // acceptable overhead).
    if (numBlocks >= 4 &&
        (avgBPV + avgBPV * acceptableOverheadRatio) < deltaBPV->bitsPerValue) {
      doBlock = true;
    }
  }
  // blockpackedreader allows us to read in huge streams of ints
  if (count >= numeric_limits<int>::max()) {
    doBlock = true;
  }

  if (tableBPV != nullptr &&
      (tableBPV->bitsPerValue +
       tableBPV->bitsPerValue * acceptableOverheadRatio) <
          deltaBPV->bitsPerValue) {
    // small number of unique values
    meta->writeByte(TABLE_COMPRESSED); // table-compressed
    std::deque<optional<int64_t>> decode = uniqueValues.toArray(
        std::deque<optional<int64_t>>(uniqueValues.size()));
    const unordered_map<int64_t, int> encode =
        unordered_map<int64_t, int>();
    int length = 1 << tableBPV->bitsPerValue;
    data->writeVInt(length);
    for (int i = 0; i < decode.size(); i++) {
      data->writeLong(decode[i]);
      encode.emplace(decode[i], i);
    }
    for (int i = decode.size(); i < length; i++) {
      data->writeLong(0);
    }

    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeLong(count);
    data->writeVInt(tableBPV->format.getId());
    data->writeVInt(tableBPV->bitsPerValue);

    shared_ptr<PackedInts::Writer> *const writer =
        PackedInts::getWriterNoHeader(
            data, tableBPV->format, static_cast<int>(count),
            tableBPV->bitsPerValue, PackedInts::DEFAULT_BUFFER_SIZE);
    for (auto nv : values) {
      writer->add(encode[nv == nullptr ? 0 : nv->longValue()]);
    }
    writer->finish();
  } else if (gcdBPV != nullptr &&
             (gcdBPV->bitsPerValue +
              gcdBPV->bitsPerValue * acceptableOverheadRatio) <
                 deltaBPV->bitsPerValue) {
    meta->writeByte(GCD_COMPRESSED);
    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeLong(count);
    data->writeLong(minValue);
    data->writeLong(gcd);
    data->writeVInt(gcdBPV->format.getId());
    data->writeVInt(gcdBPV->bitsPerValue);

    shared_ptr<PackedInts::Writer> *const writer =
        PackedInts::getWriterNoHeader(
            data, gcdBPV->format, static_cast<int>(count), gcdBPV->bitsPerValue,
            PackedInts::DEFAULT_BUFFER_SIZE);
    for (auto nv : values) {
      int64_t value = nv == nullptr ? 0 : nv->longValue();
      writer->add((value - minValue) / gcd);
    }
    writer->finish();
  } else if (doBlock) {
    meta->writeByte(BLOCK_COMPRESSED); // block delta-compressed
    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeLong(count);
    data->writeVInt(BLOCK_SIZE);
    shared_ptr<BlockPackedWriter> *const writer =
        make_shared<BlockPackedWriter>(data, BLOCK_SIZE);
    for (auto nv : values) {
      writer->add(nv == nullptr ? 0 : nv->longValue());
    }
    writer->finish();
  } else {
    meta->writeByte(DELTA_COMPRESSED); // delta-compressed
    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeLong(count);
    constexpr int64_t minDelta = deltaBPV->bitsPerValue == 64 ? 0 : minValue;
    data->writeLong(minDelta);
    data->writeVInt(deltaBPV->format.getId());
    data->writeVInt(deltaBPV->bitsPerValue);

    shared_ptr<PackedInts::Writer> *const writer =
        PackedInts::getWriterNoHeader(
            data, deltaBPV->format, static_cast<int>(count),
            deltaBPV->bitsPerValue, PackedInts::DEFAULT_BUFFER_SIZE);
    for (auto nv : values) {
      int64_t v = nv == nullptr ? 0 : nv->longValue();
      writer->add(v - minDelta);
    }
    writer->finish();
  }
}

MemoryDocValuesConsumer::~MemoryDocValuesConsumer()
{
  bool success = false;
  try {
    if (meta != nullptr) {
      meta->writeVInt(-1);          // write EOF marker
      CodecUtil::writeFooter(meta); // write checksum
    }
    if (data != nullptr) {
      CodecUtil::writeFooter(data);
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
    data = meta = nullptr;
  }
}

void MemoryDocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  addBinaryField(field, LegacyDocValuesIterables::binaryIterable(
                            field, valuesProducer, maxDoc));
}

void MemoryDocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<BytesRef>> &values) 
{
  // write the byte[] data
  meta->writeVInt(field->number);
  meta->writeByte(BYTES);
  int minLength = numeric_limits<int>::max();
  int maxLength = numeric_limits<int>::min();
  constexpr int64_t startFP = data->getFilePointer();
  bool missing = false;
  int upto = 0;
  for (auto v : values) {
    constexpr int length;
    if (v == nullptr) {
      length = 0;
      missing = true;
    } else {
      length = v->length;
    }
    if (length > MemoryDocValuesFormat::MAX_BINARY_FIELD_LENGTH) {
      throw invalid_argument(
          L"DocValuesField \"" + field->name + L"\" is too large, must be <= " +
          to_wstring(MemoryDocValuesFormat::MAX_BINARY_FIELD_LENGTH) +
          L" but got length=" + to_wstring(length) + L" v=" + v + L"; upto=" +
          to_wstring(upto) + L" values=" + values);
    }
    upto++;
    minLength = min(minLength, length);
    maxLength = max(maxLength, length);
    if (v != nullptr) {
      data->writeBytes(v->bytes, v->offset, v->length);
    }
  }
  meta->writeLong(startFP);
  meta->writeLong(data->getFilePointer() - startFP);
  if (missing) {
    int64_t start = data->getFilePointer();
    writeMissingBitset(values);
    meta->writeLong(start);
    meta->writeLong(data->getFilePointer() - start);
  } else {
    meta->writeLong(-1LL);
  }
  meta->writeVInt(minLength);
  meta->writeVInt(maxLength);

  // if minLength == maxLength, it's a fixed-length byte[], we are done (the
  // addresses are implicit) otherwise, we need to record the length fields...
  if (minLength != maxLength) {
    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeVInt(BLOCK_SIZE);

    shared_ptr<MonotonicBlockPackedWriter> *const writer =
        make_shared<MonotonicBlockPackedWriter>(data, BLOCK_SIZE);
    int64_t addr = 0;
    for (auto v : values) {
      if (v != nullptr) {
        addr += v->length;
      }
      writer->add(addr);
    }
    writer->finish();
  }
}

void MemoryDocValuesConsumer::writeFST(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<BytesRef>> &values) 
{
  meta->writeVInt(field->number);
  meta->writeByte(FST);
  meta->writeLong(data->getFilePointer());
  shared_ptr<PositiveIntOutputs> outputs = PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<int64_t>> builder =
      make_shared<Builder<int64_t>>(INPUT_TYPE::BYTE1, outputs);
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  int64_t ord = 0;
  for (auto v : values) {
    builder->add(Util::toIntsRef(v, scratch), ord);
    ord++;
  }
  shared_ptr<FST<int64_t>> fst = builder->finish();
  if (fst != nullptr) {
    fst->save(data);
  }
  meta->writeVLong(ord);
}

template <typename T1>
void MemoryDocValuesConsumer::writeMissingBitset(deque<T1> values) throw(
    IOException)
{
  int64_t bits = 0;
  int count = 0;
  for (auto v : values) {
    if (count == 64) {
      data->writeLong(bits);
      count = 0;
      bits = 0;
    }
    if (v != nullptr) {
      bits |= 1LL << (count & 0x3f);
    }
    count++;
  }
  if (count > 0) {
    data->writeLong(bits);
  }
}

void MemoryDocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  addSortedField(field,
                 LegacyDocValuesIterables::valuesIterable(
                     valuesProducer->getSorted(field)),
                 LegacyDocValuesIterables::sortedOrdIterable(valuesProducer,
                                                             field, maxDoc));
}

void MemoryDocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field, deque<std::shared_ptr<BytesRef>> &values,
    deque<std::shared_ptr<Number>> &docToOrd) 
{
  // write the ordinals as numerics
  addNumericField(field, docToOrd, false);

  // write the values as FST
  writeFST(field, values);
}

void MemoryDocValuesConsumer::addSortedNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{

  const deque<std::shared_ptr<Number>> docToValueCount =
      LegacyDocValuesIterables::sortedNumericToDocCount(valuesProducer, field,
                                                        maxDoc);
  const deque<std::shared_ptr<Number>> values =
      LegacyDocValuesIterables::sortedNumericToValues(valuesProducer, field);

  meta->writeVInt(field->number);

  if (isSingleValued(docToValueCount)) {
    meta->writeByte(SORTED_NUMERIC_SINGLETON);
    addNumericField(field, singletonView(docToValueCount, values, nullptr),
                    true);
  } else {
    meta->writeByte(SORTED_NUMERIC);

    // write the addresses:
    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeVInt(BLOCK_SIZE);
    meta->writeLong(data->getFilePointer());
    shared_ptr<MonotonicBlockPackedWriter> *const writer =
        make_shared<MonotonicBlockPackedWriter>(data, BLOCK_SIZE);
    int64_t addr = 0;
    writer->add(addr);
    for (auto v : docToValueCount) {
      addr += v->longValue();
      writer->add(addr);
    }
    writer->finish();
    int64_t valueCount = writer->ord();
    meta->writeLong(valueCount);

    // write the values
    addNumericField(field, values, true);
  }
}

void MemoryDocValuesConsumer::addSortedSetField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  deque<std::shared_ptr<BytesRef>> values =
      LegacyDocValuesIterables::valuesIterable(
          valuesProducer->getSortedSet(field));
  deque<std::shared_ptr<Number>> docToOrdCount =
      LegacyDocValuesIterables::sortedSetOrdCountIterable(valuesProducer, field,
                                                          maxDoc);
  deque<std::shared_ptr<Number>> ords =
      LegacyDocValuesIterables::sortedSetOrdsIterable(valuesProducer, field);
  meta->writeVInt(field->number);

  if (isSingleValued(docToOrdCount)) {
    meta->writeByte(SORTED_SET_SINGLETON);
    addSortedField(field, values, singletonView(docToOrdCount, ords, -1LL));
  } else {
    meta->writeByte(SORTED_SET);
    // write the ordinals as a binary field
    addBinaryField(field, make_shared<IterableAnonymousInnerClass>(
                              shared_from_this(), docToOrdCount, ords));

    // write the values as FST
    writeFST(field, values);
  }
}

MemoryDocValuesConsumer::IterableAnonymousInnerClass::
    IterableAnonymousInnerClass(
        shared_ptr<MemoryDocValuesConsumer> outerInstance,
        deque<std::shared_ptr<Number>> &docToOrdCount,
        deque<std::shared_ptr<Number>> &ords)
{
  this->outerInstance = outerInstance;
  this->docToOrdCount = docToOrdCount;
  this->ords = ords;
}

shared_ptr<Iterator<std::shared_ptr<BytesRef>>>
MemoryDocValuesConsumer::IterableAnonymousInnerClass::iterator()
{
  return make_shared<SortedSetIterator>(docToOrdCount.begin(), ords.begin());
}

MemoryDocValuesConsumer::SortedSetIterator::SortedSetIterator(
    shared_ptr<Iterator<std::shared_ptr<Number>>> counts,
    shared_ptr<Iterator<std::shared_ptr<Number>>> ords)
    : counts(counts), ords(ords)
{
}

bool MemoryDocValuesConsumer::SortedSetIterator::hasNext()
{
  return counts->hasNext();
}

shared_ptr<BytesRef> MemoryDocValuesConsumer::SortedSetIterator::next()
{
  if (!hasNext()) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }

  int count = counts->next().intValue();
  int maxSize = count * 9; // worst case
  if (maxSize > buffer.size()) {
    buffer = ArrayUtil::grow(buffer, maxSize);
  }

  try {
    encodeValues(count);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }

  ref->bytes = buffer;
  ref->offset = 0;
  ref->length = out->getPosition();

  return ref;
}

void MemoryDocValuesConsumer::SortedSetIterator::encodeValues(int count) throw(
    IOException)
{
  out->reset(buffer);
  int64_t lastOrd = 0;
  for (int i = 0; i < count; i++) {
    int64_t ord = ords->next().longValue();
    out->writeVLong(ord - lastOrd);
    lastOrd = ord;
  }
}

void MemoryDocValuesConsumer::SortedSetIterator::remove()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::memory