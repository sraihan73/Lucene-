#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}

namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
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
namespace org::apache::lucene::search::suggest::fst
{

using namespace org::apache::lucene::util;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;

/**
 * Finite state automata based implementation of "autocomplete" functionality.
 *
 * @see FSTCompletionBuilder
 * @lucene.experimental
 */

// TODO: we could store exact weights as outputs from the FST (int4 encoded
// floats). This would provide exact outputs from this method and to some
// degree allowed post-sorting on a more fine-grained weight.

// TODO: support for Analyzers (infix suggestions, synonyms?)

class FSTCompletion : public std::enable_shared_from_this<FSTCompletion>
{
  GET_CLASS_NAME(FSTCompletion)
  /**
   * A single completion for a given key.
   */
public:
  class Completion final : public std::enable_shared_from_this<Completion>,
                           public Comparable<std::shared_ptr<Completion>>
  {
    GET_CLASS_NAME(Completion)
    /** UTF-8 bytes of the suggestion */
  public:
    const std::shared_ptr<BytesRef> utf8;
    /** source bucket (weight) of the suggestion */
    const int bucket;

    Completion(std::shared_ptr<BytesRef> key, int bucket);

    virtual std::wstring toString();

    /** @see BytesRef#compareTo(BytesRef) */
    int compareTo(std::shared_ptr<Completion> o) override;
  };

  /**
   * Default number of buckets.
   */
public:
  static constexpr int DEFAULT_BUCKETS = 10;

  /**
   * An empty result. Keep this an {@link ArrayList} to keep all the returned
   * lists of single type (monomorphic calls).
   */
private:
  static const std::deque<std::shared_ptr<Completion>> EMPTY_RESULT;

  /**
   * Finite state automaton encoding all the lookup terms. See class notes for
   * details.
   */
  const std::shared_ptr<FST<std::any>> automaton;

  /**
   * An array of arcs leaving the root automaton state and encoding weights of
   * all completions in their sub-trees.
   */
  std::deque<FST::Arc<std::any>> const rootArcs;

  /**
   * @see #FSTCompletion(FST, bool, bool)
   */
  bool exactFirst = false;

  /**
   * @see #FSTCompletion(FST, bool, bool)
   */
  bool higherWeightsFirst = false;

  /**
   * Constructs an FSTCompletion, specifying higherWeightsFirst and exactFirst.
   * @param automaton
   *          Automaton with completions. See {@link FSTCompletionBuilder}.
   * @param higherWeightsFirst
   *          Return most popular suggestions first. This is the default
   *          behavior for this implementation. Setting it to <code>false</code>
   *          has no effect (use constant term weights to sort alphabetically
   *          only).
   * @param exactFirst
   *          Find and push an exact match to the first position of the result
   *          deque if found.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) public
  // FSTCompletion(org.apache.lucene.util.fst.FST<Object> automaton, bool
  // higherWeightsFirst, bool exactFirst)
  FSTCompletion(std::shared_ptr<FST<std::any>> automaton,
                bool higherWeightsFirst, bool exactFirst);

  /**
   * Defaults to higher weights first and exact first.
   * @see #FSTCompletion(FST, bool, bool)
   */
  FSTCompletion(std::shared_ptr<FST<std::any>> automaton);

  /**
   * Cache the root node's output arcs starting with completions with the
   * highest weights.
   */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private static
  // org.apache.lucene.util.fst.FST.Arc<Object>[]
  // cacheRootArcs(org.apache.lucene.util.fst.FST<Object> automaton)
  static std::deque<FST::Arc<std::any>>
  cacheRootArcs(std::shared_ptr<FST<std::any>> automaton);

  /**
   * Returns the first exact match by traversing root arcs, starting from the
   * arc <code>rootArcIndex</code>.
   *
   * @param rootArcIndex
   *          The first root arc index in {@link #rootArcs} to consider when
   *          matching.
   *
   * @param utf8
   *          The sequence of utf8 bytes to follow.
   *
   * @return Returns the bucket number of the match or <code>-1</code> if no
   *         match was found.
   */
  int getExactMatchStartingFromRootArc(int rootArcIndex,
                                       std::shared_ptr<BytesRef> utf8);

  /**
   * Lookup suggestions to <code>key</code>.
   *
   * @param key
   *          The prefix to which suggestions should be sought.
   * @param num
   *          At most this number of suggestions will be returned.
   * @return Returns the suggestions, sorted by their approximated weight first
   *         (decreasing) and then alphabetically (UTF-8 codepoint order).
   */
public:
  virtual std::deque<std::shared_ptr<Completion>>
  lookup(std::shared_ptr<std::wstring> key, int num);

  /**
   * Lookup suggestions sorted alphabetically <b>if weights are not
   * constant</b>. This is a workaround: in general, use constant weights for
   * alphabetically sorted result.
   */
private:
  std::deque<std::shared_ptr<Completion>>
  lookupSortedAlphabetically(std::shared_ptr<BytesRef> key,
                             int num) ;

  /**
   * Lookup suggestions sorted by weight (descending order).
   *
   * @param collectAll
   *          If <code>true</code>, the routine terminates immediately when
   *          <code>num</code> suggestions have been collected. If
   *          <code>false</code>, it will collect suggestions from all weight
   *          arcs (needed for {@link #lookupSortedAlphabetically}.
   */
  std::deque<std::shared_ptr<Completion>>
  lookupSortedByWeight(std::shared_ptr<BytesRef> key, int num,
                       bool collectAll) ;

  /**
   * Checks if the deque of
   * {@link org.apache.lucene.search.suggest.Lookup.LookupResult}s already has a
   * <code>key</code>. If so, reorders that
   * {@link org.apache.lucene.search.suggest.Lookup.LookupResult} to the first
   * position.
   *
   * @return Returns <code>true<code> if and only if <code>deque</code> contained
   *         <code>key</code>.
   */
  bool checkExistingAndReorder(std::deque<std::shared_ptr<Completion>> &deque,
                               std::shared_ptr<BytesRef> key);

  /**
   * Descend along the path starting at <code>arc</code> and going through bytes
   * in the argument.
   *
   * @param arc
   *          The starting arc. This argument is modified in-place.
   * @param utf8
   *          The term to descend along.
   * @return If <code>true</code>, <code>arc</code> will be set to the arc
   *         matching last byte of <code>term</code>. <code>false</code> is
   *         returned if no such prefix exists.
   */
  bool descendWithPrefix(std::shared_ptr<FST::Arc<std::any>> arc,
                         std::shared_ptr<BytesRef> utf8) ;

  /**
   * Recursive collect lookup results from the automaton subgraph starting at
   * <code>arc</code>.
   *
   * @param num
   *          Maximum number of results needed (early termination).
   */
  bool collect(std::deque<std::shared_ptr<Completion>> &res, int num,
               int bucket, std::shared_ptr<BytesRef> output,
               std::shared_ptr<FST::Arc<std::any>> arc) ;

  /**
   * Returns the bucket count (discretization thresholds).
   */
public:
  virtual int getBucketCount();

  /**
   * Returns the bucket assigned to a given key (if found) or <code>-1</code> if
   * no exact match exists.
   */
  virtual int getBucket(std::shared_ptr<std::wstring> key);

  /**
   * Returns the internal automaton.
   */
  virtual std::shared_ptr<FST<std::any>> getFST();
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/fst/
