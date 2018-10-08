using namespace std;

#include "TermAutomatonScorer.h"

namespace org::apache::lucene::search
{
using EnumAndScorer =
    org::apache::lucene::search::TermAutomatonQuery::EnumAndScorer;
using TermAutomatonWeight =
    org::apache::lucene::search::TermAutomatonQuery::TermAutomatonWeight;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;

TermAutomatonScorer::TermAutomatonScorer(
    shared_ptr<TermAutomatonWeight> weight,
    std::deque<std::shared_ptr<EnumAndScorer>> &subs, int anyTermID,
    unordered_map<int, std::shared_ptr<BytesRef>> &idToTerm,
    shared_ptr<Similarity::SimScorer> docScorer) 
    : Scorer(weight), subs(subs),
      subsOnDoc(std::deque<std::shared_ptr<EnumAndScorer>>(subs.size())),
      docIDQueue(make_shared<DocIDQueue>(subs.size())),
      posQueue(make_shared<PositionQueue>(subs.size())),
      runAutomaton(
          make_shared<TermRunAutomaton>(weight->automaton, subs.size())),
      idToTerm(idToTerm), anyTermID(anyTermID), docScorer(docScorer), cost(cost)
{
  // System.out.println("  automaton:\n" + weight.automaton.toDot());
  this->positions = std::deque<std::shared_ptr<PosState>>(4);
  for (int i = 0; i < this->positions.size(); i++) {
    this->positions[i] = make_shared<PosState>();
  }
  int64_t cost = 0;

  // Init docIDQueue:
  for (auto sub : subs) {
    if (sub != nullptr) {
      cost += sub->posEnum->cost();
      subsOnDoc[numSubsOnDoc++] = sub;
    }
  }
}

TermAutomatonScorer::DocIDQueue::DocIDQueue(int maxSize)
    : org::apache::lucene::util::PriorityQueue<
          org::apache::lucene::search::TermAutomatonQuery::EnumAndScorer>(
          maxSize, false)
{
}

bool TermAutomatonScorer::DocIDQueue::lessThan(shared_ptr<EnumAndScorer> a,
                                               shared_ptr<EnumAndScorer> b)
{
  return a->posEnum->docID() < b->posEnum->docID();
}

TermAutomatonScorer::PositionQueue::PositionQueue(int maxSize)
    : org::apache::lucene::util::PriorityQueue<
          org::apache::lucene::search::TermAutomatonQuery::EnumAndScorer>(
          maxSize, false)
{
}

bool TermAutomatonScorer::PositionQueue::lessThan(shared_ptr<EnumAndScorer> a,
                                                  shared_ptr<EnumAndScorer> b)
{
  return a->pos < b->pos;
}

void TermAutomatonScorer::popCurrentDoc()
{
  assert(numSubsOnDoc == 0);
  assert(docIDQueue->size() > 0);
  subsOnDoc[numSubsOnDoc++] = docIDQueue->pop();
  docID_ = subsOnDoc[0]->posEnum.docID();
  while (docIDQueue->size() > 0 &&
         docIDQueue->top()->posEnum->docID() == docID_) {
    subsOnDoc[numSubsOnDoc++] = docIDQueue->pop();
  }
}

void TermAutomatonScorer::pushCurrentDoc()
{
  for (int i = 0; i < numSubsOnDoc; i++) {
    docIDQueue->push_back(subsOnDoc[i]);
  }
  numSubsOnDoc = 0;
}

shared_ptr<DocIdSetIterator> TermAutomatonScorer::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

TermAutomatonScorer::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<TermAutomatonScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TermAutomatonScorer::DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->docID_;
}

int64_t TermAutomatonScorer::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->cost;
}

