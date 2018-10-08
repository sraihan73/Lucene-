using namespace std;

#include "TermVectorsConsumerPerField.h"

namespace org::apache::lucene::index
{
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using BytesRef = org::apache::lucene::util::BytesRef;

TermVectorsConsumerPerField::TermVectorsConsumerPerField(
    shared_ptr<FieldInvertState> invertState,
    shared_ptr<TermVectorsConsumer> termsWriter,
    shared_ptr<FieldInfo> fieldInfo)
    : TermsHashPerField(2, invertState, termsWriter, nullptr, fieldInfo),
      termsWriter(termsWriter)
{
}

void TermVectorsConsumerPerField::finish()
{
  if (!doVectors || bytesHash->size() == 0) {
    return;
  }
  termsWriter->addFieldToFlush(shared_from_this());
}

void TermVectorsConsumerPerField::finishDocument() 
{
  if (doVectors == false) {
    return;
  }

  doVectors = false;

  constexpr int numPostings = bytesHash->size();

  shared_ptr<BytesRef> *const flushTerm = termsWriter->flushTerm;

  assert(numPostings >= 0);

  // This is called once, after inverting all occurrences
  // of a given field in the doc.  At this point we flush
  // our hash into the DocWriter.

  shared_ptr<TermVectorsPostingsArray> postings = termVectorsPostingsArray;
  shared_ptr<TermVectorsWriter> *const tv = termsWriter->writer;

  const std::deque<int> termIDs = sortPostings();

  tv->startField(fieldInfo, numPostings, doVectorPositions, doVectorOffsets,
                 hasPayloads);

  shared_ptr<ByteSliceReader> *const posReader =
      doVectorPositions ? termsWriter->vectorSliceReaderPos : nullptr;
  shared_ptr<ByteSliceReader> *const offReader =
      doVectorOffsets ? termsWriter->vectorSliceReaderOff : nullptr;

  for (int j = 0; j < numPostings; j++) {
    constexpr int termID = termIDs[j];
    constexpr int freq = postings->freqs[termID];

    // Get BytesRef
    termBytePool->setBytesRef(flushTerm, postings->textStarts[termID]);
    tv->startTerm(flushTerm, freq);

    if (doVectorPositions || doVectorOffsets) {
      if (posReader != nullptr) {
        initReader(posReader, termID, 0);
      }
      if (offReader != nullptr) {
        initReader(offReader, termID, 1);
      }
      tv->addProx(freq, posReader, offReader);
    }
    tv->finishTerm();
  }
  tv->finishField();

  reset();

  fieldInfo->setStoreTermVectors();
}

bool TermVectorsConsumerPerField::start(shared_ptr<IndexableField> field,
                                        bool first)
{
  TermsHashPerField::start(field, first);
  assert(field->fieldType()->indexOptions() != IndexOptions::NONE);

  if (first) {

    if (bytesHash->size() != 0) {
      // Only necessary if previous doc hit a
      // non-aborting exception while writing vectors in
      // this field:
      reset();
    }

    bytesHash->reinit();

    hasPayloads = false;

    doVectors = field->fieldType()->storeTermVectors();

    if (doVectors) {

      termsWriter->hasVectors = true;

      doVectorPositions = field->fieldType()->storeTermVectorPositions();

      // Somewhat confusingly, unlike postings, you are
      // allowed to index TV offsets without TV positions:
      doVectorOffsets = field->fieldType()->storeTermVectorOffsets();

      if (doVectorPositions) {
        doVectorPayloads = field->fieldType()->storeTermVectorPayloads();
      } else {
        doVectorPayloads = false;
        if (field->fieldType()->storeTermVectorPayloads()) {
          // TODO: move this check somewhere else, and impl the other missing
          // ones
          throw invalid_argument(L"cannot index term deque payloads without "
                                 L"term deque positions (field=\"" +
                                 field->name() + L"\")");
        }
      }

    } else {
      if (field->fieldType()->storeTermVectorOffsets()) {
        throw invalid_argument(L"cannot index term deque offsets when term "
                               L"vectors are not indexed (field=\"" +
                               field->name() + L"\")");
      }
      if (field->fieldType()->storeTermVectorPositions()) {
        throw invalid_argument(L"cannot index term deque positions when term "
                               L"vectors are not indexed (field=\"" +
                               field->name() + L"\")");
      }
      if (field->fieldType()->storeTermVectorPayloads()) {
        throw invalid_argument(L"cannot index term deque payloads when term "
                               L"vectors are not indexed (field=\"" +
                               field->name() + L"\")");
      }
    }
  } else {
    if (doVectors != field->fieldType()->storeTermVectors()) {
      throw invalid_argument(
          L"all instances of a given field name must have the same term "
          L"vectors settings (storeTermVectors changed for field=\"" +
          field->name() + L"\")");
    }
    if (doVectorPositions != field->fieldType()->storeTermVectorPositions()) {
      throw invalid_argument(
          L"all instances of a given field name must have the same term "
          L"vectors settings (storeTermVectorPositions changed for field=\"" +
          field->name() + L"\")");
    }
    if (doVectorOffsets != field->fieldType()->storeTermVectorOffsets()) {
      throw invalid_argument(
          L"all instances of a given field name must have the same term "
          L"vectors settings (storeTermVectorOffsets changed for field=\"" +
          field->name() + L"\")");
    }
    if (doVectorPayloads != field->fieldType()->storeTermVectorPayloads()) {
      throw invalid_argument(
          L"all instances of a given field name must have the same term "
          L"vectors settings (storeTermVectorPayloads changed for field=\"" +
          field->name() + L"\")");
    }
  }

  if (doVectors) {
    if (doVectorOffsets) {
      offsetAttribute = fieldState->offsetAttribute;
      assert(offsetAttribute != nullptr);
    }

    if (doVectorPayloads) {
      // Can be null:
      payloadAttribute = fieldState->payloadAttribute;
    } else {
      payloadAttribute.reset();
    }
  }

  return doVectors;
}

void TermVectorsConsumerPerField::writeProx(
    shared_ptr<TermVectorsPostingsArray> postings, int termID)
{
  if (doVectorOffsets) {
    int startOffset = fieldState->offset + offsetAttribute->startOffset();
    int endOffset = fieldState->offset + offsetAttribute->endOffset();

    writeVInt(1, startOffset - postings->lastOffsets[termID]);
    writeVInt(1, endOffset - startOffset);
    postings->lastOffsets[termID] = endOffset;
  }

  if (doVectorPositions) {
    shared_ptr<BytesRef> *const payload;
    if (payloadAttribute == nullptr) {
      payload.reset();
    } else {
      payload = payloadAttribute->getPayload();
    }

    constexpr int pos = fieldState->position - postings->lastPositions[termID];
    if (payload != nullptr && payload->length > 0) {
      writeVInt(0, (pos << 1) | 1);
      writeVInt(0, payload->length);
      writeBytes(0, payload->bytes, payload->offset, payload->length);
      hasPayloads = true;
    } else {
      writeVInt(0, pos << 1);
    }
    postings->lastPositions[termID] = fieldState->position;
  }
}

void TermVectorsConsumerPerField::newTerm(int const termID)
{
  shared_ptr<TermVectorsPostingsArray> postings = termVectorsPostingsArray;

  postings->freqs[termID] = getTermFreq();
  postings->lastOffsets[termID] = 0;
  postings->lastPositions[termID] = 0;

  writeProx(postings, termID);
}

void TermVectorsConsumerPerField::addTerm(int const termID)
{
  shared_ptr<TermVectorsPostingsArray> postings = termVectorsPostingsArray;

  postings->freqs[termID] += getTermFreq();

  writeProx(postings, termID);
}

int TermVectorsConsumerPerField::getTermFreq()
{
  int freq = termFreqAtt->getTermFrequency();
  if (freq != 1) {
    if (doVectorPositions) {
      throw invalid_argument(L"field \"" + fieldInfo->name +
                             L"\": cannot index term deque positions while "
                             L"using custom TermFrequencyAttribute");
    }
    if (doVectorOffsets) {
      throw invalid_argument(L"field \"" + fieldInfo->name +
                             L"\": cannot index term deque offsets while "
                             L"using custom TermFrequencyAttribute");
    }
  }

  return freq;
}

void TermVectorsConsumerPerField::newPostingsArray()
{
  termVectorsPostingsArray =
      std::static_pointer_cast<TermVectorsPostingsArray>(postingsArray);
}

shared_ptr<ParallelPostingsArray>
TermVectorsConsumerPerField::createPostingsArray(int size)
{
  return make_shared<TermVectorsPostingsArray>(size);
}

TermVectorsConsumerPerField::TermVectorsPostingsArray::TermVectorsPostingsArray(
    int size)
    : ParallelPostingsArray(size)
{
  freqs = std::deque<int>(size);
  lastOffsets = std::deque<int>(size);
  lastPositions = std::deque<int>(size);
}

shared_ptr<ParallelPostingsArray>
TermVectorsConsumerPerField::TermVectorsPostingsArray::newInstance(int size)
{
  return make_shared<TermVectorsPostingsArray>(size);
}

void TermVectorsConsumerPerField::TermVectorsPostingsArray::copyTo(
    shared_ptr<ParallelPostingsArray> toArray, int numToCopy)
{
  assert(std::dynamic_pointer_cast<TermVectorsPostingsArray>(toArray) !=
         nullptr);
  shared_ptr<TermVectorsPostingsArray> to =
      std::static_pointer_cast<TermVectorsPostingsArray>(toArray);

  ParallelPostingsArray::copyTo(toArray, numToCopy);

  System::arraycopy(freqs, 0, to->freqs, 0, size);
  System::arraycopy(lastOffsets, 0, to->lastOffsets, 0, size);
  System::arraycopy(lastPositions, 0, to->lastPositions, 0, size);
}

int TermVectorsConsumerPerField::TermVectorsPostingsArray::bytesPerPosting()
{
  return ParallelPostingsArray::bytesPerPosting() + 3 * Integer::BYTES;
}
} // namespace org::apache::lucene::index