#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class PairOutputs;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionScorer.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/TopSuggestDocsCollector.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/ScoringPathComparator.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include  "core/src/java/org/apache/lucene/util/fst/Util.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FSTPath;
}
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

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
namespace org::apache::lucene::search::suggest::document
{

using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;
using Util = org::apache::lucene::util::fst::Util;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.NRTSuggester.PayLoadProcessor.parseSurfaceForm;

/**
 * <p>
 * NRTSuggester executes Top N search on a weighted FST specified by a {@link
 * CompletionScorer} <p> See {@link #lookup(CompletionScorer, Bits,
 * TopSuggestDocsCollector)} for more implementation details. <p> FST Format:
 * <ul>
 *   <li>Input: analyzed forms of input terms</li>
 *   <li>Output: Pair&lt;Long, BytesRef&gt; containing weight, surface form and
 * docID</li>
 * </ul>
 * <p>
 * NOTE:
 * <ul>
 *   <li>having too many deletions or using a very restrictive filter can make
 * the search inadmissible due to over-pruning of potential paths. See {@link
 * CompletionScorer#accept(int, Bits)}</li> <li>when matched documents are
 * arbitrarily filtered ({@link CompletionScorer#filtered} set to
 * <code>true</code>, it is assumed that the filter will roughly filter out half
 * the number of documents that match the provided automaton</li> <li>lookup
 * performance will degrade as more accepted completions lead to filtered out
 * documents</li>
 * </ul>
 *
 * @lucene.experimental
 */
class NRTSuggester final : public std::enable_shared_from_this<NRTSuggester>,
                           public Accountable
{
  GET_CLASS_NAME(NRTSuggester)

  /**
   * FST<Weight,Surface>:
   * input is the analyzed form, with a null byte between terms
   * and a {@link NRTSuggesterBuilder#END_BYTE} to denote the
   * end of the input
   * weight is a long
   * surface is the original, unanalyzed form followed by the docID
   */
private:
  const std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst;

  /**
   * Highest number of analyzed paths we saw for any single
   * input surface form. This can be > 1, when index analyzer
   * creates graphs or if multiple surface form(s) yields the
   * same analyzed form
   */
  const int maxAnalyzedPathsPerOutput;

  /**
   * Separator used between surface form and its docID in the FST output
   */
  const int payloadSep;

  /**
   * Maximum queue depth for TopNSearcher
   *
   * NOTE: value should be <= Integer.MAX_VALUE
   */
  static constexpr int64_t MAX_TOP_N_QUEUE_SIZE = 5000;

  NRTSuggester(
      std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst,
      int maxAnalyzedPathsPerOutput, int payloadSep);

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  /**
   * Collects at most {@link TopSuggestDocsCollector#getCountToCollect()}
   * completions that match the provided {@link CompletionScorer}. <p> The
   * {@link CompletionScorer#automaton} is intersected with the {@link #fst}.
   * {@link CompletionScorer#weight} is used to compute boosts and/or extract
   * context for each matched partial paths. A top N search is executed on
   * {@link #fst} seeded with the matched partial paths. Upon reaching a
   * completed path, {@link CompletionScorer#accept(int, Bits)} and {@link
   * CompletionScorer#score(float, float)} is used on the document id, index
   * weight and query boost to filter and score the entry, before being
   * collected via
   * {@link TopSuggestDocsCollector#collect(int, std::wstring, std::wstring,
   * float)}
   */
  void
  lookup(std::shared_ptr<CompletionScorer> scorer,
         std::shared_ptr<Bits> acceptDocs,
         std::shared_ptr<TopSuggestDocsCollector> collector) ;

private:
  class TopNSearcherAnonymousInnerClass
      : public Util::TopNSearcher<Pair<int64_t, std::shared_ptr<BytesRef>>>
  {
    GET_CLASS_NAME(TopNSearcherAnonymousInnerClass)
  private:
    std::shared_ptr<NRTSuggester> outerInstance;

    std::shared_ptr<
        org::apache::lucene::search::suggest::document::CompletionScorer>
        scorer;
    std::shared_ptr<Bits> acceptDocs;
    std::shared_ptr<
        org::apache::lucene::search::suggest::document::TopSuggestDocsCollector>
        collector;
    std::shared_ptr<CharsRefBuilder> spare;

  public:
    TopNSearcherAnonymousInnerClass(
        std::shared_ptr<NRTSuggester> outerInstance,
        std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst,
        std::shared_ptr<Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>>
            comparator,
        std::shared_ptr<ScoringPathComparator> new,
        std::shared_ptr<
            org::apache::lucene::search::suggest::document::CompletionScorer>
            scorer,
        std::shared_ptr<Bits> acceptDocs,
        std::shared_ptr<org::apache::lucene::search::suggest::document::
                            TopSuggestDocsCollector>
            collector,
        std::shared_ptr<CharsRefBuilder> spare) new;

  private:
    const std::shared_ptr<ByteArrayDataInput> scratchInput;

  protected:
    bool
    acceptPartialPath(std::shared_ptr<
                      Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>>
                          path) override;

    bool acceptResult(std::shared_ptr<
                      Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>>
                          path) override;

  protected:
    std::shared_ptr<TopNSearcherAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TopNSearcherAnonymousInnerClass>(
          org.apache.lucene.util.fst.Util
              .TopNSearcher<org.apache.lucene.util.fst.PairOutputs.Pair<
                  long, org.apache.lucene.util.BytesRef>>::shared_from_this());
    }
  };

