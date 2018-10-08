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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.MonotonicBlockPackedReader.expected;

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * A writer for large monotonically increasing sequences of positive longs.
 * <p>
 * The sequence is divided into fixed-size blocks and for each block, values
 * are modeled after a linear function f: x &rarr; A &times; x + B. The block
 * encodes deltas from the expected values computed from this function using as
 * few bits as possible.
 * <p>
 * Format:
 * <ul>
 * <li>&lt;BLock&gt;<sup>BlockCount</sup>
 * <li>BlockCount: &lceil; ValueCount / BlockSize &rceil;
 * <li>Block: &lt;Header, (Ints)&gt;
 * <li>Header: &lt;B, A, BitsPerValue&gt;
 * <li>B: the B from f: x &rarr; A &times; x + B using a
 *     {@link BitUtil#zigZagEncode(long) zig-zag encoded}
 *     {@link DataOutput#writeVLong(long) vLong}
 * <li>A: the A from f: x &rarr; A &times; x + B encoded using
 *     {@link Float#floatToIntBits(float)} on
 *     {@link DataOutput#writeInt(int) 4 bytes}
 * <li>BitsPerValue: a {@link DataOutput#writeVInt(int) variable-length int}
 * <li>Ints: if BitsPerValue is <tt>0</tt>, then there is nothing to read and
 *     all values perfectly match the result of the function. Otherwise, these
 *     are the {@link PackedInts packed} deltas from the expected value
 *     (computed from the function) using exaclty BitsPerValue bits per value.
 * </ul>
 * @see MonotonicBlockPackedReader
 * @lucene.internal
 */
class MonotonicBlockPackedWriter final : public AbstractBlockPackedWriter
{
  GET_CLASS_NAME(MonotonicBlockPackedWriter)

  /**
   * Sole constructor.
   * @param blockSize the number of values of a single block, must be a power of
   * 2
   */
public:
  MonotonicBlockPackedWriter(std::shared_ptr<DataOutput> out, int blockSize);

  void add(int64_t l)  override;

protected:
  void flush()  override;

protected:
  std::shared_ptr<MonotonicBlockPackedWriter> shared_from_this()
  {
    return std::static_pointer_cast<MonotonicBlockPackedWriter>(
        AbstractBlockPackedWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
