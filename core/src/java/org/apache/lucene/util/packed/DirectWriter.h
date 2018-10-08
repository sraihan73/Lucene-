#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataOutput;
}

namespace org::apache::lucene::util::packed
{
class BulkOperation;
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

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * Class for writing packed integers to be directly read from Directory.
 * Integers can be read on-the-fly via {@link DirectReader}.
 * <p>
 * Unlike PackedInts, it optimizes for read i/o operations and supports &gt; 2B
 * values. Example usage: <pre class="prettyprint"> int bitsPerValue =
 * DirectWriter.bitsRequired(100); // values up to and including 100 IndexOutput
 * output = dir.createOutput("packed", IOContext.DEFAULT); DirectWriter writer =
 * DirectWriter.getInstance(output, numberOfValues, bitsPerValue); for (int i =
 * 0; i &lt; numberOfValues; i++) { writer.add(value);
 *   }
 *   writer.finish();
 *   output.close();
 * </pre>
 * @see DirectReader
 */
class DirectWriter final : public std::enable_shared_from_this<DirectWriter>
{
  GET_CLASS_NAME(DirectWriter)
public:
  const int bitsPerValue;
  const int64_t numValues;
  const std::shared_ptr<DataOutput> output;

  int64_t count = 0;
  bool finished = false;

  // for now, just use the existing writer under the hood
  int off = 0;
  std::deque<char> const nextBlocks;
  std::deque<int64_t> const nextValues;
  const std::shared_ptr<BulkOperation> encoder;
  const int iterations;

  DirectWriter(std::shared_ptr<DataOutput> output, int64_t numValues,
               int bitsPerValue);

  /** Adds a value to this writer */
  void add(int64_t l) ;

private:
  void flush() ;

  /** finishes writing */
public:
  void finish() ;

  /** Returns an instance suitable for encoding {@code numValues} using {@code
   * bitsPerValue} */
  static std::shared_ptr<DirectWriter>
  getInstance(std::shared_ptr<DataOutput> output, int64_t numValues,
              int bitsPerValue);

  /**
   * Round a number of bits per value to the next amount of bits per value that
   * is supported by this writer.
   *
   * @param bitsRequired the amount of bits required
   * @return the next number of bits per value that is gte the provided value
   *         and supported by this writer
   */
private:
  static int roundBits(int bitsRequired);

  /**
   * Returns how many bits are required to hold values up
   * to and including maxValue
   *
   * @param maxValue the maximum value that should be representable.
   * @return the amount of bits needed to represent values from 0 to maxValue.
   * @see PackedInts#bitsRequired(long)
   */
public:
  static int bitsRequired(int64_t maxValue);

  /**
   * Returns how many bits are required to hold values up
   * to and including maxValue, interpreted as an unsigned value.
   *
   * @param maxValue the maximum value that should be representable.
   * @return the amount of bits needed to represent values from 0 to maxValue.
   * @see PackedInts#unsignedBitsRequired(long)
   */
  static int unsignedBitsRequired(int64_t maxValue);

  static std::deque<int> const SUPPORTED_BITS_PER_VALUE;
};

} // namespace org::apache::lucene::util::packed
