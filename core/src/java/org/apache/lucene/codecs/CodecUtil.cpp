using namespace std;

#include "CodecUtil.h"
#include "../index/CorruptIndexException.h"
#include "../index/IndexFormatTooNewException.h"
#include "../index/IndexFormatTooOldException.h"
#include "../store/BufferedChecksumIndexInput.h"
#include "../store/ChecksumIndexInput.h"
#include "../store/DataInput.h"
#include "../store/DataOutput.h"
#include "../store/IndexInput.h"
#include "../store/IndexOutput.h"
#include "../util/BytesRef.h"
#include "../util/IOUtils.h"
#include "../util/StringHelper.h"

namespace org::apache::lucene::codecs
{
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexFormatTooNewException =
    org::apache::lucene::index::IndexFormatTooNewException;
using IndexFormatTooOldException =
    org::apache::lucene::index::IndexFormatTooOldException;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;

CodecUtil::CodecUtil() {} // no instance

void CodecUtil::writeHeader(shared_ptr<DataOutput> out, const wstring &codec,
                            int version) 
{
  shared_ptr<BytesRef> bytes = make_shared<BytesRef>(codec);
  if (bytes->length != codec.length() || bytes->length >= 128) {
    throw invalid_argument(L"codec must be simple ASCII, less than 128 "
                           L"characters in length [got " +
                           codec + L"]");
  }
  out->writeInt(CODEC_MAGIC);
  out->writeString(codec);
  out->writeInt(version);
}

void CodecUtil::writeIndexHeader(shared_ptr<DataOutput> out,
                                 const wstring &codec, int version,
                                 std::deque<char> &id,
                                 const wstring &suffix) 
{
  if (id.size() != StringHelper::ID_LENGTH) {
    throw invalid_argument(L"Invalid id: " + StringHelper::idToString(id));
  }
  writeHeader(out, codec, version);
  out->writeBytes(id, 0, id.size());
  shared_ptr<BytesRef> suffixBytes = make_shared<BytesRef>(suffix);
  if (suffixBytes->length != suffix.length() || suffixBytes->length >= 256) {
    throw invalid_argument(L"suffix must be simple ASCII, less than 256 "
                           L"characters in length [got " +
                           suffix + L"]");
  }
  out->writeByte(static_cast<char>(suffixBytes->length));
  out->writeBytes(suffixBytes->bytes, suffixBytes->offset, suffixBytes->length);
}

int CodecUtil::headerLength(const wstring &codec) { return 9 + codec.length(); }

int CodecUtil::indexHeaderLength(const wstring &codec, const wstring &suffix)
{
  return headerLength(codec) + StringHelper::ID_LENGTH + 1 + suffix.length();
}

int CodecUtil::checkHeader(shared_ptr<DataInput> in_, const wstring &codec,
                           int minVersion, int maxVersion) 
{
  // Safety to guard against reading a bogus string:
  constexpr int actualHeader = in_->readInt();
  if (actualHeader != CODEC_MAGIC) {
    throw make_shared<CorruptIndexException>(
        L"codec header mismatch: actual header=" + to_wstring(actualHeader) +
            L" vs expected header=" + to_wstring(CODEC_MAGIC),
        in_);
  }
  return checkHeaderNoMagic(in_, codec, minVersion, maxVersion);
}

int CodecUtil::checkHeaderNoMagic(shared_ptr<DataInput> in_,
                                  const wstring &codec, int minVersion,
                                  int maxVersion) 
{
  const wstring actualCodec = in_->readString();
  if (actualCodec != codec) {
    throw make_shared<CorruptIndexException>(L"codec mismatch: actual codec=" +
                                                 actualCodec +
                                                 L" vs expected codec=" + codec,
                                             in_);
  }

  constexpr int actualVersion = in_->readInt();
  if (actualVersion < minVersion) {
    throw make_shared<IndexFormatTooOldException>(in_, actualVersion,
                                                  minVersion, maxVersion);
  }
  if (actualVersion > maxVersion) {
    throw make_shared<IndexFormatTooNewException>(in_, actualVersion,
                                                  minVersion, maxVersion);
  }

  return actualVersion;
}

int CodecUtil::checkIndexHeader(
    shared_ptr<DataInput> in_, const wstring &codec, int minVersion,
    int maxVersion, std::deque<char> &expectedID,
    const wstring &expectedSuffix) 
{
  int version = checkHeader(in_, codec, minVersion, maxVersion);
  checkIndexHeaderID(in_, expectedID);
  checkIndexHeaderSuffix(in_, expectedSuffix);
  return version;
}

void CodecUtil::verifyAndCopyIndexHeader(
    shared_ptr<IndexInput> in_, shared_ptr<DataOutput> out,
    std::deque<char> &expectedID) 
{
  // make sure it's large enough to have a header and footer
  if (in_->length() < footerLength() + headerLength(L"")) {
    throw make_shared<CorruptIndexException>(
        L"compound sub-files must have a valid codec header and footer: file "
        L"is too small (" +
            to_wstring(in_->length()) + L" bytes)",
        in_);
  }

  int actualHeader = in_->readInt();
  if (actualHeader != CODEC_MAGIC) {
    throw make_shared<CorruptIndexException>(
        L"compound sub-files must have a valid codec header and footer: codec "
        L"header mismatch: actual header=" +
            to_wstring(actualHeader) + L" vs expected header=" +
            to_wstring(CodecUtil::CODEC_MAGIC),
        in_);
  }

  // we can't verify these, so we pass-through:
  wstring codec = in_->readString();
  int version = in_->readInt();

  // verify id:
  checkIndexHeaderID(in_, expectedID);

  // we can't verify extension either, so we pass-through:
  int suffixLength = in_->readByte() & 0xFF;
  std::deque<char> suffixBytes(suffixLength);
  in_->readBytes(suffixBytes, 0, suffixLength);

  // now write the header we just verified
  out->writeInt(CodecUtil::CODEC_MAGIC);
  out->writeString(codec);
  out->writeInt(version);
  out->writeBytes(expectedID, 0, expectedID.size());
  out->writeByte(static_cast<char>(suffixLength));
  out->writeBytes(suffixBytes, 0, suffixLength);
}

std::deque<char>
CodecUtil::readIndexHeader(shared_ptr<IndexInput> in_) 
{
  in_->seek(0);
  constexpr int actualHeader = in_->readInt();
  if (actualHeader != CODEC_MAGIC) {
    throw make_shared<CorruptIndexException>(
        L"codec header mismatch: actual header=" + to_wstring(actualHeader) +
            L" vs expected header=" + to_wstring(CODEC_MAGIC),
        in_);
  }
  wstring codec = in_->readString();
  in_->readInt();
  in_->seek(in_->getFilePointer() + StringHelper::ID_LENGTH);
  int suffixLength = in_->readByte() & 0xFF;
  std::deque<char> bytes(headerLength(codec) + StringHelper::ID_LENGTH + 1 +
                          suffixLength);
  in_->seek(0);
  in_->readBytes(bytes, 0, bytes.size());
  return bytes;
}

std::deque<char>
CodecUtil::readFooter(shared_ptr<IndexInput> in_) 
{
  if (in_->length() < footerLength()) {
    throw make_shared<CorruptIndexException>(
        L"misplaced codec footer (file truncated?): length=" +
            to_wstring(in_->length()) + L" but footerLength==" +
            to_wstring(footerLength()),
        in_);
  }
  in_->seek(in_->length() - footerLength());
  validateFooter(in_);
  in_->seek(in_->length() - footerLength());
  std::deque<char> bytes(footerLength());
  in_->readBytes(bytes, 0, bytes.size());
  return bytes;
}

std::deque<char>
CodecUtil::checkIndexHeaderID(shared_ptr<DataInput> in_,
                              std::deque<char> &expectedID) 
{
  std::deque<char> id(StringHelper::ID_LENGTH);
  in_->readBytes(id, 0, id.size());
  if (!Arrays::equals(id, expectedID)) {
    throw make_shared<CorruptIndexException>(
        L"file mismatch, expected id=" + StringHelper::idToString(expectedID) +
            L", got=" + StringHelper::idToString(id),
        in_);
  }
  return id;
}

wstring CodecUtil::checkIndexHeaderSuffix(
    shared_ptr<DataInput> in_, const wstring &expectedSuffix) 
{
  int suffixLength = in_->readByte() & 0xFF;
  std::deque<char> suffixBytes(suffixLength);
  in_->readBytes(suffixBytes, 0, suffixBytes.size());
  wstring suffix =
      wstring(suffixBytes, 0, suffixBytes.size(), StandardCharsets::UTF_8);
  if (suffix != expectedSuffix) {
    throw make_shared<CorruptIndexException>(
        L"file mismatch, expected suffix=" + expectedSuffix + L", got=" +
            suffix,
        in_);
  }
  return suffix;
}

void CodecUtil::writeFooter(shared_ptr<IndexOutput> out) 
{
  out->writeInt(FOOTER_MAGIC);
  out->writeInt(0);
  writeCRC(out);
}

int CodecUtil::footerLength() { return 16; }

int64_t
CodecUtil::checkFooter(shared_ptr<ChecksumIndexInput> in_) 
{
  validateFooter(in_);
  int64_t actualChecksum = in_->getChecksum();
  int64_t expectedChecksum = readCRC(in_);
  if (expectedChecksum != actualChecksum) {
    throw make_shared<CorruptIndexException>(
        L"checksum failed (hardware problem?) : expected=" +
            Long::toHexString(expectedChecksum) + L" actual=" +
            Long::toHexString(actualChecksum),
        in_);
  }
  return actualChecksum;
}

void CodecUtil::checkFooter(shared_ptr<ChecksumIndexInput> in_,
                            runtime_error priorException) 
{
  if (priorException == nullptr) {
    checkFooter(in_);
  } else {
    try {
      int64_t remaining = in_->length() - in_->getFilePointer();
      if (remaining < footerLength()) {
        // corruption caused us to read into the checksum footer already: we
        // can't proceed
        priorException.addSuppressed(make_shared<CorruptIndexException>(
            L"checksum status indeterminate: remaining=" +
                to_wstring(remaining) +
                L", please run checkindex for more details",
            in_));
      } else {
        // otherwise, skip any unread bytes.
        in_->skipBytes(remaining - footerLength());

        // now check the footer
        try {
          int64_t checksum = checkFooter(in_);
          priorException.addSuppressed(make_shared<CorruptIndexException>(
              L"checksum passed (" + Long::toHexString(checksum) +
                  L"). possibly transient resource issue, or a Lucene or JVM "
                  L"bug",
              in_));
        } catch (const CorruptIndexException &t) {
          priorException.addSuppressed(t);
        }
      }
    } catch (const runtime_error &t) {
      // catch-all for things that shouldn't go wrong (e.g. OOM during readInt)
      // but could...
      priorException.addSuppressed(make_shared<CorruptIndexException>(
          L"checksum status indeterminate: unexpected exception", in_, t));
    }
    throw IOUtils::rethrowAlways(priorException);
  }
}

int64_t
CodecUtil::retrieveChecksum(shared_ptr<IndexInput> in_) 
{
  if (in_->length() < footerLength()) {
    throw make_shared<CorruptIndexException>(
        L"misplaced codec footer (file truncated?): length=" +
            to_wstring(in_->length()) + L" but footerLength==" +
            to_wstring(footerLength()),
        in_);
  }
  in_->seek(in_->length() - footerLength());
  validateFooter(in_);
  return readCRC(in_);
}

void CodecUtil::validateFooter(shared_ptr<IndexInput> in_) 
{
  int64_t remaining = in_->length() - in_->getFilePointer();
  int64_t expected = footerLength();
  if (remaining < expected) {
    throw make_shared<CorruptIndexException>(
        L"misplaced codec footer (file truncated?): remaining=" +
            to_wstring(remaining) + L", expected=" + to_wstring(expected) +
            L", fp=" + to_wstring(in_->getFilePointer()),
        in_);
  } else if (remaining > expected) {
    throw make_shared<CorruptIndexException>(
        L"misplaced codec footer (file extended?): remaining=" +
            to_wstring(remaining) + L", expected=" + to_wstring(expected) +
            L", fp=" + to_wstring(in_->getFilePointer()),
        in_);
  }

  constexpr int magic = in_->readInt();
  if (magic != FOOTER_MAGIC) {
    throw make_shared<CorruptIndexException>(
        L"codec footer mismatch (file truncated?): actual footer=" +
            to_wstring(magic) + L" vs expected footer=" +
            to_wstring(FOOTER_MAGIC),
        in_);
  }

  constexpr int algorithmID = in_->readInt();
  if (algorithmID != 0) {
    throw make_shared<CorruptIndexException>(
        L"codec footer mismatch: unknown algorithmID: " +
            to_wstring(algorithmID),
        in_);
  }
}

int64_t
CodecUtil::checksumEntireFile(shared_ptr<IndexInput> input) 
{
  shared_ptr<IndexInput> clone = input->clone();
  clone->seek(0);
  shared_ptr<ChecksumIndexInput> in_ =
      make_shared<BufferedChecksumIndexInput>(clone);
  assert(in_->getFilePointer() == 0);
  if (in_->length() < footerLength()) {
    throw make_shared<CorruptIndexException>(
        L"misplaced codec footer (file truncated?): length=" +
            to_wstring(in_->length()) + L" but footerLength==" +
            to_wstring(footerLength()),
        input);
  }
  in_->seek(in_->length() - footerLength());
  return checkFooter(in_);
}

int64_t CodecUtil::readCRC(shared_ptr<IndexInput> input) 
{
  int64_t value = input->readLong();
  if ((value & 0xFFFFFFFF00000000LL) != 0) {
    throw make_shared<CorruptIndexException>(
        L"Illegal CRC-32 checksum: " + to_wstring(value), input);
  }
  return value;
}

void CodecUtil::writeCRC(shared_ptr<IndexOutput> output) 
{
  int64_t value = output->getChecksum();
  if ((value & 0xFFFFFFFF00000000LL) != 0) {
    throw make_shared<IllegalStateException>(L"Illegal CRC-32 checksum: " +
                                             to_wstring(value) +
                                             L" (resource=" + output + L")");
  }
  output->writeLong(value);
}
} // namespace org::apache::lucene::codecs