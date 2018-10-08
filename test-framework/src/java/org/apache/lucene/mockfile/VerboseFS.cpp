using namespace std;

#include "VerboseFS.h"

namespace org::apache::lucene::mockfile
{
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

VerboseFS::VerboseFS(shared_ptr<FileSystem> delegate_,
                     shared_ptr<InfoStream> infoStream)
    : FilterFileSystemProvider(L"verbose://", delegate_),
      infoStream(infoStream), root(this->getFileSystem(nullptr)
                                       ->getPath(L".")
                                       .toAbsolutePath()
                                       .normalize())
{
}

void VerboseFS::sop(const wstring &text,
                    runtime_error exception) 
{
  if (exception == nullptr) {
    if (infoStream->isEnabled(L"FS")) {
      infoStream->message(L"FS", text);
    }
  } else {
    if (infoStream->isEnabled(L"FS")) {
      infoStream->message(L"FS", text + L" (FAILED: " + exception + L")");
    }
    throw IOUtils::rethrowAlways(exception);
  }
}

wstring VerboseFS::path(shared_ptr<Path> path)
{
  path = root->relativize(path->toAbsolutePath().normalize());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return path->toString();
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public void createDirectory(java.nio.file.Path dir,
// java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
    void VerboseFS::createDirectory(shared_ptr<Path> dir, deque<FileAttribute<?>> &attrs) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::createDirectory(dir, {attrs});
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        sop(L"createDirectory: " + path(dir), exception);
      }
    }

    void VerboseFS::delete_(shared_ptr<Path> path) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::delete (path);
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        sop(L"delete: " + this->path(path), exception);
      }
    }

    void VerboseFS::copy(shared_ptr<Path> source, shared_ptr<Path> target,
                         deque<CopyOption> &options) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::copy(source, target, {options});
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        sop(L"copy" + Arrays->toString(options) + L": " + path(source) +
                L" -> " + path(target),
            exception);
      }
    }

    void VerboseFS::move(shared_ptr<Path> source, shared_ptr<Path> target,
                         deque<CopyOption> &options) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::move(source, target, {options});
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        sop(L"move" + Arrays->toString(options) + L": " + path(source) +
                L" -> " + path(target),
            exception);
      }
    }

    void VerboseFS::setAttribute(shared_ptr<Path> path,
                                 const wstring &attribute, any value,
                                 deque<LinkOption> &options) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::setAttribute(path, attribute, value,
                                               {options});
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        sop(L"setAttribute[" + attribute + L"=" + value + L"]: " +
                this->path(path),
            exception);
      }
    }

    shared_ptr<OutputStream>
    VerboseFS::newOutputStream(shared_ptr<Path> path,
                               deque<OpenOption> &options) 
    {
      runtime_error exception = nullptr;
      try {
        return FilterFileSystemProvider::newOutputStream(path, {options});
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        sop(L"newOutputStream" + Arrays->toString(options) + L": " +
                this->path(path),
            exception);
      }
      throw make_shared<AssertionError>();
    }

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: private bool
    // containsDestructive(std::unordered_set<? extends java.nio.file.OpenOption>
    // options)
    bool VerboseFS::containsDestructive(shared_ptr<Set<T1>> options)
    {
      return (options->contains(StandardOpenOption::APPEND) ||
              options->contains(StandardOpenOption::WRITE) ||
              options->contains(StandardOpenOption::DELETE_ON_CLOSE));
    }

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<FileChannel> VerboseFS::newFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  runtime_error exception = nullptr;
  try {
    return FilterFileSystemProvider::newFileChannel(path, options, {attrs});
  } catch (const runtime_error &t) {
    exception = t;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (containsDestructive(options)) {
      sop(L"newFileChannel" + options + L": " + this->path(path), exception);
    } else {
      if (exception != nullptr) {
        throw IOUtils::rethrowAlways(exception);
      }
    }
  }
  throw make_shared<AssertionError>();
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<AsynchronousFileChannel> VerboseFS::newAsynchronousFileChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, shared_ptr<ExecutorService> executor, deque<FileAttribute<?>> &attrs) 
{
  runtime_error exception = nullptr;
  try {
    return FilterFileSystemProvider::newAsynchronousFileChannel(
        path, options, executor, {attrs});
  } catch (const runtime_error &t) {
    exception = t;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (containsDestructive(options)) {
      sop(L"newAsynchronousFileChannel" + options + L": " + this->path(path),
          exception);
    } else {
      if (exception != nullptr) {
        throw IOUtils::rethrowAlways(exception);
      }
    }
  }
  throw make_shared<AssertionError>();
}

template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
    shared_ptr<SeekableByteChannel> VerboseFS::newByteChannel(shared_ptr<Path> path, shared_ptr<Set<T1>> options, deque<FileAttribute<?>> &attrs) 
{
  runtime_error exception = nullptr;
  try {
    return FilterFileSystemProvider::newByteChannel(path, options, {attrs});
  } catch (const runtime_error &t) {
    exception = t;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (containsDestructive(options)) {
      sop(L"newByteChannel" + options + L": " + this->path(path), exception);
    } else {
      if (exception != nullptr) {
        throw IOUtils::rethrowAlways(exception);
      }
    }
  }
  throw make_shared<AssertionError>();
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public void createSymbolicLink(java.nio.file.Path
// link, java.nio.file.Path target, java.nio.file.attribute.FileAttribute<?>...
// attrs) throws java.io.IOException
    void VerboseFS::createSymbolicLink(shared_ptr<Path> link, shared_ptr<Path> target, deque<FileAttribute<?>> &attrs) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::createSymbolicLink(link, target, {attrs});
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        sop(L"createSymbolicLink: " + path(link) + L" -> " + path(target),
            exception);
      }
    }

    void VerboseFS::createLink(shared_ptr<Path> link,
                               shared_ptr<Path> existing) 
    {
      runtime_error exception = nullptr;
      try {
        FilterFileSystemProvider::createLink(link, existing);
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        sop(L"createLink: " + path(link) + L" -> " + path(existing), exception);
      }
    }

    bool VerboseFS::deleteIfExists(shared_ptr<Path> path) 
    {
      runtime_error exception = nullptr;
      try {
        return FilterFileSystemProvider::deleteIfExists(path);
      } catch (const runtime_error &t) {
        exception = t;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        sop(L"deleteIfExists: " + this->path(path), exception);
      }
      throw make_shared<AssertionError>();
    }
    } // namespace org::apache::lucene::mockfile