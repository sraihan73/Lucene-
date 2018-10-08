#pragma once
#include "stringhelper.h"
#include <memory>
#include <queue>

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
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/Entry.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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

using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.NRTSuggester.encode;

/**
 * Builder for {@link NRTSuggester}
 *
 */
class NRTSuggesterBuilder final
    : public std::enable_shared_from_this<NRTSuggesterBuilder>
{
  GET_CLASS_NAME(NRTSuggesterBuilder)

  /**
   * Label used to separate surface form and docID
   * in the output
   */
public:
  static const int PAYLOAD_SEP = ConcatenateGraphFilter::SEP_LABEL;

  /**
   * Marks end of the analyzed input and start of dedup
   * byte.
   */
  static constexpr int END_BYTE = 0x0;

private:
  const std::shared_ptr<PairOutputs<int64_t, std::shared_ptr<BytesRef>>>
      outputs;
  const std::shared_ptr<
      Builder<PairOutputs::Pair<int64_t, std::shared_ptr<BytesRef>>>>
      builder;
  const std::shared_ptr<IntsRefBuilder> scratchInts =
      std::make_shared<IntsRefBuilder>();
  const std::shared_ptr<BytesRefBuilder> analyzed =
      std::make_shared<BytesRefBuilder>();
  const std::shared_ptr<PriorityQueue<std::shared_ptr<Entry>>> entries;
  const int payloadSep;
  const int endByte;

  int maxAnalyzedPathsPerOutput = 0;

  /**
   * Create a builder for {@link NRTSuggester}
   */
public:
  NRTSuggesterBuilder();

  /**
   * Initializes an FST input term to add entries against
   */
  void startTerm(std::shared_ptr<BytesRef> analyzed);

  /**
   * Adds an entry for the latest input term, should be called after
   * {@link #startTerm(org.apache.lucene.util.BytesRef)} on the desired input
   */
  void addEntry(int docID, std::shared_ptr<BytesRef> surfaceForm,
                int64_t weight) ;

  /**
   * Writes all the entries for the FST input term
   */
  void finishTerm() ;

  /**
   * Builds and stores a FST that can be loaded with
   * {@link NRTSuggester#load(org.apache.lucene.store.IndexInput)}
   */
  bool store(std::shared_ptr<DataOutput> output) ;

  /**
   * Num arcs for nth dedup byte:
   * if n <= 5: 1 + (2 * n)
   * else: (1 + (2 * n)) * n
   * <p>
   * TODO: is there a better way to make the fst built to be
   * more TopNSearcher friendly?
   */
private:
  static int maxNumArcsForDedupByte(int currentNumDedupBytes);

private:
  class Entry final : public std::enable_shared_from_this<Entry>,
                      public Comparable<std::shared_ptr<Entry>>
  {
    GET_CLASS_NAME(Entry)
  public:
    const std::shared_ptr<BytesRef> payload;
    const int64_t weight;

    Entry(std::shared_ptr<BytesRef> payload, int64_t weight);

    int compareTo(std::shared_ptr<Entry> o) override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
