#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class PhrasePositions;
}

namespace org::apache::lucene::search
{
class PhraseQueue;
}
namespace org::apache::lucene::search
{
class PhraseQuery;
}
namespace org::apache::lucene::search
{
class PostingsAndFreq;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
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
namespace org::apache::lucene::search
{

using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * Find all slop-valid position-combinations (matches)
 * encountered while traversing/hopping the PhrasePositions.
 * <br> The sloppy frequency contribution of a match depends on the distance:
 * <br> - highest freq for distance=0 (exact match).
 * <br> - freq gets lower as distance gets higher.
 * <br>Example: for query "a b"~2, a document "x a b a y" can be matched twice:
 * once for "a b" (distance=0), and once for "b a" (distance=2).
 * <br>Possibly not all valid combinations are encountered, because for
 * efficiency we always propagate the least PhrasePosition. This allows to base
 * on PriorityQueue and move forward faster. As result, for example, document "a
 * b c b a" would score differently for queries "a b c"~4 and "c b a"~4,
 * although they really are equivalent. Similarly, for doc "a b c b a f g",
 * query "c b"~2 would get same score as "g f"~2, although "c b"~2 could be
 * matched twice. We may want to fix this in the future (currently not, for
 * performance reasons).
 */
class SloppyPhraseMatcher final : public PhraseMatcher
{
  GET_CLASS_NAME(SloppyPhraseMatcher)

private:
  std::deque<std::shared_ptr<PhrasePositions>> const phrasePositions;

  const int slop;
  const int numPostings;
  const std::shared_ptr<PhraseQueue> pq; // for advancing min position

  int end = 0; // current largest phrase position

  int leadPosition = 0;
  int leadOffset = 0;
  int currentEndPostings = 0;
  int advanceEndPostings = 0;

  bool hasRpts = false; // flag indicating that there are repetitions (as
                        // checked in first candidate doc)
  bool checkedRpts =
      false; // flag to only check for repetitions in first candidate doc
  bool hasMultiTermRpts = false;
  std::deque<std::deque<std::shared_ptr<PhrasePositions>>>
      rptGroups; // in each group are PPs that repeats each other (i.e. same
                 // term), sorted by (query) offset
  std::deque<std::shared_ptr<PhrasePositions>>
      rptStack; // temporary stack for switching colliding repeating pps

  bool positioned = false;
  int matchLength = 0;

public:
  SloppyPhraseMatcher(
      std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings,
      int slop, float matchCost);

private:
  static std::shared_ptr<DocIdSetIterator> approximation(
      std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings);

public:
  float maxFreq()  override;

  void reset()  override;

  float sloppyWeight(std::shared_ptr<Similarity::SimScorer> simScorer) override;

  bool nextMatch()  override;

  int startPosition() override;

  int endPosition() override;

  int startOffset()  override;

  int endOffset()  override;

  /** advance a PhrasePosition and update 'end', return false if exhausted */
private:
  bool advancePP(std::shared_ptr<PhrasePositions> pp) ;

  /** pp was just advanced. If that caused a repeater collision, resolve by
   * advancing the lesser of the two colliding pps. Note that there can only be
   * one collision, as by the initialization
   * there were no collisions before pp was advanced.  */
  bool advanceRpts(std::shared_ptr<PhrasePositions> pp) ;

  /** compare two pps, but only by position and offset */
  std::shared_ptr<PhrasePositions> lesser(std::shared_ptr<PhrasePositions> pp,
                                          std::shared_ptr<PhrasePositions> pp2);

  /** index of a pp2 colliding with pp, or -1 if none */
  int collide(std::shared_ptr<PhrasePositions> pp);

  /**
   * Initialize PhrasePositions in place.
   * A one time initialization for this scorer (on first doc matching all
   * terms): <ul> <li>Check if there are repetitions <li>If there are, find
   * groups of repetitions.
   * </ul>
   * Examples:
   * <ol>
   *  <li>no repetitions: <b>"ho my"~2</b>
   *  <li>repetitions: <b>"ho my my"~2</b>
   *  <li>repetitions: <b>"my ho my"~2</b>
   * </ol>
   * @return false if PPs are exhausted (and so current doc will not be a match)
   */
  bool initPhrasePositions() ;

  /** no repeats: simplest case, and most common. It is important to keep this
   * piece of the code simple and efficient */
  void initSimple() ;

  /** with repeats: not so simple. */
  bool initComplex() ;

  /** move all PPs to their first position */
  void placeFirstPositions() ;

  /** Fill the queue (all pps are already placed */
  void fillQueue();

  /** At initialization (each doc), each repetition group is sorted by (query)
   * offset. This provides the start condition: no collisions. <p>Case 1: no
   * multi-term repeats<br> It is sufficient to advance each pp in the group by
   * one less than its group index. So lesser pp is not advanced, 2nd one
   * advance once, 3rd one advanced twice, etc. <p>Case 2: multi-term
   * repeats<br>
   *
   * @return false if PPs are exhausted.
   */
  bool advanceRepeatGroups() ;

  /** initialize with checking for repeats. Heavy work, but done only for the
   * first candidate doc.<p> If there are repetitions, check if multi-term
   * postings (MTP) are involved.<p> Without MTP, once PPs are placed in the
   * first candidate doc, repeats (and groups) are visible.<br> With MTP, a more
   * complex check is needed, up-front, as there may be "hidden collisions".<br>
   * For example P1 has {A,B}, P1 has {B,C}, and the first doc is: "A C B". At
   * start, P1 would point to "A", p2 to "C", and it will not be identified that
   * P1 and P2 are repetitions of each other.<p> The more complex initialization
   * has two parts:<br> (1) identification of repetition groups.<br> (2)
   * advancing repeat groups at the start of the doc.<br> For (1), a possible
   * solution is to just create a single repetition group, made of all repeating
   * pps. But this would slow down the check for collisions, as all pps would
   * need to be checked. Instead, we compute "connected regions" on the
   * bipartite graph of postings and terms.
   */
  bool initFirstTime() ;

  /** sort each repetition group by (query) offset.
   * Done only once (at first doc) and allows to initialize faster for each doc.
   */
  void sortRptGroups(
      std::deque<std::deque<std::shared_ptr<PhrasePositions>>> &rgs);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<PhrasePositions>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SloppyPhraseMatcher> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<SloppyPhraseMatcher> outerInstance);

    int compare(std::shared_ptr<PhrasePositions> pp1,
                std::shared_ptr<PhrasePositions> pp2);
  };

  /** Detect repetition groups. Done once - for first doc */
private:
  std::deque<std::deque<std::shared_ptr<PhrasePositions>>>
  gatherRptGroups(std::shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>>
                      rptTerms) ;

  /** Actual position in doc of a PhrasePosition, relies on that position =
   * tpPos - offset) */
  int tpPos(std::shared_ptr<PhrasePositions> pp);

  /** find repeating terms and assign them ordinal values */
  std::shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>> repeatingTerms();

  /** find repeating pps, and for each, if has multi-terms, update
   * this.hasMultiTermRpts */
  std::deque<std::shared_ptr<PhrasePositions>>
  repeatingPPs(std::unordered_map<std::shared_ptr<Term>, int> &rptTerms);

  /** bit-sets - for each repeating pp, for each of its repeating terms, the
   * term ordinal values is set */
  std::deque<std::shared_ptr<FixedBitSet>>
  ppTermsBitSets(std::deque<std::shared_ptr<PhrasePositions>> &rpp,
                 std::unordered_map<std::shared_ptr<Term>, int> &tord);

  /** union (term group) bit-sets until they are disjoint (O(n^^2)), and each
   * group have different terms */
  void unionTermGroups(std::deque<std::shared_ptr<FixedBitSet>> &bb);

  /** map_obj each term to the single group that contains it */
  std::unordered_map<std::shared_ptr<Term>, int>
  termGroups(std::shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>> tord,
             std::deque<std::shared_ptr<FixedBitSet>> &bb) ;

protected:
  std::shared_ptr<SloppyPhraseMatcher> shared_from_this()
  {
    return std::static_pointer_cast<SloppyPhraseMatcher>(
        PhraseMatcher::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
