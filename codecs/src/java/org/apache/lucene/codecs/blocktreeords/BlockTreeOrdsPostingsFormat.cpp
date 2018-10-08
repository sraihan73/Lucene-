using namespace std;

#include "BlockTreeOrdsPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsReaderBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/blocktree/BlockTreeTermsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/lucene50/Lucene50PostingsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/lucene50/Lucene50PostingsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "OrdsBlockTreeTermsReader.h"
#include "OrdsBlockTreeTermsWriter.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using Lucene50PostingsReader =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsReader;
using Lucene50PostingsWriter =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IOUtils = org::apache::lucene::util::IOUtils;

BlockTreeOrdsPostingsFormat::BlockTreeOrdsPostingsFormat()
    : BlockTreeOrdsPostingsFormat(
          OrdsBlockTreeTermsWriter::DEFAULT_MIN_BLOCK_SIZE,
          OrdsBlockTreeTermsWriter::DEFAULT_MAX_BLOCK_SIZE)
{
}

BlockTreeOrdsPostingsFormat::BlockTreeOrdsPostingsFormat(int minTermBlockSize,
                                                         int maxTermBlockSize)
    : org::apache::lucene::codecs::PostingsFormat(L"BlockTreeOrds"),
      minTermBlockSize(minTermBlockSize), maxTermBlockSize(maxTermBlockSize)
{
  BlockTreeTermsWriter::validateSettings(minTermBlockSize, maxTermBlockSize);
}

wstring BlockTreeOrdsPostingsFormat::toString()
{
  return getName() + L"(blocksize=" + to_wstring(BLOCK_SIZE) + L")";
}

shared_ptr<FieldsConsumer> BlockTreeOrdsPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PostingsWriterBase> postingsWriter =
      make_shared<Lucene50PostingsWriter>(state);

  bool success = false;
  try {
    shared_ptr<FieldsConsumer> ret = make_shared<OrdsBlockTreeTermsWriter>(
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

shared_ptr<FieldsProducer> BlockTreeOrdsPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<PostingsReaderBase> postingsReader =
      make_shared<Lucene50PostingsReader>(state);
  bool success = false;
  try {
    shared_ptr<FieldsProducer> ret =
        make_shared<OrdsBlockTreeTermsReader>(postingsReader, state);
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
} // namespace org::apache::lucene::codecs::blocktreeords