using namespace std;

#include "CopyJob.h"

namespace org::apache::lucene::replicator::nrt
{
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IOUtils = org::apache::lucene::util::IOUtils;
const shared_ptr<java::util::concurrent::atomic::AtomicLong> CopyJob::counter =
    make_shared<java::util::concurrent::atomic::AtomicLong>();

CopyJob::CopyJob(const wstring &reason,
                 unordered_map<wstring, std::shared_ptr<FileMetaData>> &files,
                 shared_ptr<ReplicaNode> dest, bool highPriority,
                 shared_ptr<OnceDone> onceDone) 
    : dest(dest), files(files), highPriority(highPriority), onceDone(onceDone),
      reason(reason)
{

  // Exceptions in here are bad:
  try {
    this->toCopy = dest->getFilesToCopy(this->files);
  } catch (const runtime_error &t) {
    cancel(L"exc during init", t);
    throw make_shared<CorruptIndexException>(
        L"exception while checking local files", L"n/a", t);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void CopyJob::transferAndCancel(shared_ptr<CopyJob> prevJob) 
{
  {
    lock_guard<mutex> lock(prevJob);
    dest->message(L"CopyJob: now transfer prevJob " + prevJob);
    try {
      _transferAndCancel(prevJob);
    } catch (const runtime_error &t) {
      dest->message(L"xfer: exc during transferAndCancel");
      cancel(L"exc during transferAndCancel", t);
      throw IOUtils::rethrowAlways(t);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void CopyJob::_transferAndCancel(shared_ptr<CopyJob> prevJob) 
{

  // Caller must already be sync'd on prevJob:
  assert(Thread::holdsLock(prevJob));

  if (prevJob->exc != nullptr) {
    // Already cancelled
    dest->message(L"xfer: prevJob was already cancelled; skip transfer");
    return;
  }

  // Cancel the previous job
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: prevJob.exc = new Throwable();
  prevJob->exc = runtime_error();

  // Carry over already copied files that we also want to copy
  deque<unordered_map::Entry<wstring, std::shared_ptr<FileMetaData>>>::
      const_iterator it = toCopy.begin();
  int64_t bytesAlreadyCopied = 0;

  // Iterate over all files we think we need to copy:
  while (it != toCopy.end()) {
    unordered_map::Entry<wstring, std::shared_ptr<FileMetaData>> ent = *it;
    wstring fileName = ent.getKey();
    wstring prevTmpFileName = prevJob->copiedFiles[fileName];
    if (prevTmpFileName != L"") {
      // This fileName is common to both jobs, and the old job already finished
      // copying it (to a temp file), so we keep it:
      int64_t fileLength = ent.getValue()->length;
      bytesAlreadyCopied += fileLength;
      dest->message(L"xfer: carry over already-copied file " + fileName +
                    L" (" + prevTmpFileName + L", " + to_wstring(fileLength) +
                    L" bytes)");
      copiedFiles.emplace(fileName, prevTmpFileName);

      // So we don't try to delete it, below:
      prevJob->copiedFiles.erase(fileName);

      // So it's not in our copy deque anymore:
      (*it)->remove();
    } else if (prevJob->current != nullptr &&
               prevJob->current->name == fileName) {
      // This fileName is common to both jobs, and it's the file that the
      // previous job was in the process of copying.  In this case we continue
      // copying it from the prevoius job.  This is important for cases where we
      // are copying over a large file because otherwise we could keep failing
      // the NRT copy and restarting this file from the beginning and never
      // catch up:
      dest->message(L"xfer: carry over in-progress file " + fileName + L" (" +
                    prevJob->current->tmpName + L") bytesCopied=" +
                    to_wstring(prevJob->current->getBytesCopied()) + L" of " +
                    to_wstring(prevJob->current->bytesToCopy));
      bytesAlreadyCopied += prevJob->current->getBytesCopied();

      assert(current == nullptr);

      // must set current first, before writing/read to c.in/out in case that
      // hits an exception, so that we then close the temp IndexOutput when
      // cancelling ourselves:
      current = newCopyOneFile(prevJob->current);

      // Tell our new (primary) connection we'd like to copy this file first,
      // but resuming from how many bytes we already copied last time: We do
      // this even if bytesToCopy == bytesCopied, because we still need to
      // readLong() the checksum from the primary connection:
      assert(prevJob->current->getBytesCopied() <=
             prevJob->current->bytesToCopy);

      prevJob->current.reset();

      totBytes += current->metaData->length;

      // So it's not in our copy deque anymore:
      (*it)->remove();
    } else {
      dest->message(L"xfer: file " + fileName + L" will be fully copied");
    }
    it++;
  }
  dest->message(L"xfer: " + to_wstring(bytesAlreadyCopied) +
                L" bytes already copied of " + to_wstring(totBytes));

  // Delete all temp files the old job wrote but we don't need:
  dest->message(L"xfer: now delete old temp files: " +
                prevJob->copiedFiles.values());
  IOUtils::deleteFilesIgnoringExceptions(
      {dest->dir, prevJob->copiedFiles.values()});

  if (prevJob->current != nullptr) {
    IOUtils::closeWhileHandlingException({prevJob->current});
    if (Node::VERBOSE_FILES) {
      dest->message(L"remove partial file " + prevJob->current->tmpName);
    }
    dest->deleter->deleteNewFile(prevJob->current->tmpName);
    prevJob->current.reset();
  }
}

void CopyJob::cancel(const wstring &reason,
                     runtime_error exc) 
{
  if (this->exc != nullptr) {
    // Already cancelled
    return;
  }

  dest->message(wstring::format(
      Locale::ROOT,
      L"top: cancel after copying %s; exc=%s:\n  files=%s\n  copiedFiles=%s",
      Node::bytesToString(totBytesCopied), exc,
      files.empty() ? L"null" : files.keySet(), copiedFiles.keySet()));

  if (exc == nullptr) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: exc = new Throwable();
    exc = runtime_error();
  }

  this->exc = exc;
  this->cancelReason = reason;

  // Delete all temp files we wrote:
  IOUtils::deleteFilesIgnoringExceptions({dest->dir, copiedFiles.values()});

  if (current != nullptr) {
    IOUtils::closeWhileHandlingException({current});
    if (Node::VERBOSE_FILES) {
      dest->message(L"remove partial file " + current->tmpName);
    }
    dest->deleter->deleteNewFile(current->tmpName);
    current.reset();
  }
}
} // namespace org::apache::lucene::replicator::nrt