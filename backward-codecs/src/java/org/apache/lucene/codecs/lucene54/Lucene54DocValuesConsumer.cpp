using namespace std;

#include "Lucene54DocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/LegacyDocValuesIterables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/LongsRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/MathUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/PagedBytes.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/DirectMonotonicWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/DirectWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/PackedInts.h"
#include "LegacyStringHelper.h"
#include "Lucene54DocValuesFormat.h"

namespace org::apache::lucene::codecs::lucene54
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using LegacyDocValuesIterables =
    org::apache::lucene::codecs::LegacyDocValuesIterables;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongsRef = org::apache::lucene::util::LongsRef;
using MathUtil = org::apache::lucene::util::MathUtil;
using PagedBytesDataInput =
    org::apache::lucene::util::PagedBytes::PagedBytesDataInput;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using DirectMonotonicWriter =
    org::apache::lucene::util::packed::DirectMonotonicWriter;
using DirectWriter = org::apache::lucene::util::packed::DirectWriter;
using MonotonicBlockPackedWriter =
    org::apache::lucene::util::packed::MonotonicBlockPackedWriter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using namespace org::apache::lucene::codecs::lucene54;
//    import static org.apache.lucene.codecs.lucene54.Lucene54DocValuesFormat.*;

Lucene54DocValuesConsumer::Lucene54DocValuesConsumer(
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
        data, dataCodec, Lucene54DocValuesFormat::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    wstring metaName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, metaExtension);
    meta = state->directory->createOutput(metaName, state->context);
    CodecUtil::writeIndexHeader(
        meta, metaCodec, Lucene54DocValuesFormat::VERSION_CURRENT,
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

void Lucene54DocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  addNumericField(
      field,
      LegacyDocValuesIterables::numericIterable(field, valuesProducer, maxDoc),
      NumberType::VALUE);
}

