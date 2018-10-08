#pragma once
#include "../../index/Fields.h"
#include "../../index/PostingsEnum.h"
#include "../../index/Terms.h"
#include "../../index/TermsEnum.h"
#include "../TermVectorsReader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfos;
}

namespace org::apache::lucene::codecs::compressing
{
class CompressingStoredFieldsIndexReader;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::codecs::compressing
{
class CompressionMode;
}
namespace org::apache::lucene::codecs::compressing
{
class Decompressor;
}
namespace org::apache::lucene::util::packed
{
class BlockPackedReaderIterator;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::store
{
class AlreadyClosedException;
}
namespace org::apache::lucene::codecs
{
class TermVectorsReader;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::packed
{
class PackedInts;
}
namespace org::apache::lucene::util::packed
{
class Reader;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::util
{
class Accountable;
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

using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BlockPackedReaderIterator =
    org::apache::lucene::util::packed::BlockPackedReaderIterator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.CODEC_SFX_DAT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.CODEC_SFX_IDX;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.FLAGS_BITS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.OFFSETS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.PACKED_BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.PAYLOADS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.POSITIONS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VECTORS_EXTENSION;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VECTORS_INDEX_EXTENSION;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VERSION_CURRENT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VERSION_START;

/**
 * {@link TermVectorsReader} for {@link CompressingTermVectorsFormat}.
 * @lucene.experimental
 */
class CompressingTermVectorsReader final : public TermVectorsReader
{
  GET_CLASS_NAME(CompressingTermVectorsReader)

private:
  const std::shared_ptr<FieldInfos> fieldInfos;

public:
  const std::shared_ptr<CompressingStoredFieldsIndexReader> indexReader;
  const std::shared_ptr<IndexInput> vectorsStream;

private:
  const int version;
  const int packedIntsVersion;
  const std::shared_ptr<CompressionMode> compressionMode;
  const std::shared_ptr<Decompressor> decompressor;
  const int chunkSize;
  const int numDocs;
  bool closed = false;
  const std::shared_ptr<BlockPackedReaderIterator> reader;
  const int64_t numChunks; // number of compressed blocks written
  const int64_t
      numDirtyChunks;         // number of incomplete compressed blocks written
  const int64_t maxPointer; // end of the data section

  // used by clone
  CompressingTermVectorsReader(
      std::shared_ptr<CompressingTermVectorsReader> reader);

  /** Sole constructor. */
public:
  CompressingTermVectorsReader(
      std::shared_ptr<Directory> d, std::shared_ptr<SegmentInfo> si,
      const std::wstring &segmentSuffix, std::shared_ptr<FieldInfos> fn,
      std::shared_ptr<IOContext> context, const std::wstring &formatName,
      std::shared_ptr<CompressionMode> compressionMode) ;

  std::shared_ptr<CompressionMode> getCompressionMode();

  int getChunkSize();

  int getPackedIntsVersion();

  int getVersion();

  std::shared_ptr<CompressingStoredFieldsIndexReader> getIndexReader();

  std::shared_ptr<IndexInput> getVectorsStream();

  int64_t getMaxPointer();

  int64_t getNumChunks();

  int64_t getNumDirtyChunks();

  /**
   * @throws AlreadyClosedException if this TermVectorsReader is closed
   */
private:
  void ensureOpen() ;

public:
  virtual ~CompressingTermVectorsReader();

  std::shared_ptr<TermVectorsReader> clone() override;

  std::shared_ptr<Fields> get(int doc)  override;

  // field -> term index -> position index
private:
  std::deque<std::deque<int>>
  positionIndex(int skip, int numFields,
                std::shared_ptr<PackedInts::Reader> numTerms,
                std::deque<int> &termFreqs);

  std::deque<std::deque<int>> readPositions(
      int skip, int numFields, std::shared_ptr<PackedInts::Reader> flags,
      std::shared_ptr<PackedInts::Reader> numTerms, std::deque<int> &termFreqs,
      int flag, int const totalPositions,
      std::deque<std::deque<int>> &positionIndex) ;

private:
  class TVFields : public Fields
  {
    GET_CLASS_NAME(TVFields)
  private:
    std::shared_ptr<CompressingTermVectorsReader> outerInstance;

    std::deque<int> const fieldNums, fieldFlags, fieldNumOffs, numTerms,
        fieldLengths;
    std::deque<std::deque<int>> const prefixLengths, suffixLengths, termFreqs,
        positionIndex, positions, startOffsets, lengths, payloadIndex;
    const std::shared_ptr<BytesRef> suffixBytes, payloadBytes;

