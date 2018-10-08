using namespace std;

#include "FilterPath.h"

namespace org::apache::lucene::mockfile
{
using org::apache::lucene::util::SuppressForbidden;

FilterPath::FilterPath(shared_ptr<Path> delegate_,
                       shared_ptr<FileSystem> fileSystem)
    : delegate_(delegate_), fileSystem(fileSystem)
{
}

shared_ptr<Path> FilterPath::getDelegate() { return delegate_; }

shared_ptr<FileSystem> FilterPath::getFileSystem() { return fileSystem; }

bool FilterPath::isAbsolute() { return delegate_->isAbsolute(); }

shared_ptr<Path> FilterPath::getRoot()
{
  shared_ptr<Path> root = delegate_->getRoot();
  if (root == nullptr) {
    return nullptr;
  }
  return wrap(root);
}

shared_ptr<Path> FilterPath::getFileName()
{
  shared_ptr<Path> fileName = delegate_->getFileName();
  if (fileName == nullptr) {
    return nullptr;
  }
  return wrap(fileName);
}

shared_ptr<Path> FilterPath::getParent()
{
  shared_ptr<Path> parent = delegate_->getParent();
  if (parent == nullptr) {
    return nullptr;
  }
  return wrap(parent);
}

int FilterPath::getNameCount() { return delegate_->getNameCount(); }

shared_ptr<Path> FilterPath::getName(int index)
{
  return wrap(delegate_->getName(index));
}

shared_ptr<Path> FilterPath::subpath(int beginIndex, int endIndex)
{
  return wrap(delegate_->subpath(beginIndex, endIndex));
}

bool FilterPath::startsWith(shared_ptr<Path> other)
{
  return delegate_->startsWith(toDelegate(other));
}

bool FilterPath::startsWith(const wstring &other)
{
  return delegate_->startsWith(other);
}

bool FilterPath::endsWith(shared_ptr<Path> other)
{
  return delegate_->endsWith(toDelegate(other));
}

bool FilterPath::endsWith(const wstring &other)
{
  return delegate_->startsWith(other);
}

shared_ptr<Path> FilterPath::normalize()
{
  return wrap(delegate_->normalize());
}

shared_ptr<Path> FilterPath::resolve(shared_ptr<Path> other)
{
  return wrap(delegate_->resolve(toDelegate(other)));
}

shared_ptr<Path> FilterPath::resolve(const wstring &other)
{
  return wrap(delegate_->resolve(other));
}

shared_ptr<Path> FilterPath::resolveSibling(shared_ptr<Path> other)
{
  return wrap(delegate_->resolveSibling(toDelegate(other)));
}

shared_ptr<Path> FilterPath::resolveSibling(const wstring &other)
{
  return wrap(delegate_->resolveSibling(other));
}

shared_ptr<Path> FilterPath::relativize(shared_ptr<Path> other)
{
  return wrap(delegate_->relativize(toDelegate(other)));
}

shared_ptr<URI> FilterPath::toUri() { return delegate_->toUri(); }

wstring FilterPath::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return delegate_->toString();
}

shared_ptr<Path> FilterPath::toAbsolutePath()
{
  return wrap(delegate_->toAbsolutePath());
}

shared_ptr<Path>
FilterPath::toRealPath(deque<LinkOption> &options) 
{
  return wrap(delegate_->toRealPath(options));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressForbidden(reason = "Abstract API requires
// to use java.io.File") public java.io.File toFile()
shared_ptr<File> FilterPath::toFile()
{
  // TODO: should we throw exception here?
  return delegate_->toFile();
}

template <typename T1>
shared_ptr<WatchKey>
FilterPath::register_(shared_ptr<WatchService> watcher,
                      std::deque<Kind<T1>> &events,
                      deque<Modifier> &modifiers) 
{
  return delegate_->register_(watcher, events, modifiers);
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public java.nio.file.WatchKey
// register(java.nio.file.WatchService watcher,
// java.nio.file.WatchEvent.Kind<?>... events) throws java.io.IOException
    shared_ptr<WatchKey> FilterPath::register_(shared_ptr<WatchService> watcher, deque<Kind<?>> &events) 
    {
      return delegate_->register_(watcher, events);
    }

    shared_ptr<Iterator<std::shared_ptr<Path>>> FilterPath::iterator()
    {
      constexpr Path::const_iterator iterator = delegate_->begin();
      return make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                      iterator);
    }

    FilterPath::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<FilterPath> outerInstance,
        shared_ptr<Path::const_iterator> iterator)
    {
      this->outerInstance = outerInstance;
      this->iterator = iterator;
    }

    bool FilterPath::IteratorAnonymousInnerClass::hasNext()
    {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      return iterator->hasNext();
    }

    shared_ptr<Path> FilterPath::IteratorAnonymousInnerClass::next()
    {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      return outerInstance->wrap(iterator->next());
    }

    void FilterPath::IteratorAnonymousInnerClass::remove()
    {
      iterator->remove();
    }

    int FilterPath::compareTo(shared_ptr<Path> other)
    {
      return delegate_->compareTo(toDelegate(other));
    }

    int FilterPath::hashCode() { return delegate_->hashCode(); }

    bool FilterPath::equals(any obj)
    {
      if (shared_from_this() == obj) {
        return true;
      }
      if (obj == nullptr) {
        return false;
      }
      if (getClass() != obj.type()) {
        return false;
      }
      shared_ptr<FilterPath> other = any_cast<std::shared_ptr<FilterPath>>(obj);
      if (delegate_ == nullptr) {
        if (other->delegate_ != nullptr) {
          return false;
        }
      } else if (!delegate_->equals(other->delegate_)) {
        return false;
      }
      if (fileSystem == nullptr) {
        if (other->fileSystem != nullptr) {
          return false;
        }
      } else if (!fileSystem->equals(other->fileSystem)) {
        return false;
      }
      return true;
    }

    shared_ptr<Path> FilterPath::unwrap(shared_ptr<Path> path)
    {
      while (std::dynamic_pointer_cast<FilterPath>(path) != nullptr) {
        path = (std::static_pointer_cast<FilterPath>(path))->delegate_;
      }
      return path;
    }

    shared_ptr<Path> FilterPath::wrap(shared_ptr<Path> other)
    {
      return make_shared<FilterPath>(other, fileSystem);
    }

    shared_ptr<Path> FilterPath::toDelegate(shared_ptr<Path> path)
    {
      if (std::dynamic_pointer_cast<FilterPath>(path) != nullptr) {
        shared_ptr<FilterPath> fp = std::static_pointer_cast<FilterPath>(path);
        if (fp->fileSystem != fileSystem) {
          // C++ TODO: The following line could not be converted:
          throw java.nio.file.ProviderMismatchException(
              L"mismatch, expected: " + fileSystem.provider().getClass() +
              L", got: " + fp.fileSystem.provider().getClass());
        }
        return fp->delegate_;
      } else {
        // C++ TODO: The following line could not be converted:
        throw java.nio.file.ProviderMismatchException(
            L"mismatch, expected: FilterPath, got: " + path.getClass());
      }
    }
    } // namespace org::apache::lucene::mockfile