void Lucene54DocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field, deque<std::shared_ptr<Number>> &values,
    NumberType numberType) 
{
  int64_t count = 0;
  int64_t minValue = numeric_limits<int64_t>::max();
  int64_t maxValue = numeric_limits<int64_t>::min();
  int64_t gcd = 0;
  int64_t missingCount = 0;
  int64_t zeroCount = 0;
  // TODO: more efficient?
  unordered_set<int64_t> uniqueValues = nullptr;
  int64_t missingOrdCount = 0;
  if (numberType == NumberType::VALUE) {
    uniqueValues = unordered_set<int64_t>();

    for (auto nv : values) {
      constexpr int64_t v;
      if (nv == nullptr) {
        v = 0;
        missingCount++;
        zeroCount++;
      } else {
        v = nv->longValue();
        if (v == 0) {
          zeroCount++;
        }
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
    }
  } else {
    for (auto nv : values) {
      int64_t v = nv->longValue();
      if (v == -1LL) {
        missingOrdCount++;
      }
      minValue = min(minValue, v);
      maxValue = max(maxValue, v);
      ++count;
    }
  }

  constexpr int64_t delta = maxValue - minValue;
  constexpr int deltaBitsRequired = DirectWriter::unsignedBitsRequired(delta);
  constexpr int tableBitsRequired =
      uniqueValues == nullptr
          ? numeric_limits<int>::max()
          : DirectWriter::bitsRequired(uniqueValues.size() - 1);

  constexpr bool sparse; // 1% of docs or less have a value
  switch (numberType) {
  case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
      NumberType::VALUE:
    sparse = static_cast<double>(missingCount) / count >= 0.99;
    break;
  case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
      NumberType::ORDINAL:
    sparse = static_cast<double>(missingOrdCount) / count >= 0.99;
    break;
  default:
    throw make_shared<AssertionError>();
  }

  constexpr int format;
  if (uniqueValues != nullptr && count <= numeric_limits<int>::max() &&
      (uniqueValues.size() == 1 ||
       (uniqueValues.size() == 2 && missingCount > 0 &&
        zeroCount == missingCount))) {
    // either one unique value C or two unique values: "missing" and C
    format = Lucene54DocValuesFormat::CONST_COMPRESSED;
  } else if (sparse && count >= 1024) {
    // require at least 1024 docs to avoid flipping back and forth when doing
    // NRT search
    format = Lucene54DocValuesFormat::SPARSE_COMPRESSED;
  } else if (uniqueValues != nullptr && tableBitsRequired < deltaBitsRequired) {
    format = Lucene54DocValuesFormat::TABLE_COMPRESSED;
  } else if (gcd != 0 && gcd != 1) {
    constexpr int64_t gcdDelta = (maxValue - minValue) / gcd;
    constexpr int64_t gcdBitsRequired =
        DirectWriter::unsignedBitsRequired(gcdDelta);
    format = gcdBitsRequired < deltaBitsRequired
                 ? Lucene54DocValuesFormat::GCD_COMPRESSED
                 : Lucene54DocValuesFormat::DELTA_COMPRESSED;
  } else {
    format = Lucene54DocValuesFormat::DELTA_COMPRESSED;
  }
  meta->writeVInt(field->number);
  meta->writeByte(Lucene54DocValuesFormat::NUMERIC);
  meta->writeVInt(format);
  if (format == Lucene54DocValuesFormat::SPARSE_COMPRESSED) {
    meta->writeLong(data->getFilePointer());
    constexpr int64_t numDocsWithValue;
    switch (numberType) {
    case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
        NumberType::VALUE:
      numDocsWithValue = count - missingCount;
      break;
    case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
        NumberType::ORDINAL:
      numDocsWithValue = count - missingOrdCount;
      break;
    default:
      throw make_shared<AssertionError>();
    }
    constexpr int64_t maxDoc =
        writeSparseMissingBitset(values, numberType, numDocsWithValue);
    assert(maxDoc == count);
  } else if (missingCount == 0) {
    meta->writeLong(Lucene54DocValuesFormat::ALL_LIVE);
  } else if (missingCount == count) {
    meta->writeLong(Lucene54DocValuesFormat::ALL_MISSING);
  } else {
    meta->writeLong(data->getFilePointer());
    writeMissingBitset(values);
  }
  meta->writeLong(data->getFilePointer());
  meta->writeVLong(count);

  switch (format) {
  case Lucene54DocValuesFormat::CONST_COMPRESSED:
    // write the constant (nonzero value in the n=2 case, singleton value
    // otherwise)
    meta->writeLong(minValue < 0 ? Collections::min(uniqueValues)
                                 : Collections::max(uniqueValues));
    break;
  case Lucene54DocValuesFormat::GCD_COMPRESSED: {
    meta->writeLong(minValue);
    meta->writeLong(gcd);
    constexpr int64_t maxDelta = (maxValue - minValue) / gcd;
    constexpr int bits = DirectWriter::unsignedBitsRequired(maxDelta);
    meta->writeVInt(bits);
    shared_ptr<DirectWriter> *const quotientWriter =
        DirectWriter::getInstance(data, count, bits);
    for (auto nv : values) {
      int64_t value = nv == nullptr ? 0 : nv->longValue();
      quotientWriter->add((value - minValue) / gcd);
    }
    quotientWriter->finish();
    break;
  }
  case Lucene54DocValuesFormat::DELTA_COMPRESSED: {
    constexpr int64_t minDelta = delta < 0 ? 0 : minValue;
    meta->writeLong(minDelta);
    meta->writeVInt(deltaBitsRequired);
    shared_ptr<DirectWriter> *const writer =
        DirectWriter::getInstance(data, count, deltaBitsRequired);
    for (auto nv : values) {
      int64_t v = nv == nullptr ? 0 : nv->longValue();
      writer->add(v - minDelta);
    }
    writer->finish();
    break;
  }
  case Lucene54DocValuesFormat::TABLE_COMPRESSED: {
    const std::deque<optional<int64_t>> decode = uniqueValues.toArray(
        std::deque<optional<int64_t>>(uniqueValues.size()));
    Arrays::sort(decode);
    const unordered_map<int64_t, int> encode =
        unordered_map<int64_t, int>();
    meta->writeVInt(decode.size());
    for (int i = 0; i < decode.size(); i++) {
      meta->writeLong(decode[i]);
      encode.emplace(decode[i], i);
    }
    meta->writeVInt(tableBitsRequired);
    shared_ptr<DirectWriter> *const ordsWriter =
        DirectWriter::getInstance(data, count, tableBitsRequired);
    for (auto nv : values) {
      ordsWriter->add(encode[nv == nullptr ? 0 : nv->longValue()]);
    }
    ordsWriter->finish();
    break;
  }
  case Lucene54DocValuesFormat::SPARSE_COMPRESSED: {
    const deque<std::shared_ptr<Number>> filteredMissingValues;
    switch (numberType) {
    case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
        NumberType::VALUE:
      meta->writeByte(static_cast<char>(0));
      filteredMissingValues =
          make_shared<IterableAnonymousInnerClass>(shared_from_this(), values);
      break;
    case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
        NumberType::ORDINAL:
      meta->writeByte(static_cast<char>(1));
      filteredMissingValues =
          make_shared<IterableAnonymousInnerClass2>(shared_from_this(), values);
      break;
    default:
      throw make_shared<AssertionError>();
    }
    // Write non-missing values as a numeric field
    addNumericField(field, filteredMissingValues, numberType);
    break;
  }
  default:
    throw make_shared<AssertionError>();
  }
  meta->writeLong(data->getFilePointer());
}

Lucene54DocValuesConsumer::IterableAnonymousInnerClass::
    IterableAnonymousInnerClass(
        shared_ptr<Lucene54DocValuesConsumer> outerInstance,
        deque<std::shared_ptr<Number>> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
Lucene54DocValuesConsumer::IterableAnonymousInnerClass::iterator()
{
  return StreamSupport::stream(values.spliterator(), false)
      .filter([&](any value) { return value != nullptr; })
      .begin();
}

Lucene54DocValuesConsumer::IterableAnonymousInnerClass2::
    IterableAnonymousInnerClass2(
        shared_ptr<Lucene54DocValuesConsumer> outerInstance,
        deque<std::shared_ptr<Number>> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
Lucene54DocValuesConsumer::IterableAnonymousInnerClass2::iterator()
{
  return StreamSupport::stream(values.spliterator(), false)
      .filter([&](any value) { return value::longValue() != -1LL; })
      .begin();
}

template <typename T1>
void Lucene54DocValuesConsumer::writeMissingBitset(deque<T1> values) throw(
    IOException)
{
  char bits = 0;
  int count = 0;
  for (auto v : values) {
    if (count == 8) {
      data->writeByte(bits);
      count = 0;
      bits = 0;
    }
    if (v != nullptr) {
      bits |= 1 << (count & 7);
    }
    count++;
  }
  if (count > 0) {
    data->writeByte(bits);
  }
}

int64_t Lucene54DocValuesConsumer::writeSparseMissingBitset(
    deque<std::shared_ptr<Number>> &values, NumberType numberType,
    int64_t numDocsWithValue) 
{
  meta->writeVLong(numDocsWithValue);

  // Write doc IDs that have a value
  meta->writeVInt(Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT);
  shared_ptr<DirectMonotonicWriter> *const docIdsWriter =
      DirectMonotonicWriter::getInstance(
          meta, data, numDocsWithValue,
          Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT);
  int64_t docID = 0;
  for (auto nv : values) {
    switch (numberType) {
    case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
        NumberType::VALUE:
      if (nv != nullptr) {
        docIdsWriter->add(docID);
      }
      break;
    case org::apache::lucene::codecs::lucene54::Lucene54DocValuesConsumer::
        NumberType::ORDINAL:
      if (nv->longValue() != -1LL) {
        docIdsWriter->add(docID);
      }
      break;
    default:
      throw make_shared<AssertionError>();
    }
    docID++;
  }
  docIdsWriter->finish();
  return docID;
}

void Lucene54DocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  addBinaryField(field, LegacyDocValuesIterables::binaryIterable(
                            field, valuesProducer, maxDoc));
}

void Lucene54DocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<BytesRef>> &values) 
{
  // write the byte[] data
  meta->writeVInt(field->number);
  meta->writeByte(Lucene54DocValuesFormat::BINARY);
  int minLength = numeric_limits<int>::max();
  int maxLength = numeric_limits<int>::min();
  constexpr int64_t startFP = data->getFilePointer();
  int64_t count = 0;
  int64_t missingCount = 0;
  for (auto v : values) {
    constexpr int length;
    if (v == nullptr) {
      length = 0;
      missingCount++;
    } else {
      length = v->length;
    }
    minLength = min(minLength, length);
    maxLength = max(maxLength, length);
    if (v != nullptr) {
      data->writeBytes(v->bytes, v->offset, v->length);
    }
    count++;
  }
  meta->writeVInt(minLength == maxLength
                      ? Lucene54DocValuesFormat::BINARY_FIXED_UNCOMPRESSED
                      : Lucene54DocValuesFormat::BINARY_VARIABLE_UNCOMPRESSED);
  if (missingCount == 0) {
    meta->writeLong(Lucene54DocValuesFormat::ALL_LIVE);
  } else if (missingCount == count) {
    meta->writeLong(Lucene54DocValuesFormat::ALL_MISSING);
  } else {
    meta->writeLong(data->getFilePointer());
    writeMissingBitset(values);
  }
  meta->writeVInt(minLength);
  meta->writeVInt(maxLength);
  meta->writeVLong(count);
  meta->writeLong(startFP);

  // if minLength == maxLength, it's a fixed-length byte[], we are done (the
  // addresses are implicit) otherwise, we need to record the length fields...
  if (minLength != maxLength) {
    meta->writeLong(data->getFilePointer());
    meta->writeVInt(Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT);

    shared_ptr<DirectMonotonicWriter> *const writer =
        DirectMonotonicWriter::getInstance(
            meta, data, count + 1,
            Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT);
    int64_t addr = 0;
    writer->add(addr);
    for (auto v : values) {
      if (v != nullptr) {
        addr += v->length;
      }
      writer->add(addr);
    }
    writer->finish();
    meta->writeLong(data->getFilePointer());
  }
}

void Lucene54DocValuesConsumer::addTermsDict(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<BytesRef>> &values) 
{
  // first check if it's a "fixed-length" terms dict, and compressibility if so
  int minLength = numeric_limits<int>::max();
  int maxLength = numeric_limits<int>::min();
  int64_t numValues = 0;
  shared_ptr<BytesRefBuilder> previousValue = make_shared<BytesRefBuilder>();
  int64_t prefixSum =
      0; // only valid for fixed-width data, as we have a choice there
  for (auto v : values) {
    minLength = min(minLength, v->length);
    maxLength = max(maxLength, v->length);
    if (minLength == maxLength) {
      int termPosition =
          static_cast<int>(numValues & Lucene54DocValuesFormat::INTERVAL_MASK);
      if (termPosition == 0) {
        // first term in block, save it away to compare against the last term
        // later
        previousValue->copyBytes(v);
      } else if (termPosition == Lucene54DocValuesFormat::INTERVAL_COUNT - 1) {
        // last term in block, accumulate shared prefix against first term
        prefixSum +=
            LegacyStringHelper::bytesDifference(previousValue->get(), v);
      }
    }
    numValues++;
  }
  // for fixed width data, look at the avg(shared prefix) before deciding how to
  // encode: prefix compression "costs" worst case 2 bytes per term because we
  // must store suffix lengths. so if we share at least 3 bytes on average,
  // always compress.
  if (minLength == maxLength &&
      prefixSum <= 3 * (numValues >> Lucene54DocValuesFormat::INTERVAL_SHIFT)) {
    // no index needed: not very compressible, direct addressing by mult
    addBinaryField(field, values);
  } else if (numValues < Lucene54DocValuesFormat::REVERSE_INTERVAL_COUNT) {
    // low cardinality: waste a few KB of ram, but can't really use fancy index
    // etc
    addBinaryField(field, values);
  } else {
    assert(numValues > 0); // we don't have to handle the empty case
    // header
    meta->writeVInt(field->number);
    meta->writeByte(Lucene54DocValuesFormat::BINARY);
    meta->writeVInt(Lucene54DocValuesFormat::BINARY_PREFIX_COMPRESSED);
    meta->writeLong(-1LL);
    // now write the bytes: sharing prefixes within a block
    constexpr int64_t startFP = data->getFilePointer();
    // currently, we have to store the delta from expected for every 1/nth term
    // we could avoid this, but it's not much and less overall RAM than the
    // previous approach!
    shared_ptr<RAMOutputStream> addressBuffer = make_shared<RAMOutputStream>();
    shared_ptr<MonotonicBlockPackedWriter> termAddresses =
        make_shared<MonotonicBlockPackedWriter>(
            addressBuffer, Lucene54DocValuesFormat::MONOTONIC_BLOCK_SIZE);
    // buffers up 16 terms
    shared_ptr<RAMOutputStream> bytesBuffer = make_shared<RAMOutputStream>();
    // buffers up block header
    shared_ptr<RAMOutputStream> headerBuffer = make_shared<RAMOutputStream>();
    shared_ptr<BytesRefBuilder> lastTerm = make_shared<BytesRefBuilder>();
    lastTerm->grow(maxLength);
    int64_t count = 0;
    std::deque<int> suffixDeltas(Lucene54DocValuesFormat::INTERVAL_COUNT);
    for (auto v : values) {
      int termPosition =
          static_cast<int>(count & Lucene54DocValuesFormat::INTERVAL_MASK);
      if (termPosition == 0) {
        termAddresses->add(data->getFilePointer() - startFP);
        // abs-encode first term
        headerBuffer->writeVInt(v->length);
        headerBuffer->writeBytes(v->bytes, v->offset, v->length);
        lastTerm->copyBytes(v);
      } else {
        // prefix-code: we only share at most 255 characters, to encode the
        // length as a single byte and have random access. Larger terms just get
        // less compression.
        int sharedPrefix =
            min(255, LegacyStringHelper::bytesDifference(lastTerm->get(), v));
        bytesBuffer->writeByte(static_cast<char>(sharedPrefix));
        bytesBuffer->writeBytes(v->bytes, v->offset + sharedPrefix,
                                v->length - sharedPrefix);
        // we can encode one smaller, because terms are unique.
        suffixDeltas[termPosition] = v->length - sharedPrefix - 1;
      }

      count++;
      // flush block
      if ((count & Lucene54DocValuesFormat::INTERVAL_MASK) == 0) {
        flushTermsDictBlock(headerBuffer, bytesBuffer, suffixDeltas);
      }
    }
    // flush trailing crap
    int leftover =
        static_cast<int>(count & Lucene54DocValuesFormat::INTERVAL_MASK);
    if (leftover > 0) {
      Arrays::fill(suffixDeltas, leftover, suffixDeltas.size(), 0);
      flushTermsDictBlock(headerBuffer, bytesBuffer, suffixDeltas);
    }
    constexpr int64_t indexStartFP = data->getFilePointer();
    // write addresses of indexed terms
    termAddresses->finish();
    addressBuffer->writeTo(data);
    addressBuffer.reset();
    termAddresses.reset();
    meta->writeVInt(minLength);
    meta->writeVInt(maxLength);
    meta->writeVLong(count);
    meta->writeLong(startFP);
    meta->writeLong(indexStartFP);
    meta->writeVInt(PackedInts::VERSION_CURRENT);
    meta->writeVInt(Lucene54DocValuesFormat::MONOTONIC_BLOCK_SIZE);
    addReverseTermIndex(field, values, maxLength);
  }
}

void Lucene54DocValuesConsumer::flushTermsDictBlock(
    shared_ptr<RAMOutputStream> headerBuffer,
    shared_ptr<RAMOutputStream> bytesBuffer,
    std::deque<int> &suffixDeltas) 
{
  bool twoByte = false;
  for (int i = 1; i < suffixDeltas.size(); i++) {
    if (suffixDeltas[i] > 254) {
      twoByte = true;
    }
  }
  if (twoByte) {
    headerBuffer->writeByte(static_cast<char>(255));
    for (int i = 1; i < suffixDeltas.size(); i++) {
      headerBuffer->writeShort(static_cast<short>(suffixDeltas[i]));
    }
  } else {
    for (int i = 1; i < suffixDeltas.size(); i++) {
      headerBuffer->writeByte(static_cast<char>(suffixDeltas[i]));
    }
  }
  headerBuffer->writeTo(data);
  headerBuffer->reset();
  bytesBuffer->writeTo(data);
  bytesBuffer->reset();
}

void Lucene54DocValuesConsumer::addReverseTermIndex(
    shared_ptr<FieldInfo> field, deque<std::shared_ptr<BytesRef>> &values,
    int maxLength) 
{
  int64_t count = 0;
  shared_ptr<BytesRefBuilder> priorTerm = make_shared<BytesRefBuilder>();
  priorTerm->grow(maxLength);
  shared_ptr<BytesRef> indexTerm = make_shared<BytesRef>();
  int64_t startFP = data->getFilePointer();
  shared_ptr<PagedBytes> pagedBytes = make_shared<PagedBytes>(15);
  shared_ptr<MonotonicBlockPackedWriter> addresses =
      make_shared<MonotonicBlockPackedWriter>(
          data, Lucene54DocValuesFormat::MONOTONIC_BLOCK_SIZE);

  for (auto b : values) {
    int termPosition = static_cast<int>(
        count & Lucene54DocValuesFormat::REVERSE_INTERVAL_MASK);
    if (termPosition == 0) {
      int len = LegacyStringHelper::sortKeyLength(priorTerm->get(), b);
      indexTerm->bytes = b->bytes;
      indexTerm->offset = b->offset;
      indexTerm->length = len;
      addresses->add(pagedBytes->copyUsingLengthPrefix(indexTerm));
    } else if (termPosition == Lucene54DocValuesFormat::REVERSE_INTERVAL_MASK) {
      priorTerm->copyBytes(b);
    }
    count++;
  }
  addresses->finish();
  int64_t numBytes = pagedBytes->getPointer();
  pagedBytes->freeze(true);
  shared_ptr<PagedBytesDataInput> in_ = pagedBytes->getDataInput();
  meta->writeLong(startFP);
  data->writeVLong(numBytes);
  data->copyBytes(in_, numBytes);
}

void Lucene54DocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeVInt(field->number);
  meta->writeByte(Lucene54DocValuesFormat::SORTED);
  addTermsDict(field, LegacyDocValuesIterables::valuesIterable(
                          valuesProducer->getSorted(field)));
  addNumericField(field,
                  LegacyDocValuesIterables::sortedOrdIterable(valuesProducer,
                                                              field, maxDoc),
                  NumberType::ORDINAL);
}