int TermAutomatonScorer::DocIdSetIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  // we only need to advance docs that are positioned since all docs in the
  // pq are guaranteed to be beyond the current doc already
  for (int i = 0; i < outerInstance->numSubsOnDoc; i++) {
    shared_ptr<EnumAndScorer> sub = outerInstance->subsOnDoc[i];
    if (sub->posEnum->nextDoc() != NO_MORE_DOCS) {
      sub->posLeft = sub->posEnum->freq() - 1;
      sub->pos = sub->posEnum->nextPosition();
    }
  }
  outerInstance->pushCurrentDoc();
  return doNext();
}

int TermAutomatonScorer::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  // Both positioned docs and docs in the pq might be behind target

  // 1. Advance the PQ
  if (outerInstance->docIDQueue->size() > 0) {
    shared_ptr<EnumAndScorer> top = outerInstance->docIDQueue->top();
    while (top->posEnum->docID() < target) {
      if (top->posEnum->advance(target) != NO_MORE_DOCS) {
        top->posLeft = top->posEnum->freq() - 1;
        top->pos = top->posEnum->nextPosition();
      }
      top = outerInstance->docIDQueue->updateTop();
    }
  }

  // 2. Advance subsOnDoc
  for (int i = 0; i < outerInstance->numSubsOnDoc; i++) {
    shared_ptr<EnumAndScorer> sub = outerInstance->subsOnDoc[i];
    if (sub->posEnum->advance(target) != NO_MORE_DOCS) {
      sub->posLeft = sub->posEnum->freq() - 1;
      sub->pos = sub->posEnum->nextPosition();
    }
  }
  outerInstance->pushCurrentDoc();
  return doNext();
}

int TermAutomatonScorer::DocIdSetIteratorAnonymousInnerClass::doNext() throw(
    IOException)
{
  assert(outerInstance->numSubsOnDoc == 0);
  assert(outerInstance->docIDQueue->top()->posEnum->docID() >
         outerInstance->docID_);
  while (true) {
    // System.out.println("  doNext: cycle");
    outerInstance->popCurrentDoc();
    // System.out.println("    docID=" + docID);
    if (outerInstance->docID_ == NO_MORE_DOCS) {
      return outerInstance->docID_;
    }
    outerInstance->countMatches();
    if (outerInstance->freq_ > 0) {
      return outerInstance->docID_;
    }
    for (int i = 0; i < outerInstance->numSubsOnDoc; i++) {
      shared_ptr<EnumAndScorer> sub = outerInstance->subsOnDoc[i];
      if (sub->posEnum->nextDoc() != NO_MORE_DOCS) {
        sub->posLeft = sub->posEnum->freq() - 1;
        sub->pos = sub->posEnum->nextPosition();
      }
    }
    outerInstance->pushCurrentDoc();
  }
}

shared_ptr<PosState> TermAutomatonScorer::getPosition(int pos)
{
  return positions[pos - posShift];
}

void TermAutomatonScorer::shift(int pos)
{
  int limit = pos - posShift;
  for (int i = 0; i < limit; i++) {
    positions[i]->count = 0;
  }
  posShift = pos;
}

