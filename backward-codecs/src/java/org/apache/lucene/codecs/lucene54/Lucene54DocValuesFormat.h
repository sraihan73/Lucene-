#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::lucene54
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Lucene 5.4 DocValues format.
 * <p>
 * Encodes the five per-document value types
 * (Numeric,Binary,Sorted,SortedSet,SortedNumeric) with these strategies: <p>
 * {@link DocValuesType#NUMERIC NUMERIC}:
 * <ul>
 *    <li>Delta-compressed: per-document integers written as deltas from the
 * minimum value, compressed with bitpacking. For more information, see {@link
 * DirectWriter}. <li>Table-compressed: when the number of unique values is very
 * small (&lt; 256), and when there are unused "gaps" in the range of values
 * used (such as {@link SmallFloat}), a lookup table is written instead. Each
 * per-document entry is instead the ordinal to this table, and those ordinals
 * are compressed with bitpacking ({@link DirectWriter}). <li>GCD-compressed:
 * when all numbers share a common divisor, such as dates, the greatest common
 * denominator (GCD) is computed, and quotients are stored using
 * Delta-compressed Numerics. <li>Monotonic-compressed: when all numbers are
 * monotonically increasing offsets, they are written as blocks of bitpacked
 * integers, encoding the deviation from the expected delta.
 *    <li>Const-compressed: when there is only one possible non-missing value,
 * only the missing bitset is encoded. <li>Sparse-compressed: only documents
 * with a value are stored, and lookups are performed using binary search.
 * </ul>
 * <p>
 * {@link DocValuesType#BINARY BINARY}:
 * <ul>
 *    <li>Fixed-width Binary: one large concatenated byte[] is written, along
 * with the fixed length. Each document's value can be addressed directly with
 * multiplication ({@code docID * length}). <li>Variable-width Binary: one large
 * concatenated byte[] is written, along with end addresses for each document.
 * The addresses are written as Monotonic-compressed numerics.
 *    <li>Prefix-compressed Binary: values are written in chunks of 16, with the
 * first value written completely and other values sharing prefixes. chunk
 * addresses are written as Monotonic-compressed numerics. A reverse lookup
 * index is written from a portion of every 1024th term.
 * </ul>
 * <p>
 * {@link DocValuesType#SORTED SORTED}:
 * <ul>
 *    <li>Sorted: a mapping of ordinals to deduplicated terms is written as
 * Binary, along with the per-document ordinals written using one of the numeric
 * strategies above.
 * </ul>
 * <p>
 * {@link DocValuesType#SORTED_SET SORTED_SET}:
 * <ul>
 *    <li>Single: if all documents have 0 or 1 value, then data are written like
 * SORTED. <li>SortedSet table: when there are few unique sets of values (&lt;
 * 256) then each set is assigned an id, a lookup table is written and the
 * mapping from document to set id is written using the numeric strategies
 * above. <li>SortedSet: a mapping of ordinals to deduplicated terms is written
 * as Binary, an ordinal deque and per-document index into this deque are written
 * using the numeric strategies above.
 * </ul>
 * <p>
 * {@link DocValuesType#SORTED_NUMERIC SORTED_NUMERIC}:
 * <ul>
 *    <li>Single: if all documents have 0 or 1 value, then data are written like
 * NUMERIC. <li>SortedSet table: when there are few unique sets of values (&lt;
 * 256) then each set is assigned an id, a lookup table is written and the
 * mapping from document to set id is written using the numeric strategies
 * above. <li>SortedNumeric: a value deque and per-document index into this deque
 * are written using the numeric strategies above.
 * </ul>
 * <p>
 * Files:
 * <ol>
 *   <li><tt>.dvd</tt>: DocValues data</li>
 *   <li><tt>.dvm</tt>: DocValues metadata</li>
 * </ol>
 * @lucene.experimental
 * @deprecated Use {@link Lucene70DocValuesFormat}.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class Lucene54DocValuesFormat extends
// org.apache.lucene.codecs.DocValuesFormat
class Lucene54DocValuesFormat final : public DocValuesFormat
{

  /** Sole Constructor */
public:
  Lucene54DocValuesFormat();

  std::shared_ptr<DocValuesConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<DocValuesProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  static const std::wstring DATA_CODEC;
  static const std::wstring DATA_EXTENSION;
  static const std::wstring META_CODEC;
  static const std::wstring META_EXTENSION;
  static constexpr int VERSION_START = 0;
  static constexpr int VERSION_CURRENT = VERSION_START;

  // indicates docvalues type
  static constexpr char NUMERIC = 0;
  static constexpr char BINARY = 1;
  static constexpr char SORTED = 2;
  static constexpr char SORTED_SET = 3;
  static constexpr char SORTED_NUMERIC = 4;

  // address terms in blocks of 16 terms
  static constexpr int INTERVAL_SHIFT = 4;
  static const int INTERVAL_COUNT = 1 << INTERVAL_SHIFT;
  static const int INTERVAL_MASK = INTERVAL_COUNT - 1;

  // build reverse index from every 1024th term
  static constexpr int REVERSE_INTERVAL_SHIFT = 10;
  static const int REVERSE_INTERVAL_COUNT = 1 << REVERSE_INTERVAL_SHIFT;
  static const int REVERSE_INTERVAL_MASK = REVERSE_INTERVAL_COUNT - 1;

  // for conversion from reverse index to block
  static const int BLOCK_INTERVAL_SHIFT =
      REVERSE_INTERVAL_SHIFT - INTERVAL_SHIFT;
  static const int BLOCK_INTERVAL_COUNT = 1 << BLOCK_INTERVAL_SHIFT;
  static const int BLOCK_INTERVAL_MASK = BLOCK_INTERVAL_COUNT - 1;

  /** Compressed using packed blocks of ints. */
  static constexpr int DELTA_COMPRESSED = 0;
  /** Compressed by computing the GCD. */
  static constexpr int GCD_COMPRESSED = 1;
  /** Compressed by giving IDs to unique values. */
  static constexpr int TABLE_COMPRESSED = 2;
  /** Compressed with monotonically increasing values */
  static constexpr int MONOTONIC_COMPRESSED = 3;
  /** Compressed with constant value (uses only missing bitset) */
  static constexpr int CONST_COMPRESSED = 4;
  /** Compressed with sparse arrays. */
  static constexpr int SPARSE_COMPRESSED = 5;

  /** Uncompressed binary, written directly (fixed length). */
  static constexpr int BINARY_FIXED_UNCOMPRESSED = 0;
  /** Uncompressed binary, written directly (variable length). */
  static constexpr int BINARY_VARIABLE_UNCOMPRESSED = 1;
  /** Compressed binary with shared prefixes */
  static constexpr int BINARY_PREFIX_COMPRESSED = 2;

  /** Standard storage for sorted set values with 1 level of indirection:
   *  {@code docId -> address -> ord}. */
  static constexpr int SORTED_WITH_ADDRESSES = 0;
  /** Single-valued sorted set values, encoded as sorted values, so no level
   *  of indirection: {@code docId -> ord}. */
  static constexpr int SORTED_SINGLE_VALUED = 1;
  /** Compressed giving IDs to unique sets of values:
   * {@code docId -> setId -> ords} */
  static constexpr int SORTED_SET_TABLE = 2;

  /** placeholder for missing offset that means there are no missing values */
  static constexpr int ALL_LIVE = -1;
  /** placeholder for missing offset that means all values are missing */
  static constexpr int ALL_MISSING = -2;

  // addressing uses 16k blocks
  static constexpr int MONOTONIC_BLOCK_SIZE = 16384;
  static constexpr int DIRECT_MONOTONIC_BLOCK_SHIFT = 16;

protected:
  std::shared_ptr<Lucene54DocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene54DocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene54