void Lucene54DocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field, deque<std::shared_ptr<BytesRef>> &values,
    deque<std::shared_ptr<Number>> &ords) 
{
  meta->writeVInt(field->number);
  meta->writeByte(Lucene54DocValuesFormat::SORTED);
  addTermsDict(field, values);
  addNumericField(field, ords, NumberType::ORDINAL);
}

void Lucene54DocValuesConsumer::addSortedNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{

  const deque<std::shared_ptr<Number>> docToValueCount =
      LegacyDocValuesIterables::sortedNumericToDocCount(valuesProducer, field,
                                                        maxDoc);
  const deque<std::shared_ptr<Number>> values =
      LegacyDocValuesIterables::sortedNumericToValues(valuesProducer, field);

  meta->writeVInt(field->number);
  meta->writeByte(Lucene54DocValuesFormat::SORTED_NUMERIC);
  if (isSingleValued(docToValueCount)) {
    meta->writeVInt(Lucene54DocValuesFormat::SORTED_SINGLE_VALUED);
    // The field is single-valued, we can encode it as NUMERIC
    addNumericField(field, singletonView(docToValueCount, values, nullptr),
                    NumberType::VALUE);
  } else {
    shared_ptr<SortedSet<std::shared_ptr<LongsRef>>> *const uniqueValueSets =
        this->uniqueValueSets(docToValueCount, values);
    if (uniqueValueSets != nullptr) {
      meta->writeVInt(Lucene54DocValuesFormat::SORTED_SET_TABLE);

      // write the set_id -> values mapping
      writeDictionary(uniqueValueSets);

      // write the doc -> set_id as a numeric field
      addNumericField(field,
                      docToSetId(uniqueValueSets, docToValueCount, values),
                      NumberType::ORDINAL);
    } else {
      meta->writeVInt(Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES);
      // write the stream of values as a numeric field
      addNumericField(field, values, NumberType::VALUE);
      // write the doc -> ord count as a absolute index to the stream
      addOrdIndex(field, docToValueCount);
    }
  }
}

