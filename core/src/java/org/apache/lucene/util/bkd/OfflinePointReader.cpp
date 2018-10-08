using namespace std;

#include "OfflinePointReader.h"

namespace org::apache::lucene::util::bkd
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LongBitSet = org::apache::lucene::util::LongBitSet;

OfflinePointReader::OfflinePointReader(
    shared_ptr<Directory> tempDir, const wstring &tempFileName,
    int packedBytesLength, int64_t start, int64_t length, bool longOrds,
    bool singleValuePerDoc) 
    : packedValue(std::deque<char>(packedBytesLength)),
      singleValuePerDoc(singleValuePerDoc), bytesPerDoc(bytesPerDoc),
      name(tempFileName)
{
  int bytesPerDoc = packedBytesLength + Integer::BYTES;
  if (singleValuePerDoc == false) {
    if (longOrds) {
      bytesPerDoc += Long::BYTES;
    } else {
      bytesPerDoc += Integer::BYTES;
    }
  }

  if ((start + length) * bytesPerDoc + CodecUtil::footerLength() >
      tempDir->fileLength(tempFileName)) {
    throw invalid_argument(
        L"requested slice is beyond the length of this file: start=" +
        to_wstring(start) + L" length=" + to_wstring(length) +
        L" bytesPerDoc=" + to_wstring(bytesPerDoc) + L" fileLength=" +
        to_wstring(tempDir->fileLength(tempFileName)) + L" tempFileName=" +
        tempFileName);
  }

  // Best-effort checksumming:
  if (start == 0 && length * bytesPerDoc == tempDir->fileLength(tempFileName) -
                                                CodecUtil::footerLength()) {
    // If we are going to read the entire file, e.g. because BKDWriter is now
    // partitioning it, we open with checksums:
    in_ = tempDir->openChecksumInput(tempFileName, IOContext::READONCE);
  } else {
    // Since we are going to seek somewhere in the middle of a possibly huge
    // file, and not read all bytes from there, don't use ChecksumIndexInput
    // here. This is typically fine, because this same file will later be read
    // fully, at another level of the BKDWriter recursion
    in_ = tempDir->openInput(tempFileName, IOContext::READONCE);
  }

  int64_t seekFP = start * bytesPerDoc;
  in_->seek(seekFP);
  countLeft = length;
  this->longOrds = longOrds;
}

bool OfflinePointReader::next() 
{
  if (countLeft >= 0) {
    if (countLeft == 0) {
      return false;
    }
    countLeft--;
  }
  try {
    in_->readBytes(packedValue_, 0, packedValue_.size());
  } catch (const EOFException &eofe) {
    assert(countLeft == -1);
    return false;
  }
  docID_ = in_->readInt();
  if (singleValuePerDoc == false) {
    if (longOrds) {
      ord_ = in_->readLong();
    } else {
      ord_ = in_->readInt();
    }
  } else {
    ord_ = docID_;
  }
  return true;
}

std::deque<char> OfflinePointReader::packedValue() { return packedValue_; }

int64_t OfflinePointReader::ord() { return ord_; }

int OfflinePointReader::docID() { return docID_; }

