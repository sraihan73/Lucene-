using namespace std;

#include "FileSwitchDirectory.h"

namespace org::apache::lucene::store
{
using IOUtils = org::apache::lucene::util::IOUtils;

FileSwitchDirectory::FileSwitchDirectory(
    shared_ptr<Set<wstring>> primaryExtensions,
    shared_ptr<Directory> primaryDir, shared_ptr<Directory> secondaryDir,
    bool doClose)
    : secondaryDir(secondaryDir), primaryDir(primaryDir),
      primaryExtensions(primaryExtensions)
{
  this->doClose = doClose;
}

shared_ptr<Directory> FileSwitchDirectory::getPrimaryDir()
{
  return primaryDir;
}

shared_ptr<Directory> FileSwitchDirectory::getSecondaryDir()
{
  return secondaryDir;
}

shared_ptr<Lock>
FileSwitchDirectory::obtainLock(const wstring &name) 
{
  return getDirectory(name)->obtainLock(name);
}

FileSwitchDirectory::~FileSwitchDirectory()
{
  if (doClose) {
    IOUtils::close({primaryDir, secondaryDir});
    doClose = false;
  }
}

std::deque<wstring> FileSwitchDirectory::listAll() 
{
  shared_ptr<Set<wstring>> files = unordered_set<wstring>();
  // LUCENE-3380: either or both of our dirs could be FSDirs,
  // but if one underlying delegate is an FSDir and mkdirs() has not
  // yet been called, because so far everything is written to the other,
  // in this case, we don't want to throw a NoSuchFileException
  shared_ptr<NoSuchFileException> exc = nullptr;
  try {
    for (auto f : primaryDir->listAll()) {
      files->add(f);
    }
  } catch (const NoSuchFileException &e) {
    exc = e;
  }
  try {
    for (auto f : secondaryDir->listAll()) {
      files->add(f);
    }
  } catch (const NoSuchFileException &e) {
    // we got NoSuchFileException from both dirs
    // rethrow the first.
    if (exc != nullptr) {
      throw exc;
    }
    // we got NoSuchFileException from the secondary,
    // and the primary is empty.
    if (files->isEmpty()) {
      throw e;
    }
  }
  // we got NoSuchFileException from the primary,
  // and the secondary is empty.
  if (exc != nullptr && files->isEmpty()) {
    throw exc;
  }
  std::deque<wstring> result =
      files->toArray(std::deque<wstring>(files->size()));
  Arrays::sort(result);
  return result;
}

wstring FileSwitchDirectory::getExtension(const wstring &name)
{
  int i = (int)name.rfind(L'.');
  if (i == -1) {
    return L"";
  }
  return name.substr(i + 1, name.length() - (i + 1));
}

shared_ptr<Directory> FileSwitchDirectory::getDirectory(const wstring &name)
{
  wstring ext = getExtension(name);
  if (primaryExtensions->contains(ext)) {
    return primaryDir;
  } else {
    return secondaryDir;
  }
}

void FileSwitchDirectory::deleteFile(const wstring &name) 
{
  if (getDirectory(name) == primaryDir) {
    primaryDir->deleteFile(name);
  } else {
    secondaryDir->deleteFile(name);
  }
}

int64_t
FileSwitchDirectory::fileLength(const wstring &name) 
{
  return getDirectory(name)->fileLength(name);
}

shared_ptr<IndexOutput> FileSwitchDirectory::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  return getDirectory(name)->createOutput(name, context);
}

shared_ptr<IndexOutput> FileSwitchDirectory::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  return getDirectory(L"." + suffix)->createTempOutput(prefix, suffix, context);
}

void FileSwitchDirectory::sync(shared_ptr<deque<wstring>> names) throw(
    IOException)
{
  deque<wstring> primaryNames = deque<wstring>();
  deque<wstring> secondaryNames = deque<wstring>();

  for (auto name : names) {
    if (primaryExtensions->contains(getExtension(name))) {
      primaryNames.push_back(name);
    } else {
      secondaryNames.push_back(name);
    }
  }

  primaryDir->sync(primaryNames);
  secondaryDir->sync(secondaryNames);
}

void FileSwitchDirectory::rename(const wstring &source,
                                 const wstring &dest) 
{
  shared_ptr<Directory> sourceDir = getDirectory(source);
  // won't happen with standard lucene index files since pending and commit will
  // always have the same extension ("")
  if (sourceDir != getDirectory(dest)) {
    throw make_shared<AtomicMoveNotSupportedException>(
        source, dest, L"source and dest are in different directories");
  }
  sourceDir->rename(source, dest);
}

void FileSwitchDirectory::syncMetaData() 
{
  primaryDir->syncMetaData();
  secondaryDir->syncMetaData();
}

shared_ptr<IndexInput>
FileSwitchDirectory::openInput(const wstring &name,
                               shared_ptr<IOContext> context) 
{
  return getDirectory(name)->openInput(name, context);
}

shared_ptr<Set<wstring>>
FileSwitchDirectory::getPendingDeletions() 
{
  shared_ptr<Set<wstring>> primaryDeletions = primaryDir->getPendingDeletions();
  shared_ptr<Set<wstring>> secondaryDeletions =
      secondaryDir->getPendingDeletions();
  if (primaryDeletions->isEmpty() && secondaryDeletions->isEmpty()) {
    return Collections::emptySet();
  } else {
    unordered_set<wstring> combined = unordered_set<wstring>();
    combined.addAll(primaryDeletions);
    combined.addAll(secondaryDeletions);
    return Collections::unmodifiableSet(combined);
  }
}
} // namespace org::apache::lucene::store