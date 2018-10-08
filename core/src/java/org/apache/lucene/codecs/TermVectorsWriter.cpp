using namespace std;

#include "TermVectorsWriter.h"
#include "../index/FieldInfo.h"
#include "../index/FieldInfos.h"
#include "../index/Fields.h"
#include "../index/MergeState.h"
#include "../index/PostingsEnum.h"
#include "../index/Terms.h"
#include "../index/TermsEnum.h"
#include "../search/DocIdSetIterator.h"
#include "../store/DataInput.h"
#include "../util/BytesRef.h"
#include "../util/BytesRefBuilder.h"
#include "TermVectorsReader.h"

namespace org::apache::lucene::codecs
{
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using MergeState = org::apache::lucene::index::MergeState;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DataInput = org::apache::lucene::store::DataInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

TermVectorsWriter::TermVectorsWriter() {}

void TermVectorsWriter::finishDocument() {};

void TermVectorsWriter::finishField() {};

void TermVectorsWriter::finishTerm()  {}

void TermVectorsWriter::addProx(
    int numProx, shared_ptr<DataInput> positions,
    shared_ptr<DataInput> offsets) 
{
  int position = 0;
  int lastOffset = 0;
  shared_ptr<BytesRefBuilder> payload = nullptr;

  for (int i = 0; i < numProx; i++) {
    constexpr int startOffset;
    constexpr int endOffset;
    shared_ptr<BytesRef> *const thisPayload;

    if (positions == nullptr) {
      position = -1;
      thisPayload.reset();
    } else {
      int code = positions->readVInt();
      position += static_cast<int>(static_cast<unsigned int>(code) >> 1);
      if ((code & 1) != 0) {
        // This position has a payload
        constexpr int payloadLength = positions->readVInt();

        if (payload == nullptr) {
          payload = make_shared<BytesRefBuilder>();
        }
        payload->grow(payloadLength);

        positions->readBytes(payload->bytes(), 0, payloadLength);
        payload->setLength(payloadLength);
        thisPayload = payload->get();
      } else {
        thisPayload.reset();
      }
    }

    if (offsets == nullptr) {
      startOffset = endOffset = -1;
    } else {
      startOffset = lastOffset + offsets->readVInt();
      endOffset = startOffset + offsets->readVInt();
      lastOffset = endOffset;
    }
    addPosition(position, startOffset, endOffset, thisPayload);
  }
}

TermVectorsWriter::TermVectorsMergeSub::TermVectorsMergeSub(
    shared_ptr<MergeState::DocMap> docMap, shared_ptr<TermVectorsReader> reader,
    int maxDoc)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), reader(reader),
      maxDoc(maxDoc)
{
}

int TermVectorsWriter::TermVectorsMergeSub::nextDoc()
{
  docID++;
  if (docID == maxDoc) {
    return DocIdSetIterator::NO_MORE_DOCS;
  } else {
    return docID;
  }
}

int TermVectorsWriter::merge(shared_ptr<MergeState> mergeState) throw(
    IOException)
{

  deque<std::shared_ptr<TermVectorsMergeSub>> subs =
      deque<std::shared_ptr<TermVectorsMergeSub>>();
  for (int i = 0; i < mergeState->termVectorsReaders.size(); i++) {
    shared_ptr<TermVectorsReader> reader = mergeState->termVectorsReaders[i];
    if (reader != nullptr) {
      reader->checkIntegrity();
    }
    subs.push_back(make_shared<TermVectorsMergeSub>(
        mergeState->docMaps[i], reader, mergeState->maxDocs[i]));
  }

  shared_ptr<DocIDMerger<std::shared_ptr<TermVectorsMergeSub>>>
      *const docIDMerger = DocIDMerger::of(subs, mergeState->needsIndexSort);

  int docCount = 0;
  while (true) {
    shared_ptr<TermVectorsMergeSub> sub = docIDMerger->next();
    if (sub == nullptr) {
      break;
    }

    // NOTE: it's very important to first assign to vectors then pass it to
    // termVectorsWriter.addAllDocVectors; see LUCENE-1282
    shared_ptr<Fields> vectors;
    if (sub->reader == nullptr) {
      vectors.reset();
    } else {
      vectors = sub->reader->get(sub->docID);
    }
    addAllDocVectors(vectors, mergeState);
    docCount++;
  }
  finish(mergeState->mergeFieldInfos, docCount);
  return docCount;
}

