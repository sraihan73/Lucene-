using namespace std;

#include "DisableFsyncFS.h"

namespace org::apache::lucene::mockfile
{

DisableFsyncFS::DisableFsyncFS(shared_ptr<FileSystem> delegate_)
    : FilterFileSystemProvider(L"disablefsync://", delegate_)
{
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<FileChannel> DisableFsyncFS::newFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  return make_shared<FilterFileChannelAnonymousInnerClass>(
      shared_from_this(),
      FilterFileSystemProvider::newFileChannel(path, options, {attrs}));
}

DisableFsyncFS::FilterFileChannelAnonymousInnerClass::
    FilterFileChannelAnonymousInnerClass(
        shared_ptr<DisableFsyncFS> outerInstance,
        shared_ptr<FileChannel> newFileChannel)
    : FilterFileChannel(newFileChannel)
{
  this->outerInstance = outerInstance;
}

void DisableFsyncFS::FilterFileChannelAnonymousInnerClass::force(
    bool metaData) 
{
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<AsynchronousFileChannel> DisableFsyncFS::newAsynchronousFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, shared_ptr<ExecutorService> executor, deque<FileAttribute<?>> &attrs) 
{
  return make_shared<FilterAsynchronousFileChannelAnonymousInnerClass>(
      shared_from_this(), FilterFileSystemProvider::newAsynchronousFileChannel(
                              path, options, executor, {attrs}));
}

DisableFsyncFS::FilterAsynchronousFileChannelAnonymousInnerClass::
    FilterAsynchronousFileChannelAnonymousInnerClass(
        shared_ptr<DisableFsyncFS> outerInstance,
        shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel)
    : FilterAsynchronousFileChannel(newAsynchronousFileChannel)
{
  this->outerInstance = outerInstance;
}

void DisableFsyncFS::FilterAsynchronousFileChannelAnonymousInnerClass::force(
    bool metaData) 
{
}
} // namespace org::apache::lucene::mockfile