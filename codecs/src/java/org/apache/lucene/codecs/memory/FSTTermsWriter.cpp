using namespace std;

#include "FSTTermsWriter.h"
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
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "FSTTermOutputs.h"

namespace org::apache::lucene::codecs::memory
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
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
using Util = org::apache::lucene::util::fst::Util;
const wstring FSTTermsWriter::TERMS_EXTENSION = L"tfp";
const wstring FSTTermsWriter::TERMS_CODEC_NAME = L"FSTTerms";

FSTTermsWriter::FSTTermsWriter(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<PostingsWriterBase> postingsWriter) 
    : postingsWriter(postingsWriter), fieldInfos(state->fieldInfos),
      maxDoc(state->segmentInfo->maxDoc())
{
  const wstring termsFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_EXTENSION);

  this->out = state->directory->createOutput(termsFileName, state->context);

  bool success = false;
  try {
    CodecUtil::writeIndexHeader(out, TERMS_CODEC_NAME, TERMS_VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);

    this->postingsWriter->init(out, state);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({out});
    }
  }
}

void FSTTermsWriter::writeTrailer(shared_ptr<IndexOutput> out,
                                  int64_t dirStart) 
{
  out->writeLong(dirStart);
}

void FSTTermsWriter::write(shared_ptr<Fields> fields) 
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

FSTTermsWriter::~FSTTermsWriter()
{
  if (out != nullptr) {
    bool success = false;
    try {
      // write field summary
      constexpr int64_t dirStart = out->getFilePointer();

      out->writeVInt(fields.size());
      for (auto field : fields) {
        out->writeVInt(field->fieldInfo->number);
        out->writeVLong(field->numTerms);
        if (field->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
          out->writeVLong(field->sumTotalTermFreq);
        }
        out->writeVLong(field->sumDocFreq);
        out->writeVInt(field->docCount);
        out->writeVInt(field->longsSize);
        field->dict->save(out);
      }
      writeTrailer(out, dirStart);
      CodecUtil::writeFooter(out);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success) {
        IOUtils::close({out, postingsWriter});
      } else {
        IOUtils::closeWhileHandlingException({out, postingsWriter});
      }
      out.reset();
    }
  }
}

FSTTermsWriter::FieldMetaData::FieldMetaData(
    shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
    int64_t sumTotalTermFreq, int64_t sumDocFreq, int docCount,
    int longsSize,
    shared_ptr<FST<std::shared_ptr<FSTTermOutputs::TermData>>> fst)
    : fieldInfo(fieldInfo), numTerms(numTerms),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq),
      docCount(docCount), longsSize(longsSize), dict(fst)
{
}

FSTTermsWriter::TermsWriter::TermsWriter(
    shared_ptr<FSTTermsWriter> outerInstance, shared_ptr<FieldInfo> fieldInfo)
    : builder(make_shared<org::apache::lucene::index::FieldInfos::Builder<>>(
          FST::INPUT_TYPE::BYTE1, outputs)),
      outputs(make_shared<FSTTermOutputs>(fieldInfo, longsSize)),
      fieldInfo(fieldInfo),
      longsSize(outerInstance->postingsWriter->setField(fieldInfo)),
      outerInstance(outerInstance)
{
  this->numTerms = 0;
}

void FSTTermsWriter::TermsWriter::finishTerm(
    shared_ptr<BytesRef> text,
    shared_ptr<BlockTermState> state) 
{
  // write term meta data into fst
  shared_ptr<FSTTermOutputs::TermData> *const meta =
      make_shared<FSTTermOutputs::TermData>();
  meta->longs = std::deque<int64_t>(longsSize);
  meta->bytes.clear();
  meta->docFreq = state->docFreq;
  meta->totalTermFreq = state->totalTermFreq;
  outerInstance->postingsWriter->encodeTerm(meta->longs, metaWriter, fieldInfo,
                                            state, true);
  constexpr int bytesSize = static_cast<int>(metaWriter->getFilePointer());
  if (bytesSize > 0) {
    meta->bytes = std::deque<char>(bytesSize);
    metaWriter->writeTo(meta->bytes, 0);
    metaWriter->reset();
  }
  builder->add(Util::toIntsRef(text, scratchTerm), meta);
  numTerms++;
}

void FSTTermsWriter::TermsWriter::finish(int64_t sumTotalTermFreq,
                                         int64_t sumDocFreq,
                                         int docCount) 
{
  // save FST dict
  if (numTerms > 0) {
    shared_ptr<FST<std::shared_ptr<FSTTermOutputs::TermData>>> *const fst =
        builder->finish();
    outerInstance->fields.push_back(
        make_shared<FieldMetaData>(fieldInfo, numTerms, sumTotalTermFreq,
                                   sumDocFreq, docCount, longsSize, fst));
  }
}
} // namespace org::apache::lucene::codecs::memory