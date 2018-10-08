using namespace std;

#include "OfflinePointWriter.h"

namespace org::apache::lucene::util::bkd
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;

OfflinePointWriter::OfflinePointWriter(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    int packedBytesLength, bool longOrds, const wstring &desc,
    int64_t expectedCount, bool singleValuePerDoc) 
    : tempDir(tempDir),
      out(tempDir->createTempOutput(tempFileNamePrefix, L"bkd_" + desc,
                                    IOContext::DEFAULT)),
      name(out->getName()), packedBytesLength(packedBytesLength),
      singleValuePerDoc(singleValuePerDoc), expectedCount(expectedCount)
{
  this->longOrds = longOrds;
}

OfflinePointWriter::OfflinePointWriter(shared_ptr<Directory> tempDir,
                                       const wstring &name,
                                       int packedBytesLength, int64_t count,
                                       bool longOrds, bool singleValuePerDoc)
    : tempDir(tempDir), out(this->out.reset()), name(name),
      packedBytesLength(packedBytesLength),
      singleValuePerDoc(singleValuePerDoc), expectedCount(0)
{
  this->count = count;
  closed = true;
  this->longOrds = longOrds;
}

void OfflinePointWriter::append(std::deque<char> &packedValue, int64_t ord,
                                int docID) 
{
  assert(packedValue.size() == packedBytesLength);
  out->writeBytes(packedValue, 0, packedValue.size());
  out->writeInt(docID);
  if (singleValuePerDoc == false) {
    if (longOrds) {
      out->writeLong(ord);
    } else {
      assert((ord <= std, : numeric_limits<int>::max()));
      out->writeInt(static_cast<int>(ord));
    }
  }
  count++;
  assert(expectedCount == 0 || count <= expectedCount);
}

shared_ptr<PointReader>
OfflinePointWriter::getReader(int64_t start,
                              int64_t length) 
{
  assert(closed);
  assert((start + length <= count, L"start=" + to_wstring(start) + L" length=" +
                                       to_wstring(length) + L" count=" +
                                       to_wstring(count)));
  assert(expectedCount == 0 || count == expectedCount);
  return make_shared<OfflinePointReader>(tempDir, name, packedBytesLength,
                                         start, length, longOrds,
                                         singleValuePerDoc);
}

shared_ptr<PointReader> OfflinePointWriter::getSharedReader(
    int64_t start, int64_t length,
    deque<std::shared_ptr<Closeable>> &toCloseHeroically) 
{
  if (sharedReader == nullptr) {
    assert(start == 0);
    assert(length <= count);
    sharedReader =
        make_shared<OfflinePointReader>(tempDir, name, packedBytesLength, 0,
                                        count, longOrds, singleValuePerDoc);
    toCloseHeroically.push_back(sharedReader);
    // Make sure the OfflinePointReader intends to verify its checksum:
    assert(std::dynamic_pointer_cast<ChecksumIndexInput>(sharedReader->in_) !=
           nullptr);
  } else {
    assert((start == nextSharedRead,
            L"start=" + to_wstring(start) + L" length=" + to_wstring(length) +
                L" nextSharedRead=" + to_wstring(nextSharedRead)));
  }
  nextSharedRead += length;
  return sharedReader;
}

OfflinePointWriter::~OfflinePointWriter()
{
  if (closed == false) {
    assert(sharedReader == nullptr);
    try {
      CodecUtil::writeFooter(out);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete out;
      closed = true;
    }
  }
}

void OfflinePointWriter::destroy() 
{
  if (sharedReader != nullptr) {
    // At this point, the shared reader should have done a full sweep of the
    // file:
    assert(nextSharedRead == count);
    delete sharedReader;
    sharedReader.reset();
  }
  tempDir->deleteFile(name);
}

wstring OfflinePointWriter::toString()
{
  return L"OfflinePointWriter(count=" + to_wstring(count) + L" tempFileName=" +
         name + L")";
}
} // namespace org::apache::lucene::util::bkd