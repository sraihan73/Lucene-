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
 * A {@code FilterSeekableByteChannel} contains another
 * {@code SeekableByteChannel}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterSeekableByteChannel
    : public std::enable_shared_from_this<FilterSeekableByteChannel>,
      public SeekableByteChannel
{
  GET_CLASS_NAME(FilterSeekableByteChannel)

  /**
   * The underlying {@code SeekableByteChannel} instance.
   */
protected:
  const std::shared_ptr<SeekableByteChannel> delegate_;

  /**
   * Construct a {@code FilterSeekableByteChannel} based on
   * the specified base channel.
   * <p>
   * Note that base channel is closed if this channel is closed.
   * @param delegate specified base channel.
   */
public:
  FilterSeekableByteChannel(std::shared_ptr<SeekableByteChannel> delegate_);

  bool isOpen() override;

  virtual ~FilterSeekableByteChannel();

  int read(std::shared_ptr<ByteBuffer> dst)  override;

  int write(std::shared_ptr<ByteBuffer> src)  override;

  int64_t position()  override;

  std::shared_ptr<SeekableByteChannel>
  position(int64_t newPosition)  override;

  int64_t size()  override;

  std::shared_ptr<SeekableByteChannel>
  truncate(int64_t size)  override;
};

} // namespace org::apache::lucene::mockfile