void Lucene54DocValuesConsumer::addSortedSetField(
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
  meta->writeByte(Lucene54DocValuesFormat::SORTED_SET);

  if (isSingleValued(docToOrdCount)) {
    meta->writeVInt(Lucene54DocValuesFormat::SORTED_SINGLE_VALUED);
    // The field is single-valued, we can encode it as SORTED
    addSortedField(field, values, singletonView(docToOrdCount, ords, -1LL));
  } else {
    shared_ptr<SortedSet<std::shared_ptr<LongsRef>>> *const uniqueValueSets =
        this->uniqueValueSets(docToOrdCount, ords);
    if (uniqueValueSets != nullptr) {
      meta->writeVInt(Lucene54DocValuesFormat::SORTED_SET_TABLE);

      // write the set_id -> ords mapping
      writeDictionary(uniqueValueSets);

      // write the ord -> byte[] as a binary field
      addTermsDict(field, values);

      // write the doc -> set_id as a numeric field
      addNumericField(field, docToSetId(uniqueValueSets, docToOrdCount, ords),
                      NumberType::ORDINAL);
    } else {
      meta->writeVInt(Lucene54DocValuesFormat::SORTED_WITH_ADDRESSES);

      // write the ord -> byte[] as a binary field
      addTermsDict(field, values);

      // write the stream of ords as a numeric field
      // NOTE: we could return an iterator that delta-encodes these within a doc
      addNumericField(field, ords, NumberType::ORDINAL);

      // write the doc -> ord count as a absolute index to the stream
      addOrdIndex(field, docToOrdCount);
    }
  }
}

