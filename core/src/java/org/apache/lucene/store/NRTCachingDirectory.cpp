using namespace std;

#include "NRTCachingDirectory.h"

namespace org::apache::lucene::store
{
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using IOUtils = org::apache::lucene::util::IOUtils;

NRTCachingDirectory::NRTCachingDirectory(shared_ptr<Directory> delegate_,
                                         double maxMergeSizeMB,
                                         double maxCachedMB)
    : FilterDirectory(delegate_),
      maxMergeSizeBytes(static_cast<int64_t>(maxMergeSizeMB * 1024 * 1024)),
      maxCachedBytes(static_cast<int64_t>(maxCachedMB * 1024 * 1024))
{
}

wstring NRTCachingDirectory::toString()
{
  return L"NRTCachingDirectory(" + in_ + L"; maxCacheMB=" +
         to_wstring(maxCachedBytes / 1024 / 1024.0) + L" maxMergeSizeMB=" +
         to_wstring(maxMergeSizeBytes / 1024 / 1024.0) + L")";
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<wstring> NRTCachingDirectory::listAll() 
{
  shared_ptr<Set<wstring>> *const files = unordered_set<wstring>();
  for (auto f : cache->listAll()) {
    files->add(f);
  }
  for (auto f : in_->listAll()) {
    files->add(f);
  }
  std::deque<wstring> result =
      files->toArray(std::deque<wstring>(files->size()));
  Arrays::sort(result);
  return result;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void NRTCachingDirectory::deleteFile(const wstring &name) 
{
  if (VERBOSE) {
    wcout << L"nrtdir.deleteFile name=" << name << endl;
  }
  if (cache->fileNameExists(name)) {
    cache->deleteFile(name);
  } else {
    in_->deleteFile(name);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
NRTCachingDirectory::fileLength(const wstring &name) 
{
  if (cache->fileNameExists(name)) {
    return cache->fileLength(name);
  } else {
    return in_->fileLength(name);
  }
}

std::deque<wstring> NRTCachingDirectory::listCachedFiles()
{
  return cache->listAll();
}

shared_ptr<IndexOutput> NRTCachingDirectory::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  if (VERBOSE) {
    wcout << L"nrtdir.createOutput name=" << name << endl;
  }
  if (doCacheWrite(name, context)) {
    if (VERBOSE) {
      wcout << L"  to cache" << endl;
    }
    return cache->createOutput(name, context);
  } else {
    return in_->createOutput(name, context);
  }
}

void NRTCachingDirectory::sync(shared_ptr<deque<wstring>> fileNames) throw(
    IOException)
{
  if (VERBOSE) {
    wcout << L"nrtdir.sync files=" << fileNames << endl;
  }
  for (auto fileName : fileNames) {
    unCache(fileName);
  }
  in_->sync(fileNames);
}

void NRTCachingDirectory::rename(const wstring &source,
                                 const wstring &dest) 
{
  unCache(source);
  if (cache->fileNameExists(dest)) {
    throw invalid_argument(L"target file " + dest + L" already exists");
  }
  in_->rename(source, dest);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexInput>
NRTCachingDirectory::openInput(const wstring &name,
                               shared_ptr<IOContext> context) 
{
  if (VERBOSE) {
    wcout << L"nrtdir.openInput name=" << name << endl;
  }
  if (cache->fileNameExists(name)) {
    if (VERBOSE) {
      wcout << L"  from cache" << endl;
    }
    return cache->openInput(name, context);
  } else {
    return in_->openInput(name, context);
  }
}

NRTCachingDirectory::~NRTCachingDirectory()
{
  // NOTE: technically we shouldn't have to do this, ie,
  // IndexWriter should have sync'd all files, but we do
  // it for defensive reasons... or in case the app is
  // doing something custom (creating outputs directly w/o
  // using IndexWriter):
  bool success = false;
  try {
    if (cache->isOpen) {
      for (auto fileName : cache->listAll()) {
        unCache(fileName);
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({cache, in_});
    } else {
      IOUtils::closeWhileHandlingException({cache, in_});
    }
  }
}

bool NRTCachingDirectory::doCacheWrite(const wstring &name,
                                       shared_ptr<IOContext> context)
{
  // System.out.println(Thread.currentThread().getName() + ": CACHE check
  // merge=" + merge + " size=" + (merge==null ? 0 : merge.estimatedMergeBytes));

  int64_t bytes = 0;
  if (context->mergeInfo != nullptr) {
    bytes = context->mergeInfo->estimatedMergeBytes;
  } else if (context->flushInfo != nullptr) {
    bytes = context->flushInfo->estimatedSegmentSize;
  }

  return (bytes <= maxMergeSizeBytes) &&
         (bytes + cache->ramBytesUsed()) <= maxCachedBytes;
}

shared_ptr<IndexOutput> NRTCachingDirectory::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  if (VERBOSE) {
    wcout << L"nrtdir.createTempOutput prefix=" << prefix << L" suffix="
          << suffix << endl;
  }
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();

  // This is very ugly/messy/dangerous (can in some disastrous case maybe create
  // too many temp files), but I don't know of a cleaner way:
  bool success = false;

  shared_ptr<Directory> first;
  shared_ptr<Directory> second;
  if (doCacheWrite(prefix, context)) {
    first = cache;
    second = in_;
  } else {
    first = in_;
    second = cache;
  }

  shared_ptr<IndexOutput> out = nullptr;
  try {
    while (true) {
      out = first->createTempOutput(prefix, suffix, context);
      wstring name = out->getName();
      toDelete->add(name);
      if (slowFileExists(second, name)) {
        delete out;
      } else {
        toDelete->remove(name);
        success = true;
        break;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::deleteFiles(first, toDelete);
    } else {
      IOUtils::closeWhileHandlingException({out});
      IOUtils::deleteFilesIgnoringExceptions({first, toDelete});
    }
  }

  return out;
}

bool NRTCachingDirectory::slowFileExists(
    shared_ptr<Directory> dir, const wstring &fileName) 
{
  try {
    delete dir->openInput(fileName, IOContext::DEFAULT);
    return true;
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | FileNotFoundException e) {
    return false;
  }
}

void NRTCachingDirectory::unCache(const wstring &fileName) 
{
  // Only let one thread uncache at a time; this only
  // happens during commit() or close():
  {
    lock_guard<mutex> lock(uncacheLock);
    if (VERBOSE) {
      wcout << L"nrtdir.unCache name=" << fileName << endl;
    }
    if (!cache->fileNameExists(fileName)) {
      // Another thread beat us...
      return;
    }
    assert(
        (slowFileExists(in_, fileName) == false,
         L"fileName=" + fileName + L" exists both in cache and in delegate"));

    shared_ptr<IOContext> *const context = IOContext::DEFAULT;
    shared_ptr<IndexOutput> *const out = in_->createOutput(fileName, context);
    shared_ptr<IndexInput> in_ = nullptr;
    try {
      in_ = cache->openInput(fileName, context);
      out->copyBytes(in_, in_->length());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::close({in_, out});
    }

    // Lock order: uncacheLock -> this
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      // Must sync here because other sync methods have
      // if (cache.fileNameExists(name)) { ... } else { ... }:
      cache->deleteFile(fileName);
    }
  }
}

int64_t NRTCachingDirectory::ramBytesUsed() { return cache->ramBytesUsed(); }

shared_ptr<deque<std::shared_ptr<Accountable>>>
NRTCachingDirectory::getChildResources()
{
  return Collections::singleton(
      Accountables::namedAccountable(L"cache", cache));
}
} // namespace org::apache::lucene::store