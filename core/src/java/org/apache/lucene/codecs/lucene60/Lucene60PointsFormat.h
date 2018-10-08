#pragma once
#include "../PointsFormat.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

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
namespace org::apache::lucene::codecs::lucene60
{

using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Lucene 6.0 point format, which encodes dimensional values in a block KD-tree
 * structure for fast 1D range and N dimesional shape intersection filtering.
 * See <a
 * href="https://www.cs.duke.edu/~pankaj/publications/papers/bkd-sstd.pdf">this
 * paper</a> for details.
 *
 * <p>This data structure is written as a series of blocks on disk, with an
 * in-memory perfectly balanced binary tree of split values referencing those
 * blocks at the leaves.
 *
 * <p>The <code>.dim</code> file has both blocks and the index split
 * values, for each field.  The file starts with {@link
 * CodecUtil#writeIndexHeader}.
 *
 * <p>The blocks are written like this:
 *
 * <ul>
 *  <li> count (vInt)
 *  <li> delta-docID (vInt) <sup>count</sup> (delta coded docIDs, in sorted
 * order) <li> packedValue<sup>count</sup> (the <code>byte[]</code> value of
 * each dimension packed into a single <code>byte[]</code>)
 * </ul>
 *
 * <p>After all blocks for a field are written, then the index is written:
 * <ul>
 *  <li> numDims (vInt)
 *  <li> maxPointsInLeafNode (vInt)
 *  <li> bytesPerDim (vInt)
 *  <li> count (vInt)
 *  <li> packed index (byte[])
 * </ul>
 *
 * <p>The packed index uses hierarchical delta and prefix coding to compactly
 * encode the file pointer for all leaf blocks, once the tree is traversed, as
 * well as the split dimension and split value for each inner node of the tree.
 *
 * <p>After all fields blocks + index data are written, {@link
 * CodecUtil#writeFooter} writes the checksum.
 *
 * <p>The <code>.dii</code> file records the file pointer in the
 * <code>.dim</code> file where each field's index data was written.  It starts
 * with {@link CodecUtil#writeIndexHeader}, then has:
 *
 * <ul>
 *   <li> fieldCount (vInt)
 *   <li> (fieldNumber (vInt), fieldFilePointer (vLong))<sup>fieldCount</sup>
 * </ul>
 *
 * <p>After all fields blocks + index data are written, {@link
 * CodecUtil#writeFooter} writes the checksum.
 *
 * @lucene.experimental
 */

class Lucene60PointsFormat final : public PointsFormat
{
  GET_CLASS_NAME(Lucene60PointsFormat)

public:
  static const std::wstring DATA_CODEC_NAME;
  static const std::wstring META_CODEC_NAME;

  /**
   * Filename extension for the leaf blocks
   */
  static const std::wstring DATA_EXTENSION;

  /**
   * Filename extension for the index per field
   */
  static const std::wstring INDEX_EXTENSION;

  static constexpr int DATA_VERSION_START = 0;
  static constexpr int DATA_VERSION_CURRENT = DATA_VERSION_START;

  static constexpr int INDEX_VERSION_START = 0;
  static constexpr int INDEX_VERSION_CURRENT = INDEX_VERSION_START;

  /** Sole constructor */
  Lucene60PointsFormat();

  std::shared_ptr<PointsWriter> fieldsWriter(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<PointsReader> fieldsReader(
      std::shared_ptr<SegmentReadState> state)  override;

protected:
  std::shared_ptr<Lucene60PointsFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene60PointsFormat>(
        org.apache.lucene.codecs.PointsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene60/