  public:
    TVFields(std::shared_ptr<CompressingTermVectorsReader> outerInstance,
             std::deque<int> &fieldNums, std::deque<int> &fieldFlags,
             std::deque<int> &fieldNumOffs, std::deque<int> &numTerms,
             std::deque<int> &fieldLengths,
             std::deque<std::deque<int>> &prefixLengths,
             std::deque<std::deque<int>> &suffixLengths,
             std::deque<std::deque<int>> &termFreqs,
             std::deque<std::deque<int>> &positionIndex,
             std::deque<std::deque<int>> &positions,
             std::deque<std::deque<int>> &startOffsets,
             std::deque<std::deque<int>> &lengths,
             std::shared_ptr<BytesRef> payloadBytes,
             std::deque<std::deque<int>> &payloadIndex,
             std::shared_ptr<BytesRef> suffixBytes);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::wstring>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<TVFields> outerInstance;

    public:
      IteratorAnonymousInnerClass(std::shared_ptr<TVFields> outerInstance);

      int i = 0;
      bool hasNext();
      std::wstring next();
      void remove();
    };

  public:
    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

  protected:
    std::shared_ptr<TVFields> shared_from_this()
    {
      return std::static_pointer_cast<TVFields>(
          org.apache.lucene.index.Fields::shared_from_this());
    }
  };

private:
  class TVTerms : public Terms
  {
    GET_CLASS_NAME(TVTerms)

  private:
    const int numTerms, flags;
    std::deque<int> const prefixLengths, suffixLengths, termFreqs,
        positionIndex, positions, startOffsets, lengths, payloadIndex;
    const std::shared_ptr<BytesRef> termBytes, payloadBytes;

  public:
    TVTerms(int numTerms, int flags, std::deque<int> &prefixLengths,
            std::deque<int> &suffixLengths, std::deque<int> &termFreqs,
            std::deque<int> &positionIndex, std::deque<int> &positions,
            std::deque<int> &startOffsets, std::deque<int> &lengths,
            std::deque<int> &payloadIndex,
            std::shared_ptr<BytesRef> payloadBytes,
            std::shared_ptr<BytesRef> termBytes);

    std::shared_ptr<TermsEnum> iterator()  override;

    int64_t size()  override;

    int64_t getSumTotalTermFreq()  override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<TVTerms> shared_from_this()
    {
      return std::static_pointer_cast<TVTerms>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

private:
  class TVTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(TVTermsEnum)

  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    int numTerms = 0, startPos = 0, ord_ = 0;
    std::deque<int> prefixLengths, suffixLengths, termFreqs, positionIndex,
        positions, startOffsets, lengths, payloadIndex;
    std::shared_ptr<ByteArrayDataInput> in_;
    std::shared_ptr<BytesRef> payloads;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<BytesRef> term_;

    TVTermsEnum();

  public:
    virtual void
    reset(int numTerms, int flags, std::deque<int> &prefixLengths,
          std::deque<int> &suffixLengths, std::deque<int> &termFreqs,
          std::deque<int> &positionIndex, std::deque<int> &positions,
          std::deque<int> &startOffsets, std::deque<int> &lengths,
          std::deque<int> &payloadIndex, std::shared_ptr<BytesRef> payloads,
          std::shared_ptr<ByteArrayDataInput> in_);

    virtual void reset();

    std::shared_ptr<BytesRef> next()  override;

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    void seekExact(int64_t ord)  override;

    std::shared_ptr<BytesRef> term()  override;

    int64_t ord()  override;

    int docFreq()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override final;

  protected:
    std::shared_ptr<TVTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<TVTermsEnum>(
          org.apache.lucene.index.TermsEnum::shared_from_this());
    }
  };

private:
  class TVPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(TVPostingsEnum)

  private:
    int doc = -1;
    int termFreq = 0;
    int positionIndex = 0;
    std::deque<int> positions;
    std::deque<int> startOffsets;
    std::deque<int> lengths;
    const std::shared_ptr<BytesRef> payload;
    std::deque<int> payloadIndex;
    int basePayloadOffset = 0;
    int i = 0;

  public:
    TVPostingsEnum();

    virtual void reset(int freq, int positionIndex, std::deque<int> &positions,
                       std::deque<int> &startOffsets,
                       std::deque<int> &lengths,
                       std::shared_ptr<BytesRef> payloads,
                       std::deque<int> &payloadIndex);

  private:
    void checkDoc();

    void checkPosition();

  public:
    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int freq()  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<TVPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<TVPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

private:
  static int sum(std::deque<int> &arr);

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressingTermVectorsReader> shared_from_this()
  {
    return std::static_pointer_cast<CompressingTermVectorsReader>(
        org.apache.lucene.codecs.TermVectorsReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::compressing
