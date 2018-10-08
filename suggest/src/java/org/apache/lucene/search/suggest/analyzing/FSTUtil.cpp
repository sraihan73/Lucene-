using namespace std;

#include "FSTUtil.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Transition = org::apache::lucene::util::automaton::Transition;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;

FSTUtil::FSTUtil() {}

template <typename T>
deque<Path<T>>
FSTUtil::intersectPrefixPaths(shared_ptr<Automaton> a,
                              shared_ptr<FST<T>> fst) 
{
  assert(a->isDeterministic());
  const deque<Path<T>> queue = deque<Path<T>>();
  const deque<Path<T>> endNodes = deque<Path<T>>();
  if (a->getNumStates() == 0) {
    return endNodes;
  }

  queue.push_back(make_shared<Path<>>(
      0, fst->getFirstArc(make_shared<FST::Arc<T>>()),
      fst->outputs->getNoOutput(), make_shared<IntsRefBuilder>()));

  shared_ptr<FST::Arc<T>> *const scratchArc = make_shared<FST::Arc<T>>();
  shared_ptr<FST::BytesReader> *const fstReader = fst->getBytesReader();

  shared_ptr<Transition> t = make_shared<Transition>();

  while (queue.size() != 0) {
    shared_ptr<Path<T>> *const path = queue.pop_back();
    if (a->isAccept(path->state)) {
      endNodes.push_back(path);
      // we can stop here if we accept this path,
      // we accept all further paths too
      continue;
    }

    shared_ptr<IntsRefBuilder> currentInput = path->input;
    int count = a->initTransition(path->state, t);
    for (int i = 0; i < count; i++) {
      a->getNextTransition(t);
      constexpr int min = t->min;
      constexpr int max = t->max;
      if (min == max) {
        shared_ptr<FST::Arc<T>> *const nextArc =
            fst->findTargetArc(t->min, path->fstNode, scratchArc, fstReader);
        if (nextArc != nullptr) {
          shared_ptr<IntsRefBuilder> *const newInput =
              make_shared<IntsRefBuilder>();
          newInput->copyInts(currentInput->get());
          newInput->append(t->min);
          queue.push_back(make_shared<Path<>>(
              t->dest, make_shared<FST::Arc<T>>().copyFrom(nextArc),
              fst->outputs->add(path->output, nextArc->output), newInput));
        }
      } else {
        // TODO: if this transition's TO state is accepting, and
        // it accepts the entire range possible in the FST (ie. 0 to 255),
        // we can simply use the prefix as the accepted state instead of
        // looking up all the ranges and terminate early
        // here.  This just shifts the work from one queue
        // (this one) to another (the completion search
        // done in AnalyzingSuggester).
        shared_ptr<FST::Arc<T>> nextArc =
            Util::readCeilArc(min, fst, path->fstNode, scratchArc, fstReader);
        while (nextArc != nullptr && nextArc->label <= max) {
          assert(nextArc->label <= max);
          assert((nextArc->label >= min, to_wstring(nextArc->label) + L" "));
          +min;
          shared_ptr<IntsRefBuilder> *const newInput =
              make_shared<IntsRefBuilder>();
          newInput->copyInts(currentInput->get());
          newInput->append(nextArc->label);
          queue.push_back(make_shared<Path<>>(
              t->dest, make_shared<FST::Arc<T>>().copyFrom(nextArc),
              fst->outputs->add(path->output, nextArc->output), newInput));
          constexpr int label = nextArc->label; // used in assert
          nextArc = nextArc->isLast()
                        ? nullptr
                        : fst->readNextRealArc(nextArc, fstReader);
          assert((nextArc == nullptr || label < nextArc->label,
                  L"last: " + to_wstring(label) + L" next: " +
                      to_wstring(nextArc->label)));
        }
      }
    }
  }
  return endNodes;
}
} // namespace org::apache::lucene::search::suggest::analyzing