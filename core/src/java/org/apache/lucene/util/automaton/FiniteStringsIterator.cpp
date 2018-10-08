using namespace std;

#include "FiniteStringsIterator.h"

namespace org::apache::lucene::util::automaton
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const shared_ptr<org::apache::lucene::util::IntsRef>
    FiniteStringsIterator::EMPTY =
        make_shared<org::apache::lucene::util::IntsRef>();

FiniteStringsIterator::FiniteStringsIterator(shared_ptr<Automaton> a)
    : FiniteStringsIterator(a, 0, -1)
{
}

FiniteStringsIterator::FiniteStringsIterator(shared_ptr<Automaton> a,
                                             int startState, int endState)
    : a(a), endState(endState),
      pathStates(make_shared<BitSet>(a->getNumStates())),
      string(make_shared<IntsRefBuilder>())
{
  this->nodes = std::deque<std::shared_ptr<PathNode>>(16);
  for (int i = 0, end = nodes.size(); i < end; i++) {
    nodes[i] = make_shared<PathNode>();
  }
  this->string->setLength(0);
  this->emitEmptyString = a->isAccept(0);

  // Start iteration with node startState.
  if (a->getNumTransitions(startState) > 0) {
    pathStates->set(startState);
    nodes[0]->resetState(a, startState);
    string->append(startState);
  }
}

shared_ptr<IntsRef> FiniteStringsIterator::next()
{
  // Special case the empty string, as usual:
  if (emitEmptyString) {
    emitEmptyString = false;
    return EMPTY;
  }

  for (int depth = string->length(); depth > 0;) {
    shared_ptr<PathNode> node = nodes[depth - 1];

    // Get next label leaving the current node:
    int label = node->nextLabel(a);
    if (label != -1) {
      string->setIntAt(depth - 1, label);

      int to = node->to;
      if (a->getNumTransitions(to) != 0 && to != endState) {
        // Now recurse: the destination of this transition has outgoing
        // transitions:
        if (pathStates->get(to)) {
          throw invalid_argument(L"automaton has cycles");
        }
        pathStates->set(to);

        // Push node onto stack:
        growStack(depth);
        nodes[depth]->resetState(a, to);
        depth++;
        string->setLength(depth);
        string->grow(depth);
      } else if (endState == to || a->isAccept(to)) {
        // This transition leads to an accept state, so we save the current
        // string:
        return string->get();
      }
    } else {
      // No more transitions leaving this state, pop/return back to previous
      // state:
      int state = node->state;
      assert(pathStates->get(state));
      pathStates->clear(state);
      depth--;
      string->setLength(depth);

      if (a->isAccept(state)) {
        // This transition leads to an accept state, so we save the current
        // string:
        return string->get();
      }
    }
  }

  // Finished iteration.
  return nullptr;
}

void FiniteStringsIterator::growStack(int depth)
{
  if (nodes.size() == depth) {
    std::deque<std::shared_ptr<PathNode>> newNodes(ArrayUtil::oversize(
        nodes.size() + 1, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(nodes, 0, newNodes, 0, nodes.size());
    for (int i = depth, end = newNodes.size(); i < end; i++) {
      newNodes[i] = make_shared<PathNode>();
    }
    nodes = newNodes;
  }
}

void FiniteStringsIterator::PathNode::resetState(shared_ptr<Automaton> a,
                                                 int state)
{
  assert(a->getNumTransitions(state) != 0);
  this->state = state;
  transition = 0;
  a->getTransition(state, 0, t);
  label = t->min;
  to = t->dest;
}

int FiniteStringsIterator::PathNode::nextLabel(shared_ptr<Automaton> a)
{
  if (label > t->max) {
    // We've exhaused the current transition's labels;
    // move to next transitions:
    transition++;
    if (transition >= a->getNumTransitions(state)) {
      // We're done iterating transitions leaving this state
      label = -1;
      return -1;
    }
    a->getTransition(state, transition, t);
    label = t->min;
    to = t->dest;
  }
  return label++;
}
} // namespace org::apache::lucene::util::automaton