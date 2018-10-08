using namespace std;

#include "IDVersionPostingsFormat.h"

namespace org::apache::lucene::codecs::idversion
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using LiveFieldValues = org::apache::lucene::search::LiveFieldValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;

IDVersionPostingsFormat::IDVersionPostingsFormat()
    : IDVersionPostingsFormat(BlockTreeTermsWriter::DEFAULT_MIN_BLOCK_SIZE,
                              BlockTreeTermsWriter::DEFAULT_MAX_BLOCK_SIZE)
{
}

IDVersionPostingsFormat::IDVersionPostingsFormat(int minTermsInBlock,
                                                 int maxTermsInBlock)
    : org::apache::lucene::codecs::PostingsFormat(L"IDVersion"),
      minTermsInBlock(minTermsInBlock), maxTermsInBlock(maxTermsInBlock)
{
  BlockTreeTermsWriter::validateSettings(minTermsInBlock, maxTermsInBlock);
}

shared_ptr<FieldsConsumer> IDVersionPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PostingsWriterBase> postingsWriter =
      make_shared<IDVersionPostingsWriter>(state->liveDocs);
  bool success = false;
  try {
    shared_ptr<FieldsConsumer> ret = make_shared<VersionBlockTreeTermsWriter>(
        state, postingsWriter, minTermsInBlock, maxTermsInBlock);
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

shared_ptr<FieldsProducer> IDVersionPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<PostingsReaderBase> postingsReader =
      make_shared<IDVersionPostingsReader>();
  bool success = false;
  try {
    shared_ptr<FieldsProducer> ret =
        make_shared<VersionBlockTreeTermsReader>(postingsReader, state);
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

int64_t IDVersionPostingsFormat::bytesToLong(shared_ptr<BytesRef> bytes)
{
  return ((bytes->bytes[bytes->offset] & 0xFFLL) << 56) |
         ((bytes->bytes[bytes->offset + 1] & 0xFFLL) << 48) |
         ((bytes->bytes[bytes->offset + 2] & 0xFFLL) << 40) |
         ((bytes->bytes[bytes->offset + 3] & 0xFFLL) << 32) |
         ((bytes->bytes[bytes->offset + 4] & 0xFFLL) << 24) |
         ((bytes->bytes[bytes->offset + 5] & 0xFFLL) << 16) |
         ((bytes->bytes[bytes->offset + 6] & 0xFFLL) << 8) |
         (bytes->bytes[bytes->offset + 7] & 0xFFLL);
}

void IDVersionPostingsFormat::longToBytes(int64_t v,
                                          shared_ptr<BytesRef> bytes)
{
  if (v > MAX_VERSION || v < MIN_VERSION) {
    throw invalid_argument(L"version must be >= MIN_VERSION=" +
                           to_wstring(MIN_VERSION) + L" and <= MAX_VERSION=" +
                           to_wstring(MAX_VERSION) + L" (got: " +
                           to_wstring(v) + L")");
  }
  bytes->offset = 0;
  bytes->length = 8;
  bytes->bytes[0] = static_cast<char>(v >> 56);
  bytes->bytes[1] = static_cast<char>(v >> 48);
  bytes->bytes[2] = static_cast<char>(v >> 40);
  bytes->bytes[3] = static_cast<char>(v >> 32);
  bytes->bytes[4] = static_cast<char>(v >> 24);
  bytes->bytes[5] = static_cast<char>(v >> 16);
  bytes->bytes[6] = static_cast<char>(v >> 8);
  bytes->bytes[7] = static_cast<char>(v);
  assert((bytesToLong(bytes) == v, to_wstring(bytesToLong(bytes)) + L" vs " +
                                       to_wstring(v) + L" bytes=" + bytes));
}
} // namespace org::apache::lucene::codecs::idversion