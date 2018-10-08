#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

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
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::analysis
{
class TokenStreamToAutomaton;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::search::suggest
{
class InputIterator;
}
namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::search::suggest
{
class Lookup;
}
namespace org::apache::lucene::search::suggest
{
class LookupResult;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::search::suggest::analyzing
{
class FSTUtil;
}
namespace org::apache::lucene::search::suggest::analyzing
{
template <typename T>
class Path;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

/**
 * Suggester that first analyzes the surface form, adds the
 * analyzed form to a weighted FST, and then does the same
 * thing at lookup time.  This means lookup is based on the
 * analyzed form while suggestions are still the surface
 * form(s).
 *
 * <p>
 * This can result in powerful suggester functionality.  For
 * example, if you use an analyzer removing stop words,
 * then the partial text "ghost chr..." could see the
 * suggestion "The Ghost of Christmas Past". Note that
 * position increments MUST NOT be preserved for this example
 * to work, so you should call the constructor with
 * <code>preservePositionIncrements</code> parameter set to
 * false
 *
 * <p>
 * If SynonymFilter is used to map_obj wifi and wireless network to
 * hotspot then the partial text "wirele..." could suggest
 * "wifi router".  Token normalization like stemmers, accent
 * removal, etc., would allow suggestions to ignore such
 * variations.
 *
 * <p>
 * When two matching suggestions have the same weight, they
 * are tie-broken by the analyzed form.  If their analyzed
 * form is the same then the order is undefined.
 *
 * <p>
 * There are some limitations:
 * <ul>
 *
 *   <li> A lookup from a query like "net" in English won't
 *        be any different than "net " (ie, user added a
 *        trailing space) because analyzers don't reflect
 *        when they've seen a token separator and when they
 *        haven't.
 *
 *   <li> If you're using {@code StopFilter}, and the user will
 *        type "fast apple", but so far all they've typed is
 *        "fast a", again because the analyzer doesn't convey whether
 *        it's seen a token separator after the "a",
 *        {@code StopFilter} will remove that "a" causing
 *        far more matches than you'd expect.
 *
 *   <li> Lookups with the empty string return no results
 *        instead of all results.
 * </ul>
 *
 * @lucene.experimental
 */
// redundant 'implements Accountable' to workaround javadocs bugs
class AnalyzingSuggester : public Lookup, public Accountable
{
  GET_CLASS_NAME(AnalyzingSuggester)

  /**
   * FST&lt;Weight,Surface&gt;:
   *  input is the analyzed form, with a null byte between terms
   *  weights are encoded as costs: (Integer.MAX_VALUE-weight)
   *  surface is the original, unanalyzed form.
   */
private:
  std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst =
      nullptr;

  /**
   * Analyzer that will be used for analyzing suggestions at
   * index time.
   */
  const std::shared_ptr<Analyzer> indexAnalyzer;

  /**
   * Analyzer that will be used for analyzing suggestions at
   * query time.
   */
  const std::shared_ptr<Analyzer> queryAnalyzer;

  /**
   * True if exact match suggestions should always be returned first.
   */
  const bool exactFirst;

  /**
   * True if separator between tokens should be preserved.
   */
  const bool preserveSep;

  /** Include this flag in the options parameter to {@link
   *  #AnalyzingSuggester(Directory,std::wstring,Analyzer,Analyzer,int,int,int,bool)}
   * to always return the exact match first, regardless of score.  This has no
   * performance impact but could result in low-quality suggestions. */
public:
  static constexpr int EXACT_FIRST = 1;

  /** Include this flag in the options parameter to {@link
   *  #AnalyzingSuggester(Directory,std::wstring,Analyzer,Analyzer,int,int,int,bool)}
   * to preserve token separators when matching. */
  static constexpr int PRESERVE_SEP = 2;

  /** Represents the separation between tokens, if
   *  PRESERVE_SEP was specified */
private:
  static constexpr int SEP_LABEL = L'\u001F';

  /** Marks end of the analyzed input and start of dedup
   *  byte. */
  static constexpr int END_BYTE = 0x0;

  /** Maximum number of dup surface forms (different surface
   *  forms for the same analyzed form). */
  const int maxSurfaceFormsPerAnalyzedForm;

  /** Maximum graph paths to index for a single analyzed
   *  surface form.  This only matters if your analyzer
   *  makes lots of alternate paths (e.g. contains
   *  SynonymFilter). */
  const int maxGraphExpansions;

  const std::shared_ptr<Directory> tempDir;
  const std::wstring tempFileNamePrefix;

  /** Highest number of analyzed paths we saw for any single
   *  input surface form.  For analyzers that never create
   *  graphs this will always be 1. */
  int maxAnalyzedPathsForOneInput = 0;

  bool hasPayloads = false;

  static constexpr int PAYLOAD_SEP = L'\u001f';

  /** Whether position holes should appear in the automaton. */
  bool preservePositionIncrements = false;

  /** Number of entries the lookup was built with */
  int64_t count = 0;

  /**
   * Calls {@link
   * #AnalyzingSuggester(Directory,std::wstring,Analyzer,Analyzer,int,int,int,bool)
   * AnalyzingSuggester(analyzer, analyzer, EXACT_FIRST |
   * PRESERVE_SEP, 256, -1, true)}
   */
public:
  AnalyzingSuggester(std::shared_ptr<Directory> tempDir,
                     const std::wstring &tempFileNamePrefix,
                     std::shared_ptr<Analyzer> analyzer);

  /**
   * Calls {@link
   * #AnalyzingSuggester(Directory,std::wstring,Analyzer,Analyzer,int,int,int,bool)
   * AnalyzingSuggester(indexAnalyzer, queryAnalyzer, EXACT_FIRST |
   * PRESERVE_SEP, 256, -1, true)}
   */
  AnalyzingSuggester(std::shared_ptr<Directory> tempDir,
                     const std::wstring &tempFileNamePrefix,
                     std::shared_ptr<Analyzer> indexAnalyzer,
                     std::shared_ptr<Analyzer> queryAnalyzer);

  /**
   * Creates a new suggester.
   *
   * @param indexAnalyzer Analyzer that will be used for
   *   analyzing suggestions while building the index.
   * @param queryAnalyzer Analyzer that will be used for
   *   analyzing query text during lookup
   * @param options see {@link #EXACT_FIRST}, {@link #PRESERVE_SEP}
   * @param maxSurfaceFormsPerAnalyzedForm Maximum number of
   *   surface forms to keep for a single analyzed form.
   *   When there are too many surface forms we discard the
   *   lowest weighted ones.
   * @param maxGraphExpansions Maximum number of graph paths
   *   to expand from the analyzed form.  Set this to -1 for
   *   no limit.
   * @param preservePositionIncrements Whether position holes
   *   should appear in the automata
   */
  AnalyzingSuggester(std::shared_ptr<Directory> tempDir,
                     const std::wstring &tempFileNamePrefix,
                     std::shared_ptr<Analyzer> indexAnalyzer,
                     std::shared_ptr<Analyzer> queryAnalyzer, int options,
                     int maxSurfaceFormsPerAnalyzedForm, int maxGraphExpansions,
                     bool preservePositionIncrements);

  /** Returns byte size of the underlying FST. */
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  // Replaces SEP with epsilon or remaps them if
  // we were asked to preserve them:
private:
  std::shared_ptr<Automaton> replaceSep(std::shared_ptr<Automaton> a);

  /** Used by subclass to change the lookup automaton, if
   *  necessary. */
protected:
  virtual std::shared_ptr<Automaton>
  convertAutomaton(std::shared_ptr<Automaton> a);

public:
  virtual std::shared_ptr<TokenStreamToAutomaton> getTokenStreamToAutomaton();

private:
  class AnalyzingComparator
      : public std::enable_shared_from_this<AnalyzingComparator>,
        public Comparator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(AnalyzingComparator)

  private:
    const bool hasPayloads;

  public:
    AnalyzingComparator(bool hasPayloads);

  private:
    const std::shared_ptr<ByteArrayDataInput> readerA =
        std::make_shared<ByteArrayDataInput>();
    const std::shared_ptr<ByteArrayDataInput> readerB =
        std::make_shared<ByteArrayDataInput>();
    const std::shared_ptr<BytesRef> scratchA = std::make_shared<BytesRef>();
    const std::shared_ptr<BytesRef> scratchB = std::make_shared<BytesRef>();

  public:
    int compare(std::shared_ptr<BytesRef> a,
                std::shared_ptr<BytesRef> b) override;
  };

public:
  void
  build(std::shared_ptr<InputIterator> iterator)  override;

  bool store(std::shared_ptr<DataOutput> output)  override;

  bool load(std::shared_ptr<DataInput> input)  override;

private:
  std::shared_ptr<Lookup::LookupResult>
  getLookupResult(std::optional<int64_t> &output1,
                  std::shared_ptr<BytesRef> output2,
                  std::shared_ptr<CharsRefBuilder> spare);

  bool sameSurfaceForm(std::shared_ptr<BytesRef> key,
                       std::shared_ptr<BytesRef> output2);

public:
  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num) override;

private:
  class TopNSearcherAnonymousInnerClass
      : public Util::TopNSearcher<Pair<int64_t, std::shared_ptr<BytesRef>>>
  {
    GET_CLASS_NAME(TopNSearcherAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingSuggester> outerInstance;

    std::shared_ptr<BytesRef> utf8Key;
    std::deque<std::shared_ptr<Lookup::LookupResult>> results;

  public:
    TopNSearcherAnonymousInnerClass(
        std::shared_ptr<AnalyzingSuggester> outerInstance,
        std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst,
        std::shared_ptr<UnknownType> size, int num,
        std::shared_ptr<UnknownType> weightComparator,
        std::shared_ptr<BytesRef> utf8Key,
        std::deque<std::shared_ptr<Lookup::LookupResult>> &results);

  private:
    const std::shared_ptr<Set<std::shared_ptr<BytesRef>>> seen;

  protected:
    bool
    acceptResult(std::shared_ptr<IntsRef> input,
                 std::shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>>
                     output) override;

  protected:
    std::shared_ptr<TopNSearcherAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TopNSearcherAnonymousInnerClass>(
          org.apache.lucene.util.fst.Util
              .TopNSearcher<org.apache.lucene.util.fst.PairOutputs.Pair<
                  long, org.apache.lucene.util.BytesRef>>::shared_from_this());
    }
  };

public:
  int64_t getCount() override;

  /** Returns all prefix paths to initialize the search. */
protected:
  virtual std::deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
  getFullPrefixPaths(
      std::deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
          &prefixPaths,
      std::shared_ptr<Automaton> lookupAutomaton,
      std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>>
          fst) ;

public:
  std::shared_ptr<Automaton>
  toAutomaton(std::shared_ptr<BytesRef> surfaceForm,
              std::shared_ptr<TokenStreamToAutomaton> ts2a) ;

  std::shared_ptr<Automaton>
  toLookupAutomaton(std::shared_ptr<std::wstring> key) ;

  /**
   * Returns the weight associated with an input string,
   * or null if it does not exist.
   */
  virtual std::any get(std::shared_ptr<std::wstring> key);

  /** cost -&gt; weight */
private:
  static int decodeWeight(int64_t encoded);

  /** weight -&gt; cost */
  static int encodeWeight(int64_t value);

public:
  static const std::shared_ptr<
      Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>>
      weightComparator;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int
    compare(std::shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> left,
            std::shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> right)
        override;
  };

protected:
  std::shared_ptr<AnalyzingSuggester> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzingSuggester>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing
