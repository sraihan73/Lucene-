#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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

/**
 * Disables actual calls to fsync.
 * <p>
 * All other filesystem operations are passed thru as normal.
 */
class DisableFsyncFS : public FilterFileSystemProvider
{
  GET_CLASS_NAME(DisableFsyncFS)

  /**
   * Create a new instance, wrapping {@code delegate}.
   */
public:
  DisableFsyncFS(std::shared_ptr<FileSystem> delegate_);

      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<FileChannel> newFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

    private:
      class FilterFileChannelAnonymousInnerClass : public FilterFileChannel
      {
        GET_CLASS_NAME(FilterFileChannelAnonymousInnerClass)
      private:
        std::shared_ptr<DisableFsyncFS> outerInstance;

      public:
        FilterFileChannelAnonymousInnerClass(
            std::shared_ptr<DisableFsyncFS> outerInstance,
            std::shared_ptr<FileChannel> newFileChannel);

        void force(bool metaData)  override;

      protected:
        std::shared_ptr<FilterFileChannelAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterFileChannelAnonymousInnerClass>(
              FilterFileChannel::shared_from_this());
        }
      };

    public:
      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::shared_ptr<ExecutorService> executor, std::deque<FileAttribute<?>> &attrs) ;

    private:
      class FilterAsynchronousFileChannelAnonymousInnerClass
          : public FilterAsynchronousFileChannel
      {
        GET_CLASS_NAME(FilterAsynchronousFileChannelAnonymousInnerClass)
      private:
        std::shared_ptr<DisableFsyncFS> outerInstance;

      public:
        FilterAsynchronousFileChannelAnonymousInnerClass(
            std::shared_ptr<DisableFsyncFS> outerInstance,
            std::shared_ptr<AsynchronousFileChannel>
                newAsynchronousFileChannel);

        void force(bool metaData)  override;

      protected:
        std::shared_ptr<FilterAsynchronousFileChannelAnonymousInnerClass>
        shared_from_this()
        {
          return std::static_pointer_cast<
              FilterAsynchronousFileChannelAnonymousInnerClass>(
              FilterAsynchronousFileChannel::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<DisableFsyncFS> shared_from_this()
      {
        return std::static_pointer_cast<DisableFsyncFS>(
            FilterFileSystemProvider::shared_from_this());
      }
};

} // namespace org::apache::lucene::mockfile