OfflinePointReader::~OfflinePointReader()
{
  try {
    if (countLeft == 0 &&
        std::dynamic_pointer_cast<ChecksumIndexInput>(in_) != nullptr &&
        checked == false) {
      // System.out.println("NOW CHECK: " + name);
      checked = true;
      CodecUtil::checkFooter(std::static_pointer_cast<ChecksumIndexInput>(in_));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete in_;
  }
}

void OfflinePointReader::markOrds(
    int64_t count, shared_ptr<LongBitSet> ordBitSet) 
{
  if (countLeft < count) {
    throw make_shared<IllegalStateException>(
        L"only " + to_wstring(countLeft) + L" points remain, but " +
        to_wstring(count) + L" were requested");
  }
  int64_t fp = in_->getFilePointer() + packedValue_.size();
  if (singleValuePerDoc == false) {
    fp += Integer::BYTES;
  }
  for (int64_t i = 0; i < count; i++) {
    in_->seek(fp);
    int64_t ord;
    if (longOrds) {
      ord = in_->readLong();
    } else {
      ord = in_->readInt();
    }
    assert((ordBitSet->get(ord) == false,
            L"ord=" + to_wstring(ord) + L" i=" + to_wstring(i) +
                L" was seen twice from " + shared_from_this()));
    ordBitSet->set(ord);
    fp += bytesPerDoc;
  }
}

int64_t OfflinePointReader::split(int64_t count,
                                    shared_ptr<LongBitSet> rightTree,
                                    shared_ptr<PointWriter> left,
                                    shared_ptr<PointWriter> right,
                                    bool doClearBits) 
{

  if (std::dynamic_pointer_cast<OfflinePointWriter>(left) != nullptr == false ||
      std::dynamic_pointer_cast<OfflinePointWriter>(right) != nullptr ==
          false) {
    return PointReader::split(count, rightTree, left, right, doClearBits);
  }

  // We specialize the offline -> offline split since the default impl
  // is somewhat wasteful otherwise (e.g. decoding docID when we don't
  // need to)

  int packedBytesLength = packedValue_.size();

  int bytesPerDoc = packedBytesLength + Integer::BYTES;
  if (singleValuePerDoc == false) {
    if (longOrds) {
      bytesPerDoc += Long::BYTES;
    } else {
      bytesPerDoc += Integer::BYTES;
    }
  }

  int64_t rightCount = 0;

  shared_ptr<IndexOutput> rightOut =
      (std::static_pointer_cast<OfflinePointWriter>(right))->out;
  shared_ptr<IndexOutput> leftOut =
      (std::static_pointer_cast<OfflinePointWriter>(left))->out;

  assert((count <= countLeft, L"count=" + to_wstring(count) + L" countLeft=" +
                                  to_wstring(countLeft)));

  countLeft -= count;

  int64_t countStart = count;

  std::deque<char> buffer(bytesPerDoc);
  while (count > 0) {
    in_->readBytes(buffer, 0, buffer.size());

    int64_t ord;
    if (longOrds) {
      // A long ord, after the docID:
      ord = readLong(buffer, packedBytesLength + Integer::BYTES);
    } else if (singleValuePerDoc) {
      // docID is the ord:
      ord = readInt(buffer, packedBytesLength);
    } else {
      // An int ord, after the docID:
      ord = readInt(buffer, packedBytesLength + Integer::BYTES);
    }

    if (rightTree->get(ord)) {
      rightOut->writeBytes(buffer, 0, bytesPerDoc);
      if (doClearBits) {
        rightTree->clear(ord);
      }
      rightCount++;
    } else {
      leftOut->writeBytes(buffer, 0, bytesPerDoc);
    }

    count--;
  }

  (std::static_pointer_cast<OfflinePointWriter>(right))->count = rightCount;
  (std::static_pointer_cast<OfflinePointWriter>(left))->count =
      countStart - rightCount;

  return rightCount;
}

int64_t OfflinePointReader::readLong(std::deque<char> &bytes, int pos)
{
  constexpr int i1 = ((bytes[pos++] & 0xff) << 24) |
                     ((bytes[pos++] & 0xff) << 16) |
                     ((bytes[pos++] & 0xff) << 8) | (bytes[pos++] & 0xff);
  constexpr int i2 = ((bytes[pos++] & 0xff) << 24) |
                     ((bytes[pos++] & 0xff) << 16) |
                     ((bytes[pos++] & 0xff) << 8) | (bytes[pos++] & 0xff);
  return ((static_cast<int64_t>(i1)) << 32) | (i2 & 0xFFFFFFFFLL);
}

int OfflinePointReader::readInt(std::deque<char> &bytes, int pos)
{
  return ((bytes[pos++] & 0xFF) << 24) | ((bytes[pos++] & 0xFF) << 16) |
         ((bytes[pos++] & 0xFF) << 8) | (bytes[pos++] & 0xFF);
}
} // namespace org::apache::lucene::util::bkd