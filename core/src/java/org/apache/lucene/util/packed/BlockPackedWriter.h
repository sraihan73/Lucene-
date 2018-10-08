#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataOutput;
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
//    import static org.apache.lucene.util.BitUtil.zigZagEncode;

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * A writer for large sequences of longs.
 * <p>
 * The sequence is divided into fixed-size blocks and for each block, the
 * difference between each value and the minimum value of the block is encoded
 * using as few bits as possible. Memory usage of this class is proportional to
 * the block size. Each block has an overhead between 1 and 10 bytes to store
 * the minimum value and the number of bits per value of the block.
 * <p>
 * Format:
 * <ul>
 * <li>&lt;BLock&gt;<sup>BlockCount</sup>
 * <li>BlockCount: &lceil; ValueCount / BlockSize &rceil;
 * <li>Block: &lt;Header, (Ints)&gt;
 * <li>Header: &lt;Token, (MinValue)&gt;
 * <li>Token: a {@link DataOutput#writeByte(byte) byte}, first 7 bits are the
 *     number of bits per value (<tt>bitsPerValue</tt>). If the 8th bit is 1,
 *     then MinValue (see next) is <tt>0</tt>, otherwise MinValue and needs to
 *     be decoded
 * <li>MinValue: a
 *     <a
 * href="https://developers.google.com/protocol-buffers/docs/encoding#types">zigzag-encoded</a>
 *     {@link DataOutput#writeVLong(long) variable-length long} whose value
 *     should be added to every int from the block to restore the original
 *     values
 * <li>Ints: If the number of bits per value is <tt>0</tt>, then there is
 *     nothing to decode and all ints are equal to MinValue. Otherwise:
 * BlockSize
 *     {@link PackedInts packed ints} encoded on exactly <tt>bitsPerValue</tt>
 *     bits per value. They are the subtraction of the original values and
 *     MinValue
 * </ul>
 * @see BlockPackedReaderIterator
 * @see BlockPackedReader
 * @lucene.internal
 */
class BlockPackedWriter final : public AbstractBlockPackedWriter
{
  GET_CLASS_NAME(BlockPackedWriter)

  /**
   * Sole constructor.
   * @param blockSize the number of values of a single block, must be a power of
   * 2
   */
public:
  BlockPackedWriter(std::shared_ptr<DataOutput> out, int blockSize);

protected:
  void flush()  override;

protected:
  std::shared_ptr<BlockPackedWriter> shared_from_this()
  {
    return std::static_pointer_cast<BlockPackedWriter>(
        AbstractBlockPackedWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
