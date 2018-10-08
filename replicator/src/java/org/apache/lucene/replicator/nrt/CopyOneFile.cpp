using namespace std;

#include "CopyOneFile.h"

namespace org::apache::lucene::replicator::nrt
{
using DataInput = org::apache::lucene::store::DataInput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;

CopyOneFile::CopyOneFile(shared_ptr<DataInput> in_,
                         shared_ptr<ReplicaNode> dest, const wstring &name,
                         shared_ptr<FileMetaData> metaData,
                         std::deque<char> &buffer) 
    : in_(in_), out(dest->createTempOutput(name, L"copy", IOContext::DEFAULT)),
      dest(dest), name(name), tmpName(out->getName()), metaData(metaData),
      bytesToCopy(metaData->length - Long::BYTES),
      copyStartNS(System::nanoTime()), buffer(buffer)
{
  // TODO: pass correct IOCtx, e.g. seg total size

  // last 8 bytes are checksum, which we write ourselves after copying all bytes
  // and confirming checksum:

  if (Node::VERBOSE_FILES) {
    dest->message(L"file " + name + L": start copying to tmp file " + tmpName +
                  L" length=" + to_wstring(8 + bytesToCopy));
  }

  dest->startCopyFile(name);
}

CopyOneFile::CopyOneFile(shared_ptr<CopyOneFile> other,
                         shared_ptr<DataInput> in_)
    : in_(in_), out(other->out), dest(other->dest), name(other->name),
      tmpName(other->tmpName), metaData(other->metaData),
      bytesToCopy(other->bytesToCopy), copyStartNS(other->copyStartNS),
      buffer(other->buffer)
{
  this->bytesCopied = other->bytesCopied;
}

CopyOneFile::~CopyOneFile()
{
  delete out;
  dest->finishCopyFile(name);
}

bool CopyOneFile::visit() 
{
  // Copy up to 640 KB per visit:
  for (int i = 0; i < 10; i++) {
    int64_t bytesLeft = bytesToCopy - bytesCopied;
    if (bytesLeft == 0) {
      int64_t checksum = out->getChecksum();
      if (checksum != metaData->checksum) {
        // Bits flipped during copy!
        dest->message(L"file " + tmpName +
                      L": checksum mismatch after copy (bits flipped during "
                      L"network copy?) after-copy checksum=" +
                      to_wstring(checksum) + L" vs expected=" +
                      to_wstring(metaData->checksum) + L"; cancel job");
        // C++ TODO: The following line could not be converted:
        throw java.io.IOException(L"file " + name +
                                  L": checksum mismatch after file copy");
      }

      // Paranoia: make sure the primary node is not smoking crack, by somehow
      // sending us an already corrupted file whose checksum (in its footer)
      // disagrees with reality:
      int64_t actualChecksumIn = in_->readLong();
      if (actualChecksumIn != checksum) {
        dest->message(L"file " + tmpName +
                      L": checksum claimed by primary disagrees with the "
                      L"file's footer: claimed checksum=" +
                      to_wstring(checksum) + L" vs actual=" +
                      to_wstring(actualChecksumIn));
        // C++ TODO: The following line could not be converted:
        throw java.io.IOException(L"file " + name +
                                  L": checksum mismatch after file copy");
      }
      out->writeLong(checksum);
      bytesCopied += Long::BYTES;
      close();

      if (Node::VERBOSE_FILES) {
        dest->message(
            wstring::format(Locale::ROOT, L"file %s: done copying [%s, %.3fms]",
                            name, Node::bytesToString(metaData->length),
                            (System::nanoTime() - copyStartNS) / 1000000.0));
      }

      return true;
    }

    int toCopy = static_cast<int>(min(bytesLeft, buffer.size()));
    in_->readBytes(buffer, 0, toCopy);
    out->writeBytes(buffer, 0, toCopy);

    // TODO: rsync will fsync a range of the file; maybe we should do that here
    // for large files in case we crash/killed
    bytesCopied += toCopy;
  }

  return false;
}

int64_t CopyOneFile::getBytesCopied() { return bytesCopied; }
} // namespace org::apache::lucene::replicator::nrt