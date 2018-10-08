using namespace std;

#include "VariableGapTermsIndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermStats.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::codecs::blockterms
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using TermStats = org::apache::lucene::codecs::TermStats;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
const wstring VariableGapTermsIndexWriter::TERMS_INDEX_EXTENSION = L"tiv";
const wstring VariableGapTermsIndexWriter::CODEC_NAME =
    L"VariableGapTermsIndex";

VariableGapTermsIndexWriter::EveryNTermSelector::EveryNTermSelector(
    int interval)
    : interval(interval)
{
  // First term is first indexed term:
  count = interval;
}

bool VariableGapTermsIndexWriter::EveryNTermSelector::isIndexTerm(
    shared_ptr<BytesRef> term, shared_ptr<TermStats> stats)
{
  if (count >= interval) {
    count = 1;
    return true;
  } else {
    count++;
    return false;
  }
}

void VariableGapTermsIndexWriter::EveryNTermSelector::newField(
    shared_ptr<FieldInfo> fieldInfo)
{
  count = interval;
}

VariableGapTermsIndexWriter::EveryNOrDocFreqTermSelector::
    EveryNOrDocFreqTermSelector(int docFreqThresh, int interval)
    : docFreqThresh(docFreqThresh), interval(interval)
{

  // First term is first indexed term:
  count = interval;
}

bool VariableGapTermsIndexWriter::EveryNOrDocFreqTermSelector::isIndexTerm(
    shared_ptr<BytesRef> term, shared_ptr<TermStats> stats)
{
  if (stats->docFreq >= docFreqThresh || count >= interval) {
    count = 1;
    return true;
  } else {
    count++;
    return false;
  }
}

void VariableGapTermsIndexWriter::EveryNOrDocFreqTermSelector::newField(
    shared_ptr<FieldInfo> fieldInfo)
{
  count = interval;
}

VariableGapTermsIndexWriter::VariableGapTermsIndexWriter(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<IndexTermSelector> policy) 
{
  const wstring indexFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_INDEX_EXTENSION);
  out = state->directory->createOutput(indexFileName, state->context);
  bool success = false;
  try {
    fieldInfos = state->fieldInfos;
    this->policy = policy;
    CodecUtil::writeIndexHeader(out, CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
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

shared_ptr<FieldWriter> VariableGapTermsIndexWriter::addField(
    shared_ptr<FieldInfo> field, int64_t termsFilePointer) 
{
  ////System.out.println("VGW: field=" + field.name);
  policy->newField(field);
  shared_ptr<FSTFieldWriter> writer =
      make_shared<FSTFieldWriter>(shared_from_this(), field, termsFilePointer);
  fields.push_back(writer);
  return writer;
}

int VariableGapTermsIndexWriter::indexedTermPrefixLength(
    shared_ptr<BytesRef> priorTerm, shared_ptr<BytesRef> indexedTerm)
{
  // As long as codec sorts terms in unicode codepoint
  // order, we can safely strip off the non-distinguishing
  // suffix to save RAM in the loaded terms index.
  constexpr int idxTermOffset = indexedTerm->offset;
  constexpr int priorTermOffset = priorTerm->offset;
  constexpr int limit = min(priorTerm->length, indexedTerm->length);
  for (int byteIdx = 0; byteIdx < limit; byteIdx++) {
    if (priorTerm->bytes[priorTermOffset + byteIdx] !=
        indexedTerm->bytes[idxTermOffset + byteIdx]) {
      return byteIdx + 1;
    }
  }
  return min(1 + priorTerm->length, indexedTerm->length);
}

VariableGapTermsIndexWriter::FSTFieldWriter::FSTFieldWriter(
    shared_ptr<VariableGapTermsIndexWriter> outerInstance,
    shared_ptr<FieldInfo> fieldInfo,
    int64_t termsFilePointer) 
    : FieldWriter(outerInstance),
      fstBuilder(make_shared<org::apache::lucene::index::FieldInfos::Builder<>>(
          FST::INPUT_TYPE::BYTE1, fstOutputs)),
      fstOutputs(PositiveIntOutputs::getSingleton()),
      startTermsFilePointer(termsFilePointer), fieldInfo(fieldInfo),
      indexStart(outerInstance->out->getFilePointer()),
      outerInstance(outerInstance)
{
  ////System.out.println("VGW: field=" + fieldInfo.name);

  // Always put empty string in
  fstBuilder->add(make_shared<IntsRef>(), termsFilePointer);
}

bool VariableGapTermsIndexWriter::FSTFieldWriter::checkIndexTerm(
    shared_ptr<BytesRef> text, shared_ptr<TermStats> stats) 
{
  // System.out.println("VGW: index term=" + text.utf8ToString());
  // NOTE: we must force the first term per field to be
  // indexed, in case policy doesn't:
  if (outerInstance->policy->isIndexTerm(text, stats) || first) {
    first = false;
    // System.out.println("  YES");
    return true;
  } else {
    lastTerm->copyBytes(text);
    return false;
  }
}

void VariableGapTermsIndexWriter::FSTFieldWriter::add(
    shared_ptr<BytesRef> text, shared_ptr<TermStats> stats,
    int64_t termsFilePointer) 
{
  if (text->length == 0) {
    // We already added empty string in ctor
    assert(termsFilePointer == startTermsFilePointer);
    return;
  }
  constexpr int lengthSave = text->length;
  text->length = outerInstance->indexedTermPrefixLength(lastTerm->get(), text);
  try {
    fstBuilder->add(Util::toIntsRef(text, scratchIntsRef), termsFilePointer);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    text->length = lengthSave;
  }
  lastTerm->copyBytes(text);
}

void VariableGapTermsIndexWriter::FSTFieldWriter::finish(
    int64_t termsFilePointer) 
{
  fst = fstBuilder->finish();
  if (fst != nullptr) {
    fst->save(outerInstance->out);
  }
}

VariableGapTermsIndexWriter::~VariableGapTermsIndexWriter()
{
  if (out != nullptr) {
    try {
      constexpr int64_t dirStart = out->getFilePointer();
      constexpr int fieldCount = fields.size();

      int nonNullFieldCount = 0;
      for (int i = 0; i < fieldCount; i++) {
        shared_ptr<FSTFieldWriter> field = fields[i];
        if (field->fst != nullptr) {
          nonNullFieldCount++;
        }
      }

      out->writeVInt(nonNullFieldCount);
      for (int i = 0; i < fieldCount; i++) {
        shared_ptr<FSTFieldWriter> field = fields[i];
        if (field->fst != nullptr) {
          out->writeVInt(field->fieldInfo->number);
          out->writeVLong(field->indexStart);
        }
      }
      writeTrailer(dirStart);
      CodecUtil::writeFooter(out);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete out;
      out.reset();
    }
  }
}

void VariableGapTermsIndexWriter::writeTrailer(int64_t dirStart) throw(
    IOException)
{
  out->writeLong(dirStart);
}
} // namespace org::apache::lucene::codecs::blockterms