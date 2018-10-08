using namespace std;

#include "BlockTreeTermsWriter.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/Fields.h"
#include "../../index/IndexFileNames.h"
#include "../../index/IndexOptions.h"
#include "../../index/SegmentWriteState.h"
#include "../../index/Terms.h"
#include "../../index/TermsEnum.h"
#include "../../store/IndexOutput.h"
#include "../../store/RAMOutputStream.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/BytesRefBuilder.h"
#include "../../util/FixedBitSet.h"
#include "../../util/IOUtils.h"
#include "../../util/IntsRefBuilder.h"
#include "../../util/StringHelper.h"
#include "../../util/fst/Builder.h"
#include "../../util/fst/ByteSequenceOutputs.h"
#include "../../util/fst/BytesRefFSTEnum.h"
#include "../../util/fst/Util.h"
#include "../BlockTermState.h"
#include "../CodecUtil.h"
#include "../PostingsWriterBase.h"
#include "BlockTreeTermsReader.h"

namespace org::apache::lucene::codecs::blocktree
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
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;
using Builder = org::apache::lucene::util::fst::Builder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;

BlockTreeTermsWriter::FieldMetaData::FieldMetaData(
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<BytesRef> rootCode,
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

BlockTreeTermsWriter::BlockTreeTermsWriter(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<PostingsWriterBase> postingsWriter, int minItemsInBlock,
    int maxItemsInBlock) 
    : termsOut(state->directory->createOutput(termsName, state->context)),
      maxDoc(state->segmentInfo->maxDoc()), minItemsInBlock(minItemsInBlock),
      maxItemsInBlock(maxItemsInBlock), postingsWriter(postingsWriter),
      fieldInfos(state->fieldInfos)
{
  validateSettings(minItemsInBlock, maxItemsInBlock);

  const wstring termsName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      BlockTreeTermsReader::TERMS_EXTENSION);
  bool success = false;
  shared_ptr<IndexOutput> indexOut = nullptr;
  try {
    CodecUtil::writeIndexHeader(
        termsOut, BlockTreeTermsReader::TERMS_CODEC_NAME,
        BlockTreeTermsReader::VERSION_CURRENT, state->segmentInfo->getId(),
        state->segmentSuffix);

    const wstring indexName = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix,
        BlockTreeTermsReader::TERMS_INDEX_EXTENSION);
    indexOut = state->directory->createOutput(indexName, state->context);
    CodecUtil::writeIndexHeader(
        indexOut, BlockTreeTermsReader::TERMS_INDEX_CODEC_NAME,
        BlockTreeTermsReader::VERSION_CURRENT, state->segmentInfo->getId(),
        state->segmentSuffix);
    // segment = state.segmentInfo.name;

    postingsWriter->init(termsOut,
                         state); // have consumer write its format/header

    this->indexOut = indexOut;
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({termsOut, indexOut});
    }
  }
}

void BlockTreeTermsWriter::writeTrailer(shared_ptr<IndexOutput> out,
                                        int64_t dirStart) 
{
  out->writeLong(dirStart);
}

void BlockTreeTermsWriter::writeIndexTrailer(
    shared_ptr<IndexOutput> indexOut, int64_t dirStart) 
{
  indexOut->writeLong(dirStart);
}

void BlockTreeTermsWriter::validateSettings(int minItemsInBlock,
                                            int maxItemsInBlock)
{
  if (minItemsInBlock <= 1) {
    throw invalid_argument(L"minItemsInBlock must be >= 2; got " +
                           to_wstring(minItemsInBlock));
  }
  if (minItemsInBlock > maxItemsInBlock) {
    throw invalid_argument(
        L"maxItemsInBlock must be >= minItemsInBlock; got maxItemsInBlock=" +
        to_wstring(maxItemsInBlock) + L" minItemsInBlock=" +
        to_wstring(minItemsInBlock));
  }
  if (2 * (minItemsInBlock - 1) > maxItemsInBlock) {
    throw invalid_argument(L"maxItemsInBlock must be at least "
                           L"2*(minItemsInBlock-1); got maxItemsInBlock=" +
                           to_wstring(maxItemsInBlock) + L" minItemsInBlock=" +
                           to_wstring(minItemsInBlock));
  }
}

