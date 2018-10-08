#pragma once
#include "exceptionhelper.h"
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
namespace org::apache::lucene::util::packed
{

/**
 * Non-specialized {@link BulkOperation} for {@link
 * PackedInts.Format#PACKED_SINGLE_BLOCK}.
 */
class BulkOperationPackedSingleBlock final : public BulkOperation
{
  GET_CLASS_NAME(BulkOperationPackedSingleBlock)

private:
  static constexpr int BLOCK_COUNT = 1;

  const int bitsPerValue;
  const int valueCount;
  const int64_t mask;

public:
  BulkOperationPackedSingleBlock(int bitsPerValue);

  int longBlockCount() override;

  int byteBlockCount() override;

  int longValueCount() override;

  int byteValueCount() override;

private:
  static int64_t readLong(std::deque<char> &blocks, int blocksOffset);

  int decode(int64_t block, std::deque<int64_t> &values, int valuesOffset);

  int decode(int64_t block, std::deque<int> &values, int valuesOffset);

  int64_t encode(std::deque<int64_t> &values, int valuesOffset);

  int64_t encode(std::deque<int> &values, int valuesOffset);

public:
  void decode(std::deque<int64_t> &blocks, int blocksOffset,
              std::deque<int64_t> &values, int valuesOffset,
              int iterations) override;

  void decode(std::deque<char> &blocks, int blocksOffset,
              std::deque<int64_t> &values, int valuesOffset,
              int iterations) override;

  void decode(std::deque<int64_t> &blocks, int blocksOffset,
              std::deque<int> &values, int valuesOffset,
              int iterations) override;

  void decode(std::deque<char> &blocks, int blocksOffset,
              std::deque<int> &values, int valuesOffset,
              int iterations) override;

  void encode(std::deque<int64_t> &values, int valuesOffset,
              std::deque<int64_t> &blocks, int blocksOffset,
              int iterations) override;

  void encode(std::deque<int> &values, int valuesOffset,
              std::deque<int64_t> &blocks, int blocksOffset,
              int iterations) override;

  void encode(std::deque<int64_t> &values, int valuesOffset,
              std::deque<char> &blocks, int blocksOffset,
              int iterations) override;

  void encode(std::deque<int> &values, int valuesOffset,
              std::deque<char> &blocks, int blocksOffset,
              int iterations) override;

protected:
  std::shared_ptr<BulkOperationPackedSingleBlock> shared_from_this()
  {
    return std::static_pointer_cast<BulkOperationPackedSingleBlock>(
        BulkOperation::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
