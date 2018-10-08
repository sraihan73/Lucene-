using namespace std;

#include "AutomatonTermsEnum.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using ByteRunAutomaton = org::apache::lucene::util::automaton::ByteRunAutomaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using Transition = org::apache::lucene::util::automaton::Transition;

AutomatonTermsEnum::AutomatonTermsEnum(shared_ptr<TermsEnum> tenum,
                                       shared_ptr<CompiledAutomaton> compiled)
    : FilteredTermsEnum(tenum), runAutomaton(compiled->runAutomaton),
      commonSuffixRef(compiled->commonSuffixRef), finite(compiled->finite),
      automaton(compiled->automaton),
      visited(std::deque<int64_t>(runAutomaton->getSize()))
{
  if (compiled->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
    throw invalid_argument(
        L"please use CompiledAutomaton.getTermsEnum instead");
  }
  assert(this->runAutomaton != nullptr);

  // used for path tracking, where each bit is a numbered state.
}

AcceptStatus AutomatonTermsEnum::accept(shared_ptr<BytesRef> term)
{
  if (commonSuffixRef == nullptr ||
      StringHelper::endsWith(term, commonSuffixRef)) {
    if (runAutomaton->run(term->bytes, term->offset, term->length)) {
      return linear ? AcceptStatus::YES : AcceptStatus::YES_AND_SEEK;
    } else {
      return (linear && term->compareTo(linearUpperBound) < 0)
                 ? AcceptStatus::NO
                 : AcceptStatus::NO_AND_SEEK;
    }
  } else {
    return (linear && term->compareTo(linearUpperBound) < 0)
               ? AcceptStatus::NO
               : AcceptStatus::NO_AND_SEEK;
  }
}

shared_ptr<BytesRef>
AutomatonTermsEnum::nextSeekTerm(shared_ptr<BytesRef> term) 
{
  // System.out.println("ATE.nextSeekTerm term=" + term);
  if (term == nullptr) {
    assert(seekBytesRef->length() == 0);
    // return the empty term, as it's valid
    if (runAutomaton->isAccept(0)) {
      return seekBytesRef->get();
    }
  } else {
    seekBytesRef->copyBytes(term);
  }

  // seek to the next possible string;
  if (nextString()) {
    return seekBytesRef->get(); // reposition
  } else {
    return nullptr; // no more possible strings can match
  }
}

void AutomatonTermsEnum::setLinear(int position)
{
  assert(linear == false);

  int state = 0;
  assert(state == 0);
  int maxInterval = 0xff;
  // System.out.println("setLinear pos=" + position + " seekbytesRef=" +
  // seekBytesRef);
  for (int i = 0; i < position; i++) {
    state = runAutomaton->step(state, seekBytesRef->byteAt(i) & 0xff);
    assert((state >= 0, L"state=" + to_wstring(state)));
  }
  constexpr int numTransitions = automaton->getNumTransitions(state);
  automaton->initTransition(state, transition);
  for (int i = 0; i < numTransitions; i++) {
    automaton->getNextTransition(transition);
    if (transition->min <= (seekBytesRef->byteAt(position) & 0xff) &&
        (seekBytesRef->byteAt(position) & 0xff) <= transition->max) {
      maxInterval = transition->max;
      break;
    }
  }
  // 0xff terms don't get the optimization... not worth the trouble.
  if (maxInterval != 0xff) {
    maxInterval++;
  }
  int length = position + 1; // position + maxTransition
  if (linearUpperBound->bytes.size() < length) {
    linearUpperBound->bytes = std::deque<char>(length);
  }
  System::arraycopy(seekBytesRef->bytes(), 0, linearUpperBound->bytes, 0,
                    position);
  linearUpperBound->bytes[position] = static_cast<char>(maxInterval);
  linearUpperBound->length = length;

  linear = true;
}