void TermAutomatonScorer::countMatches() 
{
  freq_ = 0;
  for (int i = 0; i < numSubsOnDoc; i++) {
    posQueue->push_back(subsOnDoc[i]);
  }
  // System.out.println("\ncountMatches: " + numSubsOnDoc + " terms in doc=" +
  // docID + " anyTermID=" + anyTermID + " id=" +
  // reader.document(docID).get("id")); System.out.println("\ncountMatches: " +
  // numSubsOnDoc + " terms in doc=" + docID + " anyTermID=" + anyTermID);

  int lastPos = -1;

  posShift = -1;

  while (posQueue->size() != 0) {
    shared_ptr<EnumAndScorer> sub = posQueue->pop();

    // This is a graph intersection, and pos is the state this token
    // leaves from.  Until index stores posLength (which we could
    // stuff into a payload using a simple TokenFilter), this token
    // always transitions from state=pos to state=pos+1:
    constexpr int pos = sub->pos;

    if (posShift == -1) {
      posShift = pos;
    }

    if (pos + 1 - posShift >= positions.size()) {
      std::deque<std::shared_ptr<PosState>> newPositions(ArrayUtil::oversize(
          pos + 1 - posShift, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
      System::arraycopy(positions, 0, newPositions, 0, positions.size());
      for (int i = positions.size(); i < newPositions.size(); i++) {
        newPositions[i] = make_shared<PosState>();
      }
      positions = newPositions;
    }

    // System.out.println("  term=" + idToTerm.get(sub.termID).utf8ToString() +
    // " pos=" + pos + " (count=" + getPosition(pos).count + " lastPos=" +
    // lastPos + ") posQueue.size=" + posQueue.size() + " posShift=" +
    // posShift);

    shared_ptr<PosState> posState;
    shared_ptr<PosState> nextPosState;

    // Maybe advance ANY matches:
    if (lastPos != -1) {
      if (anyTermID != -1) {
        int startLastPos = lastPos;
        while (lastPos < pos) {
          posState = getPosition(lastPos);
          if (posState->count == 0 && lastPos > startLastPos) {
            // Petered out...
            lastPos = pos;
            break;
          }
          // System.out.println("  iter lastPos=" + lastPos + " count=" +
          // posState.count);

          nextPosState = getPosition(lastPos + 1);

          // Advance all states from lastPos -> pos, if they had an any arc:
          for (int i = 0; i < posState->count; i++) {
            int state = runAutomaton->step(posState->states[i], anyTermID);
            if (state != -1) {
              // System.out.println("    add pos=" + (lastPos+1) + " state=" +
              // state);
              nextPosState->add(state);
            }
          }

          lastPos++;
        }
      }
    }

    posState = getPosition(pos);
    nextPosState = getPosition(pos + 1);

    // If there are no pending matches at neither this position or the
    // next position, then it's safe to shift back to positions[0]:
    if (posState->count == 0 && nextPosState->count == 0) {
      shift(pos);
      posState = getPosition(pos);
      nextPosState = getPosition(pos + 1);
    }

    // Match current token:
    for (int i = 0; i < posState->count; i++) {
      // System.out.println("    check cur state=" + posState.states[i]);
      int state = runAutomaton->step(posState->states[i], sub->termID);
      if (state != -1) {
        // System.out.println("      --> " + state);
        nextPosState->add(state);
        if (runAutomaton->isAccept(state)) {
          // System.out.println("      *** (1)");
          freq_++;
        }
      }
    }

    // Also consider starting a new match from this position:
    int state = runAutomaton->step(0, sub->termID);
    if (state != -1) {
      // System.out.println("  add init state=" + state);
      nextPosState->add(state);
      if (runAutomaton->isAccept(state)) {
        // System.out.println("      *** (2)");
        freq_++;
      }
    }

    if (sub->posLeft > 0) {
      // Put this sub back into the posQueue:
      sub->pos = sub->posEnum->nextPosition();
      sub->posLeft--;
      posQueue->push_back(sub);
    }

    lastPos = pos;
  }

  int limit = lastPos + 1 - posShift;
  // reset
  for (int i = 0; i <= limit; i++) {
    positions[i]->count = 0;
  }
}

wstring TermAutomatonScorer::toString()
{
  return L"TermAutomatonScorer(" + weight + L")";
}

int TermAutomatonScorer::docID() { return docID_; }

float TermAutomatonScorer::score() 
{
  // TODO: we could probably do better here, e.g. look @ freqs of actual terms
  // involved in this doc and score differently
  return docScorer->score(docID_, freq_);
}

int TermAutomatonScorer::freq() { return freq_; }

TermAutomatonScorer::TermRunAutomaton::TermRunAutomaton(shared_ptr<Automaton> a,
                                                        int termCount)
    : org::apache::lucene::util::automaton::RunAutomaton(a, termCount)
{
}

void TermAutomatonScorer::PosState::add(int state)
{
  if (states.size() == count) {
    states = ArrayUtil::grow(states);
  }
  states[count++] = state;
}
} // namespace org::apache::lucene::search