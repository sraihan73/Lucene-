using namespace std;

#include "SortingTermVectorsConsumer.h"

namespace org::apache::lucene::index
{
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;

SortingTermVectorsConsumer::SortingTermVectorsConsumer(
    shared_ptr<DocumentsWriterPerThread> docWriter)
    : TermVectorsConsumer(docWriter)
{
}

void SortingTermVectorsConsumer::flush(
    unordered_map<wstring, std::shared_ptr<TermsHashPerField>> &fieldsToFlush,
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  TermVectorsConsumer::flush(fieldsToFlush, state, sortMap);
  if (tmpDirectory != nullptr) {
    if (sortMap == nullptr) {
      // we're lucky the index is already sorted, just rename the temporary file
      // and return
      for (auto entry : tmpDirectory->getTemporaryFiles()) {
        tmpDirectory->rename(entry.second, entry.first);
      }
      return;
    }
    shared_ptr<TermVectorsReader> reader =
        docWriter->codec->termVectorsFormat()->vectorsReader(
            tmpDirectory, state->segmentInfo, state->fieldInfos,
            IOContext::DEFAULT);
    shared_ptr<TermVectorsReader> mergeReader = reader->getMergeInstance();
    shared_ptr<TermVectorsWriter> writer =
        docWriter->codec->termVectorsFormat()->vectorsWriter(
            state->directory, state->segmentInfo, IOContext::DEFAULT);
    try {
      reader->checkIntegrity();
      for (int docID = 0; docID < state->segmentInfo->maxDoc(); docID++) {
        shared_ptr<Fields> vectors = mergeReader->get(sortMap->newToOld(docID));
        writeTermVectors(writer, vectors, state->fieldInfos);
      }
      writer->finish(state->fieldInfos, state->segmentInfo->maxDoc());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::close({reader, writer});
      IOUtils::deleteFiles(tmpDirectory,
                           tmpDirectory->getTemporaryFiles().values());
    }
  }
}

void SortingTermVectorsConsumer::initTermVectorsWriter() 
{
  if (writer == nullptr) {
    shared_ptr<IOContext> context =
        make_shared<IOContext>(make_shared<FlushInfo>(
            docWriter->getNumDocsInRAM(), docWriter->bytesUsed()));
    tmpDirectory =
        make_shared<TrackingTmpOutputDirectoryWrapper>(docWriter->directory);
    writer = docWriter->codec->termVectorsFormat()->vectorsWriter(
        tmpDirectory, docWriter->getSegmentInfo(), context);
    lastDocID = 0;
  }
}

void SortingTermVectorsConsumer::abort()
{
  try {
    TermVectorsConsumer::abort();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::deleteFilesIgnoringExceptions(
        {tmpDirectory, tmpDirectory->getTemporaryFiles().values()});
  }
}

void SortingTermVectorsConsumer::writeTermVectors(
    shared_ptr<TermVectorsWriter> writer, shared_ptr<Fields> vectors,
    shared_ptr<FieldInfos> fieldInfos) 
{
  if (vectors->empty()) {
    writer->startDocument(0);
    writer->finishDocument();
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
  writer->startDocument(numFields);

  wstring lastFieldName = L"";

  shared_ptr<TermsEnum> termsEnum = nullptr;
  shared_ptr<PostingsEnum> docsAndPositionsEnum = nullptr;

  int fieldCount = 0;
  for (auto fieldName : vectors) {
    fieldCount++;
    shared_ptr<FieldInfo> *const fieldInfo = fieldInfos->fieldInfo(fieldName);

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

    writer->startField(fieldInfo, numTerms, hasPositions, hasOffsets,
                       hasPayloads);
    termsEnum = terms->begin();

    int termCount = 0;
    while (termsEnum->next() != nullptr) {
      termCount++;

      constexpr int freq = static_cast<int>(termsEnum->totalTermFreq());

      writer->startTerm(termsEnum->term(), freq);

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
          writer->addPosition(pos, startOffset, endOffset, payload);
        }
      }
      writer->finishTerm();
    }
    assert(termCount == numTerms);
    writer->finishField();
  }
  assert(fieldCount == numFields);
  writer->finishDocument();
}
} // namespace org::apache::lucene::index