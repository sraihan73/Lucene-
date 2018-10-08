using namespace std;

#include "RAMDirectory.h"

namespace org::apache::lucene::store
{
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;

RAMDirectory::RAMDirectory() : RAMDirectory(new SingleInstanceLockFactory()) {}

RAMDirectory::RAMDirectory(shared_ptr<LockFactory> lockFactory)
    : BaseDirectory(lockFactory)
{
}

RAMDirectory::RAMDirectory(shared_ptr<FSDirectory> dir,
                           shared_ptr<IOContext> context) 
    : RAMDirectory(dir, false, context)
{
}

RAMDirectory::RAMDirectory(shared_ptr<FSDirectory> dir, bool closeDir,
                           shared_ptr<IOContext> context) 
    : RAMDirectory()
{
  for (auto file : dir->listAll()) {
    if (!Files::isDirectory(dir->getDirectory()->resolve(file))) {
      copyFrom(dir, file, file, context);
    }
  }
  if (closeDir) {
    delete dir;
  }
}

std::deque<wstring> RAMDirectory::listAll()
{
  ensureOpen();
  // NOTE: this returns a "weakly consistent view". Unless we change Dir API,
  // keep this, and do not synchronize or anything stronger. it's great for
  // testing! NOTE: fileMap.keySet().toArray(new std::wstring[0]) is broken in non Sun
  // JDKs, and the code below is resilient to map_obj changes during the array
  // population. NOTE: don't replace this with return names.toArray(new
  // std::wstring[names.size()]); or some files could be null at the end of the array
  // if files are being deleted concurrently
  shared_ptr<Set<wstring>> fileNames = fileMap.keySet();
  deque<wstring> names = deque<wstring>(fileNames->size());
  for (auto name : fileNames) {
    names.push_back(name);
  }
  std::deque<wstring> namesArray =
      names.toArray(std::deque<wstring>(names.size()));
  Arrays::sort(namesArray);
  return namesArray;
}

bool RAMDirectory::fileNameExists(const wstring &name)
{
  ensureOpen();
  return fileMap.find(name) != fileMap.end();
}

int64_t RAMDirectory::fileLength(const wstring &name) 
{
  ensureOpen();
  shared_ptr<RAMFile> file = fileMap[name];
  if (file == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(name);
  }
  return file->getLength();
}

int64_t RAMDirectory::ramBytesUsed()
{
  ensureOpen();
  return sizeInBytes->get();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
RAMDirectory::getChildResources()
{
  return Accountables::namedAccountables(L"file", fileMap);
}

void RAMDirectory::deleteFile(const wstring &name) 
{
  ensureOpen();
  shared_ptr<RAMFile> file = fileMap.erase(name);
  if (file != nullptr) {
    file->directory.reset();
    sizeInBytes->addAndGet(-file->sizeInBytes);
  } else {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(name);
  }
}

shared_ptr<IndexOutput>
RAMDirectory::createOutput(const wstring &name,
                           shared_ptr<IOContext> context) 
{
  ensureOpen();
  shared_ptr<RAMFile> file = newRAMFile();
  if (fileMap.try_emplace(name, file) != nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.FileAlreadyExistsException(name);
  }
  return make_shared<RAMOutputStream>(name, file, true);
}

shared_ptr<IndexOutput>
RAMDirectory::createTempOutput(const wstring &prefix, const wstring &suffix,
                               shared_ptr<IOContext> context) 
{
  ensureOpen();

  // Make the file first...
  shared_ptr<RAMFile> file = newRAMFile();

  // ... then try to find a unique name for it:
  while (true) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring name = IndexFileNames::segmentFileName(
        prefix,
        suffix + L"_" +
            Long::toString(nextTempFileCounter->getAndIncrement(),
                           Character::MAX_RADIX),
        L"tmp");
    if (fileMap.try_emplace(name, file) == nullptr) {
      return make_shared<RAMOutputStream>(name, file, true);
    }
  }
}

shared_ptr<RAMFile> RAMDirectory::newRAMFile()
{
  return make_shared<RAMFile>(shared_from_this());
}

void RAMDirectory::sync(shared_ptr<deque<wstring>> names) throw(
    IOException)
{
}

void RAMDirectory::rename(const wstring &source,
                          const wstring &dest) 
{
  ensureOpen();
  shared_ptr<RAMFile> file = fileMap[source];
  if (file == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(source);
  }
  if (fileMap.try_emplace(dest, file) != nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.nio.file.FileAlreadyExistsException(dest);
  }
  if (!fileMap.erase(source, file)) {
    throw make_shared<IllegalStateException>(
        L"file was unexpectedly replaced: " + source);
  }
  fileMap.erase(source);
}

void RAMDirectory::syncMetaData() 
{
  // we are by definition not durable!
}

shared_ptr<IndexInput>
RAMDirectory::openInput(const wstring &name,
                        shared_ptr<IOContext> context) 
{
  ensureOpen();
  shared_ptr<RAMFile> file = fileMap[name];
  if (file == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(name);
  }
  return make_shared<RAMInputStream>(name, file);
}

RAMDirectory::~RAMDirectory()
{
  isOpen = false;
  fileMap.clear();
}
} // namespace org::apache::lucene::store