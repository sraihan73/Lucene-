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
 * A {@code FilterFileChannel} contains another
 * {@code FileChannel}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterFileChannel : public FileChannel
{
  GET_CLASS_NAME(FilterFileChannel)

  /**
   * The underlying {@code FileChannel} instance.
   */
protected:
  const std::shared_ptr<FileChannel> delegate_;

  /**
   * Construct a {@code FilterFileChannel} based on
   * the specified base channel.
   * <p>
   * Note that base channel is closed if this channel is closed.
   * @param delegate specified base channel.
   */
public:
  FilterFileChannel(std::shared_ptr<FileChannel> delegate_);

  int read(std::shared_ptr<ByteBuffer> dst)  override;

  int64_t read(std::deque<std::shared_ptr<ByteBuffer>> &dsts, int offset,
                 int length)  override;

  int write(std::shared_ptr<ByteBuffer> src)  override;

  int64_t write(std::deque<std::shared_ptr<ByteBuffer>> &srcs, int offset,
                  int length)  override;

  int64_t position()  override;

  std::shared_ptr<FileChannel>
  position(int64_t newPosition)  override;

  int64_t size()  override;

  std::shared_ptr<FileChannel>
  truncate(int64_t size)  override;

  void force(bool metaData)  override;

  int64_t transferTo(
      int64_t position, int64_t count,
      std::shared_ptr<WritableByteChannel> target)  override;

  int64_t transferFrom(std::shared_ptr<ReadableByteChannel> src,
                         int64_t position,
                         int64_t count)  override;

  int read(std::shared_ptr<ByteBuffer> dst,
           int64_t position)  override;

  int write(std::shared_ptr<ByteBuffer> src,
            int64_t position)  override;

  std::shared_ptr<MappedByteBuffer>
  map_obj(std::shared_ptr<MapMode> mode, int64_t position,
      int64_t size)  override;

  std::shared_ptr<FileLock> lock(int64_t position, int64_t size,
                                 bool shared)  override;

  std::shared_ptr<FileLock> tryLock(int64_t position, int64_t size,
                                    bool shared)  override;

protected:
  void implCloseChannel()  override;

protected:
  std::shared_ptr<FilterFileChannel> shared_from_this()
  {
    return std::static_pointer_cast<FilterFileChannel>(
        java.nio.channels.FileChannel::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
