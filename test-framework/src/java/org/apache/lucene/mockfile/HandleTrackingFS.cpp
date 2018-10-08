using namespace std;

#include "HandleTrackingFS.h"

namespace org::apache::lucene::mockfile
{
using IOUtils = org::apache::lucene::util::IOUtils;

HandleTrackingFS::HandleTrackingFS(const wstring &scheme,
                                   shared_ptr<FileSystem> delegate_)
    : FilterFileSystemProvider(scheme, delegate_)
{
}

void HandleTrackingFS::callOpenHook(
    shared_ptr<Path> path, shared_ptr<Closeable> stream) 
{
  bool success = false;
  try {
    onOpen(path, stream);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({stream});
    }
  }
}

shared_ptr<InputStream>
HandleTrackingFS::newInputStream(shared_ptr<Path> path,
                                 deque<OpenOption> &options) 
{
  shared_ptr<InputStream> stream =
      make_shared<FilterInputStream2AnonymousInnerClass>(
          shared_from_this(),
          FilterFileSystemProvider::newInputStream(path, {options}), path);
  callOpenHook(path, stream);
  return stream;
}

HandleTrackingFS::FilterInputStream2AnonymousInnerClass::
    FilterInputStream2AnonymousInnerClass(
        shared_ptr<HandleTrackingFS> outerInstance,
        shared_ptr<InputStream> newInputStream, shared_ptr<Path> path)
    : FilterInputStream2(newInputStream)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

