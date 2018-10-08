using namespace std;

#include "PersistentSnapshotDeletionPolicy.h"

namespace org::apache::lucene::index
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring PersistentSnapshotDeletionPolicy::SNAPSHOTS_PREFIX =
    L"snapshots_";
const wstring PersistentSnapshotDeletionPolicy::CODEC_NAME = L"snapshots";

PersistentSnapshotDeletionPolicy::PersistentSnapshotDeletionPolicy(
    shared_ptr<IndexDeletionPolicy> primary,
    shared_ptr<Directory> dir) 
    : PersistentSnapshotDeletionPolicy(primary, dir, OpenMode::CREATE_OR_APPEND)
{
}

PersistentSnapshotDeletionPolicy::PersistentSnapshotDeletionPolicy(
    shared_ptr<IndexDeletionPolicy> primary, shared_ptr<Directory> dir,
    OpenMode mode) 
    : SnapshotDeletionPolicy(primary), dir(dir)
{

  if (mode == OpenMode::CREATE) {
    clearPriorSnapshots();
  }

  loadPriorSnapshots();

  if (mode == OpenMode::APPEND && nextWriteGen == 0) {
    throw make_shared<IllegalStateException>(
        L"no snapshots stored in this directory");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexCommit>
PersistentSnapshotDeletionPolicy::snapshot() 
{
  shared_ptr<IndexCommit> ic = SnapshotDeletionPolicy::snapshot();
  bool success = false;
  try {
    persist();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      try {
        SnapshotDeletionPolicy::release(ic);
      } catch (const runtime_error &e) {
        // Suppress so we keep throwing original exception
      }
    }
  }
  return ic;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PersistentSnapshotDeletionPolicy::release(
    shared_ptr<IndexCommit> commit) 
{
  SnapshotDeletionPolicy::release(commit);
  bool success = false;
  try {
    persist();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      try {
        incRef(commit);
      } catch (const runtime_error &e) {
        // Suppress so we keep throwing original exception
      }
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PersistentSnapshotDeletionPolicy::release(int64_t gen) 
{
  SnapshotDeletionPolicy::releaseGen(gen);
  persist();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PersistentSnapshotDeletionPolicy::persist() 
{
  wstring fileName = SNAPSHOTS_PREFIX + to_wstring(nextWriteGen);
  shared_ptr<IndexOutput> out = dir->createOutput(fileName, IOContext::DEFAULT);
  bool success = false;
  try {
    CodecUtil::writeHeader(out, CODEC_NAME, VERSION_CURRENT);
    out->writeVInt(refCounts.size());
    for (auto ent : refCounts) {
      out->writeVLong(ent.first);
      out->writeVInt(ent.second);
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({out});
      IOUtils::deleteFilesIgnoringExceptions(dir, {fileName});
    } else {
      IOUtils::close({out});
    }
  }

  dir->sync(Collections::singletonList(fileName));

  if (nextWriteGen > 0) {
    wstring lastSaveFile = SNAPSHOTS_PREFIX + to_wstring(nextWriteGen - 1);
    // exception OK: likely it didn't exist
    IOUtils::deleteFilesIgnoringExceptions(dir, {lastSaveFile});
  }

  nextWriteGen++;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PersistentSnapshotDeletionPolicy::clearPriorSnapshots() 
{
  for (auto file : dir->listAll()) {
    if (file.startsWith(SNAPSHOTS_PREFIX)) {
      dir->deleteFile(file);
    }
  }
}

wstring PersistentSnapshotDeletionPolicy::getLastSaveFile()
{
  if (nextWriteGen == 0) {
    return L"";
  } else {
    return SNAPSHOTS_PREFIX + to_wstring(nextWriteGen - 1);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PersistentSnapshotDeletionPolicy::loadPriorSnapshots() 
{
  int64_t genLoaded = -1;
  shared_ptr<IOException> ioe = nullptr;
  deque<wstring> snapshotFiles = deque<wstring>();
  for (auto file : dir->listAll()) {
    if (file.startsWith(SNAPSHOTS_PREFIX)) {
      int64_t gen = StringHelper::fromString<int64_t>(
          file.substr(SNAPSHOTS_PREFIX.length()));
      if (genLoaded == -1 || gen > genLoaded) {
        snapshotFiles.push_back(file);
        unordered_map<int64_t, int> m = unordered_map<int64_t, int>();
        shared_ptr<IndexInput> in_ = dir->openInput(file, IOContext::DEFAULT);
        try {
          CodecUtil::checkHeader(in_, CODEC_NAME, VERSION_START, VERSION_START);
          int count = in_->readVInt();
          for (int i = 0; i < count; i++) {
            int64_t commitGen = in_->readVLong();
            int refCount = in_->readVInt();
            m.emplace(commitGen, refCount);
          }
        } catch (const IOException &ioe2) {
          // Save first exception & throw in the end
          if (ioe == nullptr) {
            ioe = ioe2;
          }
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          delete in_;
        }

        genLoaded = gen;
        refCounts.clear();
        refCounts.putAll(m);
      }
    }
  }

  if (genLoaded == -1) {
    // Nothing was loaded...
    if (ioe != nullptr) {
      // ... not for lack of trying:
      throw ioe;
    }
  } else {
    if (snapshotFiles.size() > 1) {
      // Remove any broken / old snapshot files:
      wstring curFileName = SNAPSHOTS_PREFIX + to_wstring(genLoaded);
      for (auto file : snapshotFiles) {
        if (curFileName != file) {
          IOUtils::deleteFilesIgnoringExceptions(dir, {file});
        }
      }
    }
    nextWriteGen = 1 + genLoaded;
  }
}
} // namespace org::apache::lucene::index