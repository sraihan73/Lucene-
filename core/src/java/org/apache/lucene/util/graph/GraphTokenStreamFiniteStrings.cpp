using namespace std;

#include "GraphTokenStreamFiniteStrings.h"

namespace org::apache::lucene::util::graph
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using FiniteStringsIterator =
    org::apache::lucene::util::automaton::FiniteStringsIterator;
using Operations = org::apache::lucene::util::automaton::Operations;
using Transition = org::apache::lucene::util::automaton::Transition;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

GraphTokenStreamFiniteStrings::FiniteStringsTokenStream::
    FiniteStringsTokenStream(
        shared_ptr<GraphTokenStreamFiniteStrings> outerInstance,
        shared_ptr<IntsRef> ids)
    : ids(ids), end(ids->offset + ids->length), outerInstance(outerInstance)
{
  assert(ids != nullptr);
  this->offset = ids->offset;
}

bool GraphTokenStreamFiniteStrings::FiniteStringsTokenStream::
    incrementToken() 
{
  if (offset < end) {
    clearAttributes();
    int id = ids->ints[offset];
    termAtt->setBytesRef(outerInstance->idToTerm[id]);

    int incr = 1;
    if (outerInstance->idToInc.find(id) != outerInstance->idToInc.end()) {
      incr = outerInstance->idToInc[id];
    }
    posIncAtt->setPositionIncrement(incr);
    offset++;
    return true;
  }

  return false;
}

GraphTokenStreamFiniteStrings::GraphTokenStreamFiniteStrings(
    shared_ptr<TokenStream> in_) 
    : det(Operations::removeDeadStates(Operations::determinize(
          aut, Operations::DEFAULT_MAX_DETERMINIZED_STATES)))
{
  shared_ptr<Automaton> aut = build(in_);
}

bool GraphTokenStreamFiniteStrings::hasSidePath(int state)
{
  int numT = det->initTransition(state, transition);
  if (numT <= 1) {
    return false;
  }
  det->getNextTransition(transition);
  int dest = transition->dest;
  for (int i = 1; i < numT; i++) {
    det->getNextTransition(transition);
    if (dest != transition->dest) {
      return true;
    }
  }
  return false;
}

std::deque<std::shared_ptr<Term>>
GraphTokenStreamFiniteStrings::getTerms(const wstring &field, int state)
{
  int numT = det->initTransition(state, transition);
  deque<std::shared_ptr<Term>> terms = deque<std::shared_ptr<Term>>();
  for (int i = 0; i < numT; i++) {
    det->getNextTransition(transition);
    for (int id = transition->min; id <= transition->max; id++) {
      shared_ptr<Term> term = make_shared<Term>(field, idToTerm[id]);
      terms.push_back(term);
    }
  }
  return terms.toArray(std::deque<std::shared_ptr<Term>>(terms.size()));
}

shared_ptr<Iterator<std::shared_ptr<TokenStream>>>
GraphTokenStreamFiniteStrings::getFiniteStrings() 
{
  return getFiniteStrings(0, -1);
}

shared_ptr<Iterator<std::shared_ptr<TokenStream>>>
GraphTokenStreamFiniteStrings::getFiniteStrings(int startState,
                                                int endState) 
{
  shared_ptr<FiniteStringsIterator> *const it =
      make_shared<FiniteStringsIterator>(det, startState, endState);
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this(), it);
}

GraphTokenStreamFiniteStrings::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(
        shared_ptr<GraphTokenStreamFiniteStrings> outerInstance,
        shared_ptr<FiniteStringsIterator> it)
{
  this->outerInstance = outerInstance;
  this->it = it;
  finished = false;
}

bool GraphTokenStreamFiniteStrings::IteratorAnonymousInnerClass::hasNext()
{
  if (finished == false && current == nullptr) {
    current = it->next();
    if (current == nullptr) {
      finished = true;
    }
  }
  return current != nullptr;
}

shared_ptr<TokenStream>
GraphTokenStreamFiniteStrings::IteratorAnonymousInnerClass::next()
{
  if (current == nullptr) {
    hasNext();
  }
  shared_ptr<TokenStream> next =
      make_shared<FiniteStringsTokenStream>(outerInstance, current);
  current = nullptr;
  return next;
}

