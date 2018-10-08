#pragma once
#include "../../index/DocIDMerger.h"
#include "../../util/packed/PackedInts.h"
#include "../StoredFieldsWriter.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressingStoredFieldsIndexWriter.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/Compressor.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressionMode.h"
#include  "core/src/java/org/apache/lucene/store/GrowableByteArrayDataOutput.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressingStoredFieldsReader.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"

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

using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * {@link StoredFieldsWriter} impl for {@link CompressingStoredFieldsFormat}.
 * @lucene.experimental
 */
class CompressingStoredFieldsWriter final : public StoredFieldsWriter
{
  GET_CLASS_NAME(CompressingStoredFieldsWriter)

  /** Extension of stored fields file */
public:
  static const std::wstring FIELDS_EXTENSION;

  /** Extension of stored fields index file */
  static const std::wstring FIELDS_INDEX_EXTENSION;

  static constexpr int STRING = 0x00;
  static constexpr int BYTE_ARR = 0x01;
  static constexpr int NUMERIC_INT = 0x02;
  static constexpr int NUMERIC_FLOAT = 0x03;
  static constexpr int NUMERIC_LONG = 0x04;
  static constexpr int NUMERIC_DOUBLE = 0x05;

  static const int TYPE_BITS = PackedInts::bitsRequired(NUMERIC_DOUBLE);
  static const int TYPE_MASK =
      static_cast<int>(PackedInts::maxValue(TYPE_BITS));

  static const std::wstring CODEC_SFX_IDX;
  static const std::wstring CODEC_SFX_DAT;
  static constexpr int VERSION_START = 1;
  static constexpr int VERSION_CURRENT = VERSION_START;

private:
  const std::wstring segment;
  std::shared_ptr<CompressingStoredFieldsIndexWriter> indexWriter;
  std::shared_ptr<IndexOutput> fieldsStream;

  std::shared_ptr<Compressor> compressor;
  const std::shared_ptr<CompressionMode> compressionMode;
  const int chunkSize;
  const int maxDocsPerChunk;

  const std::shared_ptr<GrowableByteArrayDataOutput> bufferedDocs;
  std::deque<int> numStoredFields; // number of stored fields
  std::deque<int> endOffsets;      // end offsets in bufferedDocs
  int docBase = 0;                  // doc ID at the beginning of the chunk
  int numBufferedDocs = 0; // docBase + numBufferedDocs == current doc ID

  int64_t numChunks = 0; // number of compressed blocks written
  int64_t numDirtyChunks =
      0; // number of incomplete compressed blocks written

  /** Sole constructor. */
public:
  CompressingStoredFieldsWriter(
      std::shared_ptr<Directory> directory, std::shared_ptr<SegmentInfo> si,
      const std::wstring &segmentSuffix, std::shared_ptr<IOContext> context,
      const std::wstring &formatName,
      std::shared_ptr<CompressionMode> compressionMode, int chunkSize,
      int maxDocsPerChunk, int blockSize) ;

  virtual ~CompressingStoredFieldsWriter();

private:
  int numStoredFieldsInDoc = 0;

public:
  void startDocument()  override;

  void finishDocument()  override;

private:
  static void saveInts(std::deque<int> &values, int length,
                       std::shared_ptr<DataOutput> out) ;

  void writeHeader(int docBase, int numBufferedDocs,
                   std::deque<int> &numStoredFields, std::deque<int> &lengths,
                   bool sliced) ;

  bool triggerFlush();

  void flush() ;

public:
  void
  writeField(std::shared_ptr<FieldInfo> info,
             std::shared_ptr<IndexableField> field)  override;

  // -0 isn't compressed.
  static const int NEGATIVE_ZERO_FLOAT = Float::floatToIntBits(-0.0f);
  static const int64_t NEGATIVE_ZERO_DOUBLE = Double::doubleToLongBits(-0);

  // for compression of timestamps
  static constexpr int64_t SECOND = 1000LL;
  static const int64_t HOUR = 60 * 60 * SECOND;
  static const int64_t DAY = 24 * HOUR;
  static constexpr int SECOND_ENCODING = 0x40;
  static constexpr int HOUR_ENCODING = 0x80;
  static constexpr int DAY_ENCODING = 0xC0;

