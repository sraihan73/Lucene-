using namespace std;

#include "SloppyPhraseMatcher.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

SloppyPhraseMatcher::SloppyPhraseMatcher(
    std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings,
    int slop, float matchCost)
    : PhraseMatcher(approximation(postings), matchCost),
      phrasePositions(
          std::deque<std::shared_ptr<PhrasePositions>>(postings.size())),
      slop(slop), numPostings(postings.size()),
      pq(make_shared<PhraseQueue>(postings.size()))
{
  for (int i = 0; i < postings.size(); ++i) {
    phrasePositions[i] = make_shared<PhrasePositions>(
        postings[i]->postings, postings[i]->position, i, postings[i]->terms);
  }
}

shared_ptr<DocIdSetIterator> SloppyPhraseMatcher::approximation(
    std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> &postings)
{
  deque<std::shared_ptr<DocIdSetIterator>> iterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  for (auto posting : postings) {
    iterators.push_back(posting->postings);
  }
  return ConjunctionDISI::intersectIterators(iterators);
}

float SloppyPhraseMatcher::maxFreq() 
{
  // every term position in each postings deque can be at the head of at most
  // one matching phrase, so the maximum possible phrase freq is the sum of
  // the freqs of the postings lists.
  float maxFreq = 0;
  for (auto phrasePosition : phrasePositions) {
    maxFreq += phrasePosition->postings->freq();
  }
  return maxFreq;
}

void SloppyPhraseMatcher::reset() 
{
  this->positioned = initPhrasePositions();
  this->matchLength = numeric_limits<int>::max();
  this->leadPosition = numeric_limits<int>::max();
}

float SloppyPhraseMatcher::sloppyWeight(
    shared_ptr<Similarity::SimScorer> simScorer)
{
  return simScorer->computeSlopFactor(matchLength);
}

bool SloppyPhraseMatcher::nextMatch() 
{
  if (!positioned) {
    return false;
  }
  shared_ptr<PhrasePositions> pp = pq->pop();
  assert(pp != nullptr); // if the pq is empty, then positioned == false
  leadPosition = pp->position + pp->offset;
  leadOffset = pp->postings->startOffset();
  currentEndPostings = advanceEndPostings;
  matchLength = end - pp->position;
  int next = pq->top()->position;
  while (advancePP(pp)) {
    if (hasRpts && !advanceRpts(pp)) {
      break; // pps exhausted
    }
    if (pp->position > next) { // done minimizing current match-length
      pq->push_back(pp);
      if (matchLength <= slop) {
        return true;
      }
      pp = pq->pop();
      next = pq->top()->position;
      matchLength = end - pp->position;
    } else {
      int matchLength2 = end - pp->position;
      if (matchLength2 < matchLength) {
        matchLength = matchLength2;
      }
    }
    leadPosition = pp->position + pp->offset;
    leadOffset = pp->postings->startOffset();
    currentEndPostings = advanceEndPostings;
  }
  positioned = false;
  return matchLength <= slop;
}

int SloppyPhraseMatcher::startPosition()
{
  // when a match is detected, the top postings is advanced until it has moved
  // beyond its successor, to ensure that the match is of minimal width.  This
  // means that we need to record the lead position before it is advanced.
  // However, the priority queue doesn't guarantee that the top postings is in
  // fact the earliest in the deque, so we need to cycle through all terms to
  // check. this is slow, but Matches is slow anyway...
  for (auto pp : phrasePositions) {
    leadPosition = min(leadPosition, pp->position + pp->offset);
  }
  return leadPosition;
}

int SloppyPhraseMatcher::endPosition()
{
  return phrasePositions[currentEndPostings]->position +
         phrasePositions[currentEndPostings]->offset;
}

int SloppyPhraseMatcher::startOffset() 
{
  // when a match is detected, the top postings is advanced until it has moved
  // beyond its successor, to ensure that the match is of minimal width.  This
  // means that we need to record the lead offset before it is advanced.
  // However, the priority queue doesn't guarantee that the top postings is in
  // fact the earliest in the deque, so we need to cycle through all terms to
  // check this is slow, but Matches is slow anyway...
  for (auto pp : phrasePositions) {
    leadOffset = min(leadOffset, pp->postings->startOffset());
  }
  return leadOffset;
}

int SloppyPhraseMatcher::endOffset() 
{
  return phrasePositions[currentEndPostings]->postings.endOffset();
}

bool SloppyPhraseMatcher::advancePP(shared_ptr<PhrasePositions> pp) throw(
    IOException)
{
  if (!pp->nextPosition()) {
    return false;
  }
  if (pp->position > end) {
    end = pp->position;
    advanceEndPostings = pp->ord;
  }
  if (pp->position == end) {
    if (pp->ord > advanceEndPostings) {
      advanceEndPostings = pp->ord;
    }
  }
  return true;
}

