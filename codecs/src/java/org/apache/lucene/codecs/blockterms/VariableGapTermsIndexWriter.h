#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "TermsIndexWriterBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/codecs/blockterms/FSTFieldWriter.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/codecs/blockterms/IndexTermSelector.h"
#include  "core/src/java/org/apache/lucene/codecs/TermStats.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"

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

using TermStats = org::apache::lucene::codecs::TermStats;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

/**
 * Selects index terms according to provided pluggable
 * {@link IndexTermSelector}, and stores them in a prefix trie that's
 * loaded entirely in RAM stored as an FST.  This terms
 * index only supports unsigned byte term sort order
 * (unicode codepoint order when the bytes are UTF8).
 *
 * @lucene.experimental */
class VariableGapTermsIndexWriter : public TermsIndexWriterBase
{
  GET_CLASS_NAME(VariableGapTermsIndexWriter)
protected:
  std::shared_ptr<IndexOutput> out;

  /** Extension of terms index file */
public:
  static const std::wstring TERMS_INDEX_EXTENSION;

  static const std::wstring CODEC_NAME;
  static constexpr int VERSION_START = 3;
  static constexpr int VERSION_CURRENT = VERSION_START;

private:
  const std::deque<std::shared_ptr<FSTFieldWriter>> fields =
      std::deque<std::shared_ptr<FSTFieldWriter>>();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private final
  // org.apache.lucene.index.FieldInfos fieldInfos;
  const std::shared_ptr<FieldInfos> fieldInfos; // unread
  const std::shared_ptr<IndexTermSelector> policy;

  /**
   * Hook for selecting which terms should be placed in the terms index.
   * <p>
   * {@link #newField} is called at the start of each new field, and
   * {@link #isIndexTerm} for each term in that field.
   *
   * @lucene.experimental
   */
public:
  class IndexTermSelector
      : public std::enable_shared_from_this<IndexTermSelector>
  {
    GET_CLASS_NAME(IndexTermSelector)
    /**
     * Called sequentially on every term being written,
     * returning true if this term should be indexed
     */
  public:
    virtual bool isIndexTerm(std::shared_ptr<BytesRef> term,
                             std::shared_ptr<TermStats> stats) = 0;
    /**
     * Called when a new field is started.
     */
    virtual void newField(std::shared_ptr<FieldInfo> fieldInfo) = 0;
  };

  /** Same policy as {@link FixedGapTermsIndexWriter} */
public:
  class EveryNTermSelector final : public IndexTermSelector
  {
    GET_CLASS_NAME(EveryNTermSelector)
  private:
    int count = 0;
    const int interval;

  public:
    EveryNTermSelector(int interval);

    bool isIndexTerm(std::shared_ptr<BytesRef> term,
                     std::shared_ptr<TermStats> stats) override;

    void newField(std::shared_ptr<FieldInfo> fieldInfo) override;

  protected:
    std::shared_ptr<EveryNTermSelector> shared_from_this()
    {
      return std::static_pointer_cast<EveryNTermSelector>(
          IndexTermSelector::shared_from_this());
    }
  };

  /** Sets an index term when docFreq &gt;= docFreqThresh, or
   *  every interval terms.  This should reduce seek time
   *  to high docFreq terms.  */
public:
  class EveryNOrDocFreqTermSelector final : public IndexTermSelector
  {
    GET_CLASS_NAME(EveryNOrDocFreqTermSelector)
  private:
    int count = 0;
    const int docFreqThresh;
    const int interval;

  public:
    EveryNOrDocFreqTermSelector(int docFreqThresh, int interval);

    bool isIndexTerm(std::shared_ptr<BytesRef> term,
                     std::shared_ptr<TermStats> stats) override;

    void newField(std::shared_ptr<FieldInfo> fieldInfo) override;

  protected:
    std::shared_ptr<EveryNOrDocFreqTermSelector> shared_from_this()
    {
      return std::static_pointer_cast<EveryNOrDocFreqTermSelector>(
          IndexTermSelector::shared_from_this());
    }
  };