bool AutomatonTermsEnum::nextString()
{
  int state;
  int pos = 0;
  savedStates->grow(seekBytesRef->length() + 1);
  savedStates->setIntAt(0, 0);

  while (true) {
    curGen++;
    linear = false;
    // walk the automaton until a character is rejected.
    for (state = savedStates->intAt(pos); pos < seekBytesRef->length(); pos++) {
      visited[state] = curGen;
      int nextState =
          runAutomaton->step(state, seekBytesRef->byteAt(pos) & 0xff);
      if (nextState == -1) {
        break;
      }
      savedStates->setIntAt(pos + 1, nextState);
      // we found a loop, record it for faster enumeration
      if (!finite && !linear && visited[nextState] == curGen) {
        setLinear(pos);
      }
      state = nextState;
    }

    // take the useful portion, and the last non-reject state, and attempt to
    // append characters that will match.
    if (nextString(state, pos)) {
      return true;
    } else { // no more solutions exist from this useful portion, backtrack
      if ((pos = backtrack(pos)) < 0) // no more solutions at all
      {
        return false;
      }
      constexpr int newState = runAutomaton->step(
          savedStates->intAt(pos), seekBytesRef->byteAt(pos) & 0xff);
      if (newState >= 0 && runAutomaton->isAccept(newState)) {
        /* std::wstring is good to go as-is */
        return true;
      }
      /* else advance further */
      // TODO: paranoia? if we backtrack thru an infinite DFA, the loop
      // detection is important! for now, restart from scratch for all infinite
      // DFAs
      if (!finite) {
        pos = 0;
      }
    }
  }
}

bool AutomatonTermsEnum::nextString(int state, int position)
{
  /*
   * the next lexicographic character must be greater than the existing
   * character, if it exists.
   */
  int c = 0;
  if (position < seekBytesRef->length()) {
    c = seekBytesRef->byteAt(position) & 0xff;
    // if the next byte is 0xff and is not part of the useful portion,
    // then by definition it puts us in a reject state, and therefore this
    // path is dead. there cannot be any higher transitions. backtrack.
    if (c++ == 0xff) {
      return false;
    }
  }

  seekBytesRef->setLength(position);
  visited[state] = curGen;

  constexpr int numTransitions = automaton->getNumTransitions(state);
  automaton->initTransition(state, transition);
  // find the minimal path (lexicographic order) that is >= c

  for (int i = 0; i < numTransitions; i++) {
    automaton->getNextTransition(transition);
    if (transition->max >= c) {
      int nextChar = max(c, transition->min);
      // append either the next sequential char, or the minimum transition
      seekBytesRef->grow(seekBytesRef->length() + 1);
      seekBytesRef->append(static_cast<char>(nextChar));
      state = transition->dest;
      /*
       * as long as is possible, continue down the minimal path in
       * lexicographic order. if a loop or accept state is encountered, stop.
       */
      while (visited[state] != curGen && !runAutomaton->isAccept(state)) {
        visited[state] = curGen;
        /*
         * Note: we work with a DFA with no transitions to dead states.
         * so the below is ok, if it is not an accept state,
         * then there MUST be at least one transition.
         */
        automaton->initTransition(state, transition);
        automaton->getNextTransition(transition);
        state = transition->dest;

        // append the minimum transition
        seekBytesRef->grow(seekBytesRef->length() + 1);
        seekBytesRef->append(static_cast<char>(transition->min));

        // we found a loop, record it for faster enumeration
        if (!finite && !linear && visited[state] == curGen) {
          setLinear(seekBytesRef->length() - 1);
        }
      }
      return true;
    }
  }
  return false;
}

int AutomatonTermsEnum::backtrack(int position)
{
  while (position-- > 0) {
    int nextChar = seekBytesRef->byteAt(position) & 0xff;
    // if a character is 0xff it's a dead-end too,
    // because there is no higher character in binary sort order.
    if (nextChar++ != 0xff) {
      seekBytesRef->setByteAt(position, static_cast<char>(nextChar));
      seekBytesRef->setLength(position + 1);
      return position;
    }
  }
  return -1; // all solutions exhausted
}
} // namespace org::apache::lucene::index