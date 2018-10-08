#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/analysis/ReuseStrategy.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/search/suggest/Lookup.h"
#include  "core/src/java/org/apache/lucene/search/suggest/LookupResult.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/fst/Util.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FSTPath;
}
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"

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
namespace org::apache::lucene::search::suggest::analyzing
{

// TODO
//   - test w/ syns
//   - add pruning of low-freq ngrams?

using Analyzer = org::apache::lucene::analysis::Analyzer;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;

// import java.io.PrintWriter;

/**
 * Builds an ngram model from the text sent to {@link
 * #build} and predicts based on the last grams-1 tokens in
 * the request sent to {@link #lookup}.  This tries to
 * handle the "long tail" of suggestions for when the
 * incoming query is a never before seen query string.
 *
 * <p>Likely this suggester would only be used as a
 * fallback, when the primary suggester fails to find
 * any suggestions.
 *
 * <p>Note that the weight for each suggestion is unused,
 * and the suggestions are the analyzed forms (so your
 * analysis process should normally be very "light").
 *
 * <p>This uses the stupid backoff language model to smooth
 * scores across ngram models; see
 * "Large language models in machine translation",
 * http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.76.1126
 * for details.
 *
 * <p> From {@link #lookup}, the key of each result is the
 * ngram token; the value is Long.MAX_VALUE * score (fixed
 * point, cast to long).  Divide by Long.MAX_VALUE to get
 * the score back, which ranges from 0.0 to 1.0.
 *
 * onlyMorePopular is unused.
 *
 * @lucene.experimental
 */
// redundant 'implements Accountable' to workaround javadocs bugs
class FreeTextSuggester : public Lookup, public Accountable
{
  GET_CLASS_NAME(FreeTextSuggester)

  /** Codec name used in the header for the saved model. */
public:
  static const std::wstring CODEC_NAME;

  /** Initial version of the the saved model file format. */
  static constexpr int VERSION_START = 0;

  /** Current version of the the saved model file format. */
  static constexpr int VERSION_CURRENT = VERSION_START;

  /** By default we use a bigram model. */
  static constexpr int DEFAULT_GRAMS = 2;

  // In general this could vary with gram, but the
  // original paper seems to use this constant:
  /** The constant used for backoff smoothing; during
   *  lookup, this means that if a given trigram did not
   *  occur, and we backoff to the bigram, the overall score
   *  will be 0.4 times what the bigram model would have
   *  assigned. */
  static constexpr double ALPHA = 0.4;

  /** Holds 1gram, 2gram, 3gram models as a single FST. */
private:
  std::shared_ptr<FST<int64_t>> fst;

  /**
   * Analyzer that will be used for analyzing suggestions at
   * index time.
   */
  const std::shared_ptr<Analyzer> indexAnalyzer;

  int64_t totTokens = 0;

  /**
   * Analyzer that will be used for analyzing suggestions at
   * query time.
   */
  const std::shared_ptr<Analyzer> queryAnalyzer;

  // 2 = bigram, 3 = trigram
  const int grams;

  const char separator;

  /** Number of entries the lookup was built with */
  int64_t count = 0;

  /** The default character used to join multiple tokens
   *  into a single ngram token.  The input tokens produced
   *  by the analyzer must not contain this character. */
public:
  static constexpr char DEFAULT_SEPARATOR = 0x1e;

  /** Instantiate, using the provided analyzer for both
   *  indexing and lookup, using bigram model by default. */
  FreeTextSuggester(std::shared_ptr<Analyzer> analyzer);

  /** Instantiate, using the provided indexing and lookup
   *  analyzers, using bigram model by default. */
  FreeTextSuggester(std::shared_ptr<Analyzer> indexAnalyzer,
                    std::shared_ptr<Analyzer> queryAnalyzer);

  /** Instantiate, using the provided indexing and lookup
   *  analyzers, with the specified model (2
   *  = bigram, 3 = trigram, etc.). */
  FreeTextSuggester(std::shared_ptr<Analyzer> indexAnalyzer,
                    std::shared_ptr<Analyzer> queryAnalyzer, int grams);

  /** Instantiate, using the provided indexing and lookup
   *  analyzers, and specified model (2 = bigram, 3 =
   *  trigram ,etc.).  The separator is passed to {@link
   *  ShingleFilter#setTokenSeparator} to join multiple
   *  tokens into a single ngram token; it must be an ascii
   *  (7-bit-clean) byte.  No input tokens should have this
   *  byte, otherwise {@code IllegalArgumentException} is
   *  thrown. */
  FreeTextSuggester(std::shared_ptr<Analyzer> indexAnalyzer,
                    std::shared_ptr<Analyzer> queryAnalyzer, int grams,
                    char separator);

