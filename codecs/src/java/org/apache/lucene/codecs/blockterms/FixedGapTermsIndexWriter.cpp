using namespace std;

#include "FixedGapTermsIndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermStats.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/PackedInts.h"

namespace org::apache::lucene::codecs::blockterms
{
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using TermStats = org::apache::lucene::codecs::TermStats;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using MonotonicBlockPackedWriter =
    org::apache::lucene::util::packed::MonotonicBlockPackedWriter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
const wstring FixedGapTermsIndexWriter::TERMS_INDEX_EXTENSION = L"tii";
const wstring FixedGapTermsIndexWriter::CODEC_NAME = L"FixedGapTermsIndex";

FixedGapTermsIndexWriter::FixedGapTermsIndexWriter(
    shared_ptr<SegmentWriteState> state) 
    : FixedGapTermsIndexWriter(state, DEFAULT_TERM_INDEX_INTERVAL)
{
}

FixedGapTermsIndexWriter::FixedGapTermsIndexWriter(
    shared_ptr<SegmentWriteState> state,
    int termIndexInterval) 
    : termIndexInterval(termIndexInterval)
{
  if (termIndexInterval <= 0) {
    throw invalid_argument(L"invalid termIndexInterval: " +
                           to_wstring(termIndexInterval));
  }
  const wstring indexFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_INDEX_EXTENSION);
  out = state->directory->createOutput(indexFileName, state->context);
  bool success = false;
  try {
    CodecUtil::writeIndexHeader(out, CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    out->writeVInt(termIndexInterval);
    out->writeVInt(PackedInts::VERSION_CURRENT);
    out->writeVInt(BLOCKSIZE);
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

shared_ptr<FieldWriter>
FixedGapTermsIndexWriter::addField(shared_ptr<FieldInfo> field,
                                   int64_t termsFilePointer)
{
  // System.out.println("FGW: addFfield=" + field.name);
  shared_ptr<SimpleFieldWriter> writer = make_shared<SimpleFieldWriter>(
      shared_from_this(), field, termsFilePointer);
  fields.push_back(writer);
  return writer;
}

int FixedGapTermsIndexWriter::indexedTermPrefixLength(
    shared_ptr<BytesRef> priorTerm, shared_ptr<BytesRef> indexedTerm)
{
  // As long as codec sorts terms in unicode codepoint
  // order, we can safely strip off the non-distinguishing
  // suffix to save RAM in the loaded terms index.
  return StringHelper::sortKeyLength(priorTerm, indexedTerm);
}

FixedGapTermsIndexWriter::SimpleFieldWriter::SimpleFieldWriter(
    shared_ptr<FixedGapTermsIndexWriter> outerInstance,
    shared_ptr<FieldInfo> fieldInfo, int64_t termsFilePointer)
    : FieldWriter(outerInstance), fieldInfo(fieldInfo),
      indexStart(outerInstance->out->getFilePointer()),
      termsStart(termsFilePointer), outerInstance(outerInstance)
{
  // we write terms+1 offsets, term n's length is n+1 - n
  try {
    termOffsets->add(0LL);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

bool FixedGapTermsIndexWriter::SimpleFieldWriter::checkIndexTerm(
    shared_ptr<BytesRef> text, shared_ptr<TermStats> stats) 
{
  // First term is first indexed term:
  // System.out.println("FGW: checkIndexTerm text=" + text.utf8ToString());
  if (0 == (numTerms++ % outerInstance->termIndexInterval)) {
    return true;
  } else {
    if (0 == numTerms % outerInstance->termIndexInterval) {
      // save last term just before next index term so we
      // can compute wasted suffix
      lastTerm->copyBytes(text);
    }
    return false;
  }
}

void FixedGapTermsIndexWriter::SimpleFieldWriter::add(
    shared_ptr<BytesRef> text, shared_ptr<TermStats> stats,
    int64_t termsFilePointer) 
{
  constexpr int indexedTermLength;
  if (numIndexTerms == 0) {
    // no previous term: no bytes to write
    indexedTermLength = 0;
  } else {
    indexedTermLength =
        outerInstance->indexedTermPrefixLength(lastTerm->get(), text);
  }
  // System.out.println("FGW: add text=" + text.utf8ToString() + " " + text + "
  // fp=" + termsFilePointer);

  // write only the min prefix that shows the diff
  // against prior term
  outerInstance->out->writeBytes(text->bytes, text->offset, indexedTermLength);

  // save delta terms pointer
  termAddresses->add(termsFilePointer - termsStart);

  // save term length (in bytes)
  assert((indexedTermLength <= std, : numeric_limits<short>::max()));
  currentOffset += indexedTermLength;
  termOffsets->add(currentOffset);

  lastTerm->copyBytes(text);
  numIndexTerms++;
}

void FixedGapTermsIndexWriter::SimpleFieldWriter::finish(
    int64_t termsFilePointer) 
{

  // write primary terms dict offsets
  packedIndexStart = outerInstance->out->getFilePointer();

  // relative to our indexStart
  termAddresses->finish();
  addressBuffer->writeTo(outerInstance->out);

  packedOffsetsStart = outerInstance->out->getFilePointer();

  // write offsets into the byte[] terms
  termOffsets->finish();
  offsetsBuffer->writeTo(outerInstance->out);

  // our referrer holds onto us, while other fields are
  // being written, so don't tie up this RAM:
  termOffsets = termAddresses = nullptr;
  addressBuffer = offsetsBuffer = nullptr;
}

FixedGapTermsIndexWriter::~FixedGapTermsIndexWriter()
{
  if (out != nullptr) {
    bool success = false;
    try {
      constexpr int64_t dirStart = out->getFilePointer();
      constexpr int fieldCount = fields.size();

      int nonNullFieldCount = 0;
      for (int i = 0; i < fieldCount; i++) {
        shared_ptr<SimpleFieldWriter> field = fields[i];
        if (field->numIndexTerms > 0) {
          nonNullFieldCount++;
        }
      }

      out->writeVInt(nonNullFieldCount);
      for (int i = 0; i < fieldCount; i++) {
        shared_ptr<SimpleFieldWriter> field = fields[i];
        if (field->numIndexTerms > 0) {
          out->writeVInt(field->fieldInfo->number);
          out->writeVInt(field->numIndexTerms);
          out->writeVLong(field->termsStart);
          out->writeVLong(field->indexStart);
          out->writeVLong(field->packedIndexStart);
          out->writeVLong(field->packedOffsetsStart);
        }
      }
      writeTrailer(dirStart);
      CodecUtil::writeFooter(out);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success) {
        IOUtils::close({out});
      } else {
        IOUtils::closeWhileHandlingException({out});
      }
      out.reset();
    }
  }
}

void FixedGapTermsIndexWriter::writeTrailer(int64_t dirStart) throw(
    IOException)
{
  out->writeLong(dirStart);
}
} // namespace org::apache::lucene::codecs::blockterms