shared_ptr<SortedSet<std::shared_ptr<LongsRef>>>
Lucene54DocValuesConsumer::uniqueValueSets(
    deque<std::shared_ptr<Number>> &docToValueCount,
    deque<std::shared_ptr<Number>> &values)
{
  shared_ptr<Set<std::shared_ptr<LongsRef>>> uniqueValueSet =
      unordered_set<std::shared_ptr<LongsRef>>();
  shared_ptr<LongsRef> docValues = make_shared<LongsRef>(256);

  deque<std::shared_ptr<Number>>::const_iterator valueCountIterator =
      docToValueCount.begin();
  deque<std::shared_ptr<Number>>::const_iterator valueIterator =
      values.begin();
  int totalDictSize = 0;
  while (valueCountIterator != docToValueCount.end()) {
    docValues->length = *valueCountIterator.intValue();
    if (docValues->length > 256) {
      return nullptr;
    }
    for (int i = 0; i < docValues->length; ++i) {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      docValues->longs[i] = valueIterator.next().longValue();
    }
    if (uniqueValueSet->contains(docValues)) {
      continue;
    }
    totalDictSize += docValues->length;
    if (totalDictSize > 256) {
      return nullptr;
    }
    uniqueValueSet->add(make_shared<LongsRef>(
        Arrays::copyOf(docValues->longs, docValues->length), 0,
        docValues->length));
    valueCountIterator++;
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assert(valueIterator.hasNext() == false);
  return set<>(uniqueValueSet);
}

void Lucene54DocValuesConsumer::writeDictionary(
    shared_ptr<SortedSet<std::shared_ptr<LongsRef>>>
        uniqueValueSets) 
{
  int lengthSum = 0;
  for (auto longs : uniqueValueSets) {
    lengthSum += longs->length;
  }

  meta->writeInt(lengthSum);
  for (auto valueSet : uniqueValueSets) {
    for (int i = 0; i < valueSet->length; ++i) {
      meta->writeLong(valueSet->longs[valueSet->offset + i]);
    }
  }

  meta->writeInt(uniqueValueSets->size());
  for (auto valueSet : uniqueValueSets) {
    meta->writeInt(valueSet->length);
  }
}

deque<std::shared_ptr<Number>> Lucene54DocValuesConsumer::docToSetId(
    shared_ptr<SortedSet<std::shared_ptr<LongsRef>>> uniqueValueSets,
    deque<std::shared_ptr<Number>> &docToValueCount,
    deque<std::shared_ptr<Number>> &values)
{
  const unordered_map<std::shared_ptr<LongsRef>, int> setIds =
      unordered_map<std::shared_ptr<LongsRef>, int>();
  int i = 0;
  for (auto set : uniqueValueSets) {
    setIds.emplace(set, i++);
  }
  assert(i == uniqueValueSets->size());

  return make_shared<IterableAnonymousInnerClass3>(
      shared_from_this(), docToValueCount, values, setIds, i);
}

Lucene54DocValuesConsumer::IterableAnonymousInnerClass3::
    IterableAnonymousInnerClass3(
        shared_ptr<Lucene54DocValuesConsumer> outerInstance,
        deque<std::shared_ptr<Number>> &docToValueCount,
        deque<std::shared_ptr<Number>> &values,
        unordered_map<std::shared_ptr<LongsRef>, int> &setIds, int i)
{
  this->outerInstance = outerInstance;
  this->docToValueCount = docToValueCount;
  this->values = values;
  this->setIds = setIds;
  this->i = i;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
Lucene54DocValuesConsumer::IterableAnonymousInnerClass3::iterator()
{
  constexpr deque<std::shared_ptr<Number>>::const_iterator valueCountIterator =
      docToValueCount.begin();
  constexpr deque<std::shared_ptr<Number>>::const_iterator valueIterator =
      values.begin();
  shared_ptr<LongsRef> *const docValues = make_shared<LongsRef>(256);
  return make_shared<IteratorAnonymousInnerClass>(
      shared_from_this(), valueCountIterator, valueIterator, docValues);
}

Lucene54DocValuesConsumer::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass3> outerInstance,
        deque<Number>::const_iterator valueCountIterator,
        deque<Number>::const_iterator valueIterator,
        shared_ptr<LongsRef> docValues)
{
  this->outerInstance = outerInstance;
  this->valueCountIterator = valueCountIterator;
  this->valueIterator = valueIterator;
  this->docValues = docValues;
}

bool Lucene54DocValuesConsumer::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return valueCountIterator.hasNext();
}

