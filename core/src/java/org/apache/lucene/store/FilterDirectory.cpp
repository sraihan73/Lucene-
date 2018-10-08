using namespace std;

#include "FilterDirectory.h"

namespace org::apache::lucene::store
{

shared_ptr<Directory> FilterDirectory::unwrap(shared_ptr<Directory> dir)
{
  while (std::dynamic_pointer_cast<FilterDirectory>(dir) != nullptr) {
    dir = (std::static_pointer_cast<FilterDirectory>(dir))->in_;
  }
  return dir;
}

FilterDirectory::FilterDirectory(shared_ptr<Directory> in_) : in_(in_) {}

shared_ptr<Directory> FilterDirectory::getDelegate() { return in_; }

std::deque<wstring> FilterDirectory::listAll() 
{
  return in_->listAll();
}

void FilterDirectory::deleteFile(const wstring &name) 
{
  in_->deleteFile(name);
}

int64_t FilterDirectory::fileLength(const wstring &name) 
{
  return in_->fileLength(name);
}

shared_ptr<IndexOutput>
FilterDirectory::createOutput(const wstring &name,
                              shared_ptr<IOContext> context) 
{
  return in_->createOutput(name, context);
}

shared_ptr<IndexOutput> FilterDirectory::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  return in_->createTempOutput(prefix, suffix, context);
}

void FilterDirectory::sync(shared_ptr<deque<wstring>> names) throw(
    IOException)
{
  in_->sync(names);
}

void FilterDirectory::rename(const wstring &source,
                             const wstring &dest) 
{
  in_->rename(source, dest);
}

void FilterDirectory::syncMetaData()  { in_->syncMetaData(); }

shared_ptr<IndexInput>
FilterDirectory::openInput(const wstring &name,
                           shared_ptr<IOContext> context) 
{
  return in_->openInput(name, context);
}

shared_ptr<Lock>
FilterDirectory::obtainLock(const wstring &name) 
{
  return in_->obtainLock(name);
}

FilterDirectory::~FilterDirectory() { delete in_; }

wstring FilterDirectory::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}

shared_ptr<Set<wstring>>
FilterDirectory::getPendingDeletions() 
{
  return Directory::getPendingDeletions();
}
} // namespace org::apache::lucene::store