HandleTrackingFS::FilterInputStream2AnonymousInnerClass::
    ~FilterInputStream2AnonymousInnerClass()
{
  try {
    if (!closed) {
      closed = true;
      outerInstance->onClose(path, shared_from_this());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}

wstring HandleTrackingFS::FilterInputStream2AnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"InputStream(" + path->toString() + L")";
}

int HandleTrackingFS::FilterInputStream2AnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HandleTrackingFS::FilterInputStream2AnonymousInnerClass::equals(any obj)
{
  return shared_from_this() == obj;
}

shared_ptr<OutputStream> HandleTrackingFS::newOutputStream(
    shared_ptr<Path> path, deque<OpenOption> &options) 
{
  shared_ptr<OutputStream> stream =
      make_shared<FilterOutputStream2AnonymousInnerClass>(
          shared_from_this(),
          delegate_->newOutputStream(toDelegate(path), options), path);
  callOpenHook(path, stream);
  return stream;
}

HandleTrackingFS::FilterOutputStream2AnonymousInnerClass::
    FilterOutputStream2AnonymousInnerClass(
        shared_ptr<HandleTrackingFS> outerInstance,
        shared_ptr<UnknownType> newOutputStream, shared_ptr<Path> path)
    : FilterOutputStream2(newOutputStream)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

HandleTrackingFS::FilterOutputStream2AnonymousInnerClass::
    ~FilterOutputStream2AnonymousInnerClass()
{
  try {
    if (!closed) {
      closed = true;
      outerInstance->onClose(path, shared_from_this());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}

wstring HandleTrackingFS::FilterOutputStream2AnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"OutputStream(" + path->toString() + L")";
}

int HandleTrackingFS::FilterOutputStream2AnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HandleTrackingFS::FilterOutputStream2AnonymousInnerClass::equals(any obj)
{
  return shared_from_this() == obj;
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<FileChannel> HandleTrackingFS::newFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  shared_ptr<FileChannel> channel =
      make_shared<FilterFileChannelAnonymousInnerClass>(
          shared_from_this(),
          delegate_->newFileChannel(toDelegate(path), options, attrs), path);
  callOpenHook(path, channel);
  return channel;
}

HandleTrackingFS::FilterFileChannelAnonymousInnerClass::
    FilterFileChannelAnonymousInnerClass(
        shared_ptr<HandleTrackingFS> outerInstance,
        shared_ptr<UnknownType> newFileChannel, shared_ptr<Path> path)
    : FilterFileChannel(newFileChannel)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

void HandleTrackingFS::FilterFileChannelAnonymousInnerClass::
    implCloseChannel() 
{
  if (!closed) {
    closed = true;
    try {
      outerInstance->onClose(path, shared_from_this());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      outerInstance->super->implCloseChannel();
    }
  }
}

wstring HandleTrackingFS::FilterFileChannelAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FileChannel(" + path->toString() + L")";
}

int HandleTrackingFS::FilterFileChannelAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HandleTrackingFS::FilterFileChannelAnonymousInnerClass::equals(any obj)
{
  return shared_from_this() == obj;
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<AsynchronousFileChannel> HandleTrackingFS::newAsynchronousFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, shared_ptr<ExecutorService> executor, deque<FileAttribute<?>> &attrs) 
{
  shared_ptr<AsynchronousFileChannel> channel =
      make_shared<FilterAsynchronousFileChannelAnonymousInnerClass>(
          shared_from_this(),
          FilterFileSystemProvider::newAsynchronousFileChannel(
              path, options, executor, {attrs}),
          path);
  callOpenHook(path, channel);
  return channel;
}

HandleTrackingFS::FilterAsynchronousFileChannelAnonymousInnerClass::
    FilterAsynchronousFileChannelAnonymousInnerClass(
        shared_ptr<HandleTrackingFS> outerInstance,
        shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel,
        shared_ptr<Path> path)
    : FilterAsynchronousFileChannel(newAsynchronousFileChannel)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

HandleTrackingFS::FilterAsynchronousFileChannelAnonymousInnerClass::
    ~FilterAsynchronousFileChannelAnonymousInnerClass()
{
  try {
    if (!closed) {
      closed = true;
      outerInstance->onClose(path, shared_from_this());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}

wstring
HandleTrackingFS::FilterAsynchronousFileChannelAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"AsynchronousFileChannel(" + path->toString() + L")";
}

int HandleTrackingFS::FilterAsynchronousFileChannelAnonymousInnerClass::
    hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HandleTrackingFS::FilterAsynchronousFileChannelAnonymousInnerClass::equals(
    any obj)
{
  return shared_from_this() == obj;
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<SeekableByteChannel> HandleTrackingFS::newByteChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  shared_ptr<SeekableByteChannel> channel =
      make_shared<FilterSeekableByteChannelAnonymousInnerClass>(
          shared_from_this(),
          FilterFileSystemProvider::newByteChannel(path, options, {attrs}),
          path);
  callOpenHook(path, channel);
  return channel;
}

HandleTrackingFS::FilterSeekableByteChannelAnonymousInnerClass::
    FilterSeekableByteChannelAnonymousInnerClass(
        shared_ptr<HandleTrackingFS> outerInstance,
        shared_ptr<SeekableByteChannel> newByteChannel, shared_ptr<Path> path)
    : FilterSeekableByteChannel(newByteChannel)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

HandleTrackingFS::FilterSeekableByteChannelAnonymousInnerClass::
    ~FilterSeekableByteChannelAnonymousInnerClass()
{
  try {
    if (!closed) {
      closed = true;
      outerInstance->onClose(path, shared_from_this());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}

wstring
HandleTrackingFS::FilterSeekableByteChannelAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"SeekableByteChannel(" + path->toString() + L")";
}

int HandleTrackingFS::FilterSeekableByteChannelAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HandleTrackingFS::FilterSeekableByteChannelAnonymousInnerClass::equals(
    any obj)
{
  return shared_from_this() == obj;
}

template <typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: @Override public
// java.nio.file.DirectoryStream<java.nio.file.Path>
// newDirectoryStream(java.nio.file.Path dir,
// java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path> filter)
// throws java.io.IOException
shared_ptr<DirectoryStream<std::shared_ptr<Path>>>
HandleTrackingFS::newDirectoryStream(
    shared_ptr<Path> dir,
    shared_ptr<DirectoryStream::Filter<T1>> filter) 
{
  shared_ptr<DirectoryStream::Filter<std::shared_ptr<Path>>> wrappedFilter =
      make_shared<FilterAnonymousInnerClass>(shared_from_this(), filter);
  shared_ptr<DirectoryStream<std::shared_ptr<Path>>> stream =
      delegate_->newDirectoryStream(toDelegate(dir), wrappedFilter);
  stream = make_shared<FilterDirectoryStreamAnonymousInnerClass>(
      shared_from_this(), stream, fileSystem, dir);
  callOpenHook(dir, stream);
  return stream;
}

template <typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public FilterAnonymousInnerClass(HandleTrackingFS
// outerInstance, java.nio.file.DirectoryStream.Filter<? super
// java.nio.file.Path> filter)
HandleTrackingFS::FilterAnonymousInnerClass::FilterAnonymousInnerClass(
    shared_ptr<HandleTrackingFS> outerInstance,
    shared_ptr<DirectoryStream::Filter<T1>> filter)
{
  this->outerInstance = outerInstance;
  this->filter = filter;
}

bool HandleTrackingFS::FilterAnonymousInnerClass::accept(
    shared_ptr<Path> entry) 
{
  return filter->accept(
      make_shared<FilterPath>(entry, outerInstance->fileSystem));
}

HandleTrackingFS::FilterDirectoryStreamAnonymousInnerClass::
    FilterDirectoryStreamAnonymousInnerClass(
        shared_ptr<HandleTrackingFS> outerInstance,
        shared_ptr<DirectoryStream<std::shared_ptr<Path>>> stream,
        shared_ptr<FileSystem> fileSystem, shared_ptr<Path> dir)
    : FilterDirectoryStream(stream, fileSystem)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

HandleTrackingFS::FilterDirectoryStreamAnonymousInnerClass::
    ~FilterDirectoryStreamAnonymousInnerClass()
{
  try {
    if (!closed) {
      closed = true;
      outerInstance->onClose(dir, shared_from_this());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}

wstring HandleTrackingFS::FilterDirectoryStreamAnonymousInnerClass::toString()
{
  return L"DirectoryStream(" + dir + L")";
}

int HandleTrackingFS::FilterDirectoryStreamAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool HandleTrackingFS::FilterDirectoryStreamAnonymousInnerClass::equals(any obj)
{
  return shared_from_this() == obj;
}
} // namespace org::apache::lucene::mockfile