shared_ptr<Number> Lucene54DocValuesConsumer::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  docValues->length = valueCountIterator.next().intValue();
  for (int i = 0; i < docValues->length; ++i) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    docValues->longs[i] = valueIterator.next().longValue();
  }
  const optional<int> id = outerInstance->setIds[docValues];
  assert(id);
  return id;
}

void Lucene54DocValuesConsumer::addOrdIndex(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeVInt(field->number);
  meta->writeByte(Lucene54DocValuesFormat::NUMERIC);
  meta->writeVInt(Lucene54DocValuesFormat::MONOTONIC_COMPRESSED);
  meta->writeLong(-1LL);
  meta->writeLong(data->getFilePointer());
  meta->writeVLong(maxDoc);
  meta->writeVInt(Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT);

  shared_ptr<DirectMonotonicWriter> *const writer =
      DirectMonotonicWriter::getInstance(
          meta, data, maxDoc + 1,
          Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT);
  int64_t addr = 0;
  writer->add(addr);
  for (auto v : values) {
    addr += v->longValue();
    writer->add(addr);
  }
  writer->finish();
  meta->writeLong(data->getFilePointer());
}

Lucene54DocValuesConsumer::~Lucene54DocValuesConsumer()
{
  bool success = false;
  try {
    if (meta != nullptr) {
      meta->writeVInt(-1);          // write EOF marker
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
} // namespace org::apache::lucene::codecs::lucene54