  // TODO: it'd be nice to let the FST builder prune based
  // on term count of each node (the prune1/prune2 that it
  // accepts), and build the index based on that.  This
  // should result in a more compact terms index, more like
  // a prefix trie than the other selectors, because it
  // only stores enough leading bytes to get down to N
  // terms that may complete that prefix.  It becomes
  // "deeper" when terms are dense, and "shallow" when they
  // are less dense.
  //
  // However, it's not easy to make that work this this
  // API, because that pruning doesn't immediately know on
  // seeing each term whether that term will be a seek point
  // or not.  It requires some non-causality in the API, ie
  // only on seeing some number of future terms will the
  // builder decide which past terms are seek points.
  // Somehow the API'd need to be able to return a "I don't
  // know" value, eg like a Future, which only later on is
  // flipped (frozen) to true or false.
  //
  // We could solve this with a 2-pass approach, where the
  // first pass would build an FSA (no outputs) solely to
  // determine which prefixes are the 'leaves' in the
  // pruning. The 2nd pass would then look at this prefix
  // trie to mark the seek points and build the FST mapping
  // to the true output.
  //
  // But, one downside to this approach is that it'd result
  // in uneven index term selection.  EG with prune1=10, the
  // resulting index terms could be as frequent as every 10
  // terms or as rare as every <maxArcCount> * 10 (eg 2560),
  // in the extremes.

public:
  VariableGapTermsIndexWriter(
      std::shared_ptr<SegmentWriteState> state,
      std::shared_ptr<IndexTermSelector> policy) ;

  std::shared_ptr<FieldWriter>
  addField(std::shared_ptr<FieldInfo> field,
           int64_t termsFilePointer)  override;

  /** NOTE: if your codec does not sort in unicode code
   *  point order, you must override this method, to simply
   *  return indexedTerm.length. */
protected:
  virtual int indexedTermPrefixLength(std::shared_ptr<BytesRef> priorTerm,
                                      std::shared_ptr<BytesRef> indexedTerm);

private:
  class FSTFieldWriter : public FieldWriter
  {
    GET_CLASS_NAME(FSTFieldWriter)
  private:
    std::shared_ptr<VariableGapTermsIndexWriter> outerInstance;

    const std::shared_ptr<Builder<int64_t>> fstBuilder;
    const std::shared_ptr<PositiveIntOutputs> fstOutputs;
    const int64_t startTermsFilePointer;

  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    std::shared_ptr<FST<int64_t>> fst;
    const int64_t indexStart;

  private:
    const std::shared_ptr<BytesRefBuilder> lastTerm =
        std::make_shared<BytesRefBuilder>();
    bool first = true;

  public:
    FSTFieldWriter(std::shared_ptr<VariableGapTermsIndexWriter> outerInstance,
                   std::shared_ptr<FieldInfo> fieldInfo,
                   int64_t termsFilePointer) ;

    bool checkIndexTerm(
        std::shared_ptr<BytesRef> text,
        std::shared_ptr<TermStats> stats)  override;

  private:
    const std::shared_ptr<IntsRefBuilder> scratchIntsRef =
        std::make_shared<IntsRefBuilder>();

  public:
    void add(std::shared_ptr<BytesRef> text, std::shared_ptr<TermStats> stats,
             int64_t termsFilePointer)  override;

    void finish(int64_t termsFilePointer)  override;

  protected:
    std::shared_ptr<FSTFieldWriter> shared_from_this()
    {
      return std::static_pointer_cast<FSTFieldWriter>(
          FieldWriter::shared_from_this());
    }
  };

public:
  virtual ~VariableGapTermsIndexWriter();

private:
  void writeTrailer(int64_t dirStart) ;

protected:
  std::shared_ptr<VariableGapTermsIndexWriter> shared_from_this()
  {
    return std::static_pointer_cast<VariableGapTermsIndexWriter>(
        TermsIndexWriterBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/blockterms/
