using namespace std;

#include "CompletionFieldsConsumer.h"

namespace org::apache::lucene::search::suggest::document
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.CODEC_NAME;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.COMPLETION_VERSION_CURRENT;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.DICT_EXTENSION;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.INDEX_EXTENSION;

CompletionFieldsConsumer::CompletionFieldsConsumer(
    shared_ptr<PostingsFormat> delegatePostingsFormat,
    shared_ptr<SegmentWriteState> state) 
    : delegatePostingsFormatName(delegatePostingsFormat->getName()),
      state(state)
{
  wstring dictFile = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, DICT_EXTENSION);
  bool success = false;
  try {
    this->delegateFieldsConsumer =
        delegatePostingsFormat->fieldsConsumer(state);
    dictOut = state->directory->createOutput(dictFile, state->context);
    CodecUtil::writeIndexHeader(dictOut, CODEC_NAME, COMPLETION_VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      IOUtils::closeWhileHandlingException({dictOut, delegateFieldsConsumer});
    }
  }
}

void CompletionFieldsConsumer::write(shared_ptr<Fields> fields) throw(
    IOException)
{
  delegateFieldsConsumer->write(fields);

  for (auto field : fields) {
    shared_ptr<CompletionTermWriter> termWriter =
        make_shared<CompletionTermWriter>();
    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      // this can happen from ghost fields, where the incoming Fields iterator
      // claims a field exists but it does not
      continue;
    }
    shared_ptr<TermsEnum> termsEnum = terms->begin();

    // write terms
    shared_ptr<BytesRef> term;
    while ((term = termsEnum->next()) != nullptr) {
      termWriter->write(term, termsEnum);
    }

    // store lookup, if needed
    int64_t filePointer = dictOut->getFilePointer();
    if (termWriter->finish(dictOut)) {
      seenFields.emplace(field, make_shared<CompletionMetaData>(
                                    filePointer, termWriter->minWeight,
                                    termWriter->maxWeight, termWriter->type));
    }
  }
}

CompletionFieldsConsumer::~CompletionFieldsConsumer()
{
  if (closed) {
    return;
  }
  closed = true;
  wstring indexFile = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, INDEX_EXTENSION);
  bool success = false;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput indexOut
  // = state.directory.createOutput(indexFile, state.context))
  {
    org::apache::lucene::store::IndexOutput indexOut =
        state->directory->createOutput(indexFile, state->context);
    try {
      delete delegateFieldsConsumer;
      CodecUtil::writeIndexHeader(
          indexOut, CODEC_NAME, COMPLETION_VERSION_CURRENT,
          state->segmentInfo->getId(), state->segmentSuffix);
      /*
       * we write the delegate postings format name so we can load it
       * without getting an instance in the ctor
       */
      indexOut->writeString(delegatePostingsFormatName);
      // write # of seen fields
      indexOut->writeVInt(seenFields.size());
      // write field numbers and dictOut offsets
      for (auto seenField : seenFields) {
        shared_ptr<FieldInfo> fieldInfo =
            state->fieldInfos->fieldInfo(seenField.first);
        indexOut->writeVInt(fieldInfo->number);
        shared_ptr<CompletionMetaData> metaData = seenField.second;
        indexOut->writeVLong(metaData->filePointer);
        indexOut->writeVLong(metaData->minWeight);
        indexOut->writeVLong(metaData->maxWeight);
        indexOut->writeByte(metaData->type);
      }
      CodecUtil::writeFooter(indexOut);
      CodecUtil::writeFooter(dictOut);
      IOUtils::close({dictOut});
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success == false) {
        IOUtils::closeWhileHandlingException({dictOut, delegateFieldsConsumer});
      }
    }
  }
}

CompletionFieldsConsumer::CompletionMetaData::CompletionMetaData(
    int64_t filePointer, int64_t minWeight, int64_t maxWeight, char type)
    : filePointer(filePointer), minWeight(minWeight), maxWeight(maxWeight),
      type(type)
{
}

CompletionFieldsConsumer::CompletionTermWriter::CompletionTermWriter()
    : builder(make_shared<NRTSuggesterBuilder>())
{
  first = true;
}

bool CompletionFieldsConsumer::CompletionTermWriter::finish(
    shared_ptr<IndexOutput> output) 
{
  bool stored = builder->store(output);
  assert((stored || docCount == 0,
          L"the FST is null but docCount is != 0 actual value: [" +
              to_wstring(docCount) + L"]"));
  if (docCount == 0) {
    minWeight = 0;
  }
  return stored;
}

void CompletionFieldsConsumer::CompletionTermWriter::write(
    shared_ptr<BytesRef> term,
    shared_ptr<TermsEnum> termsEnum) 
{
  postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::PAYLOADS);
  builder->startTerm(term);
  int docFreq = 0;
  while (postingsEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    int docID = postingsEnum->docID();
    for (int i = 0; i < postingsEnum->freq(); i++) {
      postingsEnum->nextPosition();
      assert(postingsEnum->getPayload() != nullptr);
      shared_ptr<BytesRef> payload = postingsEnum->getPayload();
      shared_ptr<ByteArrayDataInput> input = make_shared<ByteArrayDataInput>(
          payload->bytes, payload->offset, payload->length);
      int len = input->readVInt();
      scratch->grow(len);
      scratch->setLength(len);
      input->readBytes(scratch->bytes(), 0, scratch->length());
      int64_t weight = input->readVInt() - 1;
      maxWeight = max(maxWeight, weight);
      minWeight = min(minWeight, weight);
      char type = input->readByte();
      if (first) {
        this->type = type;
        first = false;
      } else if (this->type != type) {
        throw invalid_argument(L"single field name has mixed types");
      }
      builder->addEntry(docID, scratch->get(), weight);
    }
    docFreq++;
    docCount = max(docCount, docFreq + 1);
  }
  builder->finishTerm();
}
} // namespace org::apache::lucene::search::suggest::document