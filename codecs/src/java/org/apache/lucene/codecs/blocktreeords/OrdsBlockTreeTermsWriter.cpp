using namespace std;

#include "OrdsBlockTreeTermsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/blocktree/BlockTreeTermsWriter.h"
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
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/BytesRefFSTEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "FSTOrdsOutputs.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
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
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;
using Builder = org::apache::lucene::util::fst::Builder;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;
const shared_ptr<FSTOrdsOutputs> OrdsBlockTreeTermsWriter::FST_OUTPUTS =
    make_shared<FSTOrdsOutputs>();
const shared_ptr<
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output>
    OrdsBlockTreeTermsWriter::NO_OUTPUT = FST_OUTPUTS->getNoOutput();
const wstring OrdsBlockTreeTermsWriter::TERMS_EXTENSION = L"tio";
const wstring OrdsBlockTreeTermsWriter::TERMS_CODEC_NAME =
    L"OrdsBlockTreeTerms";
const wstring OrdsBlockTreeTermsWriter::TERMS_INDEX_EXTENSION = L"tipo";
const wstring OrdsBlockTreeTermsWriter::TERMS_INDEX_CODEC_NAME =
    L"OrdsBlockTreeIndex";

OrdsBlockTreeTermsWriter::FieldMetaData::FieldMetaData(
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<Output> rootCode,
    int64_t numTerms, int64_t indexStartFP, int64_t sumTotalTermFreq,
    int64_t sumDocFreq, int docCount, int longsSize,
    shared_ptr<BytesRef> minTerm, shared_ptr<BytesRef> maxTerm)
    : fieldInfo(fieldInfo), rootCode(rootCode), numTerms(numTerms),
      indexStartFP(indexStartFP), sumTotalTermFreq(sumTotalTermFreq),
      sumDocFreq(sumDocFreq), docCount(docCount), longsSize(longsSize),
      minTerm(minTerm), maxTerm(maxTerm)
{
  assert(numTerms > 0);
  assert((rootCode != nullptr,
          L"field=" + fieldInfo->name + L" numTerms=" + to_wstring(numTerms)));
}

OrdsBlockTreeTermsWriter::OrdsBlockTreeTermsWriter(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<PostingsWriterBase> postingsWriter, int minItemsInBlock,
    int maxItemsInBlock) 
    : out(state->directory->createOutput(termsFileName, state->context)),
      maxDoc(state->segmentInfo->maxDoc())
{
  BlockTreeTermsWriter::validateSettings(minItemsInBlock, maxItemsInBlock);

  const wstring termsFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, TERMS_EXTENSION);
  bool success = false;
  shared_ptr<IndexOutput> indexOut = nullptr;
  try {
    fieldInfos = state->fieldInfos;
    this->minItemsInBlock = minItemsInBlock;
    this->maxItemsInBlock = maxItemsInBlock;
    CodecUtil::writeIndexHeader(out, TERMS_CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);

    const wstring termsIndexFileName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix, TERMS_INDEX_EXTENSION);
    indexOut =
        state->directory->createOutput(termsIndexFileName, state->context);
    CodecUtil::writeIndexHeader(indexOut, TERMS_INDEX_CODEC_NAME,
                                VERSION_CURRENT, state->segmentInfo->getId(),
                                state->segmentSuffix);

    this->postingsWriter = postingsWriter;
    // segment = state.segmentInfo.name;

    // System.out.println("BTW.init seg=" + state.segmentName);

    postingsWriter->init(out, state); // have consumer write its format/header
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({out, indexOut});
    }
  }
  this->indexOut = indexOut;
}

