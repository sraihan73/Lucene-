using namespace std;

#include "FSTOrdTermsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::codecs::memory
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
const wstring FSTOrdTermsWriter::TERMS_INDEX_EXTENSION = L"tix";
const wstring FSTOrdTermsWriter::TERMS_BLOCK_EXTENSION = L"tbk";
const wstring FSTOrdTermsWriter::TERMS_CODEC_NAME = L"FSTOrdTerms";
const wstring FSTOrdTermsWriter::TERMS_INDEX_CODEC_NAME = L"FSTOrdIndex";

FSTOrdTermsWriter::FSTOrdTermsWriter(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<PostingsWriterBase> postingsWriter) 
    : postingsWriter(postingsWriter), fieldInfos(state->fieldInfos),
      maxDoc(state->segmentInfo->maxDoc())
{
  const wstring termsIndexFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_INDEX_EXTENSION);
  const wstring termsBlockFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_BLOCK_EXTENSION);

  bool success = false;
  try {
    this->indexOut =
        state->directory->createOutput(termsIndexFileName, state->context);
    this->blockOut =
        state->directory->createOutput(termsBlockFileName, state->context);
    CodecUtil::writeIndexHeader(indexOut, TERMS_INDEX_CODEC_NAME,
                                VERSION_CURRENT, state->segmentInfo->getId(),
                                state->segmentSuffix);
    CodecUtil::writeIndexHeader(blockOut, TERMS_CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    this->postingsWriter->init(blockOut, state);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({indexOut, blockOut});
    }
  }
}

void FSTOrdTermsWriter::write(shared_ptr<Fields> fields) 
{
  for (auto field : fields) {
    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(field);
    bool hasFreq = fieldInfo->getIndexOptions().compareTo(
                       IndexOptions::DOCS_AND_FREQS) >= 0;
    shared_ptr<TermsEnum> termsEnum = terms->begin();
    shared_ptr<TermsWriter> termsWriter =
        make_shared<TermsWriter>(shared_from_this(), fieldInfo);

    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    shared_ptr<FixedBitSet> docsSeen = make_shared<FixedBitSet>(maxDoc);
    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      shared_ptr<BlockTermState> termState =
          postingsWriter->writeTerm(term, termsEnum, docsSeen);
      if (termState != nullptr) {
        termsWriter->finishTerm(term, termState);
        sumTotalTermFreq += termState->totalTermFreq;
        sumDocFreq += termState->docFreq;
      }
    }

    termsWriter->finish(hasFreq ? sumTotalTermFreq : -1, sumDocFreq,
                        docsSeen->cardinality());
  }
}

FSTOrdTermsWriter::~FSTOrdTermsWriter()
{
  if (blockOut != nullptr) {
    bool success = false;
    try {
      constexpr int64_t blockDirStart = blockOut->getFilePointer();

      // write field summary
      blockOut->writeVInt(fields.size());
      for (auto field : fields) {
        blockOut->writeVInt(field->fieldInfo->number);
        blockOut->writeVLong(field->numTerms);
        if (field->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
          blockOut->writeVLong(field->sumTotalTermFreq);
        }
        blockOut->writeVLong(field->sumDocFreq);
        blockOut->writeVInt(field->docCount);
        blockOut->writeVInt(field->longsSize);
        blockOut->writeVLong(field->statsOut->getFilePointer());
        blockOut->writeVLong(field->metaLongsOut->getFilePointer());
        blockOut->writeVLong(field->metaBytesOut->getFilePointer());

        field->skipOut->writeTo(blockOut);
        field->statsOut->writeTo(blockOut);
        field->metaLongsOut->writeTo(blockOut);
        field->metaBytesOut->writeTo(blockOut);
        field->dict->save(indexOut);
      }
      writeTrailer(blockOut, blockDirStart);
      CodecUtil::writeFooter(indexOut);
      CodecUtil::writeFooter(blockOut);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success) {
        IOUtils::close({blockOut, indexOut, postingsWriter});
      } else {
        IOUtils::closeWhileHandlingException(
            {blockOut, indexOut, postingsWriter});
      }
      blockOut.reset();
    }
  }
}

