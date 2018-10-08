using namespace std;

#include "Lucene50PostingsFormat.h"
#include "../../index/SegmentReadState.h"
#include "../../index/SegmentWriteState.h"
#include "../../index/TermState.h"
#include "../../util/IOUtils.h"
#include "../FieldsConsumer.h"
#include "../FieldsProducer.h"
#include "../PostingsReaderBase.h"
#include "../PostingsWriterBase.h"
#include "../blocktree/BlockTreeTermsReader.h"
#include "../blocktree/BlockTreeTermsWriter.h"
#include "Lucene50PostingsReader.h"
#include "Lucene50PostingsWriter.h"

namespace org::apache::lucene::codecs::lucene50
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using MultiLevelSkipListWriter =
    org::apache::lucene::codecs::MultiLevelSkipListWriter;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using BlockTreeTermsReader =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsReader;
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermState = org::apache::lucene::index::TermState;
using DataOutput = org::apache::lucene::store::DataOutput;
using IOUtils = org::apache::lucene::util::IOUtils;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
const wstring Lucene50PostingsFormat::DOC_EXTENSION = L"doc";
const wstring Lucene50PostingsFormat::POS_EXTENSION = L"pos";
const wstring Lucene50PostingsFormat::PAY_EXTENSION = L"pay";
const wstring Lucene50PostingsFormat::TERMS_CODEC =
    L"Lucene50PostingsWriterTerms";
const wstring Lucene50PostingsFormat::DOC_CODEC = L"Lucene50PostingsWriterDoc";
const wstring Lucene50PostingsFormat::POS_CODEC = L"Lucene50PostingsWriterPos";
const wstring Lucene50PostingsFormat::PAY_CODEC = L"Lucene50PostingsWriterPay";

Lucene50PostingsFormat::Lucene50PostingsFormat()
    : Lucene50PostingsFormat(BlockTreeTermsWriter::DEFAULT_MIN_BLOCK_SIZE,
                             BlockTreeTermsWriter::DEFAULT_MAX_BLOCK_SIZE)
{
}

Lucene50PostingsFormat::Lucene50PostingsFormat(int minTermBlockSize,
                                               int maxTermBlockSize)
    : org::apache::lucene::codecs::PostingsFormat(L"Lucene50"),
      minTermBlockSize(minTermBlockSize), maxTermBlockSize(maxTermBlockSize)
{
  BlockTreeTermsWriter::validateSettings(minTermBlockSize, maxTermBlockSize);
}

wstring Lucene50PostingsFormat::toString()
{
  return getName() + L"(blocksize=" + to_wstring(BLOCK_SIZE) + L")";
}

shared_ptr<FieldsConsumer> Lucene50PostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PostingsWriterBase> postingsWriter =
      make_shared<Lucene50PostingsWriter>(state);

  bool success = false;
  try {
    shared_ptr<FieldsConsumer> ret = make_shared<BlockTreeTermsWriter>(
        state, postingsWriter, minTermBlockSize, maxTermBlockSize);
    success = true;
    return ret;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({postingsWriter});
    }
  }
}

shared_ptr<FieldsProducer> Lucene50PostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<PostingsReaderBase> postingsReader =
      make_shared<Lucene50PostingsReader>(state);
  bool success = false;
  try {
    shared_ptr<FieldsProducer> ret =
        make_shared<BlockTreeTermsReader>(postingsReader, state);
    success = true;
    return ret;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({postingsReader});
    }
  }
}

shared_ptr<IntBlockTermState> Lucene50PostingsFormat::IntBlockTermState::clone()
{
  shared_ptr<IntBlockTermState> other = make_shared<IntBlockTermState>();
  other->copyFrom(shared_from_this());
  return other;
}

void Lucene50PostingsFormat::IntBlockTermState::copyFrom(
    shared_ptr<TermState> _other)
{
  BlockTermState::copyFrom(_other);
  shared_ptr<IntBlockTermState> other =
      std::static_pointer_cast<IntBlockTermState>(_other);
  docStartFP = other->docStartFP;
  posStartFP = other->posStartFP;
  payStartFP = other->payStartFP;
  lastPosBlockOffset = other->lastPosBlockOffset;
  skipOffset = other->skipOffset;
  singletonDocID = other->singletonDocID;
}

wstring Lucene50PostingsFormat::IntBlockTermState::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return BlockTermState::toString() + L" docStartFP=" + to_wstring(docStartFP) +
         L" posStartFP=" + to_wstring(posStartFP) + L" payStartFP=" +
         to_wstring(payStartFP) + L" lastPosBlockOffset=" +
         to_wstring(lastPosBlockOffset) + L" singletonDocID=" +
         to_wstring(singletonDocID);
}
} // namespace org::apache::lucene::codecs::lucene50