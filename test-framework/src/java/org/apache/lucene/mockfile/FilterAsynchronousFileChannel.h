#pragma once
#include "stringhelper.h"
#include <memory>

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
 * A {@code FilterAsynchronousFileChannel} contains another
 * {@code AsynchronousFileChannel}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterAsynchronousFileChannel : public AsynchronousFileChannel
{
  GET_CLASS_NAME(FilterAsynchronousFileChannel)

  /**
   * The underlying {@code AsynchronousFileChannel} instance.
   */
protected:
  const std::shared_ptr<AsynchronousFileChannel> delegate_;

  /**
   * Construct a {@code FilterAsynchronousFileChannel} based on
   * the specified base channel.
   * <p>
   * Note that base channel is closed if this channel is closed.
   * @param delegate specified base channel.
   */
public:
  FilterAsynchronousFileChannel(
      std::shared_ptr<AsynchronousFileChannel> delegate_);

  virtual ~FilterAsynchronousFileChannel();

  bool isOpen() override;

  int64_t size()  override;

  std::shared_ptr<AsynchronousFileChannel>
  truncate(int64_t size)  override;

  void force(bool metaData)  override;

  template <typename A, typename T1>
  void lock(int64_t position, int64_t size, bool shared, A attachment,
            std::shared_ptr<CompletionHandler<T1>> handler);

  std::shared_ptr<Future<std::shared_ptr<FileLock>>>
  lock(int64_t position, int64_t size, bool shared) override;

  std::shared_ptr<FileLock> tryLock(int64_t position, int64_t size,
                                    bool shared)  override;

  template <typename A, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: @Override public <A> void read(java.nio.ByteBuffer dst, long
  // position, A attachment, java.nio.channels.CompletionHandler<int,? super A>
  // handler)
  void read(std::shared_ptr<ByteBuffer> dst, int64_t position, A attachment,
            std::shared_ptr<CompletionHandler<T1>> handler);

  std::shared_ptr<Future<int>> read(std::shared_ptr<ByteBuffer> dst,
                                    int64_t position) override;

  template <typename A, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: @Override public <A> void write(java.nio.ByteBuffer src,
  // long position, A attachment, java.nio.channels.CompletionHandler<int,? super
  // A> handler)
  void write(std::shared_ptr<ByteBuffer> src, int64_t position, A attachment,
             std::shared_ptr<CompletionHandler<T1>> handler);

  std::shared_ptr<Future<int>> write(std::shared_ptr<ByteBuffer> src,
                                     int64_t position) override;

protected:
  std::shared_ptr<FilterAsynchronousFileChannel> shared_from_this()
  {
    return std::static_pointer_cast<FilterAsynchronousFileChannel>(
        java.nio.channels.AsynchronousFileChannel::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