void FSTOrdTermsWriter::writeTrailer(shared_ptr<IndexOutput> out,
                                     int64_t dirStart) 
{
  out->writeLong(dirStart);
}

FSTOrdTermsWriter::TermsWriter::TermsWriter(
    shared_ptr<FSTOrdTermsWriter> outerInstance,
    shared_ptr<FieldInfo> fieldInfo)
    : builder(make_shared<org::apache::lucene::index::FieldInfos::Builder<>>(
          FST::INPUT_TYPE::BYTE1, outputs)),
      outputs(PositiveIntOutputs::getSingleton()), fieldInfo(fieldInfo),
      longsSize(outerInstance->postingsWriter->setField(fieldInfo)),
      outerInstance(outerInstance)
{
  this->numTerms = 0;

  this->lastBlockStatsFP = 0;
  this->lastBlockMetaLongsFP = 0;
  this->lastBlockMetaBytesFP = 0;
  this->lastBlockLongs = std::deque<int64_t>(longsSize);

  this->lastLongs = std::deque<int64_t>(longsSize);
  this->lastMetaBytesFP = 0;
}

void FSTOrdTermsWriter::TermsWriter::finishTerm(
    shared_ptr<BytesRef> text,
    shared_ptr<BlockTermState> state) 
{
  if (numTerms > 0 && numTerms % SKIP_INTERVAL == 0) {
    bufferSkip();
  }
  // write term meta data into fst
  const std::deque<int64_t> longs = std::deque<int64_t>(longsSize);
  constexpr int64_t delta = state->totalTermFreq - state->docFreq;
  if (state->totalTermFreq > 0) {
    if (delta == 0) {
      statsOut->writeVInt(state->docFreq << 1 | 1);
    } else {
      statsOut->writeVInt(state->docFreq << 1);
      statsOut->writeVLong(state->totalTermFreq - state->docFreq);
    }
  } else {
    statsOut->writeVInt(state->docFreq);
  }
  outerInstance->postingsWriter->encodeTerm(longs, metaBytesOut, fieldInfo,
                                            state, true);
  for (int i = 0; i < longsSize; i++) {
    metaLongsOut->writeVLong(longs[i] - lastLongs[i]);
    lastLongs[i] = longs[i];
  }
  metaLongsOut->writeVLong(metaBytesOut->getFilePointer() - lastMetaBytesFP);

  builder->add(Util::toIntsRef(text, scratchTerm), numTerms);
  numTerms++;

  lastMetaBytesFP = metaBytesOut->getFilePointer();
}

void FSTOrdTermsWriter::TermsWriter::finish(int64_t sumTotalTermFreq,
                                            int64_t sumDocFreq,
                                            int docCount) 
{
  if (numTerms > 0) {
    shared_ptr<FieldMetaData> *const metadata = make_shared<FieldMetaData>();
    metadata->fieldInfo = fieldInfo;
    metadata->numTerms = numTerms;
    metadata->sumTotalTermFreq = sumTotalTermFreq;
    metadata->sumDocFreq = sumDocFreq;
    metadata->docCount = docCount;
    metadata->longsSize = longsSize;
    metadata->skipOut = skipOut;
    metadata->statsOut = statsOut;
    metadata->metaLongsOut = metaLongsOut;
    metadata->metaBytesOut = metaBytesOut;
    metadata->dict = builder->finish();
    outerInstance->fields.push_back(metadata);
  }
}

void FSTOrdTermsWriter::TermsWriter::bufferSkip() 
{
  skipOut->writeVLong(statsOut->getFilePointer() - lastBlockStatsFP);
  skipOut->writeVLong(metaLongsOut->getFilePointer() - lastBlockMetaLongsFP);
  skipOut->writeVLong(metaBytesOut->getFilePointer() - lastBlockMetaBytesFP);
  for (int i = 0; i < longsSize; i++) {
    skipOut->writeVLong(lastLongs[i] - lastBlockLongs[i]);
  }
  lastBlockStatsFP = statsOut->getFilePointer();
  lastBlockMetaLongsFP = metaLongsOut->getFilePointer();
  lastBlockMetaBytesFP = metaBytesOut->getFilePointer();
  System::arraycopy(lastLongs, 0, lastBlockLongs, 0, longsSize);
}
} // namespace org::apache::lucene::codecs::memory