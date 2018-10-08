using namespace std;

#include "FilterFileSystem.h"

namespace org::apache::lucene::mockfile
{

FilterFileSystem::FilterFileSystem(shared_ptr<FilterFileSystemProvider> parent,
                                   shared_ptr<FileSystem> delegate_)
    : parent(Objects::requireNonNull(parent)),
      delegate_(Objects::requireNonNull(delegate_))
{
}

shared_ptr<FileSystemProvider> FilterFileSystem::provider() { return parent; }

FilterFileSystem::~FilterFileSystem()
{
  if (delegate_ == FileSystems::getDefault()) {
    // you can't close the default provider!
    parent->onClose();
  } else {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.nio.file.FileSystem d = delegate)
    {
      java::nio::file::FileSystem d = delegate_;
      assert(d != nullptr); // avoid stupid compiler warning
      parent->onClose();
    }
  }
}

bool FilterFileSystem::isOpen() { return delegate_->isOpen(); }

bool FilterFileSystem::isReadOnly() { return delegate_->isReadOnly(); }

wstring FilterFileSystem::getSeparator() { return delegate_->getSeparator(); }

deque<std::shared_ptr<Path>> FilterFileSystem::getRootDirectories()
{
  const deque<std::shared_ptr<Path>> roots = delegate_->getRootDirectories();
  return [&]() {
    constexpr deque<std::shared_ptr<java::nio::file::Path>>::const_iterator
        iterator = roots.begin();
    return make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                    iterator);
  };
}

FilterFileSystem::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<FilterFileSystem> outerInstance,
    deque<Path>::const_iterator iterator)
{
  this->outerInstance = outerInstance;
  this->iterator = iterator;
}

bool FilterFileSystem::IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return iterator.hasNext();
}

shared_ptr<Path> FilterFileSystem::IteratorAnonymousInnerClass::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return make_shared<FilterPath>(iterator.next(), outerInstance);
}

void FilterFileSystem::IteratorAnonymousInnerClass::remove()
{
  iterator.remove();
}

deque<std::shared_ptr<FileStore>> FilterFileSystem::getFileStores()
{
  const deque<std::shared_ptr<FileStore>> fileStores =
      delegate_->getFileStores();
  return [&]() {
    constexpr deque<
        std::shared_ptr<java::nio::file::FileStore>>::const_iterator iterator =
        fileStores.begin();
    return make_shared<IteratorAnonymousInnerClass2>(shared_from_this(),
                                                     iterator);
  };
}

FilterFileSystem::IteratorAnonymousInnerClass2::IteratorAnonymousInnerClass2(
    shared_ptr<FilterFileSystem> outerInstance,
    deque<FileStore>::const_iterator iterator)
{
  this->outerInstance = outerInstance;
  this->iterator = iterator;
}

bool FilterFileSystem::IteratorAnonymousInnerClass2::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return iterator.hasNext();
}

shared_ptr<FileStore> FilterFileSystem::IteratorAnonymousInnerClass2::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return make_shared<FilterFileStoreAnonymousInnerClass>(
      shared_from_this(), iterator.next(), outerInstance->parent->getScheme());
}

FilterFileSystem::IteratorAnonymousInnerClass2::
    FilterFileStoreAnonymousInnerClass::FilterFileStoreAnonymousInnerClass(
        shared_ptr<IteratorAnonymousInnerClass2> outerInstance,
        shared_ptr<UnknownType> next, const wstring &getScheme)
    : FilterFileStore(next, getScheme)
{
  this->outerInstance = outerInstance;
}

void FilterFileSystem::IteratorAnonymousInnerClass2::remove()
{
  iterator.remove();
}

shared_ptr<Set<wstring>> FilterFileSystem::supportedFileAttributeViews()
{
  return delegate_->supportedFileAttributeViews();
}

shared_ptr<Path> FilterFileSystem::getPath(const wstring &first,
                                           deque<wstring> &more)
{
  return make_shared<FilterPath>(delegate_->getPath(first, more),
                                 shared_from_this());
}

shared_ptr<PathMatcher>
FilterFileSystem::getPathMatcher(const wstring &syntaxAndPattern)
{
  shared_ptr<PathMatcher> *const matcher =
      delegate_->getPathMatcher(syntaxAndPattern);
  return [&](any path) {
    if (std::dynamic_pointer_cast<FilterPath>(path) != nullptr) {
      return matcher->matches(
          (std::static_pointer_cast<FilterPath>(path))->delegate_);
    }
    return false;
  };
}

shared_ptr<UserPrincipalLookupService>
FilterFileSystem::getUserPrincipalLookupService()
{
  return delegate_->getUserPrincipalLookupService();
}

shared_ptr<WatchService> FilterFileSystem::newWatchService() 
{
  return delegate_->newWatchService();
}

shared_ptr<FileSystem> FilterFileSystem::getDelegate() { return delegate_; }

shared_ptr<FileSystemProvider> FilterFileSystem::getParent() { return parent; }
} // namespace org::apache::lucene::mockfile