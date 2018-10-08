using namespace std;

#include "Lucene53NormsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/LegacyDocValuesIterables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::codecs::lucene53
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using LegacyDocValuesIterables =
    org::apache::lucene::codecs::LegacyDocValuesIterables;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using IOUtils = org::apache::lucene::util::IOUtils;
//    import static
//    org.apache.lucene.codecs.lucene53.Lucene53NormsFormat.VERSION_CURRENT;

Lucene53NormsConsumer::Lucene53NormsConsumer(
    shared_ptr<SegmentWriteState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
{
  bool success = false;
  try {
    wstring dataName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, dataExtension);
    data = state->directory->createOutput(dataName, state->context);
    CodecUtil::writeIndexHeader(data, dataCodec, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    wstring metaName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, metaExtension);
    meta = state->directory->createOutput(metaName, state->context);
    CodecUtil::writeIndexHeader(meta, metaCodec, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
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

void Lucene53NormsConsumer::addNormsField(
    shared_ptr<FieldInfo> field,
    shared_ptr<NormsProducer> normsProducer) 
{
  addNormsField(field, LegacyDocValuesIterables::normsIterable(
                           field, normsProducer, maxDoc));
}

void Lucene53NormsConsumer::addNormsField(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeVInt(field->number);
  int64_t minValue = numeric_limits<int64_t>::max();
  int64_t maxValue = numeric_limits<int64_t>::min();
  int count = 0;

  for (auto nv : values) {
    if (nv == nullptr) {
      throw make_shared<IllegalStateException>(
          L"illegal norms data for field " + field->name +
          L", got null for value: " + to_wstring(count));
    }
    constexpr int64_t v = nv->longValue();
    minValue = min(minValue, v);
    maxValue = max(maxValue, v);
    count++;
  }

  if (count != maxDoc) {
    throw make_shared<IllegalStateException>(
        L"illegal norms data for field " + field->name + L", expected count=" +
        to_wstring(maxDoc) + L", got=" + to_wstring(count));
  }

  if (minValue == maxValue) {
    addConstant(minValue);
  } else if (minValue >= numeric_limits<char>::min() &&
             maxValue <= numeric_limits<char>::max()) {
    addByte1(values);
  } else if (minValue >= numeric_limits<short>::min() &&
             maxValue <= numeric_limits<short>::max()) {
    addByte2(values);
  } else if (minValue >= numeric_limits<int>::min() &&
             maxValue <= numeric_limits<int>::max()) {
    addByte4(values);
  } else {
    addByte8(values);
  }
}

void Lucene53NormsConsumer::addConstant(int64_t constant) 
{
  meta->writeByte(static_cast<char>(0));
  meta->writeLong(constant);
}

void Lucene53NormsConsumer::addByte1(
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeByte(static_cast<char>(1));
  meta->writeLong(data->getFilePointer());

  for (auto value : values) {
    data->writeByte(value->byteValue());
  }
}

void Lucene53NormsConsumer::addByte2(
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeByte(static_cast<char>(2));
  meta->writeLong(data->getFilePointer());

  for (auto value : values) {
    data->writeShort(value->shortValue());
  }
}

void Lucene53NormsConsumer::addByte4(
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeByte(static_cast<char>(4));
  meta->writeLong(data->getFilePointer());

  for (auto value : values) {
    data->writeInt(value->intValue());
  }
}

void Lucene53NormsConsumer::addByte8(
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeByte(static_cast<char>(8));
  meta->writeLong(data->getFilePointer());

  for (auto value : values) {
    data->writeLong(value->longValue());
  }
}

Lucene53NormsConsumer::~Lucene53NormsConsumer()
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
} // namespace org::apache::lucene::codecs::lucene53