void TermVectorsWriter::addAllDocVectors(
    shared_ptr<Fields> vectors,
    shared_ptr<MergeState> mergeState) 
{
  if (vectors->empty()) {
    startDocument(0);
    finishDocument();
    return;
  }

  int numFields = vectors->size();
  if (numFields == -1) {
    // count manually! TODO: Maybe enforce that Fields.size() returns something
    // valid?
    numFields = 0;
    for (shared_ptr<final Iterator<wstring>> it = vectors->begin();
         it->hasNext();) {
      it->next();
      numFields++;
    }
  }
  startDocument(numFields);

  wstring lastFieldName = L"";

  shared_ptr<TermsEnum> termsEnum = nullptr;
  shared_ptr<PostingsEnum> docsAndPositionsEnum = nullptr;

  int fieldCount = 0;
  for (auto fieldName : vectors) {
    fieldCount++;
    shared_ptr<FieldInfo> *const fieldInfo =
        mergeState->mergeFieldInfos->fieldInfo(fieldName);

    assert((lastFieldName == L"" || fieldName.compareTo(lastFieldName) > 0,
            L"lastFieldName=" + lastFieldName + L" fieldName=" + fieldName));
    lastFieldName = fieldName;

    shared_ptr<Terms> *const terms = vectors->terms(fieldName);
    if (terms == nullptr) {
      // FieldsEnum shouldn't lie...
      continue;
    }

    constexpr bool hasPositions = terms->hasPositions();
    constexpr bool hasOffsets = terms->hasOffsets();
    constexpr bool hasPayloads = terms->hasPayloads();
    assert(!hasPayloads || hasPositions);

    int numTerms = static_cast<int>(terms->size());
    if (numTerms == -1) {
      // count manually. It is stupid, but needed, as Terms.size() is not a
      // mandatory statistics function
      numTerms = 0;
      termsEnum = terms->begin();
      while (termsEnum->next() != nullptr) {
        numTerms++;
      }
    }

    startField(fieldInfo, numTerms, hasPositions, hasOffsets, hasPayloads);
    termsEnum = terms->begin();

    int termCount = 0;
    while (termsEnum->next() != nullptr) {
      termCount++;

      constexpr int freq = static_cast<int>(termsEnum->totalTermFreq());

      startTerm(termsEnum->term(), freq);

      if (hasPositions || hasOffsets) {
        docsAndPositionsEnum =
            termsEnum->postings(docsAndPositionsEnum,
                                PostingsEnum::OFFSETS | PostingsEnum::PAYLOADS);
        assert(docsAndPositionsEnum != nullptr);

        constexpr int docID = docsAndPositionsEnum->nextDoc();
        assert(docID != DocIdSetIterator::NO_MORE_DOCS);
        assert(docsAndPositionsEnum->freq() == freq);

        for (int posUpto = 0; posUpto < freq; posUpto++) {
          constexpr int pos = docsAndPositionsEnum->nextPosition();
          constexpr int startOffset = docsAndPositionsEnum->startOffset();
          constexpr int endOffset = docsAndPositionsEnum->endOffset();

          shared_ptr<BytesRef> *const payload =
              docsAndPositionsEnum->getPayload();

          assert(!hasPositions || pos >= 0);
          addPosition(pos, startOffset, endOffset, payload);
        }
      }
      finishTerm();
    }
    assert(termCount == numTerms);
    finishField();
  }
  assert(fieldCount == numFields);
  finishDocument();
}
} // namespace org::apache::lucene::codecs