bool SloppyPhraseMatcher::advanceRpts(shared_ptr<PhrasePositions> pp) throw(
    IOException)
{
  if (pp->rptGroup < 0) {
    return true; // not a repeater
  }
  std::deque<std::shared_ptr<PhrasePositions>> rg = rptGroups[pp->rptGroup];
  shared_ptr<FixedBitSet> bits = make_shared<FixedBitSet>(
      rg.size()); // for re-queuing after collisions are resolved
  int k0 = pp->rptInd;
  int k;
  while ((k = collide(pp)) >= 0) {
    pp = lesser(
        pp, rg[k]); // always advance the lesser of the (only) two colliding pps
    if (!advancePP(pp)) {
      return false; // exhausted
    }
    if (k != k0) { // careful: mark only those currently in the queue
      bits = FixedBitSet::ensureCapacity(bits, k);
      bits->set(k); // mark that pp2 need to be re-queued
    }
  }
  // collisions resolved, now re-queue
  // empty (partially) the queue until seeing all pps advanced for resolving
  // collisions
  int n = 0;
  // TODO would be good if we can avoid calling cardinality() in each iteration!
  int numBits = bits->length(); // larges bit we set
  while (bits->cardinality() > 0) {
    shared_ptr<PhrasePositions> pp2 = pq->pop();
    rptStack[n++] = pp2;
    if (pp2->rptGroup >= 0 && pp2->rptInd < numBits && bits->get(pp2->rptInd)) {
      bits->clear(pp2->rptInd);
    }
  }
  // add back to queue
  for (int i = n - 1; i >= 0; i--) {
    pq->push_back(rptStack[i]);
  }
  return true;
}

shared_ptr<PhrasePositions>
SloppyPhraseMatcher::lesser(shared_ptr<PhrasePositions> pp,
                            shared_ptr<PhrasePositions> pp2)
{
  if (pp->position < pp2->position ||
      (pp->position == pp2->position && pp->offset < pp2->offset)) {
    return pp;
  }
  return pp2;
}

int SloppyPhraseMatcher::collide(shared_ptr<PhrasePositions> pp)
{
  int tpPos = this->tpPos(pp);
  std::deque<std::shared_ptr<PhrasePositions>> rg = rptGroups[pp->rptGroup];
  for (int i = 0; i < rg.size(); i++) {
    shared_ptr<PhrasePositions> pp2 = rg[i];
    if (pp2 != pp && this->tpPos(pp2) == tpPos) {
      return pp2->rptInd;
    }
  }
  return -1;
}

bool SloppyPhraseMatcher::initPhrasePositions() 
{
  end = numeric_limits<int>::min();
  if (!checkedRpts) {
    return initFirstTime();
  }
  if (!hasRpts) {
    initSimple();
    return true; // PPs available
  }
  return initComplex();
}

void SloppyPhraseMatcher::initSimple() 
{
  // System.err.println("initSimple: doc: "+min.doc);
  pq->clear();
  // position pps and build queue from deque
  for (auto pp : phrasePositions) {
    pp->firstPosition();
    if (pp->position > end) {
      end = pp->position;
      advanceEndPostings = pp->ord;
    }
    if (pp->position == end) {
      if (pp->ord > advanceEndPostings) {
        advanceEndPostings = pp->ord;
      }
    }
    pq->push_back(pp);
  }
}

bool SloppyPhraseMatcher::initComplex() 
{
  // System.err.println("initComplex: doc: "+min.doc);
  placeFirstPositions();
  if (!advanceRepeatGroups()) {
    return false; // PPs exhausted
  }
  fillQueue();
  return true; // PPs available
}

void SloppyPhraseMatcher::placeFirstPositions() 
{
  for (auto pp : phrasePositions) {
    pp->firstPosition();
  }
}

void SloppyPhraseMatcher::fillQueue()
{
  pq->clear();
  for (auto pp :
       phrasePositions) { // iterate cyclic deque: done once handled max
    if (pp->position > end) {
      end = pp->position;
      advanceEndPostings = pp->ord;
    }
    if (pp->position == end) {
      if (pp->ord > advanceEndPostings) {
        advanceEndPostings = pp->ord;
      }
    }
    pq->push_back(pp);
  }
}

