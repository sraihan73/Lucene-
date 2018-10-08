#pragma once
#include "../../util/packed/PackedInts.h"
#include "../TermVectorsWriter.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressingStoredFieldsIndexWriter.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressionMode.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/Compressor.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/FieldData.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/GrowableByteArrayDataOutput.h"
#include  "core/src/java/org/apache/lucene/util/packed/BlockPackedWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressingTermVectorsReader.h"

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

using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BlockPackedWriter = org::apache::lucene::util::packed::BlockPackedWriter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

/**
 * {@link TermVectorsWriter} for {@link CompressingTermVectorsFormat}.
 * @lucene.experimental
 */
class CompressingTermVectorsWriter final : public TermVectorsWriter
{
  GET_CLASS_NAME(CompressingTermVectorsWriter)

  // hard limit on the maximum number of documents per chunk
public:
  static constexpr int MAX_DOCUMENTS_PER_CHUNK = 128;

  static const std::wstring VECTORS_EXTENSION;
  static const std::wstring VECTORS_INDEX_EXTENSION;

  static const std::wstring CODEC_SFX_IDX;
  static const std::wstring CODEC_SFX_DAT;

  static constexpr int VERSION_START = 1;
  static constexpr int VERSION_CURRENT = VERSION_START;

  static constexpr int PACKED_BLOCK_SIZE = 64;

  static constexpr int POSITIONS = 0x01;
  static constexpr int OFFSETS = 0x02;
  static constexpr int PAYLOADS = 0x04;
  static const int FLAGS_BITS =
      PackedInts::bitsRequired(POSITIONS | OFFSETS | PAYLOADS);

private:
  const std::wstring segment;
  std::shared_ptr<CompressingStoredFieldsIndexWriter> indexWriter;
  std::shared_ptr<IndexOutput> vectorsStream;

  const std::shared_ptr<CompressionMode> compressionMode;
  const std::shared_ptr<Compressor> compressor;
  const int chunkSize;

  int64_t numChunks = 0; // number of compressed blocks written
  int64_t numDirtyChunks =
      0; // number of incomplete compressed blocks written

  /** a pending doc */
private:
  class DocData : public std::enable_shared_from_this<DocData>
  {
    GET_CLASS_NAME(DocData)
  private:
    std::shared_ptr<CompressingTermVectorsWriter> outerInstance;

  public:
    const int numFields;
    const std::shared_ptr<Deque<std::shared_ptr<FieldData>>> fields;
    const int posStart, offStart, payStart;
    DocData(std::shared_ptr<CompressingTermVectorsWriter> outerInstance,
            int numFields, int posStart, int offStart, int payStart);
    virtual std::shared_ptr<FieldData> addField(int fieldNum, int numTerms,
                                                bool positions, bool offsets,
                                                bool payloads);
  };

private:
  std::shared_ptr<DocData> addDocData(int numVectorFields);

  /** a pending field */
private:
  class FieldData : public std::enable_shared_from_this<FieldData>
  {
    GET_CLASS_NAME(FieldData)
  private:
    std::shared_ptr<CompressingTermVectorsWriter> outerInstance;

  public:
    const bool hasPositions, hasOffsets, hasPayloads;
    const int fieldNum, flags, numTerms;
    std::deque<int> const freqs, prefixLengths, suffixLengths;
    const int posStart, offStart, payStart;
    int totalPositions = 0;
    int ord = 0;
    FieldData(std::shared_ptr<CompressingTermVectorsWriter> outerInstance,
              int fieldNum, int numTerms, bool positions, bool offsets,
              bool payloads, int posStart, int offStart, int payStart);
    virtual void addTerm(int freq, int prefixLength, int suffixLength);
    virtual void addPosition(int position, int startOffset, int length,
                             int payloadLength);
  };

private:
  int numDocs = 0; // total number of docs seen
  const std::shared_ptr<Deque<std::shared_ptr<DocData>>>
      pendingDocs;                     // pending docs
  std::shared_ptr<DocData> curDoc;     // current document
  std::shared_ptr<FieldData> curField; // current field
  const std::shared_ptr<BytesRef> lastTerm;
  std::deque<int> positionsBuf, startOffsetsBuf, lengthsBuf, payloadLengthsBuf;
  const std::shared_ptr<GrowableByteArrayDataOutput>
      termSuffixes; // buffered term suffixes
  const std::shared_ptr<GrowableByteArrayDataOutput>
      payloadBytes; // buffered term payloads
  const std::shared_ptr<BlockPackedWriter> writer;

  /** Sole constructor. */
public:
  CompressingTermVectorsWriter(std::shared_ptr<Directory> directory,
                               std::shared_ptr<SegmentInfo> si,
                               const std::wstring &segmentSuffix,
                               std::shared_ptr<IOContext> context,
                               const std::wstring &formatName,
                               std::shared_ptr<CompressionMode> compressionMode,
                               int chunkSize, int blockSize) ;

  virtual ~CompressingTermVectorsWriter();

  void startDocument(int numVectorFields)  override;

  void finishDocument()  override;

  void startField(std::shared_ptr<FieldInfo> info, int numTerms, bool positions,
                  bool offsets, bool payloads)  override;

  void finishField()  override;

  void startTerm(std::shared_ptr<BytesRef> term,
                 int freq)  override;

  void
  addPosition(int position, int startOffset, int endOffset,
              std::shared_ptr<BytesRef> payload)  override;

private:
  bool triggerFlush();

  void flush() ;

  int flushNumFields(int chunkDocs) ;

  /** Returns a sorted array containing unique field numbers */
  std::deque<int> flushFieldNums() ;

  void flushFields(int totalFields,
                   std::deque<int> &fieldNums) ;

  void flushFlags(int totalFields,
                  std::deque<int> &fieldNums) ;

  void flushNumTerms(int totalFields) ;

  void flushTermLengths() ;

  void flushTermFreqs() ;

  void flushPositions() ;

  void flushOffsets(std::deque<int> &fieldNums) ;

  void flushPayloadLengths() ;

public:
  void finish(std::shared_ptr<FieldInfos> fis,
              int numDocs)  override;

  void addProx(int numProx, std::shared_ptr<DataInput> positions,
               std::shared_ptr<DataInput> offsets)  override;

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
  static CompressingTermVectorsWriter::StaticConstructor staticConstructor;

public:
  int merge(std::shared_ptr<MergeState> mergeState)  override;

  /**
   * Returns true if we should recompress this reader, even though we could bulk
   * merge compressed data <p> The last chunk written for a segment is typically
   * incomplete, so without recompressing, in some worst-case situations (e.g.
   * frequent reopen with tiny flushes), over time the compression ratio can
   * degrade. This is a safety switch.
   */
  bool tooDirty(std::shared_ptr<CompressingTermVectorsReader> candidate);

protected:
  std::shared_ptr<CompressingTermVectorsWriter> shared_from_this()
  {
    return std::static_pointer_cast<CompressingTermVectorsWriter>(
        org.apache.lucene.codecs.TermVectorsWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/compressing/
