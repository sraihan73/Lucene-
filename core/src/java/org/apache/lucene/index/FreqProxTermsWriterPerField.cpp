using namespace std;

#include "FreqProxTermsWriterPerField.h"

namespace org::apache::lucene::index
{
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

FreqProxTermsWriterPerField::FreqProxTermsWriterPerField(
    shared_ptr<FieldInvertState> invertState, shared_ptr<TermsHash> termsHash,
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<TermsHashPerField> nextPerField)
    : TermsHashPerField(fieldInfo->getIndexOptions().compareTo(
                            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0
                            ? 2
                            : 1,
                        invertState, termsHash, nextPerField, fieldInfo),
      hasFreq(indexOptions->compareTo(IndexOptions::DOCS_AND_FREQS) >= 0),
      hasProx(indexOptions->compareTo(
                  IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0),
      hasOffsets(indexOptions->compareTo(
                     IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >=
                 0)
{
  IndexOptions indexOptions = fieldInfo->getIndexOptions();
  assert(indexOptions != IndexOptions::NONE);
}

void FreqProxTermsWriterPerField::finish() 
{
  TermsHashPerField::finish();
  sumDocFreq += fieldState->uniqueTermCount;
  sumTotalTermFreq += fieldState->length;
  if (fieldState->length > 0) {
    docCount++;
  }

  if (sawPayloads) {
    fieldInfo->setStorePayloads();
  }
}

bool FreqProxTermsWriterPerField::start(shared_ptr<IndexableField> f,
                                        bool first)
{
  TermsHashPerField::start(f, first);
  payloadAttribute = fieldState->payloadAttribute;
  offsetAttribute = fieldState->offsetAttribute;
  return true;
}

void FreqProxTermsWriterPerField::writeProx(int termID, int proxCode)
{
  if (payloadAttribute == nullptr) {
    writeVInt(1, proxCode << 1);
  } else {
    shared_ptr<BytesRef> payload = payloadAttribute->getPayload();
    if (payload != nullptr && payload->length > 0) {
      writeVInt(1, (proxCode << 1) | 1);
      writeVInt(1, payload->length);
      writeBytes(1, payload->bytes, payload->offset, payload->length);
      sawPayloads = true;
    } else {
      writeVInt(1, proxCode << 1);
    }
  }

  assert(postingsArray == freqProxPostingsArray);
  freqProxPostingsArray->lastPositions[termID] = fieldState->position;
}

void FreqProxTermsWriterPerField::writeOffsets(int termID, int offsetAccum)
{
  constexpr int startOffset = offsetAccum + offsetAttribute->startOffset();
  constexpr int endOffset = offsetAccum + offsetAttribute->endOffset();
  assert(startOffset - freqProxPostingsArray->lastOffsets[termID] >= 0);
  writeVInt(1, startOffset - freqProxPostingsArray->lastOffsets[termID]);
  writeVInt(1, endOffset - startOffset);
  freqProxPostingsArray->lastOffsets[termID] = startOffset;
}

void FreqProxTermsWriterPerField::newTerm(int const termID)
{
  // First time we're seeing this term since the last
  // flush
  shared_ptr<FreqProxPostingsArray> *const postings = freqProxPostingsArray;

  postings->lastDocIDs[termID] = docState->docID;
  if (!hasFreq) {
    assert(postings->termFreqs.empty());
    postings->lastDocCodes[termID] = docState->docID;
    fieldState->maxTermFrequency = max(1, fieldState->maxTermFrequency);
  } else {
    postings->lastDocCodes[termID] = docState->docID << 1;
    postings->termFreqs[termID] = getTermFreq();
    if (hasProx) {
      writeProx(termID, fieldState->position);
      if (hasOffsets) {
        writeOffsets(termID, fieldState->offset);
      }
    } else {
      assert(!hasOffsets);
    }
    fieldState->maxTermFrequency =
        max(postings->termFreqs[termID], fieldState->maxTermFrequency);
  }
  fieldState->uniqueTermCount++;
}

void FreqProxTermsWriterPerField::addTerm(int const termID)
{
  shared_ptr<FreqProxPostingsArray> *const postings = freqProxPostingsArray;
  assert(!hasFreq || postings->termFreqs[termID] > 0);

  if (!hasFreq) {
    assert(postings->termFreqs.empty());
    if (termFreqAtt->getTermFrequency() != 1) {
      throw make_shared<IllegalStateException>(
          L"field \"" + fieldInfo->name +
          L"\": must index term freq while using custom "
          L"TermFrequencyAttribute");
    }
    if (docState->docID != postings->lastDocIDs[termID]) {
      // New document; now encode docCode for previous doc:
      assert(docState->docID > postings->lastDocIDs[termID]);
      writeVInt(0, postings->lastDocCodes[termID]);
      postings->lastDocCodes[termID] =
          docState->docID - postings->lastDocIDs[termID];
      postings->lastDocIDs[termID] = docState->docID;
      fieldState->uniqueTermCount++;
    }
  } else if (docState->docID != postings->lastDocIDs[termID]) {
    assert((docState->docID > postings->lastDocIDs[termID],
            L"id: " + to_wstring(docState->docID) + L" postings ID: " +
                to_wstring(postings->lastDocIDs[termID]) + L" termID: " +
                to_wstring(termID)));
    // Term not yet seen in the current doc but previously
    // seen in other doc(s) since the last flush

    // Now that we know doc freq for previous doc,
    // write it & lastDocCode
    if (1 == postings->termFreqs[termID]) {
      writeVInt(0, postings->lastDocCodes[termID] | 1);
    } else {
      writeVInt(0, postings->lastDocCodes[termID]);
      writeVInt(0, postings->termFreqs[termID]);
    }

    // Init freq for the current document
    postings->termFreqs[termID] = getTermFreq();
    fieldState->maxTermFrequency =
        max(postings->termFreqs[termID], fieldState->maxTermFrequency);
    postings->lastDocCodes[termID] =
        (docState->docID - postings->lastDocIDs[termID]) << 1;
    postings->lastDocIDs[termID] = docState->docID;
    if (hasProx) {
      writeProx(termID, fieldState->position);
      if (hasOffsets) {
        postings->lastOffsets[termID] = 0;
        writeOffsets(termID, fieldState->offset);
      }
    } else {
      assert(!hasOffsets);
    }
    fieldState->uniqueTermCount++;
  } else {
    postings->termFreqs[termID] =
        Math::addExact(postings->termFreqs[termID], getTermFreq());
    fieldState->maxTermFrequency =
        max(fieldState->maxTermFrequency, postings->termFreqs[termID]);
    if (hasProx) {
      writeProx(termID, fieldState->position - postings->lastPositions[termID]);
      if (hasOffsets) {
        writeOffsets(termID, fieldState->offset);
      }
    }
  }
}

int FreqProxTermsWriterPerField::getTermFreq()
{
  int freq = termFreqAtt->getTermFrequency();
  if (freq != 1) {
    if (hasProx) {
      throw make_shared<IllegalStateException>(
          L"field \"" + fieldInfo->name +
          L"\": cannot index positions while using custom "
          L"TermFrequencyAttribute");
    }
  }

  return freq;
}

void FreqProxTermsWriterPerField::newPostingsArray()
{
  freqProxPostingsArray =
      std::static_pointer_cast<FreqProxPostingsArray>(postingsArray);
}

shared_ptr<ParallelPostingsArray>
FreqProxTermsWriterPerField::createPostingsArray(int size)
{
  IndexOptions indexOptions = fieldInfo->getIndexOptions();
  assert(indexOptions != IndexOptions::NONE);
  bool hasFreq = indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS) >= 0;
  bool hasProx =
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  bool hasOffsets =
      indexOptions.compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
  return make_shared<FreqProxPostingsArray>(size, hasFreq, hasProx, hasOffsets);
}

FreqProxTermsWriterPerField::FreqProxPostingsArray::FreqProxPostingsArray(
    int size, bool writeFreqs, bool writeProx, bool writeOffsets)
    : ParallelPostingsArray(size)
{
  if (writeFreqs) {
    termFreqs = std::deque<int>(size);
  }
  lastDocIDs = std::deque<int>(size);
  lastDocCodes = std::deque<int>(size);
  if (writeProx) {
    lastPositions = std::deque<int>(size);
    if (writeOffsets) {
      lastOffsets = std::deque<int>(size);
    }
  } else {
    assert(!writeOffsets);
  }
  // System.out.println("PA init freqs=" + writeFreqs + " pos=" + writeProx + "
  // offs=" + writeOffsets);
}

shared_ptr<ParallelPostingsArray>
FreqProxTermsWriterPerField::FreqProxPostingsArray::newInstance(int size)
{
  return make_shared<FreqProxPostingsArray>(size, termFreqs.size() > 0,
                                            lastPositions.size() > 0,
                                            lastOffsets.size() > 0);
}

void FreqProxTermsWriterPerField::FreqProxPostingsArray::copyTo(
    shared_ptr<ParallelPostingsArray> toArray, int numToCopy)
{
  assert(std::dynamic_pointer_cast<FreqProxPostingsArray>(toArray) != nullptr);
  shared_ptr<FreqProxPostingsArray> to =
      std::static_pointer_cast<FreqProxPostingsArray>(toArray);

  ParallelPostingsArray::copyTo(toArray, numToCopy);

  System::arraycopy(lastDocIDs, 0, to->lastDocIDs, 0, numToCopy);
  System::arraycopy(lastDocCodes, 0, to->lastDocCodes, 0, numToCopy);
  if (lastPositions.size() > 0) {
    assert(to->lastPositions.size() > 0);
    System::arraycopy(lastPositions, 0, to->lastPositions, 0, numToCopy);
  }
  if (lastOffsets.size() > 0) {
    assert(to->lastOffsets.size() > 0);
    System::arraycopy(lastOffsets, 0, to->lastOffsets, 0, numToCopy);
  }
  if (termFreqs.size() > 0) {
    assert(to->termFreqs.size() > 0);
    System::arraycopy(termFreqs, 0, to->termFreqs, 0, numToCopy);
  }
}

int FreqProxTermsWriterPerField::FreqProxPostingsArray::bytesPerPosting()
{
  int bytes = ParallelPostingsArray::BYTES_PER_POSTING + 2 * Integer::BYTES;
  if (lastPositions.size() > 0) {
    bytes += Integer::BYTES;
  }
  if (lastOffsets.size() > 0) {
    bytes += Integer::BYTES;
  }
  if (termFreqs.size() > 0) {
    bytes += Integer::BYTES;
  }

  return bytes;
}
} // namespace org::apache::lucene::index