#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
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
namespace org::apache::lucene::codecs::compressing
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.BitUtil.zigZagEncode;

using IndexOutput = org::apache::lucene::store::IndexOutput;

/**
 * Efficient index format for block-based {@link Codec}s.
 * <p> This writer generates a file which can be loaded into memory using
 * memory-efficient data structures to quickly locate the block that contains
 * any document.
 * <p>In order to have a compact in-memory representation, for every block of
 * 1024 chunks, this index computes the average number of bytes per
 * chunk and for every chunk, only stores the difference between<ul>
 * <li>${chunk number} * ${average length of a chunk}</li>
 * <li>and the actual start offset of the chunk</li></ul>
 * <p>Data is written as follows:
 * <ul>
 * <li>PackedIntsVersion, &lt;Block&gt;<sup>BlockCount</sup>,
 * BlocksEndMarker</li> <li>PackedIntsVersion --&gt; {@link
 * PackedInts#VERSION_CURRENT} as a {@link DataOutput#writeVInt VInt}</li>
 * <li>BlocksEndMarker --&gt; <tt>0</tt> as a {@link DataOutput#writeVInt VInt},
 * this marks the end of blocks since blocks are not allowed to start with
 * <tt>0</tt></li> <li>Block --&gt; BlockChunks, &lt;DocBases&gt;,
 * &lt;StartPointers&gt;</li> <li>BlockChunks --&gt; a {@link
 * DataOutput#writeVInt VInt} which is the number of chunks encoded in the
 * block</li> <li>DocBases --&gt; DocBase, AvgChunkDocs, BitsPerDocBaseDelta,
 * DocBaseDeltas</li> <li>DocBase --&gt; first document ID of the block of
 * chunks, as a {@link DataOutput#writeVInt VInt}</li> <li>AvgChunkDocs --&gt;
 * average number of documents in a single chunk, as a {@link
 * DataOutput#writeVInt VInt}</li> <li>BitsPerDocBaseDelta --&gt; number of bits
 * required to represent a delta from the average using <a
 * href="https://developers.google.com/protocol-buffers/docs/encoding#types">ZigZag
 * encoding</a></li> <li>DocBaseDeltas --&gt; {@link PackedInts packed} array of
 * BlockChunks elements of BitsPerDocBaseDelta bits each, representing the
 * deltas from the average doc base using <a
 * href="https://developers.google.com/protocol-buffers/docs/encoding#types">ZigZag
 * encoding</a>.</li> <li>StartPointers --&gt; StartPointerBase, AvgChunkSize,
 * BitsPerStartPointerDelta, StartPointerDeltas</li> <li>StartPointerBase --&gt;
 * the first start pointer of the block, as a {@link DataOutput#writeVLong
 * VLong}</li> <li>AvgChunkSize --&gt; the average size of a chunk of compressed
 * documents, as a {@link DataOutput#writeVLong VLong}</li>
 * <li>BitsPerStartPointerDelta --&gt; number of bits required to represent a
 * delta from the average using <a
 * href="https://developers.google.com/protocol-buffers/docs/encoding#types">ZigZag
 * encoding</a></li> <li>StartPointerDeltas --&gt; {@link PackedInts packed}
 * array of BlockChunks elements of BitsPerStartPointerDelta bits each,
 * representing the deltas from the average start pointer using <a
 * href="https://developers.google.com/protocol-buffers/docs/encoding#types">ZigZag
 * encoding</a></li> <li>Footer --&gt; {@link CodecUtil#writeFooter
 * CodecFooter}</li>
 * </ul>
 * <p>Notes
 * <ul>
 * <li>For any block, the doc base of the n-th chunk can be restored with
 * <code>DocBase + AvgChunkDocs * n + DocBaseDeltas[n]</code>.</li>
 * <li>For any block, the start pointer of the n-th chunk can be restored with
 * <code>StartPointerBase + AvgChunkSize * n +
 * StartPointerDeltas[n]</code>.</li> <li>Once data is loaded into memory, you
 * can lookup the start pointer of any document chunk by performing two binary
 * searches: a first one based on the values of DocBase in order to find the
 * right block, and then inside the block based on DocBaseDeltas (by
 * reconstructing the doc bases for every chunk).</li>
 * </ul>
 * @lucene.internal
 */
class CompressingStoredFieldsIndexWriter final
    : public std::enable_shared_from_this<CompressingStoredFieldsIndexWriter>
{
  GET_CLASS_NAME(CompressingStoredFieldsIndexWriter)

public:
  const std::shared_ptr<IndexOutput> fieldsIndexOut;
  const int blockSize;
  int totalDocs = 0;
  int blockDocs = 0;
  int blockChunks = 0;
  int64_t firstStartPointer = 0;
  int64_t maxStartPointer = 0;
  std::deque<int> const docBaseDeltas;
  std::deque<int64_t> const startPointerDeltas;

  CompressingStoredFieldsIndexWriter(std::shared_ptr<IndexOutput> indexOutput,
                                     int blockSize) ;

private:
  void reset();

  void writeBlock() ;

public:
  void writeIndex(int numDocs, int64_t startPointer) ;

  void finish(int numDocs, int64_t maxPointer) ;

  virtual ~CompressingStoredFieldsIndexWriter();
};

} // namespace org::apache::lucene::codecs::compressing
