using namespace std;

#include "FilterFileSystemProvider.h"

namespace org::apache::lucene::mockfile
{

FilterFileSystemProvider::FilterFileSystemProvider(
    const wstring &scheme, shared_ptr<FileSystem> delegateInstance)
    : delegate_(delegateInstance->provider()),
      scheme(Objects::requireNonNull(scheme))
{
  Objects::requireNonNull(delegateInstance);
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  this->fileSystem =
      make_shared<FilterFileSystem>(shared_from_this(), delegateInstance);
}

FilterFileSystemProvider::FilterFileSystemProvider(
    const wstring &scheme, shared_ptr<FileSystemProvider> delegate_)
    : delegate_(Objects::requireNonNull(delegate_)),
      scheme(Objects::requireNonNull(scheme))
{
}

wstring FilterFileSystemProvider::getScheme() { return scheme; }

template <typename wstring, typename T1>
shared_ptr<FileSystem> FilterFileSystemProvider::newFileSystem(
    shared_ptr<URI> uri, unordered_map<T1> env) 
{
  if (fileSystem == nullptr) {
    throw make_shared<IllegalStateException>(
        L"subclass did not initialize singleton filesystem");
  }
  return fileSystem;
}

template <typename wstring, typename T1>
shared_ptr<FileSystem> FilterFileSystemProvider::newFileSystem(
    shared_ptr<Path> path, unordered_map<T1> env) 
{
  if (fileSystem == nullptr) {
    throw make_shared<IllegalStateException>(
        L"subclass did not initialize singleton filesystem");
  }
  return fileSystem;
}

shared_ptr<FileSystem>
FilterFileSystemProvider::getFileSystem(shared_ptr<URI> uri)
{
  if (fileSystem == nullptr) {
    throw make_shared<IllegalStateException>(
        L"subclass did not initialize singleton filesystem");
  }
  return fileSystem;
}

shared_ptr<Path> FilterFileSystemProvider::getPath(shared_ptr<URI> uri)
{
  if (fileSystem == nullptr) {
    throw make_shared<IllegalStateException>(
        L"subclass did not initialize singleton filesystem");
  }
  shared_ptr<Path> path = delegate_->getPath(uri);
  return make_shared<FilterPath>(path, fileSystem);
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public void createDirectory(java.nio.file.Path dir,
// java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
    void FilterFileSystemProvider::createDirectory(shared_ptr<Path> dir, deque<FileAttribute<?>> &attrs) 
    {
      delegate_->createDirectory(toDelegate(dir), attrs);
    }

    void
    FilterFileSystemProvider::delete_(shared_ptr<Path> path) 
    {
      delegate_->delete (toDelegate(path));
    }

    void FilterFileSystemProvider::copy(
        shared_ptr<Path> source, shared_ptr<Path> target,
        deque<CopyOption> &options) 
    {
      delegate_->copy(toDelegate(source), toDelegate(target), options);
    }

    void FilterFileSystemProvider::move(
        shared_ptr<Path> source, shared_ptr<Path> target,
        deque<CopyOption> &options) 
    {
      delegate_->move(toDelegate(source), toDelegate(target), options);
    }

    bool FilterFileSystemProvider::isSameFile(
        shared_ptr<Path> path, shared_ptr<Path> path2) 
    {
      return delegate_->isSameFile(toDelegate(path), toDelegate(path2));
    }

    bool
    FilterFileSystemProvider::isHidden(shared_ptr<Path> path) 
    {
      return delegate_->isHidden(toDelegate(path));
    }

    shared_ptr<FileStore> FilterFileSystemProvider::getFileStore(
        shared_ptr<Path> path) 
    {
      return delegate_->getFileStore(toDelegate(path));
    }

    void FilterFileSystemProvider::checkAccess(
        shared_ptr<Path> path, deque<AccessMode> &modes) 
    {
      delegate_->checkAccess(toDelegate(path), modes);
    }

    template <typename V>
    V FilterFileSystemProvider::getFileAttributeView(
        shared_ptr<Path> path, type_info<V> &type, deque<LinkOption> &options)
    {
      static_assert(
          is_base_of<java.nio.file.attribute.FileAttributeView, V>::value,
          L"V must inherit from java.nio.file.attribute.FileAttributeView");

      return delegate_->getFileAttributeView(toDelegate(path), type, options);
    }

    template <typename A>
    A FilterFileSystemProvider::readAttributes(
        shared_ptr<Path> path, type_info<A> &type,
        deque<LinkOption> &options) 
    {
      static_assert(
          is_base_of<java.nio.file.attribute.BasicFileAttributes, A>::value,
          L"A must inherit from java.nio.file.attribute.BasicFileAttributes");

      return delegate_->readAttributes(toDelegate(path), type, options);
    }

    unordered_map<wstring, any> FilterFileSystemProvider::readAttributes(
        shared_ptr<Path> path, const wstring &attributes,
        deque<LinkOption> &options) 
    {
      return delegate_->readAttributes(toDelegate(path), attributes, options);
    }

    void FilterFileSystemProvider::setAttribute(
        shared_ptr<Path> path, const wstring &attribute, any value,
        deque<LinkOption> &options) 
    {
      delegate_->setAttribute(toDelegate(path), attribute, value, options);
    }

    shared_ptr<InputStream> FilterFileSystemProvider::newInputStream(
        shared_ptr<Path> path, deque<OpenOption> &options) 
    {
      return delegate_->newInputStream(toDelegate(path), options);
    }

    shared_ptr<OutputStream> FilterFileSystemProvider::newOutputStream(
        shared_ptr<Path> path, deque<OpenOption> &options) 
    {
      return delegate_->newOutputStream(toDelegate(path), options);
    }

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<FileChannel> FilterFileSystemProvider::newFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  return delegate_->newFileChannel(toDelegate(path), options, attrs);
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<AsynchronousFileChannel> FilterFileSystemProvider::newAsynchronousFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, shared_ptr<ExecutorService> executor, deque<FileAttribute<?>> &attrs) 
{
  return delegate_->newAsynchronousFileChannel(toDelegate(path), options,
                                               executor, attrs);
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<SeekableByteChannel> FilterFileSystemProvider::newByteChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  return delegate_->newByteChannel(toDelegate(path), options, attrs);
}

template <typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: @Override public
// java.nio.file.DirectoryStream<java.nio.file.Path>
// newDirectoryStream(java.nio.file.Path dir, final
// java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path> filter)
// throws java.io.IOException
shared_ptr<DirectoryStream<std::shared_ptr<Path>>>
FilterFileSystemProvider::newDirectoryStream(
    shared_ptr<Path> dir,
    shared_ptr<DirectoryStream::Filter<T1>> filter) 
{
  shared_ptr<DirectoryStream::Filter<std::shared_ptr<Path>>> wrappedFilter =
      make_shared<FilterAnonymousInnerClass>(shared_from_this(), filter);
  return make_shared<FilterDirectoryStream>(
      delegate_->newDirectoryStream(toDelegate(dir), wrappedFilter),
      fileSystem);
}

template <typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public FilterAnonymousInnerClass(FilterFileSystemProvider
// outerInstance, java.nio.file.DirectoryStream.Filter<? super
// java.nio.file.Path> filter)
FilterFileSystemProvider::FilterAnonymousInnerClass::FilterAnonymousInnerClass(
    shared_ptr<FilterFileSystemProvider> outerInstance,
    shared_ptr<DirectoryStream::Filter<T1>> filter)
{
  this->outerInstance = outerInstance;
  this->filter = filter;
}

bool FilterFileSystemProvider::FilterAnonymousInnerClass::accept(
    shared_ptr<Path> entry) 
{
  return filter->accept(
      make_shared<FilterPath>(entry, outerInstance->fileSystem));
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public void createSymbolicLink(java.nio.file.Path
// link, java.nio.file.Path target, java.nio.file.attribute.FileAttribute<?>...
// attrs) throws java.io.IOException
    void FilterFileSystemProvider::createSymbolicLink(shared_ptr<Path> link, shared_ptr<Path> target, deque<FileAttribute<?>> &attrs) 
    {
      delegate_->createSymbolicLink(toDelegate(link), toDelegate(target),
                                    attrs);
    }

    void FilterFileSystemProvider::createLink(
        shared_ptr<Path> link, shared_ptr<Path> existing) 
    {
      delegate_->createLink(toDelegate(link), toDelegate(existing));
    }

    bool FilterFileSystemProvider::deleteIfExists(shared_ptr<Path> path) throw(
        IOException)
    {
      return delegate_->deleteIfExists(toDelegate(path));
    }

    shared_ptr<Path> FilterFileSystemProvider::readSymbolicLink(
        shared_ptr<Path> link) 
    {
      return delegate_->readSymbolicLink(toDelegate(link));
    }

    shared_ptr<Path> FilterFileSystemProvider::toDelegate(shared_ptr<Path> path)
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

    void FilterFileSystemProvider::onClose() {}

    wstring FilterFileSystemProvider::toString()
    {
      return getClass().getSimpleName() + L"(" + delegate_ + L")";
    }
    } // namespace org::apache::lucene::mockfile