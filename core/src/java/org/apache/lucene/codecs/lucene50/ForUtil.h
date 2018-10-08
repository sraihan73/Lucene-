#pragma once
#include "../../util/packed/PackedInts.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

namespace org::apache::lucene::util::packed
{
class Decoder;
}
namespace org::apache::lucene::util::packed
{
class Encoder;
}
namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::codecs::lucene50
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Decoder = org::apache::lucene::util::packed::PackedInts::Decoder;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;

/**
 * Encode all values in normal area with fixed bit width,
 * which is determined by the max value in this block.
 */
class ForUtil final : public std::enable_shared_from_this<ForUtil>
{
  GET_CLASS_NAME(ForUtil)

  /**
   * Special number of bits per value used whenever all values to encode are
   * equal.
   */
private:
  static constexpr int ALL_VALUES_EQUAL = 0;

  /**
   * Upper limit of the number of bytes that might be required to stored
   * <code>BLOCK_SIZE</code> encoded values.
   */
public:
  static const int MAX_ENCODED_SIZE = BLOCK_SIZE * 4;

  /**
   * Upper limit of the number of values that might be decoded in a single call
   * to
   * {@link #readBlock(IndexInput, byte[], int[])}. Although values after
   * <code>BLOCK_SIZE</code> are garbage, it is necessary to allocate value
   * buffers whose size is {@code >= MAX_DATA_SIZE} to avoid {@link
   * ArrayIndexOutOfBoundsException}s.
   */
  static const int MAX_DATA_SIZE = 0;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static ForUtil::StaticConstructor staticConstructor;

  /**
   * Compute the number of iterations required to decode <code>BLOCK_SIZE</code>
   * values with the provided {@link Decoder}.
   */
  static int computeIterations(std::shared_ptr<Decoder> decoder);

  /**
   * Compute the number of bytes required to encode a block of values that
   * require <code>bitsPerValue</code> bits per value with format
   * <code>format</code>.
   */
  static int encodedSize(PackedInts::Format format, int packedIntsVersion,
                         int bitsPerValue);

  std::deque<int> const encodedSizes;
  std::deque<std::shared_ptr<PackedInts::Encoder>> const encoders;
  std::deque<std::shared_ptr<Decoder>> const decoders;
  std::deque<int> const iterations;

  /**
   * Create a new {@link ForUtil} instance and save state into <code>out</code>.
   */
public:
  ForUtil(float acceptableOverheadRatio,
          std::shared_ptr<DataOutput> out) ;

  /**
   * Restore a {@link ForUtil} from a {@link DataInput}.
   */
  ForUtil(std::shared_ptr<DataInput> in_) ;

  /**
   * Write a block of data (<code>For</code> format).
   *
   * @param data     the data to write
   * @param encoded  a buffer to use to encode data
   * @param out      the destination output
   * @throws IOException If there is a low-level I/O error
   */
  void writeBlock(std::deque<int> &data, std::deque<char> &encoded,
                  std::shared_ptr<IndexOutput> out) ;

  /**
   * Read the next block of data (<code>For</code> format).
   *
   * @param in        the input to use to read data
   * @param encoded   a buffer that can be used to store encoded data
   * @param decoded   where to write decoded data
   * @throws IOException If there is a low-level I/O error
   */
  void readBlock(std::shared_ptr<IndexInput> in_, std::deque<char> &encoded,
                 std::deque<int> &decoded) ;

  /**
   * Skip the next block of data.
   *
   * @param in      the input where to read data
   * @throws IOException If there is a low-level I/O error
   */
  void skipBlock(std::shared_ptr<IndexInput> in_) ;

private:
  static bool isAllEqual(std::deque<int> &data);

  /**
   * Compute the number of bits required to serialize any of the longs in
   * <code>data</code>.
   */
  static int bitsRequired(std::deque<int> &data);
};

} // namespace org::apache::lucene::codecs::lucene50
