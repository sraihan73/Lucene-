using namespace std;

#include "BloomFilteringPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include "BloomFilterFactory.h"
#include "FuzzySet.h"

namespace org::apache::lucene::codecs::bloom
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using ContainsResult =
    org::apache::lucene::codecs::bloom::FuzzySet::ContainsResult;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
const wstring BloomFilteringPostingsFormat::BLOOM_CODEC_NAME = L"BloomFilter";
const wstring BloomFilteringPostingsFormat::BLOOM_EXTENSION = L"blm";

BloomFilteringPostingsFormat::BloomFilteringPostingsFormat(
    shared_ptr<PostingsFormat> delegatePostingsFormat,
    shared_ptr<BloomFilterFactory> bloomFilterFactory)
    : org::apache::lucene::codecs::PostingsFormat(BLOOM_CODEC_NAME)
{
  this->delegatePostingsFormat = delegatePostingsFormat;
  this->bloomFilterFactory = bloomFilterFactory;
}

BloomFilteringPostingsFormat::BloomFilteringPostingsFormat(
    shared_ptr<PostingsFormat> delegatePostingsFormat)
    : BloomFilteringPostingsFormat(delegatePostingsFormat,
                                   new DefaultBloomFilterFactory())
{
}

BloomFilteringPostingsFormat::BloomFilteringPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(BLOOM_CODEC_NAME)
{
}

shared_ptr<FieldsConsumer> BloomFilteringPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  if (delegatePostingsFormat == nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"Error - " + getClassName() +
        L" has been constructed without a choice of PostingsFormat");
  }
  shared_ptr<FieldsConsumer> fieldsConsumer =
      delegatePostingsFormat->fieldsConsumer(state);
  return make_shared<BloomFilteredFieldsConsumer>(shared_from_this(),
                                                  fieldsConsumer, state);
}

shared_ptr<FieldsProducer> BloomFilteringPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<BloomFilteredFieldsProducer>(state);
}

BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredFieldsProducer(shared_ptr<SegmentReadState> state) throw(
        IOException)
{

  wstring bloomFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, BLOOM_EXTENSION);
  shared_ptr<ChecksumIndexInput> bloomIn = nullptr;
  bool success = false;
  try {
    bloomIn =
        state->directory->openChecksumInput(bloomFileName, state->context);
    CodecUtil::checkIndexHeader(bloomIn, BLOOM_CODEC_NAME, VERSION_START,
                                VERSION_CURRENT, state->segmentInfo->getId(),
                                state->segmentSuffix);
    // // Load the hash function used in the BloomFilter
    // hashFunction = HashFunction.forName(bloomIn.readString());
    // Load the delegate postings format
    shared_ptr<PostingsFormat> delegatePostingsFormat =
        PostingsFormat::forName(bloomIn->readString());

    this->delegateFieldsProducer =
        delegatePostingsFormat->fieldsProducer(state);
    int numBlooms = bloomIn->readInt();
    for (int i = 0; i < numBlooms; i++) {
      int fieldNum = bloomIn->readInt();
      shared_ptr<FuzzySet> bloom = FuzzySet::deserialize(bloomIn);
      shared_ptr<FieldInfo> fieldInfo = state->fieldInfos->fieldInfo(fieldNum);
      bloomsByFieldName.emplace(fieldInfo->name, bloom);
    }
    CodecUtil::checkFooter(bloomIn);
    IOUtils::close({bloomIn});
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({bloomIn, delegateFieldsProducer});
    }
  }
}

shared_ptr<Iterator<wstring>>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::iterator()
{
  return delegateFieldsProducer->begin();
}

BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    ~BloomFilteredFieldsProducer()
{
  delegateFieldsProducer->close();
}

shared_ptr<Terms>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::terms(
    const wstring &field) 
{
  shared_ptr<FuzzySet> filter = bloomsByFieldName[field];
  if (filter == nullptr) {
    return delegateFieldsProducer->terms(field);
  } else {
    shared_ptr<Terms> result = delegateFieldsProducer->terms(field);
    if (result == nullptr) {
      return nullptr;
    }
    return make_shared<BloomFilteredTerms>(result, filter);
  }
}

int BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::size()
{
  return delegateFieldsProducer->size();
}

BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::BloomFilteredTerms::
    BloomFilteredTerms(shared_ptr<Terms> terms, shared_ptr<FuzzySet> filter)
{
  this->delegateTerms = terms;
  this->filter = filter;
}