void BlockTreeTermsWriter::write(shared_ptr<Fields> fields) 
{
  // if (DEBUG) System.out.println("\nBTTW.write seg=" + segment);

  wstring lastField = L"";
  for (auto field : fields) {
    assert(lastField == L"" || lastField.compare(field) < 0);
    lastField = field;

    // if (DEBUG) System.out.println("\nBTTW.write seg=" + segment + " field=" +
    // field);
    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();
    shared_ptr<TermsWriter> termsWriter = make_shared<TermsWriter>(
        shared_from_this(), fieldInfos->fieldInfo(field));
    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      // if (DEBUG) System.out.println("BTTW: next term " + term);

      if (term == nullptr) {
        break;
      }

      // if (DEBUG) System.out.println("write field=" + fieldInfo.name + "
      // term=" + brToString(term));
      termsWriter->write(term, termsEnum);
    }

    termsWriter->finish();

    // if (DEBUG) System.out.println("\nBTTW.write done seg=" + segment + "
    // field=" + field);
  }
}

int64_t BlockTreeTermsWriter::encodeOutput(int64_t fp, bool hasTerms,
                                             bool isFloor)
{
  assert(fp < (1LL << 62));
  return (fp << 2) |
         (hasTerms ? BlockTreeTermsReader::OUTPUT_FLAG_HAS_TERMS : 0) |
         (isFloor ? BlockTreeTermsReader::OUTPUT_FLAG_IS_FLOOR : 0);
}

BlockTreeTermsWriter::PendingEntry::PendingEntry(bool isTerm) : isTerm(isTerm)
{
}

BlockTreeTermsWriter::PendingTerm::PendingTerm(shared_ptr<BytesRef> term,
                                               shared_ptr<BlockTermState> state)
    : PendingEntry(true), termBytes(std::deque<char>(term->length)),
      state(state)
{
  System::arraycopy(term->bytes, term->offset, termBytes, 0, term->length);
}

