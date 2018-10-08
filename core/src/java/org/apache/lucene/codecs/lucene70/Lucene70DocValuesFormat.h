#pragma once
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
namespace org::apache::lucene::codecs::lucene70
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Lucene 7.0 DocValues format.
 * <p>
 * Documents that have a value for the field are encoded in a way that it is
 * always possible to know the ordinal of the current document in the set of
 * documents that have a value. For instance, say the set of documents that have
 * a value for the field is <tt>{1, 5, 6, 11}</tt>. When the iterator is on
 * <tt>6</tt>, it knows that this is the 3rd item of the set. This way, values
 * can be stored densely and accessed based on their index at search time. If
 * all documents in a segment have a value for the field, the index is the same
 * as the doc ID, so this case is encoded implicitly and is very fast at query
 * time. On the other hand if some documents are missing a value for the field
 * then the set of documents that have a value is encoded into blocks. All doc
 * IDs that share the same upper 16 bits are encoded into the same block with
 * the following strategies: <ul> <li>SPARSE: This strategy is used when a block
 * contains at most 4095 documents. The lower 16 bits of doc IDs are stored as
 * {@link DataOutput#writeShort(short) shorts} while the upper 16 bits are given
 * by the block ID. <li>DENSE: This strategy is used when a block contains
 * between 4096 and 65535 documents. The lower bits of doc IDs are stored in a
 * bit set. Advancing is performed using
 *         {@link Long#numberOfTrailingZeros(long) ntz} operations while the
 * index is computed by accumulating the {@link Long#bitCount(long) bit counts}
 * of the visited longs. <li>ALL: This strategy is used when a block contains
 * exactly 65536 documents, meaning that the block is full. In that case doc IDs
 * do not need to be stored explicitly. This is typically faster than both
 * SPARSE and DENSE which is a reason why it is preferable to have all documents
 * that have a value for a field using contiguous doc IDs, for instance by using
 * {@link IndexWriterConfig#setIndexSort(org.apache.lucene.search.Sort) index
 * sorting}.
 * </ul>
 * <p>
 * Then the five per-document value types
 * (Numeric,Binary,Sorted,SortedSet,SortedNumeric) are encoded using the
 * following strategies: <p>
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
 *    <li>Const-compressed: when there is only one possible value, no
 * per-document data is needed and this value is encoded alone.
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
 * Prefix-compressed Binary, along with the per-document ordinals written using
 * one of the numeric strategies above.
 * </ul>
 * <p>
 * {@link DocValuesType#SORTED_SET SORTED_SET}:
 * <ul>
 *    <li>Single: if all documents have 0 or 1 value, then data are written like
 * SORTED. <li>SortedSet: a mapping of ordinals to deduplicated terms is written
 * as Binary, an ordinal deque and per-document index into this deque are written
 * using the numeric strategies above.
 * </ul>
 * <p>
 * {@link DocValuesType#SORTED_NUMERIC SORTED_NUMERIC}:
 * <ul>
 *    <li>Single: if all documents have 0 or 1 value, then data are written like
 * NUMERIC. <li>SortedNumeric: a value deque and per-document index into this
 * deque are written using the numeric strategies above.
 * </ul>
 * <p>
 * Files:
 * <ol>
 *   <li><tt>.dvd</tt>: DocValues data</li>
 *   <li><tt>.dvm</tt>: DocValues metadata</li>
 * </ol>
 * @lucene.experimental
 */
class Lucene70DocValuesFormat final : public DocValuesFormat
{
  GET_CLASS_NAME(Lucene70DocValuesFormat)

  /** Sole Constructor */
public:
  Lucene70DocValuesFormat();

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

  static constexpr int DIRECT_MONOTONIC_BLOCK_SHIFT = 16;

  static constexpr int NUMERIC_BLOCK_SHIFT = 14;
  static const int NUMERIC_BLOCK_SIZE = 1 << NUMERIC_BLOCK_SHIFT;

  static constexpr int TERMS_DICT_BLOCK_SHIFT = 4;
  static const int TERMS_DICT_BLOCK_SIZE = 1 << TERMS_DICT_BLOCK_SHIFT;
  static const int TERMS_DICT_BLOCK_MASK = TERMS_DICT_BLOCK_SIZE - 1;

  static constexpr int TERMS_DICT_REVERSE_INDEX_SHIFT = 10;
  static const int TERMS_DICT_REVERSE_INDEX_SIZE =
      1 << TERMS_DICT_REVERSE_INDEX_SHIFT;
  static const int TERMS_DICT_REVERSE_INDEX_MASK =
      TERMS_DICT_REVERSE_INDEX_SIZE - 1;

protected:
  std::shared_ptr<Lucene70DocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene70DocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene70