  /**
   * Compares partial completion paths using {@link
   * CompletionScorer#score(float, float)}, breaks ties comparing path inputs
   */
private:
  class ScoringPathComparator
      : public std::enable_shared_from_this<ScoringPathComparator>,
        public Comparator<
            Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>>
  {
    GET_CLASS_NAME(ScoringPathComparator)
  private:
    const std::shared_ptr<CompletionScorer> scorer;

  public:
    ScoringPathComparator(std::shared_ptr<CompletionScorer> scorer);

    int compare(std::shared_ptr<
                    Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>>
                    first,
                std::shared_ptr<
                    Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>>
                    second) override;
  };

private:
  static std::shared_ptr<Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>>
  getComparator();

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> o1,
                std::shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> o2);
  };

  /**
   * Simple heuristics to try to avoid over-pruning potential suggestions by the
   * TopNSearcher. Since suggestion entries can be rejected if they belong
   * to a deleted document, the length of the TopNSearcher queue has to
   * be increased by some factor, to account for the filtered out suggestions.
   * This heuristic will try to make the searcher admissible, but the search
   * can still lead to over-pruning
   * <p>
   * If a <code>filter</code> is applied, the queue size is increased by
   * half the number of live documents.
   * <p>
   * The maximum queue size is {@link #MAX_TOP_N_QUEUE_SIZE}
   */
private:
  int getMaxTopNSearcherQueueSize(int topN, int numDocs, double liveDocsRatio,
                                  bool filterEnabled);

  static double calculateLiveDocRatio(int numDocs, int maxDocs);

  /**
   * Loads a {@link NRTSuggester} from {@link
   * org.apache.lucene.store.IndexInput}
   */
public:
  static std::shared_ptr<NRTSuggester>
  load(std::shared_ptr<IndexInput> input) ;

  static int64_t encode(int64_t input);

  static int64_t decode(int64_t output);

  /**
   * Helper to encode/decode payload (surface + PAYLOAD_SEP + docID) output
   */
public:
  class PayLoadProcessor final
      : public std::enable_shared_from_this<PayLoadProcessor>
  {
    GET_CLASS_NAME(PayLoadProcessor)
  private:
    static constexpr int MAX_DOC_ID_LEN_WITH_SEP =
        6; // vint takes at most 5 bytes

  public:
    static int parseSurfaceForm(std::shared_ptr<BytesRef> output,
                                int payloadSep,
                                std::shared_ptr<CharsRefBuilder> spare);

    static std::shared_ptr<BytesRef> make(std::shared_ptr<BytesRef> surface,
                                          int docID,
                                          int payloadSep) ;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
