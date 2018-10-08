#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
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
namespace org::apache::lucene::util::packed
{

using IndexOutput = org::apache::lucene::store::IndexOutput;

/**
 * Write monotonically-increasing sequences of integers. This writer splits
 * data into blocks and then for each block, computes the average slope, the
 * minimum value and only encode the delta from the expected value using a
 * {@link DirectWriter}.
 *
 * @see DirectMonotonicReader
 * @lucene.internal
 */
class DirectMonotonicWriter final
    : public std::enable_shared_from_this<DirectMonotonicWriter>
{
  GET_CLASS_NAME(DirectMonotonicWriter)

public:
  static constexpr int MIN_BLOCK_SHIFT = 2;
  static constexpr int MAX_BLOCK_SHIFT = 22;

  const std::shared_ptr<IndexOutput> meta;
  const std::shared_ptr<IndexOutput> data;
  const int64_t numValues;
  const int64_t baseDataPointer;
  std::deque<int64_t> const buffer;
  int bufferSize = 0;
  int64_t count = 0;
  bool finished = false;

  DirectMonotonicWriter(std::shared_ptr<IndexOutput> metaOut,
                        std::shared_ptr<IndexOutput> dataOut,
                        int64_t numValues, int blockShift);

private:
  void flush() ;

public:
  int64_t previous = std::numeric_limits<int64_t>::min();

  /** Write a new value. Note that data might not make it to storage until
   * {@link #finish()} is called.
   *  @throws IllegalArgumentException if values don't come in order */
  void add(int64_t v) ;

  /** This must be called exactly once after all values have been {@link
   * #add(long) added}. */
  void finish() ;

  /** Returns an instance suitable for encoding {@code numValues} into monotonic
   *  blocks of 2<sup>{@code blockShift}</sup> values. Metadata will be written
   *  to {@code metaOut} and actual data to {@code dataOut}. */
  static std::shared_ptr<DirectMonotonicWriter>
  getInstance(std::shared_ptr<IndexOutput> metaOut,
              std::shared_ptr<IndexOutput> dataOut, int64_t numValues,
              int blockShift);
};

} // namespace org::apache::lucene::util::packed