  /** Returns byte size of the underlying FST. */
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

private:
  std::shared_ptr<Analyzer> addShingles(std::shared_ptr<Analyzer> other);

private:
  class AnalyzerWrapperAnonymousInnerClass : public AnalyzerWrapper
  {
    GET_CLASS_NAME(AnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<FreeTextSuggester> outerInstance;

    std::shared_ptr<Analyzer> other;

  public:
    AnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<FreeTextSuggester> outerInstance,
        std::shared_ptr<Analyzer::ReuseStrategy> getReuseStrategy,
        std::shared_ptr<Analyzer> other);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

    std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
        const std::wstring &fieldName,
        std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

  protected:
    std::shared_ptr<AnalyzerWrapperAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerWrapperAnonymousInnerClass>(
          org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
    }
  };

public:
  void
  build(std::shared_ptr<InputIterator> iterator)  override;

  /** Build the suggest index, using up to the specified
   *  amount of temporary RAM while building.  Note that
   *  the weights for the suggestions are ignored. */
  virtual void build(std::shared_ptr<InputIterator> iterator,
                     double ramBufferSizeMB) ;

  bool store(std::shared_ptr<DataOutput> output)  override;

  bool load(std::shared_ptr<DataInput> input)  override;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key, bool onlyMorePopular,
         int num) override;

  /** Lookup, without any context. */
  virtual std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key, int num);

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num) override;

  int64_t getCount() override;

private:
  int countGrams(std::shared_ptr<BytesRef> token);

  /** Retrieve suggestions. */
public:
  virtual std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         int num) ;

private:
  class TopNSearcherAnonymousInnerClass : public Util::TopNSearcher<int64_t>
  {
    GET_CLASS_NAME(TopNSearcherAnonymousInnerClass)
  private:
    std::shared_ptr<FreeTextSuggester> outerInstance;

    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> seen;
    std::shared_ptr<BytesRefBuilder> finalLastToken;

  public:
    TopNSearcherAnonymousInnerClass(
        std::shared_ptr<FreeTextSuggester> outerInstance,
        std::shared_ptr<FST<int64_t>> fst, int num,
        std::shared_ptr<UnknownType> size,
        std::shared_ptr<UnknownType> weightComparator,
        std::shared_ptr<Set<std::shared_ptr<BytesRef>>> seen,
        std::shared_ptr<BytesRefBuilder> finalLastToken);

    std::shared_ptr<BytesRefBuilder> scratchBytes;

  protected:
    void
    addIfCompetitive(std::shared_ptr<Util::FSTPath<int64_t>> path) override;

    bool acceptResult(std::shared_ptr<IntsRef> input,
                      std::optional<int64_t> &output) override;

  protected:
    std::shared_ptr<TopNSearcherAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TopNSearcherAnonymousInnerClass>(
          org.apache.lucene.util.fst.Util
              .TopNSearcher<long>::shared_from_this());
    }
  };

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<FreeTextSuggester> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<FreeTextSuggester> outerInstance);

    int compare(std::shared_ptr<Lookup::LookupResult> a,
                std::shared_ptr<Lookup::LookupResult> b);
  };

  /** weight -&gt; cost */
private:
  int64_t encodeWeight(int64_t ngramCount);

  /** cost -&gt; weight */
  // private long decodeWeight(Pair<Long,BytesRef> output) {
  int64_t decodeWeight(std::optional<int64_t> &output);

  // NOTE: copied from WFSTCompletionLookup & tweaked
  std::optional<int64_t>
  lookupPrefix(std::shared_ptr<FST<int64_t>> fst,
               std::shared_ptr<FST::BytesReader> bytesReader,
               std::shared_ptr<BytesRef> scratch,
               std::shared_ptr<FST::Arc<int64_t>> arc) ;

public:
  static const std::shared_ptr<Comparator<int64_t>> weightComparator;

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<int64_t>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  public:
    ComparatorAnonymousInnerClass2();

    int compare(std::optional<int64_t> &left,
                std::optional<int64_t> &right) override;
  };

  /**
   * Returns the weight associated with an input string,
   * or null if it does not exist.
   */
public:
  virtual std::any get(std::shared_ptr<std::wstring> key);

protected:
  std::shared_ptr<FreeTextSuggester> shared_from_this()
  {
    return std::static_pointer_cast<FreeTextSuggester>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/analyzing/
