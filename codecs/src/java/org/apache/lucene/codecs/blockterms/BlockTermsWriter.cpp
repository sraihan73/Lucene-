using namespace std;

#include "BlockTermsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermStats.h"
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
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "TermsIndexWriterBase.h"

namespace org::apache::lucene::codecs::blockterms
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using TermStats = org::apache::lucene::codecs::TermStats;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const wstring BlockTermsWriter::CODEC_NAME = L"BlockTermsWriter";
const wstring BlockTermsWriter::TERMS_EXTENSION = L"tib";

BlockTermsWriter::FieldMetaData::FieldMetaData(shared_ptr<FieldInfo> fieldInfo,
                                               int64_t numTerms,
                                               int64_t termsStartPointer,
                                               int64_t sumTotalTermFreq,
                                               int64_t sumDocFreq,
                                               int docCount, int longsSize)
    : fieldInfo(fieldInfo), numTerms(numTerms),
      termsStartPointer(termsStartPointer), sumTotalTermFreq(sumTotalTermFreq),
      sumDocFreq(sumDocFreq), docCount(docCount), longsSize(longsSize)
{
  assert(numTerms > 0);
}

BlockTermsWriter::BlockTermsWriter(
    shared_ptr<TermsIndexWriterBase> termsIndexWriter,
    shared_ptr<SegmentWriteState> state,
    shared_ptr<PostingsWriterBase> postingsWriter) 
    : termsIndexWriter(termsIndexWriter), maxDoc(state->segmentInfo->maxDoc())
{
  const wstring termsFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_EXTENSION);
  out = state->directory->createOutput(termsFileName, state->context);
  bool success = false;
  try {
    fieldInfos = state->fieldInfos;
    CodecUtil::writeIndexHeader(out, CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    currentField.reset();
    this->postingsWriter = postingsWriter;
    // segment = state.segmentName;

    // System.out.println("BTW.init seg=" + state.segmentName);

    postingsWriter->init(out, state); // have consumer write its format/header
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

void BlockTermsWriter::write(shared_ptr<Fields> fields) 
{

  for (auto field : fields) {

    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();

    shared_ptr<TermsWriter> termsWriter =
        addField(fieldInfos->fieldInfo(field));

    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }

      termsWriter->write(term, termsEnum);
    }

    termsWriter->finish();
  }
}

shared_ptr<TermsWriter>
BlockTermsWriter::addField(shared_ptr<FieldInfo> field) 
{
  // System.out.println("\nBTW.addField seg=" + segment + " field=" +
  // field.name);
  assert(currentField == nullptr ||
         currentField->name.compare(field->name) < 0);
  currentField = field;
  shared_ptr<TermsIndexWriterBase::FieldWriter> fieldIndexWriter =
      termsIndexWriter->addField(field, out->getFilePointer());
  return make_shared<TermsWriter>(shared_from_this(), fieldIndexWriter, field,
                                  postingsWriter);
}

BlockTermsWriter::~BlockTermsWriter()
{
  if (out != nullptr) {
    try {
      constexpr int64_t dirStart = out->getFilePointer();

      out->writeVInt(fields.size());
      for (auto field : fields) {
        out->writeVInt(field->fieldInfo->number);
        out->writeVLong(field->numTerms);
        out->writeVLong(field->termsStartPointer);
        if (field->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
          out->writeVLong(field->sumTotalTermFreq);
        }
        out->writeVLong(field->sumDocFreq);
        out->writeVInt(field->docCount);
        out->writeVInt(field->longsSize);
      }
      writeTrailer(dirStart);
      CodecUtil::writeFooter(out);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::close({out, postingsWriter, termsIndexWriter});
      out.reset();
    }
  }
}

void BlockTermsWriter::writeTrailer(int64_t dirStart) 
{
  out->writeLong(dirStart);
}

BlockTermsWriter::TermsWriter::TermsWriter(
    shared_ptr<BlockTermsWriter> outerInstance,
    shared_ptr<TermsIndexWriterBase::FieldWriter> fieldIndexWriter,
    shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<PostingsWriterBase> postingsWriter)
    : fieldInfo(fieldInfo), postingsWriter(postingsWriter),
      termsStartPointer(outerInstance->out->getFilePointer()),
      fieldIndexWriter(fieldIndexWriter),
      docsSeen(make_shared<FixedBitSet>(outerInstance->maxDoc)),
      outerInstance(outerInstance)
{
  pendingTerms = std::deque<std::shared_ptr<TermEntry>>(32);
  for (int i = 0; i < pendingTerms.size(); i++) {
    pendingTerms[i] = make_shared<TermEntry>();
  }
  this->longsSize = postingsWriter->setField(fieldInfo);
}

