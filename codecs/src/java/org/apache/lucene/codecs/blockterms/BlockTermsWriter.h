#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/codecs/blockterms/TermsIndexWriterBase.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/codecs/blockterms/TermsWriter.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/codecs/blockterms/FieldWriter.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/RAMOutputStream.h"

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
namespace org::apache::lucene::codecs::blockterms
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

// TODO: currently we encode all terms between two indexed
// terms as a block; but, we could decouple the two, ie
// allow several blocks in between two indexed terms

/**
 * Writes terms dict, block-encoding (column stride) each
 * term's metadata for each set of terms between two
 * index terms.
 *
 * @lucene.experimental
 */

class BlockTermsWriter : public FieldsConsumer
{
  GET_CLASS_NAME(BlockTermsWriter)

public:
  static const std::wstring CODEC_NAME;

  // Initial format
  static constexpr int VERSION_START = 4;
  static constexpr int VERSION_CURRENT = VERSION_START;

  /** Extension of terms file */
  static const std::wstring TERMS_EXTENSION;

protected:
  std::shared_ptr<IndexOutput> out;

public:
  const std::shared_ptr<PostingsWriterBase> postingsWriter;
  const std::shared_ptr<FieldInfos> fieldInfos;
  std::shared_ptr<FieldInfo> currentField;

private:
  const std::shared_ptr<TermsIndexWriterBase> termsIndexWriter;
  const int maxDoc;

private:
  class FieldMetaData : public std::enable_shared_from_this<FieldMetaData>
  {
    GET_CLASS_NAME(FieldMetaData)
  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int64_t numTerms;
    const int64_t termsStartPointer;
    const int64_t sumTotalTermFreq;
    const int64_t sumDocFreq;
    const int docCount;
    const int longsSize;

    FieldMetaData(std::shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
                  int64_t termsStartPointer, int64_t sumTotalTermFreq,
                  int64_t sumDocFreq, int docCount, int longsSize);
  };

private:
  const std::deque<std::shared_ptr<FieldMetaData>> fields =
      std::deque<std::shared_ptr<FieldMetaData>>();

  // private final std::wstring segment;

public:
  BlockTermsWriter(
      std::shared_ptr<TermsIndexWriterBase> termsIndexWriter,
      std::shared_ptr<SegmentWriteState> state,
      std::shared_ptr<PostingsWriterBase> postingsWriter) ;

  void write(std::shared_ptr<Fields> fields)  override;

private:
  std::shared_ptr<TermsWriter>
  addField(std::shared_ptr<FieldInfo> field) ;

public:
  virtual ~BlockTermsWriter();

private:
  void writeTrailer(int64_t dirStart) ;

private:
  class TermEntry : public std::enable_shared_from_this<TermEntry>
  {
    GET_CLASS_NAME(TermEntry)
  public:
    const std::shared_ptr<BytesRefBuilder> term =
        std::make_shared<BytesRefBuilder>();
    std::shared_ptr<BlockTermState> state;
  };

public:
  class TermsWriter : public std::enable_shared_from_this<TermsWriter>
  {
    GET_CLASS_NAME(TermsWriter)
  private:
    std::shared_ptr<BlockTermsWriter> outerInstance;

    const std::shared_ptr<FieldInfo> fieldInfo;
    const std::shared_ptr<PostingsWriterBase> postingsWriter;
    const int64_t termsStartPointer;
    int64_t numTerms = 0;
    const std::shared_ptr<TermsIndexWriterBase::FieldWriter> fieldIndexWriter;
    const std::shared_ptr<FixedBitSet> docsSeen;

  public:
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    int docCount = 0;
    int longsSize = 0;

  private:
    std::deque<std::shared_ptr<TermEntry>> pendingTerms;

    int pendingCount = 0;

  public:
    TermsWriter(
        std::shared_ptr<BlockTermsWriter> outerInstance,
        std::shared_ptr<TermsIndexWriterBase::FieldWriter> fieldIndexWriter,
        std::shared_ptr<FieldInfo> fieldInfo,
        std::shared_ptr<PostingsWriterBase> postingsWriter);

  private:
    const std::shared_ptr<BytesRefBuilder> lastPrevTerm =
        std::make_shared<BytesRefBuilder>();

  public:
    virtual void write(std::shared_ptr<BytesRef> text,
                       std::shared_ptr<TermsEnum> termsEnum) ;

    // Finishes all terms in this field
    virtual void finish() ;

  private:
    int sharedPrefix(std::shared_ptr<BytesRef> term1,
                     std::shared_ptr<BytesRef> term2);

    const std::shared_ptr<RAMOutputStream> bytesWriter =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> bufferWriter =
        std::make_shared<RAMOutputStream>();

    void flushBlock() ;
  };

protected:
  std::shared_ptr<BlockTermsWriter> shared_from_this()
  {
    return std::static_pointer_cast<BlockTermsWriter>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/blockterms/
