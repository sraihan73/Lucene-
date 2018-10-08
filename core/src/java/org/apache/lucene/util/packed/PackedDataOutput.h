#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * A {@link DataOutput} wrapper to write unaligned, variable-length packed
 * integers.
 * @see PackedDataInput
 * @lucene.internal
 */
class PackedDataOutput final
    : public std::enable_shared_from_this<PackedDataOutput>
{
  GET_CLASS_NAME(PackedDataOutput)

public:
  const std::shared_ptr<DataOutput> out;
  int64_t current = 0;
  int remainingBits = 0;

  /**
   * Create a new instance that wraps <code>out</code>.
   */
  PackedDataOutput(std::shared_ptr<DataOutput> out);

  /**
   * Write a value using exactly <code>bitsPerValue</code> bits.
   */
  void writeLong(int64_t value, int bitsPerValue) ;

  /**
   * Flush pending bits to the underlying {@link DataOutput}.
   */
  void flush() ;
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