void BlockTermsWriter::TermsWriter::write(
    shared_ptr<BytesRef> text,
    shared_ptr<TermsEnum> termsEnum) 
{

  shared_ptr<BlockTermState> state =
      postingsWriter->writeTerm(text, termsEnum, docsSeen);
  if (state == nullptr) {
    // No docs for this term:
    return;
  }
  sumDocFreq += state->docFreq;
  sumTotalTermFreq += state->totalTermFreq;

  assert(state->docFreq > 0);
  // System.out.println("BTW: finishTerm term=" + fieldInfo.name + ":" +
  // text.utf8ToString() + " " + text + " seg=" + segment + " df=" +
  // stats.docFreq);

  shared_ptr<TermStats> stats =
      make_shared<TermStats>(state->docFreq, state->totalTermFreq);
  constexpr bool isIndexTerm = fieldIndexWriter->checkIndexTerm(text, stats);

  if (isIndexTerm) {
    if (pendingCount > 0) {
      // Instead of writing each term, live, we gather terms
      // in RAM in a pending buffer, and then write the
      // entire block in between index terms:
      flushBlock();
    }
    fieldIndexWriter->add(text, stats, outerInstance->out->getFilePointer());
    // System.out.println("  index term!");
  }

  if (pendingTerms.size() == pendingCount) {
    pendingTerms = Arrays::copyOf(
        pendingTerms,
        ArrayUtil::oversize(pendingCount + 1,
                            RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    for (int i = pendingCount; i < pendingTerms.size(); i++) {
      pendingTerms[i] = make_shared<TermEntry>();
    }
  }
  shared_ptr<TermEntry> *const te = pendingTerms[pendingCount];
  te->term->copyBytes(text);
  te->state = state;

  pendingCount++;
  numTerms++;
}

void BlockTermsWriter::TermsWriter::finish() 
{
  if (pendingCount > 0) {
    flushBlock();
  }
  // EOF marker:
  outerInstance->out->writeVInt(0);

  fieldIndexWriter->finish(outerInstance->out->getFilePointer());
  if (numTerms > 0) {
    outerInstance->fields.push_back(make_shared<FieldMetaData>(
        fieldInfo, numTerms, termsStartPointer,
        fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
                0
            ? sumTotalTermFreq
            : -1,
        sumDocFreq, docsSeen->cardinality(), longsSize));
  }
}

int BlockTermsWriter::TermsWriter::sharedPrefix(shared_ptr<BytesRef> term1,
                                                shared_ptr<BytesRef> term2)
{
  assert(term1->offset == 0);
  assert(term2->offset == 0);
  int pos1 = 0;
  int pos1End = pos1 + min(term1->length, term2->length);
  int pos2 = 0;
  while (pos1 < pos1End) {
    if (term1->bytes[pos1] != term2->bytes[pos2]) {
      return pos1;
    }
    pos1++;
    pos2++;
  }
  return pos1;
}

void BlockTermsWriter::TermsWriter::flushBlock() 
{
  // System.out.println("BTW.flushBlock seg=" + segment + " pendingCount=" +
  // pendingCount + " fp=" + out.getFilePointer());

  // First pass: compute common prefix for all terms
  // in the block, against term before first term in
  // this block:
  int commonPrefix =
      sharedPrefix(lastPrevTerm->get(), pendingTerms[0]->term->get());
  for (int termCount = 1; termCount < pendingCount; termCount++) {
    commonPrefix =
        min(commonPrefix, sharedPrefix(lastPrevTerm->get(),
                                       pendingTerms[termCount]->term->get()));
  }

  outerInstance->out->writeVInt(pendingCount);
  outerInstance->out->writeVInt(commonPrefix);

  // 2nd pass: write suffixes, as separate byte[] blob
  for (int termCount = 0; termCount < pendingCount; termCount++) {
    constexpr int suffix =
        pendingTerms[termCount]->term->length() - commonPrefix;
    // TODO: cutover to better intblock codec, instead
    // of interleaving here:
    bytesWriter->writeVInt(suffix);
    bytesWriter->writeBytes(pendingTerms[termCount]->term.bytes(), commonPrefix,
                            suffix);
  }
  outerInstance->out->writeVInt(
      static_cast<int>(bytesWriter->getFilePointer()));
  bytesWriter->writeTo(outerInstance->out);
  bytesWriter->reset();

  // 3rd pass: write the freqs as byte[] blob
  // TODO: cutover to better intblock codec.  simple64?
  // write prefix, suffix first:
  for (int termCount = 0; termCount < pendingCount; termCount++) {
    shared_ptr<BlockTermState> *const state = pendingTerms[termCount]->state;
    assert(state != nullptr);
    bytesWriter->writeVInt(state->docFreq);
    if (fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
      bytesWriter->writeVLong(state->totalTermFreq - state->docFreq);
    }
  }
  outerInstance->out->writeVInt(
      static_cast<int>(bytesWriter->getFilePointer()));
  bytesWriter->writeTo(outerInstance->out);
  bytesWriter->reset();

  // 4th pass: write the metadata
  std::deque<int64_t> longs(longsSize);
  bool absolute = true;
  for (int termCount = 0; termCount < pendingCount; termCount++) {
    shared_ptr<BlockTermState> *const state = pendingTerms[termCount]->state;
    postingsWriter->encodeTerm(longs, bufferWriter, fieldInfo, state, absolute);
    for (int i = 0; i < longsSize; i++) {
      bytesWriter->writeVLong(longs[i]);
    }
    bufferWriter->writeTo(bytesWriter);
    bufferWriter->reset();
    absolute = false;
  }
  outerInstance->out->writeVInt(
      static_cast<int>(bytesWriter->getFilePointer()));
  bytesWriter->writeTo(outerInstance->out);
  bytesWriter->reset();

  lastPrevTerm->copyBytes(pendingTerms[pendingCount - 1]->term);
  pendingCount = 0;
}
} // namespace org::apache::lucene::codecs::blockterms