shared_ptr<TermsEnum>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::BloomFilteredTerms::
    intersect(shared_ptr<CompiledAutomaton> compiled,
              shared_ptr<BytesRef> startTerm) 
{
  return delegateTerms->intersect(compiled, startTerm);
}

shared_ptr<TermsEnum>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::BloomFilteredTerms::
    iterator() 
{
  return make_shared<BloomFilteredTermsEnum>(delegateTerms, filter);
}

int64_t BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::size() 
{
  return delegateTerms->size();
}

int64_t BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::getSumTotalTermFreq() 
{
  return delegateTerms->getSumTotalTermFreq();
}

int64_t BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::getSumDocFreq() 
{
  return delegateTerms->getSumDocFreq();
}

int BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::getDocCount() 
{
  return delegateTerms->getDocCount();
}

bool BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::hasFreqs()
{
  return delegateTerms->hasFreqs();
}

bool BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::hasOffsets()
{
  return delegateTerms->hasOffsets();
}

bool BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::hasPositions()
{
  return delegateTerms->hasPositions();
}

bool BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::hasPayloads()
{
  return delegateTerms->hasPayloads();
}

shared_ptr<BytesRef> BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::getMin() 
{
  return delegateTerms->getMin();
}

shared_ptr<BytesRef> BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTerms::getMax() 
{
  return delegateTerms->getMax();
}

BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::BloomFilteredTermsEnum(
        shared_ptr<Terms> delegateTerms,
        shared_ptr<FuzzySet> filter) 
    : filter(filter)
{
  this->delegateTerms = delegateTerms;
}

void BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::reset(shared_ptr<Terms> delegateTerms) throw(
        IOException)
{
  this->delegateTerms = delegateTerms;
  this->delegateTermsEnum.reset();
}

shared_ptr<TermsEnum>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::delegate_() 
{
  if (delegateTermsEnum == nullptr) {
    /* pull the iterator only if we really need it -
     * this can be a relativly heavy operation depending on the
     * delegate postings format and they underlying directory
     * (clone IndexInput) */
    delegateTermsEnum = delegateTerms->begin();
  }
  return delegateTermsEnum;
}

shared_ptr<BytesRef> BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::next() 
{
  return delegate_()->next();
}

bool BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::seekExact(shared_ptr<BytesRef> text) throw(
        IOException)
{
  // The magical fail-fast speed up that is the entire point of all of
  // this code - save a disk seek if there is a match on an in-memory
  // structure
  // that may occasionally give a false positive but guaranteed no false
  // negatives
  if (filter->contains(text) == ContainsResult::NO) {
    return false;
  }
  return delegate_()->seekExact(text);
}

SeekStatus BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::seekCeil(shared_ptr<BytesRef> text) throw(
        IOException)
{
  return delegate_()->seekCeil(text);
}

void BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::seekExact(int64_t ord) 
{
  delegate_()->seekExact(ord);
}

shared_ptr<BytesRef> BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::term() 
{
  return delegate_()->term();
}

int64_t BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::ord() 
{
  return delegate_()->ord();
}

int BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::docFreq() 
{
  return delegate_()->docFreq();
}

int64_t BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::totalTermFreq() 
{
  return delegate_()->totalTermFreq();
}

shared_ptr<PostingsEnum>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    BloomFilteredTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                     int flags) 
{
  return delegate_()->postings(reuse, flags);
}

