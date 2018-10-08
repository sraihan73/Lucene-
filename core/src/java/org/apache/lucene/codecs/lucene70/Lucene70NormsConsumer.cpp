using namespace std;

#include "Lucene70NormsConsumer.h"

namespace org::apache::lucene::codecs::lucene70
{
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70NormsFormat.VERSION_CURRENT;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using IOUtils = org::apache::lucene::util::IOUtils;

Lucene70NormsConsumer::Lucene70NormsConsumer(
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

Lucene70NormsConsumer::~Lucene70NormsConsumer()
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

void Lucene70NormsConsumer::addNormsField(
    shared_ptr<FieldInfo> field,
    shared_ptr<NormsProducer> normsProducer) 
{
  shared_ptr<NumericDocValues> values = normsProducer->getNorms(field);
  int numDocsWithValue = 0;
  int64_t min = numeric_limits<int64_t>::max();
  int64_t max = numeric_limits<int64_t>::min();
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    numDocsWithValue++;
    int64_t v = values->longValue();
    min = min(min, v);
    max = max(max, v);
  }
  assert(numDocsWithValue <= maxDoc);

  meta->writeInt(field->number);

  if (numDocsWithValue == 0) {
    meta->writeLong(-2);
    meta->writeLong(0LL);
  } else if (numDocsWithValue == maxDoc) {
    meta->writeLong(-1);
    meta->writeLong(0LL);
  } else {
    int64_t offset = data->getFilePointer();
    meta->writeLong(offset);
    values = normsProducer->getNorms(field);
    IndexedDISI::writeBitSet(values, data);
    meta->writeLong(data->getFilePointer() - offset);
  }

  meta->writeInt(numDocsWithValue);
  int numBytesPerValue = this->numBytesPerValue(min, max);

  meta->writeByte(static_cast<char>(numBytesPerValue));
  if (numBytesPerValue == 0) {
    meta->writeLong(min);
  } else {
    meta->writeLong(data->getFilePointer());
    values = normsProducer->getNorms(field);
    writeValues(values, numBytesPerValue, data);
  }
}

int Lucene70NormsConsumer::numBytesPerValue(int64_t min, int64_t max)
{
  if (min >= max) {
    return 0;
  } else if (min >= numeric_limits<char>::min() &&
             max <= numeric_limits<char>::max()) {
    return 1;
  } else if (min >= numeric_limits<short>::min() &&
             max <= numeric_limits<short>::max()) {
    return 2;
  } else if (min >= numeric_limits<int>::min() &&
             max <= numeric_limits<int>::max()) {
    return 4;
  } else {
    return 8;
  }
}

void Lucene70NormsConsumer::writeValues(
    shared_ptr<NumericDocValues> values, int numBytesPerValue,
    shared_ptr<IndexOutput> out) 
{
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    int64_t value = values->longValue();
    switch (numBytesPerValue) {
    case 1:
      out->writeByte(static_cast<char>(value));
      break;
    case 2:
      out->writeShort(static_cast<short>(value));
      break;
    case 4:
      out->writeInt(static_cast<int>(value));
      break;
    case 8:
      out->writeLong(value);
      break;
    default:
      throw make_shared<AssertionError>();
    }
  }
}
} // namespace org::apache::lucene::codecs::lucene70