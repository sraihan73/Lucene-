#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

// This file has been automatically generated, DO NOT EDIT

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

/**
 * Efficient sequential read/write of packed integers.
 */
class BulkOperation : public std::enable_shared_from_this<BulkOperation>,
                      public PackedInts::Decoder,
                      public PackedInts::Encoder
{
  GET_CLASS_NAME(BulkOperation)
private:
  static std::deque<std::shared_ptr<BulkOperation>> const packedBulkOps;

  // NOTE: this is sparse (some entries are null):
  static std::deque<std::shared_ptr<BulkOperation>> const
      packedSingleBlockBulkOps;

public:
  static std::shared_ptr<BulkOperation> of(PackedInts::Format format,
                                           int bitsPerValue);

protected:
  virtual int writeLong(int64_t block, std::deque<char> &blocks,
                        int blocksOffset);

  /**
   * For every number of bits per value, there is a minimum number of
   * blocks (b) / values (v) you need to write in order to reach the next block
   * boundary:
   *  - 16 bits per value -&gt; b=2, v=1
   *  - 24 bits per value -&gt; b=3, v=1
   *  - 50 bits per value -&gt; b=25, v=4
   *  - 63 bits per value -&gt; b=63, v=8
   *  - ...
   *
   * A bulk read consists in copying <code>iterations*v</code> values that are
   * contained in <code>iterations*b</code> blocks into a <code>long[]</code>
   * (higher values of <code>iterations</code> are likely to yield a better
   * throughput): this requires n * (b + 8v) bytes of memory.
   *
   * This method computes <code>iterations</code> as
   * <code>ramBudget / (b + 8v)</code> (since a long is 8 bytes).
   */
public:
  int computeIterations(int valueCount, int ramBudget);
};

} // namespace org::apache::lucene::util::packed