bool SloppyPhraseMatcher::advanceRepeatGroups() 
{
  for (auto rg : rptGroups) {
    if (hasMultiTermRpts) {
      // more involved, some may not collide
      int incr;
      for (int i = 0; i < rg.size(); i += incr) {
        incr = 1;
        shared_ptr<PhrasePositions> pp = rg[i];
        int k;
        while ((k = collide(pp)) >= 0) {
          shared_ptr<PhrasePositions> pp2 = lesser(pp, rg[k]);
          if (!advancePP(pp2)) { // at initialization always advance pp with
                                 // higher offset
            return false;        // exhausted
          }
          if (pp2->rptInd < i) { // should not happen?
            incr = 0;
            break;
          }
        }
      }
    } else {
      // simpler, we know exactly how much to advance
      for (int j = 1; j < rg.size(); j++) {
        for (int k = 0; k < j; k++) {
          if (!rg[j]->nextPosition()) {
            return false; // PPs exhausted
          }
        }
      }
    }
  }
  return true; // PPs available
}

bool SloppyPhraseMatcher::initFirstTime() 
{
  // System.err.println("initFirstTime: doc: "+min.doc);
  checkedRpts = true;
  placeFirstPositions();

  shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>> rptTerms =
      repeatingTerms();
  hasRpts = !rptTerms->isEmpty();

  if (hasRpts) {
    rptStack = std::deque<std::shared_ptr<PhrasePositions>>(
        numPostings); // needed with repetitions
    deque<deque<std::shared_ptr<PhrasePositions>>> rgs =
        gatherRptGroups(rptTerms);
    sortRptGroups(rgs);
    if (!advanceRepeatGroups()) {
      return false; // PPs exhausted
    }
  }

  fillQueue();
  return true; // PPs available
}

void SloppyPhraseMatcher::sortRptGroups(
    deque<deque<std::shared_ptr<PhrasePositions>>> &rgs)
{
  rptGroups =
      std::deque<std::deque<std::shared_ptr<PhrasePositions>>>(rgs.size());
  shared_ptr<Comparator<std::shared_ptr<PhrasePositions>>> cmprtr =
      make_shared<ComparatorAnonymousInnerClass>(shared_from_this());
  for (int i = 0; i < rptGroups.size(); i++) {
    std::deque<std::shared_ptr<PhrasePositions>> rg =
        rgs[i].toArray(std::deque<std::shared_ptr<PhrasePositions>>(0));
    Arrays::sort(rg, cmprtr);
    rptGroups[i] = rg;
    for (int j = 0; j < rg.size(); j++) {
      rg[j]->rptInd = j; // we use this index for efficient re-queuing
    }
  }
}

SloppyPhraseMatcher::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(shared_ptr<SloppyPhraseMatcher> outerInstance)
{
  this->outerInstance = outerInstance;
}

int SloppyPhraseMatcher::ComparatorAnonymousInnerClass::compare(
    shared_ptr<PhrasePositions> pp1, shared_ptr<PhrasePositions> pp2)
{
  return pp1->offset - pp2->offset;
}

deque<deque<std::shared_ptr<PhrasePositions>>>
SloppyPhraseMatcher::gatherRptGroups(
    shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>>
        rptTerms) 
{
  std::deque<std::shared_ptr<PhrasePositions>> rpp = repeatingPPs(rptTerms);
  deque<deque<std::shared_ptr<PhrasePositions>>> res =
      deque<deque<std::shared_ptr<PhrasePositions>>>();
  if (!hasMultiTermRpts) {
    // simpler - no multi-terms - can base on positions in first doc
    for (int i = 0; i < rpp.size(); i++) {
      shared_ptr<PhrasePositions> pp = rpp[i];
      if (pp->rptGroup >= 0) {
        continue; // already marked as a repetition
      }
      int tpPos = this->tpPos(pp);
      for (int j = i + 1; j < rpp.size(); j++) {
        shared_ptr<PhrasePositions> pp2 = rpp[j];
        if (pp2->rptGroup >= 0 || pp2->offset == pp->offset ||
            this->tpPos(pp2) != tpPos) { // not a repetition
          continue;
        }
        // a repetition
        int g = pp->rptGroup;
        if (g < 0) {
          g = res.size();
          pp->rptGroup = g;
          deque<std::shared_ptr<PhrasePositions>> rl =
              deque<std::shared_ptr<PhrasePositions>>(2);
          rl.push_back(pp);
          res.push_back(rl);
        }
        pp2->rptGroup = g;
        res[g].push_back(pp2);
      }
    }
  } else {
    // more involved - has multi-terms
    deque<unordered_set<std::shared_ptr<PhrasePositions>>> tmp =
        deque<unordered_set<std::shared_ptr<PhrasePositions>>>();
    deque<std::shared_ptr<FixedBitSet>> bb = ppTermsBitSets(rpp, rptTerms);
    unionTermGroups(bb);
    unordered_map<std::shared_ptr<Term>, int> tg = termGroups(rptTerms, bb);
    unordered_set<int> distinctGroupIDs = unordered_set<int>(tg.values());
    for (int i = 0; i < distinctGroupIDs.size(); i++) {
      tmp.push_back(unordered_set<std::shared_ptr<PhrasePositions>>());
    }
    for (auto pp : rpp) {
      for (auto t : pp->terms) {
        if (rptTerms->containsKey(t)) {
          int g = tg[t];
          tmp[g].insert(pp);
          assert(pp->rptGroup == -1 || pp->rptGroup == g);
          pp->rptGroup = g;
        }
      }
    }
    for (auto hs : tmp) {
      res.push_back(deque<>(hs));
    }
  }
  return res;
}

