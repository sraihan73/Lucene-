#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
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

using DataInput = org::apache::lucene::store::DataInput;

/**
 * A {@link DataInput} wrapper to read unaligned, variable-length packed
 * integers. This API is much slower than the {@link PackedInts} fixed-length
 * API but can be convenient to save space.
 * @see PackedDataOutput
 * @lucene.internal
 */
class PackedDataInput final
    : public std::enable_shared_from_this<PackedDataInput>
{
  GET_CLASS_NAME(PackedDataInput)

public:
  const std::shared_ptr<DataInput> in_;
  int64_t current = 0;
  int remainingBits = 0;

  /**
   * Create a new instance that wraps <code>in</code>.
   */
  PackedDataInput(std::shared_ptr<DataInput> in_);

  /**
   * Read the next long using exactly <code>bitsPerValue</code> bits.
   */
  int64_t readLong(int bitsPerValue) ;

  /**
   * If there are pending bits (at most 7), they will be ignored and the next
   * value will be read starting at the next byte.
   */
  void skipToNextByte();
};

} // namespace org::apache::lucene::util::packed
