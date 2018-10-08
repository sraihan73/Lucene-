using namespace std;

#include "DirectDocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/LegacyDocValuesIterables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "DirectDocValuesFormat.h"

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
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.BYTES; import
//    static org.apache.lucene.codecs.memory.DirectDocValuesProducer.NUMBER;
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED; import
//    static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_NUMERIC;
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_NUMERIC_SINGLETON;
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_SET; import
//    static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_SET_SINGLETON;
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.VERSION_CURRENT;

DirectDocValuesConsumer::DirectDocValuesConsumer(
    shared_ptr<SegmentWriteState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : maxDoc(state->segmentInfo->maxDoc())
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

void DirectDocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeVInt(field->number);
  meta->writeByte(NUMBER);
  addNumericFieldValues(field, LegacyDocValuesIterables::numericIterable(
                                   field, valuesProducer, maxDoc));
}

void DirectDocValuesConsumer::addNumericFieldValues(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<Number>> &values) 
{
  meta->writeLong(data->getFilePointer());
  int64_t minValue = numeric_limits<int64_t>::max();
  int64_t maxValue = numeric_limits<int64_t>::min();
  bool missing = false;

  int64_t count = 0;
  for (auto nv : values) {
    if (nv != nullptr) {
      int64_t v = nv->longValue();
      minValue = min(minValue, v);
      maxValue = max(maxValue, v);
    } else {
      missing = true;
    }
    count++;
    if (count >= DirectDocValuesFormat::MAX_SORTED_SET_ORDS) {
      throw invalid_argument(
          L"DocValuesField \"" + field->name + L"\" is too large, must be <= " +
          to_wstring(DirectDocValuesFormat::MAX_SORTED_SET_ORDS) +
          L" values/total ords");
    }
  }
  meta->writeInt(static_cast<int>(count));

  if (missing) {
    int64_t start = data->getFilePointer();
    writeMissingBitset(values);
    meta->writeLong(start);
    meta->writeLong(data->getFilePointer() - start);
  } else {
    meta->writeLong(-1LL);
  }

  char byteWidth;
  if (minValue >= numeric_limits<char>::min() &&
      maxValue <= numeric_limits<char>::max()) {
    byteWidth = 1;
  } else if (minValue >= numeric_limits<short>::min() &&
             maxValue <= numeric_limits<short>::max()) {
    byteWidth = 2;
  } else if (minValue >= numeric_limits<int>::min() &&
             maxValue <= numeric_limits<int>::max()) {
    byteWidth = 4;
  } else {
    byteWidth = 8;
  }
  meta->writeByte(byteWidth);

  for (auto nv : values) {
    int64_t v;
    if (nv != nullptr) {
      v = nv->longValue();
    } else {
      v = 0;
    }

    switch (byteWidth) {
    case 1:
      data->writeByte(static_cast<char>(v));
      break;
    case 2:
      data->writeShort(static_cast<short>(v));
      break;
    case 4:
      data->writeInt(static_cast<int>(v));
      break;
    case 8:
      data->writeLong(v);
      break;
    }
  }
}

DirectDocValuesConsumer::~DirectDocValuesConsumer()
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

void DirectDocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeVInt(field->number);
  meta->writeByte(BYTES);
  addBinaryFieldValues(field, LegacyDocValuesIterables::binaryIterable(
                                  field, valuesProducer, maxDoc));
}

