using namespace std;

#include "PushPostingsWriterBase.h"
#include "../index/FieldInfo.h"
#include "../index/PostingsEnum.h"
#include "../index/TermsEnum.h"
#include "../util/BytesRef.h"
#include "../util/FixedBitSet.h"
#include "BlockTermState.h"

namespace org::apache::lucene::codecs
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

PushPostingsWriterBase::PushPostingsWriterBase() {}

int PushPostingsWriterBase::setField(shared_ptr<FieldInfo> fieldInfo)
{
  this->fieldInfo = fieldInfo;
  indexOptions = fieldInfo->getIndexOptions();

  writeFreqs = indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS) >= 0;
  writePositions =
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  writeOffsets =
      indexOptions.compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
  writePayloads = fieldInfo->hasPayloads();

  if (writeFreqs == false) {
    enumFlags = 0;
  } else if (writePositions == false) {
    enumFlags = PostingsEnum::FREQS;
  } else if (writeOffsets == false) {
    if (writePayloads) {
      enumFlags = PostingsEnum::PAYLOADS;
    } else {
      enumFlags = PostingsEnum::POSITIONS;
    }
  } else {
    if (writePayloads) {
      enumFlags = PostingsEnum::PAYLOADS | PostingsEnum::OFFSETS;
    } else {
      enumFlags = PostingsEnum::OFFSETS;
    }
  }

  return 0;
}

shared_ptr<BlockTermState> PushPostingsWriterBase::writeTerm(
    shared_ptr<BytesRef> term, shared_ptr<TermsEnum> termsEnum,
    shared_ptr<FixedBitSet> docsSeen) 
{
  startTerm();
  postingsEnum = termsEnum->postings(postingsEnum, enumFlags);
  assert(postingsEnum != nullptr);

  int docFreq = 0;
  int64_t totalTermFreq = 0;
  while (true) {
    int docID = postingsEnum->nextDoc();
    if (docID == PostingsEnum::NO_MORE_DOCS) {
      break;
    }
    docFreq++;
    docsSeen->set(docID);
    int freq;
    if (writeFreqs) {
      freq = postingsEnum->freq();
      totalTermFreq += freq;
    } else {
      freq = -1;
    }
    startDoc(docID, freq);

    if (writePositions) {
      for (int i = 0; i < freq; i++) {
        int pos = postingsEnum->nextPosition();
        shared_ptr<BytesRef> payload =
            writePayloads ? postingsEnum->getPayload() : nullptr;
        int startOffset;
        int endOffset;
        if (writeOffsets) {
          startOffset = postingsEnum->startOffset();
          endOffset = postingsEnum->endOffset();
        } else {
          startOffset = -1;
          endOffset = -1;
        }
        addPosition(pos, payload, startOffset, endOffset);
      }
    }

    finishDoc();
  }

  if (docFreq == 0) {
    return nullptr;
  } else {
    shared_ptr<BlockTermState> state = newTermState();
    state->docFreq = docFreq;
    state->totalTermFreq = writeFreqs ? totalTermFreq : -1;
    finishTerm(state);
    return state;
  }
}
} // namespace org::apache::lucene::codecs