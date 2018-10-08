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
 * A {@code FilterOutputStream2} contains another
 * {@code OutputStream}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 * <p>
 * Note: unlike {@link FilterOutputStream} this class
 * delegates every method by default. This means to transform
 * {@code write} calls, you need to override multiple methods.
 * On the other hand, it is less trappy: a simple implementation
 * that just overrides {@code close} will not force bytes to be
 * written one-at-a-time.
 */
class FilterOutputStream2 : public OutputStream
{
  GET_CLASS_NAME(FilterOutputStream2)

  /**
   * The underlying {@code OutputStream} instance.
   */
protected:
  const std::shared_ptr<OutputStream> delegate_;

  /**
   * Construct a {@code FilterOutputStream2} based on
   * the specified base stream.
   * <p>
   * Note that base stream is closed if this stream is closed.
   * @param delegate specified base stream.
   */
public:
  FilterOutputStream2(std::shared_ptr<OutputStream> delegate_);

  void write(std::deque<char> &b)  override;

  void write(std::deque<char> &b, int off,
             int len)  override;

  void flush()  override;

  virtual ~FilterOutputStream2();

  void write(int b)  override;

protected:
  std::shared_ptr<FilterOutputStream2> shared_from_this()
  {
    return std::static_pointer_cast<FilterOutputStream2>(
        java.io.OutputStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