int64_t
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::ramBytesUsed()
{
  int64_t sizeInBytes = ((delegateFieldsProducer->size() > 0)
                               ? delegateFieldsProducer->ramBytesUsed()
                               : 0);
  for (auto entry : bloomsByFieldName) {
    sizeInBytes += entry.first->length() * Character::BYTES;
    sizeInBytes += entry.second::ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(
      Accountables::namedAccountables(L"field", bloomsByFieldName));
  if (delegateFieldsProducer->size() > 0) {
    resources.push_back(
        Accountables::namedAccountable(L"delegate", delegateFieldsProducer));
  }
  return Collections::unmodifiableList(resources);
}

void BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::
    checkIntegrity() 
{
  delegateFieldsProducer->checkIntegrity();
}

wstring BloomFilteringPostingsFormat::BloomFilteredFieldsProducer::toString()
{
  return getClass().getSimpleName() + L"(fields=" + bloomsByFieldName.size() +
         L",delegate=" + delegateFieldsProducer + L")";
}

BloomFilteringPostingsFormat::BloomFilteredFieldsConsumer::
    BloomFilteredFieldsConsumer(
        shared_ptr<BloomFilteringPostingsFormat> outerInstance,
        shared_ptr<FieldsConsumer> fieldsConsumer,
        shared_ptr<SegmentWriteState> state)
    : outerInstance(outerInstance)
{
  this->delegateFieldsConsumer = fieldsConsumer;
  this->state = state;
}

void BloomFilteringPostingsFormat::BloomFilteredFieldsConsumer::write(
    shared_ptr<Fields> fields) 
{

  // Delegate must write first: it may have opened files
  // on creating the class
  // (e.g. Lucene41PostingsConsumer), and write() will
  // close them; alternatively, if we delayed pulling
  // the fields consumer until here, we could do it
  // afterwards:
  delegateFieldsConsumer->write(fields);

  for (auto field : fields) {
    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }
    shared_ptr<FieldInfo> fieldInfo = state->fieldInfos->fieldInfo(field);
    shared_ptr<TermsEnum> termsEnum = terms->begin();

    shared_ptr<FuzzySet> bloomFilter = nullptr;

    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      if (bloomFilter == nullptr) {
        bloomFilter =
            outerInstance->bloomFilterFactory->getSetForField(state, fieldInfo);
        if (bloomFilter == nullptr) {
          // Field not bloom'd
          break;
        }
        assert(bloomFilters.find(field) != bloomFilters.end() == false);
        bloomFilters.emplace(fieldInfo, bloomFilter);
      }
      // Make sure there's at least one doc for this term:
      postingsEnum = termsEnum->postings(postingsEnum, 0);
      if (postingsEnum->nextDoc() != PostingsEnum::NO_MORE_DOCS) {
        bloomFilter->addValue(term);
      }
    }
  }
}

BloomFilteringPostingsFormat::BloomFilteredFieldsConsumer::
    ~BloomFilteredFieldsConsumer()
{
  if (closed) {
    return;
  }
  closed = true;
  delete delegateFieldsConsumer;

  // Now we are done accumulating values for these fields
  deque<unordered_map::Entry<std::shared_ptr<FieldInfo>,
                              std::shared_ptr<FuzzySet>>>
      nonSaturatedBlooms =
          deque<unordered_map::Entry<std::shared_ptr<FieldInfo>,
                                      std::shared_ptr<FuzzySet>>>();

  for (auto entry : bloomFilters) {
    shared_ptr<FuzzySet> bloomFilter = entry.second;
    if (!outerInstance->bloomFilterFactory->isSaturated(bloomFilter,
                                                        entry.first)) {
      nonSaturatedBlooms.push_back(entry);
    }
  }
  wstring bloomFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, BLOOM_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput
  // bloomOutput = state.directory.createOutput(bloomFileName, state.context))
  {
    org::apache::lucene::store::IndexOutput bloomOutput =
        state->directory->createOutput(bloomFileName, state->context);
    CodecUtil::writeIndexHeader(bloomOutput, BLOOM_CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    // remember the name of the postings format we will delegate to
    bloomOutput->writeString(outerInstance->delegatePostingsFormat->getName());

    // First field in the output file is the number of fields+blooms saved
    bloomOutput->writeInt(nonSaturatedBlooms.size());
    for (auto entry : nonSaturatedBlooms) {
      shared_ptr<FieldInfo> fieldInfo = entry.first;
      shared_ptr<FuzzySet> bloomFilter = entry.second;
      bloomOutput->writeInt(fieldInfo->number);
      saveAppropriatelySizedBloomFilter(bloomOutput, bloomFilter, fieldInfo);
    }
    CodecUtil::writeFooter(bloomOutput);
  }
  // We are done with large bitsets so no need to keep them hanging around
  bloomFilters.clear();
}

void BloomFilteringPostingsFormat::BloomFilteredFieldsConsumer::
    saveAppropriatelySizedBloomFilter(
        shared_ptr<IndexOutput> bloomOutput, shared_ptr<FuzzySet> bloomFilter,
        shared_ptr<FieldInfo> fieldInfo) 
{

  shared_ptr<FuzzySet> rightSizedSet =
      outerInstance->bloomFilterFactory->downsize(fieldInfo, bloomFilter);
  if (rightSizedSet == nullptr) {
    rightSizedSet = bloomFilter;
  }
  rightSizedSet->serialize(bloomOutput);
}

wstring BloomFilteringPostingsFormat::toString()
{
  return L"BloomFilteringPostingsFormat(" + delegatePostingsFormat + L")";
}
} // namespace org::apache::lucene::codecs::bloom