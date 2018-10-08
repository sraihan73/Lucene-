using namespace std;

#include "CompletionFieldsProducer.h"

namespace org::apache::lucene::search::suggest::document
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using IOUtils = org::apache::lucene::util::IOUtils;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.CODEC_NAME;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.COMPLETION_CODEC_VERSION;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.COMPLETION_VERSION_CURRENT;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.DICT_EXTENSION;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.INDEX_EXTENSION;

CompletionFieldsProducer::CompletionFieldsProducer(
    shared_ptr<FieldsProducer> delegateFieldsProducer,
    unordered_map<wstring, std::shared_ptr<CompletionsTermsReader>> &readers)
{
  this->delegateFieldsProducer = delegateFieldsProducer;
  this->readers = readers;
}

CompletionFieldsProducer::CompletionFieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  wstring indexFile = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, INDEX_EXTENSION);
  delegateFieldsProducer.reset();
  bool success = false;

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // index = state.directory.openChecksumInput(indexFile, state.context))
  {
    org::apache::lucene::store::ChecksumIndexInput index =
        state->directory->openChecksumInput(indexFile, state->context);
    try {
      // open up dict file containing all fsts
      wstring dictFile = IndexFileNames::segmentFileName(
          state->segmentInfo->name, state->segmentSuffix, DICT_EXTENSION);
      dictIn = state->directory->openInput(dictFile, state->context);
      CodecUtil::checkIndexHeader(dictIn, CODEC_NAME, COMPLETION_CODEC_VERSION,
                                  COMPLETION_VERSION_CURRENT,
                                  state->segmentInfo->getId(),
                                  state->segmentSuffix);
      // just validate the footer for the dictIn
      CodecUtil::retrieveChecksum(dictIn);

      // open up index file (fieldNumber, offset)
      CodecUtil::checkIndexHeader(index, CODEC_NAME, COMPLETION_CODEC_VERSION,
                                  COMPLETION_VERSION_CURRENT,
                                  state->segmentInfo->getId(),
                                  state->segmentSuffix);
      // load delegate PF
      shared_ptr<PostingsFormat> delegatePostingsFormat =
          PostingsFormat::forName(index->readString());
      delegateFieldsProducer = delegatePostingsFormat->fieldsProducer(state);

      // read suggest field numbers and their offsets in the terms file from
      // index
      int numFields = index->readVInt();
      readers = unordered_map<>(numFields);
      for (int i = 0; i < numFields; i++) {
        int fieldNumber = index->readVInt();
        int64_t offset = index->readVLong();
        int64_t minWeight = index->readVLong();
        int64_t maxWeight = index->readVLong();
        char type = index->readByte();
        shared_ptr<FieldInfo> fieldInfo =
            state->fieldInfos->fieldInfo(fieldNumber);
        // we don't load the FST yet
        readers.emplace(fieldInfo->name,
                        make_shared<CompletionsTermsReader>(
                            dictIn, offset, minWeight, maxWeight, type));
      }
      CodecUtil::checkFooter(index);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success == false) {
        IOUtils::closeWhileHandlingException({delegateFieldsProducer, dictIn});
      }
    }
  }
}

CompletionFieldsProducer::~CompletionFieldsProducer()
{
  bool success = false;
  try {
    delegateFieldsProducer->close();
    IOUtils::close({dictIn});
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      IOUtils::closeWhileHandlingException({delegateFieldsProducer, dictIn});
    }
  }
}

void CompletionFieldsProducer::checkIntegrity() 
{
  delegateFieldsProducer->checkIntegrity();
  // TODO: checkIntegrity should checksum the dictionary and index
}

shared_ptr<FieldsProducer>
CompletionFieldsProducer::getMergeInstance() 
{
  return make_shared<CompletionFieldsProducer>(delegateFieldsProducer, readers);
}

int64_t CompletionFieldsProducer::ramBytesUsed()
{
  int64_t ramBytesUsed = delegateFieldsProducer->ramBytesUsed();
  for (auto reader : readers) {
    ramBytesUsed += reader->second.ramBytesUsed();
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
CompletionFieldsProducer::getChildResources()
{
  deque<std::shared_ptr<Accountable>> accountableList =
      deque<std::shared_ptr<Accountable>>();
  for (auto readerEntry : readers) {
    accountableList.push_back(
        Accountables::namedAccountable(readerEntry.first, readerEntry.second));
  }
  return Collections::unmodifiableCollection(accountableList);
}

shared_ptr<Iterator<wstring>> CompletionFieldsProducer::iterator()
{
  return readers.keySet().begin();
}

shared_ptr<Terms>
CompletionFieldsProducer::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = delegateFieldsProducer->terms(field);
  if (terms == nullptr) {
    return nullptr;
  }
  return make_shared<CompletionTerms>(terms, readers[field]);
}

int CompletionFieldsProducer::size() { return readers.size(); }
} // namespace org::apache::lucene::search::suggest::document