wstring BlockTreeTermsWriter::PendingTerm::toString()
{
  return L"TERM: " + brToString(termBytes);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
// brToString(org.apache.lucene.util.BytesRef b)
wstring BlockTreeTermsWriter::brToString(shared_ptr<BytesRef> b)
{
  if (b == nullptr) {
    return L"(null)";
  } else {
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
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring brToString(byte[] b)
wstring BlockTreeTermsWriter::brToString(std::deque<char> &b)
{
  return brToString(make_shared<BytesRef>(b));
}

BlockTreeTermsWriter::PendingBlock::PendingBlock(
    shared_ptr<BytesRef> prefix, int64_t fp, bool hasTerms, bool isFloor,
    int floorLeadByte, deque<FST<std::shared_ptr<BytesRef>>> &subIndices)
    : PendingEntry(false), prefix(prefix), fp(fp), hasTerms(hasTerms),
      isFloor(isFloor), floorLeadByte(floorLeadByte)
{
  this->subIndices = subIndices;
}

wstring BlockTreeTermsWriter::PendingBlock::toString()
{
  return L"BLOCK: prefix=" + brToString(prefix);
}

void BlockTreeTermsWriter::PendingBlock::compileIndex(
    deque<std::shared_ptr<PendingBlock>> &blocks,
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
  scratchBytes->writeVLong(encodeOutput(fp, hasTerms, isFloor));
  if (isFloor) {
    scratchBytes->writeVInt(blocks.size() - 1);
    for (int i = 1; i < blocks.size(); i++) {
      shared_ptr<PendingBlock> sub = blocks[i];
      assert(sub->floorLeadByte != -1);
      // if (DEBUG) {
      //  System.out.println("    write floorLeadByte=" +
      //  Integer.toHexString(sub.floorLeadByte&0xff));
      //}
      scratchBytes->writeByte(static_cast<char>(sub->floorLeadByte));
      assert(sub->fp > fp);
      scratchBytes->writeVLong((sub->fp - fp) << 1 | (sub->hasTerms ? 1 : 0));
    }
  }

  shared_ptr<ByteSequenceOutputs> *const outputs =
      ByteSequenceOutputs::getSingleton();
  shared_ptr<Builder<std::shared_ptr<BytesRef>>> *const indexBuilder =
      make_shared<Builder<std::shared_ptr<BytesRef>>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, false, numeric_limits<int>::max(),
          outputs, true, 15);
  // if (DEBUG) {
  //  System.out.println("  compile index for prefix=" + prefix);
  //}
  // indexBuilder.DEBUG = false;
  const std::deque<char> bytes =
      std::deque<char>(static_cast<int>(scratchBytes->getFilePointer()));
  assert(bytes.size() > 0);
  scratchBytes->writeTo(bytes, 0);
  indexBuilder->add(Util::toIntsRef(prefix, scratchIntsRef),
                    make_shared<BytesRef>(bytes, 0, bytes.size()));
  scratchBytes->reset();

  // Copy over index for all sub-blocks
  for (auto block : blocks) {
    if (block->subIndices.size() > 0) {
      for (auto subIndex : block->subIndices) {
        append(indexBuilder, subIndex, scratchIntsRef);
      }
      block->subIndices.clear();
    }
  }

  index = indexBuilder->finish();

  assert(subIndices.empty());

  /*
  Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"));
  Util.toDot(index, w, false, false);
  System.out.println("SAVED to out.dot");
  w.close();
  */
}

void BlockTreeTermsWriter::PendingBlock::append(
    shared_ptr<Builder<std::shared_ptr<BytesRef>>> builder,
    shared_ptr<FST<std::shared_ptr<BytesRef>>> subIndex,
    shared_ptr<IntsRefBuilder> scratchIntsRef) 
{
  shared_ptr<BytesRefFSTEnum<std::shared_ptr<BytesRef>>> *const subIndexEnum =
      make_shared<BytesRefFSTEnum<std::shared_ptr<BytesRef>>>(subIndex);
  shared_ptr<BytesRefFSTEnum::InputOutput<std::shared_ptr<BytesRef>>> indexEnt;
  while ((indexEnt = subIndexEnum->next()) != nullptr) {
    // if (DEBUG) {
    //  System.out.println("      add sub=" + indexEnt.input + " " +
    //  indexEnt.input + " output=" + indexEnt.output);
    //}
    builder->add(Util::toIntsRef(indexEnt->input, scratchIntsRef),
                 indexEnt->output);
  }
}

const shared_ptr<org::apache::lucene::util::BytesRef>
    BlockTreeTermsWriter::EMPTY_BYTES_REF =
        make_shared<org::apache::lucene::util::BytesRef>();

void BlockTreeTermsWriter::TermsWriter::writeBlocks(
    int prefixLength, int count) 
{

  assert(count > 0);

  // if (DEBUG2) {
  //  BytesRef br = new BytesRef(lastTerm.bytes());
  //  br.length = prefixLength;
  //  System.out.println("writeBlocks: seg=" + segment + " prefix=" +
  //  brToString(br) + " count=" + count);
  //}

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

    shared_ptr<PendingEntry> ent = pending[i];

    int suffixLeadLabel;

    if (ent->isTerm) {
      shared_ptr<PendingTerm> term = std::static_pointer_cast<PendingTerm>(ent);
      if (term->termBytes.size() == prefixLength) {
        // Suffix is 0, i.e. prefix 'foo' and term is
        // 'foo' so the term has empty string suffix
        // in this block
        assert((lastSuffixLeadLabel == -1,
                L"i=" + to_wstring(i) + L" lastSuffixLeadLabel=" +
                    to_wstring(lastSuffixLeadLabel)));
        suffixLeadLabel = -1;
      } else {
        suffixLeadLabel = term->termBytes[prefixLength] & 0xff;
      }
    } else {
      shared_ptr<PendingBlock> block =
          std::static_pointer_cast<PendingBlock>(ent);
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

  shared_ptr<PendingBlock> firstBlock = newBlocks[0];

  assert(firstBlock->isFloor || newBlocks.size() == 1);

  firstBlock->compileIndex(newBlocks, outerInstance->scratchBytes,
                           outerInstance->scratchIntsRef);

  // Remove slice from the top of the pending stack, that we just wrote:
  pending.subList(pending.size() - count, pending.size())->clear();

  // Append new block
  pending.push_back(firstBlock);

  newBlocks.clear();
}

shared_ptr<PendingBlock> BlockTreeTermsWriter::TermsWriter::writeBlock(
    int prefixLength, bool isFloor, int floorLeadLabel, int start, int end,
    bool hasTerms, bool hasSubBlocks) 
{

  assert(end > start);

  int64_t startFP = outerInstance->termsOut->getFilePointer();

  bool hasFloorLeadLabel = isFloor && floorLeadLabel != -1;

  shared_ptr<BytesRef> *const prefix =
      make_shared<BytesRef>(prefixLength + (hasFloorLeadLabel ? 1 : 0));
  System::arraycopy(lastTerm->get().bytes, 0, prefix->bytes, 0, prefixLength);
  prefix->length = prefixLength;

  // if (DEBUG2) System.out.println("    writeBlock field=" + fieldInfo.name + "
  // prefix=" + brToString(prefix) + " fp=" + startFP + " isFloor=" + isFloor + "
  // isLastInFloor=" + (end == pending.size()) + " floorLeadLabel=" +
  // floorLeadLabel + " start=" + start + " end=" + end + " hasTerms=" + hasTerms
  // + " hasSubBlocks=" + hasSubBlocks);

  // Write block header:
  int numEntries = end - start;
  int code = numEntries << 1;
  if (end == pending.size()) {
    // Last block:
    code |= 1;
  }
  outerInstance->termsOut->writeVInt(code);

  /*
  if (DEBUG) {
    System.out.println("  writeBlock " + (isFloor ? "(floor) " : "") + "seg=" +
  segment + " pending.size()=" + pending.size() + " prefixLength=" +
  prefixLength + " indexPrefix=" + brToString(prefix) + " entCount=" +
  (end-start+1) + " startFP=" + startFP + (isFloor ? (" floorLeadLabel=" +
  Integer.toHexString(floorLeadLabel)) : ""));
  }
  */

  // 1st pass: pack term suffix bytes into byte[] blob
  // TODO: cutover to bulk int codec... simple64?

  // We optimize the leaf block case (block has only terms), writing a more
  // compact format in this case:
  bool isLeafBlock = hasSubBlocks == false;

  // System.out.println("  isLeaf=" + isLeafBlock);

  const deque<FST<std::shared_ptr<BytesRef>>> subIndices;

  bool absolute = true;

  if (isLeafBlock) {
    // Block contains only ordinary terms:
    subIndices.clear();
    for (int i = start; i < end; i++) {
      shared_ptr<PendingEntry> ent = pending[i];
      assert((ent->isTerm, L"i=" + to_wstring(i)));

      shared_ptr<PendingTerm> term = std::static_pointer_cast<PendingTerm>(ent);

      assert((StringHelper::startsWith(term->termBytes, prefix),
              L"term.term=" + term->termBytes + L" prefix=" + prefix));
      shared_ptr<BlockTermState> state = term->state;
      constexpr int suffix = term->termBytes.size() - prefixLength;
      // if (DEBUG2) {
      //  BytesRef suffixBytes = new BytesRef(suffix);
      //  System.arraycopy(term.termBytes, prefixLength, suffixBytes.bytes, 0,
      //  suffix); suffixBytes.length = suffix; System.out.println("    write
      //  term suffix=" + brToString(suffixBytes));
      //}

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
  } else {
    // Block has at least one prefix term or a sub block:
    subIndices = deque<>();
    for (int i = start; i < end; i++) {
      shared_ptr<PendingEntry> ent = pending[i];
      if (ent->isTerm) {
        shared_ptr<PendingTerm> term =
            std::static_pointer_cast<PendingTerm>(ent);

        assert((StringHelper::startsWith(term->termBytes, prefix),
                L"term.term=" + term->termBytes + L" prefix=" + prefix));
        shared_ptr<BlockTermState> state = term->state;
        constexpr int suffix = term->termBytes.size() - prefixLength;
        // if (DEBUG2) {
        //  BytesRef suffixBytes = new BytesRef(suffix);
        //  System.arraycopy(term.termBytes, prefixLength, suffixBytes.bytes, 0,
        //  suffix); suffixBytes.length = suffix; System.out.println(" write
        //  term suffix=" + brToString(suffixBytes));
        //}

        // For non-leaf block we borrow 1 bit to record
        // if entry is term or sub-block, and 1 bit to record if
        // it's a prefix term.  Terms cannot be larger than ~32 KB
        // so we won't run out of bits:

        suffixWriter->writeVInt(suffix << 1);
        suffixWriter->writeBytes(term->termBytes, prefixLength, suffix);

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
      } else {
        shared_ptr<PendingBlock> block =
            std::static_pointer_cast<PendingBlock>(ent);
        assert((StringHelper::startsWith(block->prefix, prefix)));
        constexpr int suffix = block->prefix->length - prefixLength;
        assert((StringHelper::startsWith(block->prefix, prefix)));

        assert(suffix > 0);

        // For non-leaf block we borrow 1 bit to record
        // if entry is term or sub-block:f
        suffixWriter->writeVInt((suffix << 1) | 1);
        suffixWriter->writeBytes(block->prefix->bytes, prefixLength, suffix);

        // if (DEBUG2) {
        //  BytesRef suffixBytes = new BytesRef(suffix);
        //  System.arraycopy(block.prefix.bytes, prefixLength,
        //  suffixBytes.bytes, 0, suffix); suffixBytes.length = suffix;
        //  System.out.println("      write sub-block suffix=" +
        //  brToString(suffixBytes) + " subFP=" + block.fp + " subCode=" +
        //  (startFP-block.fp) + " floor=" + block.isFloor);
        //}

        assert(
            (floorLeadLabel == -1 ||
                 (block->prefix->bytes[prefixLength] & 0xff) >= floorLeadLabel,
             L"floorLeadLabel=" + to_wstring(floorLeadLabel) + L" suffixLead=" +
                 to_wstring(block->prefix->bytes[prefixLength] & 0xff)));
        assert(block->fp < startFP);

        suffixWriter->writeVLong(startFP - block->fp);
        subIndices.push_back(block->index);
      }
    }

    assert(subIndices.size() != 0);
  }

  // TODO: we could block-write the term suffix pointers;
  // this would take more space but would enable binary
  // search on lookup

  // Write suffixes byte[] blob to terms dict output:
  outerInstance->termsOut->writeVInt(
      static_cast<int>(suffixWriter->getFilePointer() << 1) |
      (isLeafBlock ? 1 : 0));
  suffixWriter->writeTo(outerInstance->termsOut);
  suffixWriter->reset();

  // Write term stats byte[] blob
  outerInstance->termsOut->writeVInt(
      static_cast<int>(statsWriter->getFilePointer()));
  statsWriter->writeTo(outerInstance->termsOut);
  statsWriter->reset();

  // Write term meta data byte[] blob
  outerInstance->termsOut->writeVInt(
      static_cast<int>(metaWriter->getFilePointer()));
  metaWriter->writeTo(outerInstance->termsOut);
  metaWriter->reset();

  // if (DEBUG) {
  //   System.out.println("      fpEnd=" + out.getFilePointer());
  // }

  if (hasFloorLeadLabel) {
    // We already allocated to length+1 above:
    prefix->bytes[prefix->length++] = static_cast<char>(floorLeadLabel);
  }

  return make_shared<PendingBlock>(prefix, startFP, hasTerms, isFloor,
                                   floorLeadLabel, subIndices);
}

BlockTreeTermsWriter::TermsWriter::TermsWriter(
    shared_ptr<BlockTreeTermsWriter> outerInstance,
    shared_ptr<FieldInfo> fieldInfo)
    : fieldInfo(fieldInfo),
      longsSize(outerInstance->postingsWriter->setField(fieldInfo)),
      docsSeen(make_shared<FixedBitSet>(outerInstance->maxDoc)),
      longs(std::deque<int64_t>(longsSize)), outerInstance(outerInstance)
{
  assert(fieldInfo->getIndexOptions() != IndexOptions::NONE);
}

void BlockTreeTermsWriter::TermsWriter::write(
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
    pushTerm(text);

    shared_ptr<PendingTerm> term = make_shared<PendingTerm>(text, state);
    pending.push_back(term);
    // if (DEBUG) System.out.println("    add pending term = " + text + "
    // pending.size()=" + pending.size());

    sumDocFreq += state->docFreq;
    sumTotalTermFreq += state->totalTermFreq;
    numTerms++;
    if (firstPendingTerm == nullptr) {
      firstPendingTerm = term;
    }
    lastPendingTerm = term;
  }
}

void BlockTreeTermsWriter::TermsWriter::pushTerm(
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

void BlockTreeTermsWriter::TermsWriter::finish() 
{
  if (numTerms > 0) {
    // if (DEBUG) System.out.println("BTTW: finish prefixStarts=" +
    // Arrays.toString(prefixStarts));

    // Add empty term to force closing of all final blocks:
    pushTerm(make_shared<BytesRef>());

    // TODO: if pending.size() is already 1 with a non-zero prefix length
    // we can save writing a "degenerate" root block, but we have to
    // fix all the places that assume the root block's prefix is the empty
    // string:
    pushTerm(make_shared<BytesRef>());
    writeBlocks(0, pending.size());

    // We better have one final "root" block:
    assert((pending.size() == 1 && !pending[0]->isTerm,
            L"pending.size()=" + pending.size() + L" pending=" + pending));
    shared_ptr<PendingBlock> *const root =
        std::static_pointer_cast<PendingBlock>(pending[0]);
    assert(root->prefix->length == 0);
    assert(root->index->getEmptyOutput() != nullptr);

    // Write FST to index
    indexStartFP = outerInstance->indexOut->getFilePointer();
    root->index->save(outerInstance->indexOut);
    // System.out.println("  write FST " + indexStartFP + " field=" +
    // fieldInfo.name);

    /*
    if (DEBUG) {
      final std::wstring dotFileName = segment + "_" + fieldInfo.name + ".dot";
      Writer w = new OutputStreamWriter(new FileOutputStream(dotFileName));
      Util.toDot(root.index, w, false, false);
      System.out.println("SAVED to " + dotFileName);
      w.close();
    }
    */
    assert(firstPendingTerm != nullptr);
    shared_ptr<BytesRef> minTerm =
        make_shared<BytesRef>(firstPendingTerm->termBytes);

    assert(lastPendingTerm != nullptr);
    shared_ptr<BytesRef> maxTerm =
        make_shared<BytesRef>(lastPendingTerm->termBytes);

    outerInstance->fields.push_back(make_shared<FieldMetaData>(
        fieldInfo,
        (std::static_pointer_cast<PendingBlock>(pending[0]))
            ->index->getEmptyOutput(),
        numTerms, indexStartFP, sumTotalTermFreq, sumDocFreq,
        docsSeen->cardinality(), longsSize, minTerm, maxTerm));
  } else {
    assert(sumTotalTermFreq == 0 ||
           fieldInfo->getIndexOptions() == IndexOptions::DOCS &&
               sumTotalTermFreq == -1);
    assert(sumDocFreq == 0);
    assert(docsSeen->cardinality() == 0);
  }
}

BlockTreeTermsWriter::~BlockTreeTermsWriter()
{
  if (closed) {
    return;
  }
  closed = true;

  bool success = false;
  try {

    constexpr int64_t dirStart = termsOut->getFilePointer();
    constexpr int64_t indexDirStart = indexOut->getFilePointer();

    termsOut->writeVInt(fields.size());

    for (auto field : fields) {
      // System.out.println("  field " + field.fieldInfo.name + " " +
      // field.numTerms + " terms");
      termsOut->writeVInt(field->fieldInfo->number);
      assert(field->numTerms > 0);
      termsOut->writeVLong(field->numTerms);
      termsOut->writeVInt(field->rootCode->length);
      termsOut->writeBytes(field->rootCode->bytes, field->rootCode->offset,
                           field->rootCode->length);
      assert(field->fieldInfo->getIndexOptions() != IndexOptions::NONE);
      if (field->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
        termsOut->writeVLong(field->sumTotalTermFreq);
      }
      termsOut->writeVLong(field->sumDocFreq);
      termsOut->writeVInt(field->docCount);
      termsOut->writeVInt(field->longsSize);
      indexOut->writeVLong(field->indexStartFP);
      writeBytesRef(termsOut, field->minTerm);
      writeBytesRef(termsOut, field->maxTerm);
    }
    writeTrailer(termsOut, dirStart);
    CodecUtil::writeFooter(termsOut);
    writeIndexTrailer(indexOut, indexDirStart);
    CodecUtil::writeFooter(indexOut);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({termsOut, indexOut, postingsWriter});
    } else {
      IOUtils::closeWhileHandlingException(
          {termsOut, indexOut, postingsWriter});
    }
  }
}

void BlockTreeTermsWriter::writeBytesRef(
    shared_ptr<IndexOutput> out, shared_ptr<BytesRef> bytes) 
{
  out->writeVInt(bytes->length);
  out->writeBytes(bytes->bytes, bytes->offset, bytes->length);
}
} // namespace org::apache::lucene::codecs::blocktree