void DirectDocValuesConsumer::addBinaryFieldValues(
    shared_ptr<FieldInfo> field,
    deque<std::shared_ptr<BytesRef>> &values) 
{
  // write the byte[] data
  constexpr int64_t startFP = data->getFilePointer();
  bool missing = false;
  int64_t totalBytes = 0;
  int count = 0;
  for (auto v : values) {
    if (v != nullptr) {
      data->writeBytes(v->bytes, v->offset, v->length);
      totalBytes += v->length;
      if (totalBytes > DirectDocValuesFormat::MAX_TOTAL_BYTES_LENGTH) {
        throw invalid_argument(
            L"DocValuesField \"" + field->name +
            L"\" is too large, cannot have more than "
            L"DirectDocValuesFormat.MAX_TOTAL_BYTES_LENGTH (" +
            to_wstring(DirectDocValuesFormat::MAX_TOTAL_BYTES_LENGTH) +
            L") bytes");
      }
    } else {
      missing = true;
    }
    count++;
  }

  meta->writeLong(startFP);
  meta->writeInt(static_cast<int>(totalBytes));
  meta->writeInt(count);
  if (missing) {
    int64_t start = data->getFilePointer();
    writeMissingBitset(values);
    meta->writeLong(start);
    meta->writeLong(data->getFilePointer() - start);
  } else {
    meta->writeLong(-1LL);
  }

  int addr = 0;
  for (auto v : values) {
    data->writeInt(addr);
    if (v != nullptr) {
      addr += v->length;
    }
  }
  data->writeInt(addr);
}

template <typename T1>
void DirectDocValuesConsumer::writeMissingBitset(deque<T1> values) throw(
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

void DirectDocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  meta->writeVInt(field->number);
  meta->writeByte(SORTED);

  // write the ordinals as numerics
  addNumericFieldValues(field, LegacyDocValuesIterables::sortedOrdIterable(
                                   valuesProducer, field, maxDoc));
  // write the values as binary
  addBinaryFieldValues(field, LegacyDocValuesIterables::valuesIterable(
                                  valuesProducer->getSorted(field)));
}

void DirectDocValuesConsumer::addSortedNumericField(
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
    addNumericFieldValues(field,
                          singletonView(docToValueCount, values, nullptr));
  } else {
    meta->writeByte(SORTED_NUMERIC);

    // First write docToValueCounts, except we "aggregate" the
    // counts so they turn into addresses, and add a final
    // value = the total aggregate:
    addNumericFieldValues(field, countToAddressIterator(docToValueCount));

    // Write values for all docs, appended into one big
    // numerics:
    addNumericFieldValues(field, values);
  }
}

void DirectDocValuesConsumer::addSortedSetField(
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
    // Write ordinals for all docs, appended into one big
    // numerics:
    addNumericFieldValues(field, singletonView(docToOrdCount, ords, -1LL));

    // write the values as binary
    addBinaryFieldValues(field, values);
  } else {
    meta->writeByte(SORTED_SET);

    // First write docToOrdCounts, except we "aggregate" the
    // counts so they turn into addresses, and add a final
    // value = the total aggregate:
    addNumericFieldValues(field, countToAddressIterator(docToOrdCount));

    // Write ordinals for all docs, appended into one big
    // numerics:
    addNumericFieldValues(field, ords);

    // write the values as binary
    addBinaryFieldValues(field, values);
  }
}

deque<std::shared_ptr<Number>> DirectDocValuesConsumer::countToAddressIterator(
    deque<std::shared_ptr<Number>> &counts)
{
  return make_shared<IterableAnonymousInnerClass>(shared_from_this(), counts);
}

DirectDocValuesConsumer::IterableAnonymousInnerClass::
    IterableAnonymousInnerClass(
        shared_ptr<DirectDocValuesConsumer> outerInstance,
        deque<std::shared_ptr<Number>> &counts)
{
  this->outerInstance = outerInstance;
  this->counts = counts;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
DirectDocValuesConsumer::IterableAnonymousInnerClass::iterator()
{
  constexpr deque<std::shared_ptr<Number>>::const_iterator iter =
      counts.begin();

  return make_shared<IteratorAnonymousInnerClass>(shared_from_this(), iter);
}

DirectDocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass> outerInstance,
        deque<Number>::const_iterator iter)
{
  this->outerInstance = outerInstance;
  this->iter = iter;
}

bool DirectDocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return iter.hasNext() || !ended;
}

shared_ptr<Number> DirectDocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  int64_t toReturn = sum;

  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (iter.hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<Number> n = iter.next();
    if (n != nullptr) {
      sum += n->longValue();
    }
  } else if (!ended) {
    ended = true;
  } else {
    assert(false);
  }

  return toReturn;
}

void DirectDocValuesConsumer::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::memory