  /**
   * Writes a float in a variable-length format.  Writes between one and
   * five bytes. Small integral values typically take fewer bytes.
   * <p>
   * ZFloat --&gt; Header, Bytes*?
   * <ul>
   *    <li>Header --&gt; {@link DataOutput#writeByte Uint8}. When it is
   *       equal to 0xFF then the value is negative and stored in the next
   *       4 bytes. Otherwise if the first bit is set then the other bits
   *       in the header encode the value plus one and no other
   *       bytes are read. Otherwise, the value is a positive float value
   *       whose first byte is the header, and 3 bytes need to be read to
   *       complete it.
   *    <li>Bytes --&gt; Potential additional bytes to read depending on the
   *       header.
   * </ul>
   */
  static void writeZFloat(std::shared_ptr<DataOutput> out,
                          float f) ;

  /**
   * Writes a float in a variable-length format.  Writes between one and
   * five bytes. Small integral values typically take fewer bytes.
   * <p>
   * ZFloat --&gt; Header, Bytes*?
   * <ul>
   *    <li>Header --&gt; {@link DataOutput#writeByte Uint8}. When it is
   *       equal to 0xFF then the value is negative and stored in the next
   *       8 bytes. When it is equal to 0xFE then the value is stored as a
   *       float in the next 4 bytes. Otherwise if the first bit is set
   *       then the other bits in the header encode the value plus one and
   *       no other bytes are read. Otherwise, the value is a positive float
   *       value whose first byte is the header, and 7 bytes need to be read
   *       to complete it.
   *    <li>Bytes --&gt; Potential additional bytes to read depending on the
   *       header.
   * </ul>
   */
  static void writeZDouble(std::shared_ptr<DataOutput> out,
                           double d) ;

  /**
   * Writes a long in a variable-length format.  Writes between one and
   * ten bytes. Small values or values representing timestamps with day,
   * hour or second precision typically require fewer bytes.
   * <p>
   * ZLong --&gt; Header, Bytes*?
   * <ul>
   *    <li>Header --&gt; The first two bits indicate the compression scheme:
   *       <ul>
   *          <li>00 - uncompressed
   *          <li>01 - multiple of 1000 (second)
   *          <li>10 - multiple of 3600000 (hour)
   *          <li>11 - multiple of 86400000 (day)
   *       </ul>
   *       Then the next bit is a continuation bit, indicating whether more
   *       bytes need to be read, and the last 5 bits are the lower bits of
   *       the encoded value. In order to reconstruct the value, you need to
   *       combine the 5 lower bits of the header with a vLong in the next
   *       bytes (if the continuation bit is set to 1). Then
   *       {@link BitUtil#zigZagDecode(int) zigzag-decode} it and finally
   *       multiply by the multiple corresponding to the compression scheme.
   *    <li>Bytes --&gt; Potential additional bytes to read depending on the
   *       header.
   * </ul>
   */
  // T for "timestamp"
  static void writeTLong(std::shared_ptr<DataOutput> out,
                         int64_t l) ;

  void finish(std::shared_ptr<FieldInfos> fis,
              int numDocs)  override;

  // bulk merge is scary: its caused corruption bugs in the past.
  // we try to be extra safe with this impl, but add an escape hatch to
  // have a workaround for undiscovered bugs.
  static const std::wstring BULK_MERGE_ENABLED_SYSPROP;
  static const bool BULK_MERGE_ENABLED = false;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static CompressingStoredFieldsWriter::StaticConstructor staticConstructor;

public:
  int merge(std::shared_ptr<MergeState> mergeState)  override;

  /**
   * Returns true if we should recompress this reader, even though we could bulk
   * merge compressed data <p> The last chunk written for a segment is typically
   * incomplete, so without recompressing, in some worst-case situations (e.g.
   * frequent reopen with tiny flushes), over time the compression ratio can
   * degrade. This is a safety switch.
   */
  bool tooDirty(std::shared_ptr<CompressingStoredFieldsReader> candidate);

private:
  class CompressingStoredFieldsMergeSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(CompressingStoredFieldsMergeSub)
  private:
    const std::shared_ptr<CompressingStoredFieldsReader> reader;
    const int maxDoc;

  public:
    int docID = -1;

    CompressingStoredFieldsMergeSub(
        std::shared_ptr<CompressingStoredFieldsReader> reader,
        std::shared_ptr<MergeState::DocMap> docMap, int maxDoc);

    int nextDoc() override;

  protected:
    std::shared_ptr<CompressingStoredFieldsMergeSub> shared_from_this()
    {
      return std::static_pointer_cast<CompressingStoredFieldsMergeSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CompressingStoredFieldsWriter> shared_from_this()
  {
    return std::static_pointer_cast<CompressingStoredFieldsWriter>(
        org.apache.lucene.codecs.StoredFieldsWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/compressing/