std::deque<int> GraphTokenStreamFiniteStrings::articulationPoints()
{
  if (det->getNumStates() == 0) {
    return std::deque<int>(0);
  }
  //
  shared_ptr<Automaton::Builder> undirect = make_shared<Automaton::Builder>();
  undirect->copy(det);
  for (int i = 0; i < det->getNumStates(); i++) {
    int numT = det->initTransition(i, transition);
    for (int j = 0; j < numT; j++) {
      det->getNextTransition(transition);
      undirect->addTransition(transition->dest, i, transition->min);
    }
  }
  int numStates = det->getNumStates();
  shared_ptr<BitSet> visited = make_shared<BitSet>(numStates);
  std::deque<int> depth(det->getNumStates());
  std::deque<int> low(det->getNumStates());
  std::deque<int> parent(det->getNumStates());
  Arrays::fill(parent, -1);
  deque<int> points = deque<int>();
  articulationPointsRecurse(undirect->finish(), 0, 0, depth, low, parent,
                            visited, points);
  reverse(points.begin(), points.end());
  return points.stream().mapToInt([&](any p) { return p; }).toArray();
}

shared_ptr<Automaton> GraphTokenStreamFiniteStrings::build(
    shared_ptr<TokenStream> in_) 
{
  shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();
  shared_ptr<TermToBytesRefAttribute> *const termBytesAtt =
      in_->addAttribute(TermToBytesRefAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> *const posIncAtt =
      in_->addAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionLengthAttribute> *const posLengthAtt =
      in_->addAttribute(PositionLengthAttribute::typeid);

  in_->reset();

  int pos = -1;
  int prevIncr = 1;
  int state = -1;
  while (in_->incrementToken()) {
    int currentIncr = posIncAtt->getPositionIncrement();
    if (pos == -1 && currentIncr < 1) {
      throw make_shared<IllegalStateException>(
          L"Malformed TokenStream, start token can't have increment less than "
          L"1");
    }

    // always use inc 1 while building, but save original increment
    int incr = min(1, currentIncr);
    if (incr > 0) {
      pos += incr;
    }

    int endPos = pos + posLengthAtt->getPositionLength();
    while (state < endPos) {
      state = builder->createState();
    }

    shared_ptr<BytesRef> term = termBytesAtt->getBytesRef();
    int id = getTermID(currentIncr, prevIncr, term);
    builder->addTransition(pos, endPos, id);

    // only save last increment on non-zero increment in case we have multiple
    // stacked tokens
    if (currentIncr > 0) {
      prevIncr = currentIncr;
    }
  }

  in_->end();
  if (state != -1) {
    builder->setAccept(state, true);
  }
  return builder->finish();
}

int GraphTokenStreamFiniteStrings::getTermID(int incr, int prevIncr,
                                             shared_ptr<BytesRef> term)
{
  assert(term != nullptr);
  bool isStackedGap = incr == 0 && prevIncr > 1;
  int id = idToTerm.size();
  idToTerm.emplace(id, BytesRef::deepCopyOf(term));
  // stacked token should have the same increment as original token at this
  // position
  if (isStackedGap) {
    idToInc.emplace(id, prevIncr);
  } else if (incr > 1) {
    idToInc.emplace(id, incr);
  }
  return id;
}

void GraphTokenStreamFiniteStrings::articulationPointsRecurse(
    shared_ptr<Automaton> a, int state, int d, std::deque<int> &depth,
    std::deque<int> &low, std::deque<int> &parent, shared_ptr<BitSet> visited,
    deque<int> &points)
{
  visited->set(state);
  depth[state] = d;
  low[state] = d;
  int childCount = 0;
  bool isArticulation = false;
  shared_ptr<Transition> t = make_shared<Transition>();
  int numT = a->initTransition(state, t);
  for (int i = 0; i < numT; i++) {
    a->getNextTransition(t);
    if (visited->get(t->dest) == false) {
      parent[t->dest] = state;
      articulationPointsRecurse(a, t->dest, d + 1, depth, low, parent, visited,
                                points);
      childCount++;
      if (low[t->dest] >= depth[state]) {
        isArticulation = true;
      }
      low[state] = min(low[state], low[t->dest]);
    } else if (t->dest != parent[state]) {
      low[state] = min(low[state], depth[t->dest]);
    }
  }
  if ((parent[state] != -1 && isArticulation) ||
      (parent[state] == -1 && childCount > 1)) {
    points.push_back(state);
  }
}
} // namespace org::apache::lucene::util::graph