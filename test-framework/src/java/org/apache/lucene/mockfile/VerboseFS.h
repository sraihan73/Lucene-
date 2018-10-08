#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class InfoStream;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::mockfile
{

using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * FileSystem that records all major destructive filesystem activities.
 */
class VerboseFS : public FilterFileSystemProvider
{
  GET_CLASS_NAME(VerboseFS)
public:
  const std::shared_ptr<InfoStream> infoStream;
  const std::shared_ptr<Path> root;

  /**
   * Create a new instance, recording major filesystem write activities
   * (create, delete, etc) to the specified {@code InfoStream}.
   * @param delegate delegate filesystem to wrap.
   * @param infoStream infoStream to send messages to. The component for
   * messages is named "FS".
   */
  VerboseFS(std::shared_ptr<FileSystem> delegate_,
            std::shared_ptr<InfoStream> infoStream);

  /** Records message, and rethrows exception if not null */
private:
  void sop(const std::wstring &text,
           std::runtime_error exception) ;

  std::wstring path(std::shared_ptr<Path> path);

public:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public void createDirectory(java.nio.file.Path
  // dir, java.nio.file.attribute.FileAttribute<?>... attrs) throws
  // java.io.IOException
      void createDirectory(std::shared_ptr<Path> dir, std::deque<FileAttribute<?>> &attrs)  override;

      void delete_(std::shared_ptr<Path> path)  override;

      void copy(std::shared_ptr<Path> source, std::shared_ptr<Path> target,
                std::deque<CopyOption> &options)  override;

      void move(std::shared_ptr<Path> source, std::shared_ptr<Path> target,
                std::deque<CopyOption> &options)  override;

      void setAttribute(
          std::shared_ptr<Path> path, const std::wstring &attribute,
          std::any value,
          std::deque<LinkOption> &options)  override;

      std::shared_ptr<OutputStream> newOutputStream(
          std::shared_ptr<Path> path,
          std::deque<OpenOption> &options)  override;

    private:
      template <typename T1>
      // C++ TODO: There is no native C++ template equivalent to this generic
      // constraint: ORIGINAL LINE: private bool
      // containsDestructive(std::unordered_set<? extends java.nio.file.OpenOption>
      // options)
      bool containsDestructive(std::shared_ptr<Set<T1>> options);

    public:
      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<FileChannel> newFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::shared_ptr<ExecutorService> executor, std::deque<FileAttribute<?>> &attrs) ;

      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<SeekableByteChannel> newByteChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: @Override public void
      // createSymbolicLink(java.nio.file.Path link, java.nio.file.Path target,
      // java.nio.file.attribute.FileAttribute<?>... attrs) throws
      // java.io.IOException
      void createSymbolicLink(std::shared_ptr<Path> link, std::shared_ptr<Path> target, std::deque<FileAttribute<?>> &attrs)  override;

      void
      createLink(std::shared_ptr<Path> link,
                 std::shared_ptr<Path> existing)  override;

      bool
      deleteIfExists(std::shared_ptr<Path> path)  override;

    protected:
      std::shared_ptr<VerboseFS> shared_from_this()
      {
        return std::static_pointer_cast<VerboseFS>(
            FilterFileSystemProvider::shared_from_this());
      }
};

} // namespace org::apache::lucene::mockfile
