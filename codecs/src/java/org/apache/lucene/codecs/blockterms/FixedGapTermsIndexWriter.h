#pragma once
#include "TermsIndexWriterBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::codecs::blockterms
{
class SimpleFieldWriter;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class RAMOutputStream;
}
namespace org::apache::lucene::util::packed
{
class MonotonicBlockPackedWriter;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::codecs
{
class TermStats;
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
namespace org::apache::lucene::codecs::blockterms
{

using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using TermStats = org::apache::lucene::codecs::TermStats;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using MonotonicBlockPackedWriter =
    org::apache::lucene::util::packed::MonotonicBlockPackedWriter;

/**
 * Selects every Nth term as and index term, and hold term
 * bytes (mostly) fully expanded in memory.  This terms index
 * supports seeking by ord.  See {@link
 * VariableGapTermsIndexWriter} for a more memory efficient
 * terms index that does not support seeking by ord.
 *
 * @lucene.experimental */
class FixedGapTermsIndexWriter : public TermsIndexWriterBase
{
  GET_CLASS_NAME(FixedGapTermsIndexWriter)
protected:
  std::shared_ptr<IndexOutput> out;

  /** Extension of terms index file */
public:
  static const std::wstring TERMS_INDEX_EXTENSION;

  static const std::wstring CODEC_NAME;
  static constexpr int VERSION_START = 4;
  static constexpr int VERSION_CURRENT = VERSION_START;

  static constexpr int BLOCKSIZE = 4096;

private:
  const int termIndexInterval;

public:
  static constexpr int DEFAULT_TERM_INDEX_INTERVAL = 32;

private:
  const std::deque<std::shared_ptr<SimpleFieldWriter>> fields =
      std::deque<std::shared_ptr<SimpleFieldWriter>>();

public:
  FixedGapTermsIndexWriter(std::shared_ptr<SegmentWriteState> state) throw(
      IOException);

  FixedGapTermsIndexWriter(std::shared_ptr<SegmentWriteState> state,
                           int termIndexInterval) ;

  std::shared_ptr<FieldWriter> addField(std::shared_ptr<FieldInfo> field,
                                        int64_t termsFilePointer) override;

  /** NOTE: if your codec does not sort in unicode code
   *  point order, you must override this method, to simply
   *  return indexedTerm.length. */
protected:
  virtual int indexedTermPrefixLength(std::shared_ptr<BytesRef> priorTerm,
                                      std::shared_ptr<BytesRef> indexedTerm);

private:
  class SimpleFieldWriter : public FieldWriter
  {
    GET_CLASS_NAME(SimpleFieldWriter)
  private:
    std::shared_ptr<FixedGapTermsIndexWriter> outerInstance;

  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    int numIndexTerms = 0;
    const int64_t indexStart;
    const int64_t termsStart;
    int64_t packedIndexStart = 0;
    int64_t packedOffsetsStart = 0;

  private:
    int64_t numTerms = 0;

    std::shared_ptr<RAMOutputStream> offsetsBuffer =
        std::make_shared<RAMOutputStream>();
    std::shared_ptr<MonotonicBlockPackedWriter> termOffsets =
        std::make_shared<MonotonicBlockPackedWriter>(offsetsBuffer, BLOCKSIZE);
    int64_t currentOffset = 0;

    std::shared_ptr<RAMOutputStream> addressBuffer =
        std::make_shared<RAMOutputStream>();
    std::shared_ptr<MonotonicBlockPackedWriter> termAddresses =
        std::make_shared<MonotonicBlockPackedWriter>(addressBuffer, BLOCKSIZE);

    const std::shared_ptr<BytesRefBuilder> lastTerm =
        std::make_shared<BytesRefBuilder>();

  public:
    SimpleFieldWriter(std::shared_ptr<FixedGapTermsIndexWriter> outerInstance,
                      std::shared_ptr<FieldInfo> fieldInfo,
                      int64_t termsFilePointer);

    bool checkIndexTerm(
        std::shared_ptr<BytesRef> text,
        std::shared_ptr<TermStats> stats)  override;

    void add(std::shared_ptr<BytesRef> text, std::shared_ptr<TermStats> stats,
             int64_t termsFilePointer)  override;

    void finish(int64_t termsFilePointer)  override;

  protected:
    std::shared_ptr<SimpleFieldWriter> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFieldWriter>(
          FieldWriter::shared_from_this());
    }
  };

public:
  virtual ~FixedGapTermsIndexWriter();

private:
  void writeTrailer(int64_t dirStart) ;

protected:
  std::shared_ptr<FixedGapTermsIndexWriter> shared_from_this()
  {
    return std::static_pointer_cast<FixedGapTermsIndexWriter>(
        TermsIndexWriterBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blockterms
