#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
}

namespace org::apache::lucene::util
{
class LongsRef;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.BitUtil.zigZagDecode;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.BPV_SHIFT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MAX_BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MIN_BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MIN_VALUE_EQUALS_0;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;

using DataInput = org::apache::lucene::store::DataInput;
using LongsRef = org::apache::lucene::util::LongsRef;

/**
 * Reader for sequences of longs written with {@link BlockPackedWriter}.
 * @see BlockPackedWriter
 * @lucene.internal
 */
class BlockPackedReaderIterator final
    : public std::enable_shared_from_this<BlockPackedReaderIterator>
{
  GET_CLASS_NAME(BlockPackedReaderIterator)

  // same as DataInput.readVLong but supports negative values
public:
  static int64_t readVLong(std::shared_ptr<DataInput> in_) ;

  std::shared_ptr<DataInput> in_;
  const int packedIntsVersion;
  int64_t valueCount = 0;
  const int blockSize;
  std::deque<int64_t> const values;
  const std::shared_ptr<LongsRef> valuesRef;
  std::deque<char> blocks;
  int off = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t ord_ = 0;

  /** Sole constructor.
   * @param blockSize the number of values of a block, must be equal to the
   *                  block size of the {@link BlockPackedWriter} which has
   *                  been used to write the stream
   */
  BlockPackedReaderIterator(std::shared_ptr<DataInput> in_,
                            int packedIntsVersion, int blockSize,
                            int64_t valueCount);

  /** Reset the current reader to wrap a stream of <code>valueCount</code>
   * values contained in <code>in</code>. The block size remains unchanged. */
  void reset(std::shared_ptr<DataInput> in_, int64_t valueCount);

  /** Skip exactly <code>count</code> values. */
  void skip(int64_t count) ;

private:
  void skipBytes(int64_t count) ;

  /** Read the next value. */
public:
  int64_t next() ;

  /** Read between <tt>1</tt> and <code>count</code> values. */
  std::shared_ptr<LongsRef> next(int count) ;

private:
  void refill() ;

  /** Return the offset of the next value to read. */
public:
  int64_t ord();
};

} // namespace org::apache::lucene::util::packed