int SloppyPhraseMatcher::tpPos(shared_ptr<PhrasePositions> pp)
{
  return pp->position + pp->offset;
}

shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>>
SloppyPhraseMatcher::repeatingTerms()
{
  shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>> tord =
      make_shared<LinkedHashMap<std::shared_ptr<Term>, int>>();
  unordered_map<std::shared_ptr<Term>, int> tcnt =
      unordered_map<std::shared_ptr<Term>, int>();
  for (auto pp : phrasePositions) {
    for (auto t : pp->terms) {
      optional<int> cnt0 = tcnt[t];
      optional<int> cnt =
          !cnt0 ? optional<int>(1) : optional<int>(1 + cnt0.value());
      tcnt.emplace(t, cnt);
      if (cnt == 2) {
        tord->put(t, tord->size());
      }
    }
  }
  return tord;
}

std::deque<std::shared_ptr<PhrasePositions>> SloppyPhraseMatcher::repeatingPPs(
    unordered_map<std::shared_ptr<Term>, int> &rptTerms)
{
  deque<std::shared_ptr<PhrasePositions>> rp =
      deque<std::shared_ptr<PhrasePositions>>();
  for (auto pp : phrasePositions) {
    for (auto t : pp->terms) {
      if (rptTerms.find(t) != rptTerms.end()) {
        rp.push_back(pp);
        hasMultiTermRpts |= (pp->terms.size() > 1);
        break;
      }
    }
  }
  return rp.toArray(std::deque<std::shared_ptr<PhrasePositions>>(0));
}

deque<std::shared_ptr<FixedBitSet>> SloppyPhraseMatcher::ppTermsBitSets(
    std::deque<std::shared_ptr<PhrasePositions>> &rpp,
    unordered_map<std::shared_ptr<Term>, int> &tord)
{
  deque<std::shared_ptr<FixedBitSet>> bb =
      deque<std::shared_ptr<FixedBitSet>>(rpp.size());
  for (auto pp : rpp) {
    shared_ptr<FixedBitSet> b = make_shared<FixedBitSet>(tord.size());
    optional<int> ord;
    for (auto t : pp->terms) {
      if ((ord = tord[t])) {
        b->set(ord);
      }
    }
    bb.push_back(b);
  }
  return bb;
}

void SloppyPhraseMatcher::unionTermGroups(
    deque<std::shared_ptr<FixedBitSet>> &bb)
{
  int incr;
  for (int i = 0; i < bb.size() - 1; i += incr) {
    incr = 1;
    int j = i + 1;
    while (j < bb.size()) {
      if (bb[i]->intersects(bb[j])) {
        bb[i]->or (bb[j]);
        bb.erase(bb.begin() + j);
        incr = 0;
      } else {
        ++j;
      }
    }
  }
}

unordered_map<std::shared_ptr<Term>, int> SloppyPhraseMatcher::termGroups(
    shared_ptr<LinkedHashMap<std::shared_ptr<Term>, int>> tord,
    deque<std::shared_ptr<FixedBitSet>> &bb) 
{
  unordered_map<std::shared_ptr<Term>, int> tg =
      unordered_map<std::shared_ptr<Term>, int>();
  std::deque<std::shared_ptr<Term>> t =
      tord->keySet().toArray(std::deque<std::shared_ptr<Term>>(0));
  for (int i = 0; i < bb.size(); i++) { // i is the group no.
    shared_ptr<FixedBitSet> bits = bb[i];
    for (int ord = bits->nextSetBit(0); ord != DocIdSetIterator::NO_MORE_DOCS;
         ord = ord + 1 >= bits->length() ? DocIdSetIterator::NO_MORE_DOCS
                                         : bits->nextSetBit(ord + 1)) {
      tg.emplace(t[ord], i);
    }
  }
  return tg;
}
} // namespace org::apache::lucene::search