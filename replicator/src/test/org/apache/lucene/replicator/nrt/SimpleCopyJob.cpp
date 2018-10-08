using namespace std;

#include "SimpleCopyJob.h"

namespace org::apache::lucene::replicator::nrt
{
using IOUtils = org::apache::lucene::util::IOUtils;

SimpleCopyJob::SimpleCopyJob(
    const wstring &reason, shared_ptr<Connection> c,
    shared_ptr<CopyState> copyState, shared_ptr<SimpleReplicaNode> dest,
    unordered_map<wstring, std::shared_ptr<FileMetaData>> &files,
    bool highPriority, shared_ptr<OnceDone> onceDone) 
    : CopyJob(reason, files, dest, highPriority, onceDone), c(c),
      copyState(copyState)
{
  dest->message(L"create SimpleCopyJob o" + to_wstring(ord));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SimpleCopyJob::start() 
{
  if (iter == nullptr) {
    iter = toCopy.begin();

    // Send all file names / offsets up front to avoid ping-ping latency:
    try {

      // This means we resumed an already in-progress copy; we do this one
      // first:
      if (current != nullptr) {
        c->out->writeByte(static_cast<char>(0));
        c->out->writeString(current->name);
        c->out->writeVLong(current->getBytesCopied());
        totBytes += current->metaData->length;
      }

      for (auto ent : toCopy) {
        wstring fileName = ent.first;
        shared_ptr<FileMetaData> metaData = ent.second;
        totBytes += metaData->length;
        c->out->writeByte(static_cast<char>(0));
        c->out->writeString(fileName);
        c->out->writeVLong(0);
      }
      c->out->writeByte(static_cast<char>(1));
      c->flush();
      c->s->shutdownOutput();

      if (current != nullptr) {
        // Do this only at the end, after sending all requested files, so we
        // don't deadlock due to socket buffering waiting for primary to send us
        // this length:
        int64_t len = c->in_->readVLong();
        if (len != current->metaData->length) {
          throw make_shared<IllegalStateException>(
              L"file " + current->name + L": meta data says length=" +
              to_wstring(current->metaData->length) + L" but c.in says " +
              to_wstring(len));
        }
      }

      dest->message(L"SimpleCopyJob.init: done start files count=" +
                    toCopy.size() + L" totBytes=" + to_wstring(totBytes));

    } catch (const runtime_error &t) {
      cancel(L"exc during start", t);
      throw make_shared<NodeCommunicationException>(L"exc during start", t);
    }
  } else {
    throw make_shared<IllegalStateException>(L"already started");
  }
}

int64_t SimpleCopyJob::getTotalBytesCopied() { return totBytesCopied; }

shared_ptr<Set<wstring>> SimpleCopyJob::getFileNamesToCopy()
{
  shared_ptr<Set<wstring>> fileNames = unordered_set<wstring>();
  for (auto ent : toCopy) {
    fileNames->add(ent.first);
  }
  return fileNames;
}

shared_ptr<Set<wstring>> SimpleCopyJob::getFileNames()
{
  return files.keySet();
}

int SimpleCopyJob::compareTo(shared_ptr<CopyJob> _other)
{
  shared_ptr<SimpleCopyJob> other =
      std::static_pointer_cast<SimpleCopyJob>(_other);
  if (highPriority != other->highPriority) {
    return highPriority ? -1 : 1;
  } else if (ord < other->ord) {
    return -1;
  } else if (ord > other->ord) {
    return 1;
  } else {
    return 0;
  }
}

void SimpleCopyJob::finish() 
{
  dest->message(wstring::format(Locale::ROOT,
                                L"top: file copy done; took %.1f msec to copy "
                                L"%d bytes; now rename %d tmp files",
                                (System::nanoTime() - startNS) / 1000000.0,
                                totBytesCopied, copiedFiles.size()));

  // NOTE: if any of the files we copied overwrote a file in the current commit
  // point, we (ReplicaNode) removed the commit point up front so that the
  // commit is not corrupt.  This way if we hit exc here, or if we crash here,
  // we won't leave a corrupt commit in the index:
  for (auto ent : copiedFiles) {
    wstring tmpFileName = ent.second;
    wstring fileName = ent.first;

    if (Node::VERBOSE_FILES) {
      dest->message(L"rename file " + tmpFileName + L" to " + fileName);
    }

    // NOTE: if this throws exception, then some files have been moved to their
    // true names, and others are leftover .tmp files.  I don't think heroic
    // exception handling is necessary (no harm will come, except some leftover
    // files),  nor warranted here (would make the code more complex, for the
    // exceptional cases when something is wrong w/ your IO system):
    dest->dir->rename(tmpFileName, fileName);
  }

  copiedFiles.clear();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool SimpleCopyJob::visit() 
{
  if (exc != nullptr) {
    // We were externally cancelled:
    return true;
  }

  if (current == nullptr) {
    if (iter->hasNext() == false) {
      delete c;
      return true;
    }

    unordered_map::Entry<wstring, std::shared_ptr<FileMetaData>> next =
        iter->next();
    shared_ptr<FileMetaData> metaData = next.getValue();
    wstring fileName = next.getKey();
    int64_t len = c->in_->readVLong();
    if (len != metaData->length) {
      throw make_shared<IllegalStateException>(
          L"file " + fileName + L": meta data says length=" +
          to_wstring(metaData->length) + L" but c.in says " + to_wstring(len));
    }
    current =
        make_shared<CopyOneFile>(c->in_, dest, fileName, metaData, copyBuffer);
  }

  if (current->visit()) {
    // This file is done copying
    copiedFiles.emplace(current->name, current->tmpName);
    totBytesCopied += current->getBytesCopied();
    assert((totBytesCopied <= totBytes,
            L"totBytesCopied=" + to_wstring(totBytesCopied) + L" totBytes=" +
                to_wstring(totBytes)));
    current.reset();
    return false;
  }

  return false;
}

shared_ptr<CopyOneFile>
SimpleCopyJob::newCopyOneFile(shared_ptr<CopyOneFile> prev)
{
  return make_shared<CopyOneFile>(prev, c->in_);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SimpleCopyJob::transferAndCancel(shared_ptr<CopyJob> prevJob) throw(
    IOException)
{
  try {
    CopyJob::transferAndCancel(prevJob);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException(
        {(std::static_pointer_cast<SimpleCopyJob>(prevJob))->c});
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SimpleCopyJob::cancel(const wstring &reason,
                           runtime_error exc) 
{
  try {
    CopyJob::cancel(reason, exc);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({c});
  }
}

bool SimpleCopyJob::getFailed() { return exc != nullptr; }

wstring SimpleCopyJob::toString()
{
  return L"SimpleCopyJob(ord=" + to_wstring(ord) + L" " + reason +
         L" highPriority=" + StringHelper::toString(highPriority) +
         L" files count=" + files.size() + L" bytesCopied=" +
         to_wstring(totBytesCopied) + L" (of " + to_wstring(totBytes) +
         L") filesCopied=" + copiedFiles.size() + L")";
}

void SimpleCopyJob::runBlocking() 
{
  while (visit() == false) {
    ;
  }

  if (getFailed()) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("copy failed: " + cancelReason,
    // exc);
    throw runtime_error(L"copy failed: " + cancelReason);
  }
}

shared_ptr<CopyState> SimpleCopyJob::getCopyState() { return copyState; }

// C++ WARNING: The following method was originally marked 'synchronized':
bool SimpleCopyJob::conflicts(shared_ptr<CopyJob> _other)
{
  shared_ptr<Set<wstring>> filesToCopy = unordered_set<wstring>();
  for (auto ent : toCopy) {
    filesToCopy->add(ent.first);
  }

  shared_ptr<SimpleCopyJob> other =
      std::static_pointer_cast<SimpleCopyJob>(_other);
  {
    lock_guard<mutex> lock(other);
    for (auto ent : other->toCopy) {
      if (filesToCopy->contains(ent.first)) {
        return true;
      }
    }
  }

  return false;
}
} // namespace org::apache::lucene::replicator::nrt