void OrdsBlockTreeTermsWriter::write(shared_ptr<Fields> fields) throw(
    IOException)
{

  wstring lastField = L"";
  for (auto field : fields) {
    assert(lastField == L"" || lastField.compare(field) < 0);
    lastField = field;

    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();

    shared_ptr<BlockTreeTermsWriter::TermsWriter> termsWriter =
        make_shared<BlockTreeTermsWriter::TermsWriter>(
            fieldInfos->fieldInfo(field));
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

int64_t OrdsBlockTreeTermsWriter::encodeOutput(int64_t fp, bool hasTerms,
                                                 bool isFloor)
{
  assert(fp < (1LL << 62));
  return (fp << 2) | (hasTerms ? OUTPUT_FLAG_HAS_TERMS : 0) |
         (isFloor ? OUTPUT_FLAG_IS_FLOOR : 0);
}

OrdsBlockTreeTermsWriter::PendingEntry::PendingEntry(bool isTerm)
    : isTerm(isTerm)
{
}

OrdsBlockTreeTermsWriter::PendingTerm::PendingTerm(
    shared_ptr<BytesRef> term, shared_ptr<BlockTermState> state)
    : PendingEntry(true), termBytes(std::deque<char>(term->length)),
      state(state)
{
  System::arraycopy(term->bytes, term->offset, termBytes, 0, term->length);
}

wstring OrdsBlockTreeTermsWriter::PendingTerm::toString()
{
  return BlockTreeTermsWriter::brToString(termBytes);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
// brToString(org.apache.lucene.util.BytesRef b)
wstring OrdsBlockTreeTermsWriter::brToString(shared_ptr<BytesRef> b)
{
  try {
    return b->utf8ToString() + L" " + b;
  } catch (const runtime_error &t) {
    // If BytesRef isn't actually UTF8, or it's eg a
    // prefix of UTF8 that ends mid-unicode-char, we
    // fallback to hex:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return b->toString();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring brToString(byte[] b)
wstring OrdsBlockTreeTermsWriter::brToString(std::deque<char> &b)
{
  return brToString(make_shared<BytesRef>(b));
}

OrdsBlockTreeTermsWriter::SubIndex::SubIndex(
    shared_ptr<FST<std::shared_ptr<Output>>> index, int64_t termOrdStart)
    : index(index), termOrdStart(termOrdStart)
{
}

OrdsBlockTreeTermsWriter::PendingBlock::PendingBlock(
    shared_ptr<BytesRef> prefix, int64_t fp, bool hasTerms,
    int64_t totalTermCount, bool isFloor, int floorLeadByte,
    deque<std::shared_ptr<SubIndex>> &subIndices)
    : PendingEntry(false), prefix(prefix), fp(fp), hasTerms(hasTerms),
      isFloor(isFloor), floorLeadByte(floorLeadByte),
      totalTermCount(totalTermCount)
{
  assert(totalTermCount > 0);
  this->subIndices = subIndices;
}

wstring OrdsBlockTreeTermsWriter::PendingBlock::toString()
{
  return L"BLOCK: " + BlockTreeTermsWriter::brToString(prefix);
}

void OrdsBlockTreeTermsWriter::PendingBlock::compileIndex(
    deque<std::shared_ptr<BlockTreeTermsWriter::PendingBlock>> &blocks,
    shared_ptr<RAMOutputStream> scratchBytes,
    shared_ptr<IntsRefBuilder> scratchIntsRef) 
{

  assert(isFloor && blocks.size() > 1) ||
      (isFloor == false && blocks.size() == 1)
      : L"isFloor=" + StringHelper::toString(isFloor) + L" blocks=" + blocks;
  assert(shared_from_this() == blocks[0]);

  assert(scratchBytes->getFilePointer() == 0);

  // TODO: try writing the leading vLong in MSB order
  // (opposite of what Lucene does today), for better
  // outputs sharing in the FST
  int64_t lastSumTotalTermCount = 0;
  int64_t sumTotalTermCount = totalTermCount;
  scratchBytes->writeVLong(
      BlockTreeTermsWriter::encodeOutput(fp, hasTerms, isFloor));
  if (isFloor) {
    scratchBytes->writeVInt(blocks.size() - 1);
    for (int i = 1; i < blocks.size(); i++) {
      shared_ptr<BlockTreeTermsWriter::PendingBlock> sub = blocks[i];
      assert(sub->floorLeadByte != -1);
      // if (DEBUG) {
      //  System.out.println("    write floorLeadByte=" +
      //  Integer.toHexString(sub.floorLeadByte&0xff));
      //}
      scratchBytes->writeByte(static_cast<char>(sub->floorLeadByte));
      // System.out.println("  write floor byte=" + (byte) sub.floorLeadByte + "
      // ordShift=" + sumTotalTermCount);
      scratchBytes->writeVLong(sumTotalTermCount - lastSumTotalTermCount);
      lastSumTotalTermCount = sumTotalTermCount;
      sumTotalTermCount += sub->totalTermCount;
      assert(sub->fp > fp);
      scratchBytes->writeVLong((sub->fp - fp) << 1 | (sub->hasTerms ? 1 : 0));
    }
  }

  shared_ptr<Builder<std::shared_ptr<Output>>> *const indexBuilder =
      make_shared<Builder<std::shared_ptr<Output>>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, false, numeric_limits<int>::max(),
          FST_OUTPUTS, true, 15);
  // if (DEBUG) {
  //  System.out.println("  compile index for prefix=" + prefix);
  //}
  // indexBuilder.DEBUG = false;
  const std::deque<char> bytes =
      std::deque<char>(static_cast<int>(scratchBytes->getFilePointer()));
  assert(bytes.size() > 0);
  // System.out.println("  bytes=" + bytes.length);
  scratchBytes->writeTo(bytes, 0);
  indexBuilder->add(
      Util::toIntsRef(prefix, scratchIntsRef),
      FST_OUTPUTS->newOutput(make_shared<BytesRef>(bytes, 0, bytes.size()), 0,
                             numeric_limits<int64_t>::max() -
                                 (sumTotalTermCount - 1)));
  scratchBytes->reset();

  // Copy over index for all sub-blocks

  int64_t termOrdOffset = 0;
  for (auto block : blocks) {
    if (block->subIndices.size() > 0) {
      for (auto subIndex : block->subIndices) {
        append(indexBuilder, subIndex->index,
               termOrdOffset + subIndex->termOrdStart, scratchIntsRef);
      }
      block->subIndices.clear();
    }
    termOrdOffset += block->totalTermCount;
  }
  totFloorTermCount = termOrdOffset;

  assert(sumTotalTermCount == totFloorTermCount);

  index = indexBuilder->finish();
  assert(subIndices.empty());

  /*
  Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"));
  Util.toDot(index, w, false, false);
  System.out.println("SAVED to out.dot");
  w.close();
  */
}

void OrdsBlockTreeTermsWriter::PendingBlock::append(
    shared_ptr<Builder<std::shared_ptr<Output>>> builder,
    shared_ptr<FST<std::shared_ptr<Output>>> subIndex, int64_t termOrdOffset,
    shared_ptr<IntsRefBuilder> scratchIntsRef) 
{
  shared_ptr<BytesRefFSTEnum<std::shared_ptr<Output>>> *const subIndexEnum =
      make_shared<BytesRefFSTEnum<std::shared_ptr<Output>>>(subIndex);
  shared_ptr<BytesRefFSTEnum::InputOutput<std::shared_ptr<Output>>> indexEnt;
  while ((indexEnt = subIndexEnum->next()) != nullptr) {
    // if (DEBUG) {
    //  System.out.println("      add sub=" + indexEnt.input + " " +
    //  indexEnt.input + " output=" + indexEnt.output);
    //}
    shared_ptr<Output> output = indexEnt->output;
    // long blockTermCount = output.endOrd - output.startOrd + 1;
    shared_ptr<Output> newOutput =
        FST_OUTPUTS->newOutput(output->bytes, termOrdOffset + output->startOrd,
                               output->endOrd - termOrdOffset);
    // System.out.println("  append sub=" + indexEnt.input + " output=" +
    // indexEnt.output + " termOrdOffset=" + termOrdOffset + " blockTermCount=" +
    // blockTermCount  + " newOutput=" + newOutput  + " endOrd=" +
    // (termOrdOffset+Long.MAX_VALUE-output.endOrd));
    builder->add(Util::toIntsRef(indexEnt->input, scratchIntsRef), newOutput);
  }
}

void OrdsBlockTreeTermsWriter::TermsWriter::writeBlocks(
    int prefixLength, int count) 
{

  assert(count > 0);

  /*
  if (DEBUG) {
    BytesRef br = new BytesRef(lastTerm.bytes);
    br.offset = lastTerm.offset;
    br.length = prefixLength;
    System.out.println("writeBlocks: " + br.utf8ToString() + " count=" + count);
  }
  */

  // Root block better write all remaining pending entries:
  assert(prefixLength > 0 || count == pending.size());

  int lastSuffixLeadLabel = -1;

  // True if we saw at least one term in this block (we record if a block
  // only points to sub-blocks in the terms index so we can avoid seeking
  // to it when we are looking for a term):
  bool hasTerms = false;
  bool hasSubBlocks = false;

  int start = pending.size() - count;
  int end = pending.size();
  int nextBlockStart = start;
  int nextFloorLeadLabel = -1;

  for (int i = start; i < end; i++) {

    shared_ptr<BlockTreeTermsWriter::PendingEntry> ent = pending[i];

    int suffixLeadLabel;

    if (ent->isTerm) {
      shared_ptr<BlockTreeTermsWriter::PendingTerm> term =
          std::static_pointer_cast<BlockTreeTermsWriter::PendingTerm>(ent);
      if (term->termBytes.size() == prefixLength) {
        // Suffix is 0, i.e. prefix 'foo' and term is
        // 'foo' so the term has empty string suffix
        // in this block
        assert(lastSuffixLeadLabel == -1);
        suffixLeadLabel = -1;
      } else {
        suffixLeadLabel = term->termBytes[prefixLength] & 0xff;
      }
    } else {
      shared_ptr<BlockTreeTermsWriter::PendingBlock> block =
          std::static_pointer_cast<BlockTreeTermsWriter::PendingBlock>(ent);
      assert(block->prefix->length > prefixLength);
      suffixLeadLabel =
          block->prefix->bytes[block->prefix->offset + prefixLength] & 0xff;
    }
    // if (DEBUG) System.out.println("  i=" + i + " ent=" + ent + "
    // suffixLeadLabel=" + suffixLeadLabel);

    if (suffixLeadLabel != lastSuffixLeadLabel) {
      int itemsInBlock = i - nextBlockStart;
      if (itemsInBlock >= outerInstance->minItemsInBlock &&
          end - nextBlockStart > outerInstance->maxItemsInBlock) {
        // The count is too large for one block, so we must break it into
        // "floor" blocks, where we record the leading label of the suffix of
        // the first term in each floor block, so at search time we can jump to
        // the right floor block.  We just use a naive greedy segmenter here:
        // make a new floor block as soon as we have at least minItemsInBlock.
        // This is not always best: it often produces a too-small block as the
        // final block:
        bool isFloor = itemsInBlock < count;
        newBlocks.push_back(writeBlock(prefixLength, isFloor,
                                       nextFloorLeadLabel, nextBlockStart, i,
                                       hasTerms, hasSubBlocks));

        hasTerms = false;
        hasSubBlocks = false;
        nextFloorLeadLabel = suffixLeadLabel;
        nextBlockStart = i;
      }

      lastSuffixLeadLabel = suffixLeadLabel;
    }

    if (ent->isTerm) {
      hasTerms = true;
    } else {
      hasSubBlocks = true;
    }
  }

  // Write last block, if any:
  if (nextBlockStart < end) {
    int itemsInBlock = end - nextBlockStart;
    bool isFloor = itemsInBlock < count;
    newBlocks.push_back(writeBlock(prefixLength, isFloor, nextFloorLeadLabel,
                                   nextBlockStart, end, hasTerms,
                                   hasSubBlocks));
  }

  assert(newBlocks.empty() == false);

  shared_ptr<BlockTreeTermsWriter::PendingBlock> firstBlock = newBlocks[0];

  assert(firstBlock->isFloor || newBlocks.size() == 1);

  firstBlock->compileIndex(newBlocks, outerInstance->scratchBytes,
                           outerInstance->scratchIntsRef);

  // Remove slice from the top of the pending stack, that we just wrote:
  pending.subList(pending.size() - count, pending.size())->clear();

  // Append new block
  pending.push_back(firstBlock);

  newBlocks.clear();
}

shared_ptr<BlockTreeTermsWriter::PendingBlock>
OrdsBlockTreeTermsWriter::TermsWriter::writeBlock(
    int prefixLength, bool isFloor, int floorLeadLabel, int start, int end,
    bool hasTerms, bool hasSubBlocks) 
{

  assert(end > start);

  int64_t startFP = outerInstance->out->getFilePointer();

  // if (DEBUG) System.out.println("    writeBlock fp=" + startFP + " isFloor="
  // + isFloor + " floorLeadLabel=" + floorLeadLabel + " start=" + start + "
  // end=" + end + " hasTerms=" + hasTerms + " hasSubBlocks=" + hasSubBlocks);

  bool hasFloorLeadLabel = isFloor && floorLeadLabel != -1;

  shared_ptr<BytesRef> *const prefix =
      make_shared<BytesRef>(prefixLength + (hasFloorLeadLabel ? 1 : 0));
  System::arraycopy(lastTerm->bytes(), 0, prefix->bytes, 0, prefixLength);
  prefix->length = prefixLength;

  // Write block header:
  int numEntries = end - start;
  int code = numEntries << 1;
  if (end == pending.size()) {
    // Last block:
    code |= 1;
  }
  outerInstance->out->writeVInt(code);

  // if (DEBUG) {
  //   System.out.println("  writeBlock " + (isFloor ? "(floor) " : "") + "seg="
  //   + segment + " pending.size()=" + pending.size() + " prefixLength=" +
  //   prefixLength + " indexPrefix=" + brToString(prefix) + " entCount=" +
  //   length + " startFP=" + startFP + (isFloor ? (" floorLeadByte=" +
  //   Integer.toHexString(floorLeadByte&0xff)) : "") + " isLastInFloor=" +
  //   isLastInFloor);
  // }

  const deque<std::shared_ptr<SubIndex>> subIndices;

  // We optimize the leaf block case (block has only terms), writing a more
  // compact format in this case:
  bool isLeafBlock = hasSubBlocks == false;

  // Number of terms in this block and all sub-blocks (recursively)
  int64_t totalTermCount;

  bool absolute = true;

  if (isLeafBlock) {
    // Only terms:
    subIndices.clear();
    for (int i = start; i < end; i++) {
      shared_ptr<BlockTreeTermsWriter::PendingEntry> ent = pending[i];
      assert((ent->isTerm, L"i=" + to_wstring(i)));

      shared_ptr<BlockTreeTermsWriter::PendingTerm> term =
          std::static_pointer_cast<BlockTreeTermsWriter::PendingTerm>(ent);
      assert((StringHelper::startsWith(term->termBytes, prefix),
              L"term.term=" + term->termBytes + L" prefix=" + prefix));
      shared_ptr<BlockTermState> state = term->state;
      constexpr int suffix = term->termBytes.size() - prefixLength;
      /*
      if (DEBUG) {
        BytesRef suffixBytes = new BytesRef(suffix);
        System.arraycopy(term.term.bytes, prefixLength, suffixBytes.bytes, 0,
      suffix); suffixBytes.length = suffix; System.out.println("    write term
      suffix=" + suffixBytes);
      }
      */
      // For leaf block we write suffix straight
      suffixWriter->writeVInt(suffix);
      suffixWriter->writeBytes(term->termBytes, prefixLength, suffix);
      assert(floorLeadLabel == -1 ||
             (term->termBytes[prefixLength] & 0xff) >= floorLeadLabel);

      // Write term stats, to separate byte[] blob:
      statsWriter->writeVInt(state->docFreq);
      if (fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
        assert((state->totalTermFreq >= state->docFreq,
                to_wstring(state->totalTermFreq) + L" vs " +
                    to_wstring(state->docFreq)));
        statsWriter->writeVLong(state->totalTermFreq - state->docFreq);
      }

      // Write term meta data
      outerInstance->postingsWriter->encodeTerm(longs, bytesWriter, fieldInfo,
                                                state, absolute);
      for (int pos = 0; pos < longsSize; pos++) {
        assert(longs[pos] >= 0);
        metaWriter->writeVLong(longs[pos]);
      }
      bytesWriter->writeTo(metaWriter);
      bytesWriter->reset();
      absolute = false;
    }
    totalTermCount = end - start;
  } else {
    // Mixed terms and sub-blocks:
    subIndices = deque<>();
    totalTermCount = 0;
    for (int i = start; i < end; i++) {
      shared_ptr<BlockTreeTermsWriter::PendingEntry> ent = pending[i];
      if (ent->isTerm) {
        shared_ptr<BlockTreeTermsWriter::PendingTerm> term =
            std::static_pointer_cast<BlockTreeTermsWriter::PendingTerm>(ent);
        assert((StringHelper::startsWith(term->termBytes, prefix),
                L"term.term=" + term->termBytes + L" prefix=" + prefix));
        shared_ptr<BlockTermState> state = term->state;
        constexpr int suffix = term->termBytes.size() - prefixLength;
        /*
        if (DEBUG) {
          BytesRef suffixBytes = new BytesRef(suffix);
          System.arraycopy(term.term.bytes, prefixLength, suffixBytes.bytes, 0,
        suffix); suffixBytes.length = suffix; System.out.println("    write term
        suffix=" + suffixBytes);
        }
        */
        // For non-leaf block we borrow 1 bit to record
        // if entry is term or sub-block
        suffixWriter->writeVInt(suffix << 1);
        suffixWriter->writeBytes(term->termBytes, prefixLength, suffix);
        assert(floorLeadLabel == -1 ||
               (term->termBytes[prefixLength] & 0xff) >= floorLeadLabel);

        // Write term stats, to separate byte[] blob:
        statsWriter->writeVInt(state->docFreq);
        if (fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
          assert(state->totalTermFreq >= state->docFreq);
          statsWriter->writeVLong(state->totalTermFreq - state->docFreq);
        }

        // TODO: now that terms dict "sees" these longs,
        // we can explore better column-stride encodings
        // to encode all long[0]s for this block at
        // once, all long[1]s, etc., e.g. using
        // Simple64.  Alternatively, we could interleave
        // stats + meta ... no reason to have them
        // separate anymore:

        // Write term meta data
        outerInstance->postingsWriter->encodeTerm(longs, bytesWriter, fieldInfo,
                                                  state, absolute);
        for (int pos = 0; pos < longsSize; pos++) {
          assert(longs[pos] >= 0);
          metaWriter->writeVLong(longs[pos]);
        }
        bytesWriter->writeTo(metaWriter);
        bytesWriter->reset();
        absolute = false;

        totalTermCount++;
      } else {
        shared_ptr<BlockTreeTermsWriter::PendingBlock> block =
            std::static_pointer_cast<BlockTreeTermsWriter::PendingBlock>(ent);
        assert((StringHelper::startsWith(block->prefix, prefix)));
        constexpr int suffix = block->prefix->length - prefixLength;

        assert(suffix > 0);

        // For non-leaf block we borrow 1 bit to record
        // if entry is term or sub-block
        suffixWriter->writeVInt((suffix << 1) | 1);
        suffixWriter->writeBytes(block->prefix->bytes, prefixLength, suffix);

        assert(floorLeadLabel == -1 ||
               (block->prefix->bytes[prefixLength] & 0xff) >= floorLeadLabel);

        assert(block->fp < startFP);

        /*
        if (DEBUG) {
          BytesRef suffixBytes = new BytesRef(suffix);
          System.arraycopy(block.prefix.bytes, prefixLength, suffixBytes.bytes,
        0, suffix); suffixBytes.length = suffix; System.out.println("    write
        sub-block suffix=" + brToString(suffixBytes) + " subFP=" + block.fp + "
        subCode=" + (startFP-block.fp) + " floor=" + block.isFloor);
        }
        */

        suffixWriter->writeVLong(startFP - block->fp);
        suffixWriter->writeVLong(block->totFloorTermCount);
        subIndices.push_back(
            make_shared<SubIndex>(block->index, totalTermCount));
        totalTermCount += block->totFloorTermCount;
      }
    }

    assert(subIndices.size() != 0);
  }

  // TODO: we could block-write the term suffix pointers;
  // this would take more space but would enable binary
  // search on lookup

  // Write suffixes byte[] blob to terms dict output:
  outerInstance->out->writeVInt(
      static_cast<int>(suffixWriter->getFilePointer() << 1) |
      (isLeafBlock ? 1 : 0));
  suffixWriter->writeTo(outerInstance->out);
  suffixWriter->reset();

  // Write term stats byte[] blob
  outerInstance->out->writeVInt(
      static_cast<int>(statsWriter->getFilePointer()));
  statsWriter->writeTo(outerInstance->out);
  statsWriter->reset();

  // Write term meta data byte[] blob
  outerInstance->out->writeVInt(static_cast<int>(metaWriter->getFilePointer()));
  metaWriter->writeTo(outerInstance->out);
  metaWriter->reset();

  // if (DEBUG) {
  //   System.out.println("      fpEnd=" + out.getFilePointer());
  // }

  if (hasFloorLeadLabel) {
    // We already allocated to length+1 above:
    prefix->bytes[prefix->length++] = static_cast<char>(floorLeadLabel);
  }

  return make_shared<BlockTreeTermsWriter::PendingBlock>(
      prefix, startFP, hasTerms, totalTermCount, isFloor, floorLeadLabel,
      subIndices);
}

OrdsBlockTreeTermsWriter::TermsWriter::TermsWriter(
    shared_ptr<OrdsBlockTreeTermsWriter> outerInstance,
    shared_ptr<FieldInfo> fieldInfo)
    : fieldInfo(fieldInfo),
      longsSize(outerInstance->postingsWriter->setField(fieldInfo)),
      docsSeen(make_shared<FixedBitSet>(outerInstance->maxDoc)),
      longs(std::deque<int64_t>(longsSize)), outerInstance(outerInstance)
{
}

void OrdsBlockTreeTermsWriter::TermsWriter::write(
    shared_ptr<BytesRef> text,
    shared_ptr<TermsEnum> termsEnum) 
{
  /*
  if (DEBUG) {
    int[] tmp = new int[lastTerm.length];
    System.arraycopy(prefixStarts, 0, tmp, 0, tmp.length);
    System.out.println("BTTW: write term=" + brToString(text) + " prefixStarts="
  + Arrays.toString(tmp) + " pending.size()=" + pending.size());
  }
  */

  shared_ptr<BlockTermState> state =
      outerInstance->postingsWriter->writeTerm(text, termsEnum, docsSeen);
  if (state != nullptr) {
    assert(state->docFreq != 0);
    assert((fieldInfo->getIndexOptions() == IndexOptions::DOCS ||
                state->totalTermFreq >= state->docFreq,
            L"postingsWriter=" + outerInstance->postingsWriter));
    sumDocFreq += state->docFreq;
    sumTotalTermFreq += state->totalTermFreq;
    pushTerm(text);

    shared_ptr<BlockTreeTermsWriter::PendingTerm> term =
        make_shared<BlockTreeTermsWriter::PendingTerm>(
            BytesRef::deepCopyOf(text), state);
    pending.push_back(term);
    numTerms++;
    if (firstPendingTerm == nullptr) {
      firstPendingTerm = term;
    }
    lastPendingTerm = term;
  }
}

void OrdsBlockTreeTermsWriter::TermsWriter::pushTerm(
    shared_ptr<BytesRef> text) 
{
  int limit = min(lastTerm->length(), text->length);

  // Find common prefix between last term and current term:
  int pos = 0;
  while (pos < limit &&
         lastTerm->byteAt(pos) == text->bytes[text->offset + pos]) {
    pos++;
  }

  // if (DEBUG) System.out.println("  shared=" + pos + "  lastTerm.length=" +
  // lastTerm.length);

  // Close the "abandoned" suffix now:
  for (int i = lastTerm->length() - 1; i >= pos; i--) {

    // How many items on top of the stack share the current suffix
    // we are closing:
    int prefixTopSize = pending.size() - prefixStarts[i];
    if (prefixTopSize >= outerInstance->minItemsInBlock) {
      // if (DEBUG) System.out.println("pushTerm i=" + i + " prefixTopSize=" +
      // prefixTopSize + " minItemsInBlock=" + minItemsInBlock);
      writeBlocks(i + 1, prefixTopSize);
      prefixStarts[i] -= prefixTopSize - 1;
    }
  }

  if (prefixStarts.size() < text->length) {
    prefixStarts = ArrayUtil::grow(prefixStarts, text->length);
  }

  // Init new tail:
  for (int i = pos; i < text->length; i++) {
    prefixStarts[i] = pending.size();
  }

  lastTerm->copyBytes(text);
}

void OrdsBlockTreeTermsWriter::TermsWriter::finish() 
{
  if (numTerms > 0) {
    // if (DEBUG) System.out.println("BTTW.finish pending.size()=" +
    // pending.size());

    // TODO: if pending.size() is already 1 with a non-zero prefix length
    // we can save writing a "degenerate" root block, but we have to
    // fix all the places that assume the root block's prefix is the empty
    // string:
    writeBlocks(0, pending.size());

    // We better have one final "root" block:
    assert((pending.size() == 1 && !pending[0]->isTerm,
            L"pending.size()=" + pending.size() + L" pending=" + pending));
    shared_ptr<BlockTreeTermsWriter::PendingBlock> *const root =
        std::static_pointer_cast<BlockTreeTermsWriter::PendingBlock>(
            pending[0]);
    assert(root->prefix->length == 0);
    assert(root->index->getEmptyOutput() != nullptr);

    // Write FST to index
    indexStartFP = outerInstance->indexOut->getFilePointer();
    root->index->save(outerInstance->indexOut);
    // System.out.println("  write FST " + indexStartFP + " field=" +
    // fieldInfo.name);

    // if (SAVE_DOT_FILES || DEBUG) {
    //   final std::wstring dotFileName = segment + "_" + fieldInfo.name + ".dot";
    //   Writer w = new OutputStreamWriter(new FileOutputStream(dotFileName));
    //   Util.toDot(root.index, w, false, false);
    //   System.out.println("SAVED to " + dotFileName);
    //   w.close();
    // }

    assert(firstPendingTerm != nullptr);
    shared_ptr<BytesRef> minTerm =
        make_shared<BytesRef>(firstPendingTerm->termBytes);

    assert(lastPendingTerm != nullptr);
    shared_ptr<BytesRef> maxTerm =
        make_shared<BytesRef>(lastPendingTerm->termBytes);

    outerInstance->fields.push_back(
        make_shared<BlockTreeTermsWriter::FieldMetaData>(
            fieldInfo,
            (std::static_pointer_cast<BlockTreeTermsWriter::PendingBlock>(
                 pending[0]))
                ->index->getEmptyOutput(),
            numTerms, indexStartFP, sumTotalTermFreq, sumDocFreq,
            docsSeen->cardinality(), longsSize, minTerm, maxTerm));
  } else {
    assert(docsSeen->cardinality() == 0);
  }
}

OrdsBlockTreeTermsWriter::~OrdsBlockTreeTermsWriter()
{
  if (closed) {
    return;
  }
  closed = true;

  bool success = false;
  try {

    constexpr int64_t dirStart = out->getFilePointer();
    constexpr int64_t indexDirStart = indexOut->getFilePointer();

    out->writeVInt(fields.size());

    for (auto field : fields) {
      // System.out.println("  field " + field.fieldInfo.name + " " +
      // field.numTerms + " terms longsSize=" + field.longsSize);
      out->writeVInt(field->fieldInfo->number);
      assert(field->numTerms > 0);
      out->writeVLong(field->numTerms);
      out->writeVInt(field->rootCode->bytes.size());
      out->writeBytes(field->rootCode->bytes.bytes,
                      field->rootCode->bytes.offset,
                      field->rootCode->bytes.size());
      if (field->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
        out->writeVLong(field->sumTotalTermFreq);
      }
      out->writeVLong(field->sumDocFreq);
      out->writeVInt(field->docCount);
      out->writeVInt(field->longsSize);
      indexOut->writeVLong(field->indexStartFP);
      writeBytesRef(out, field->minTerm);
      writeBytesRef(out, field->maxTerm);
    }
    out->writeLong(dirStart);
    CodecUtil::writeFooter(out);
    indexOut->writeLong(indexDirStart);
    CodecUtil::writeFooter(indexOut);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({out, indexOut, postingsWriter});
    } else {
      IOUtils::closeWhileHandlingException({out, indexOut, postingsWriter});
    }
  }
}

void OrdsBlockTreeTermsWriter::writeBytesRef(
    shared_ptr<IndexOutput> out, shared_ptr<BytesRef> bytes) 
{
  out->writeVInt(bytes->length);
  out->writeBytes(bytes->bytes, bytes->offset, bytes->length);
}
} // namespace org::apache::lucene::codecs::blocktreeords