using namespace std;

#include "CompiledAutomaton.h"

namespace org::apache::lucene::util::automaton
{
using SingleTermsEnum = org::apache::lucene::index::SingleTermsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using StringHelper = org::apache::lucene::util::StringHelper;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

CompiledAutomaton::CompiledAutomaton(shared_ptr<Automaton> automaton)
    : CompiledAutomaton(automaton, nullptr, true)
{
}

int CompiledAutomaton::findSinkState(shared_ptr<Automaton> automaton)
{
  int numStates = automaton->getNumStates();
  shared_ptr<Transition> t = make_shared<Transition>();
  int foundState = -1;
  for (int s = 0; s < numStates; s++) {
    if (automaton->isAccept(s)) {
      int count = automaton->initTransition(s, t);
      bool isSinkState = false;
      for (int i = 0; i < count; i++) {
        automaton->getNextTransition(t);
        if (t->dest == s && t->min == 0 && t->max == 0xff) {
          isSinkState = true;
          break;
        }
      }
      if (isSinkState) {
        foundState = s;
        break;
      }
    }
  }

  return foundState;
}

CompiledAutomaton::CompiledAutomaton(shared_ptr<Automaton> automaton,
                                     optional<bool> &finite, bool simplify)
    : CompiledAutomaton(automaton, finite, simplify,
                        Operations::DEFAULT_MAX_DETERMINIZED_STATES, false)
{
}

CompiledAutomaton::CompiledAutomaton(shared_ptr<Automaton> automaton,
                                     optional<bool> &finite, bool simplify,
                                     int maxDeterminizedStates, bool isBinary)
{
  if (automaton->getNumStates() == 0) {
    automaton = make_shared<Automaton>();
    automaton->createState();
  }

  if (simplify) {

    // Test whether the automaton is a "simple" form and
    // if so, don't create a runAutomaton.  Note that on a
    // large automaton these tests could be costly:

    if (Operations::isEmpty(automaton)) {
      // matches nothing
      type = AUTOMATON_TYPE::NONE;
      term.reset();
      commonSuffixRef.reset();
      runAutomaton.reset();
      this->automaton.reset();
      this->finite = nullopt;
      sinkState = -1;
      return;
    }

    bool isTotal;

    // NOTE: only approximate, because automaton may not be minimal:
    if (isBinary) {
      isTotal = Operations::isTotal(automaton, 0, 0xff);
    } else {
      isTotal = Operations::isTotal(automaton);
    }

    if (isTotal) {
      // matches all possible strings
      type = AUTOMATON_TYPE::ALL;
      term.reset();
      commonSuffixRef.reset();
      runAutomaton.reset();
      this->automaton.reset();
      this->finite = nullopt;
      sinkState = -1;
      return;
    }

    automaton = Operations::determinize(automaton, maxDeterminizedStates);

    shared_ptr<IntsRef> singleton = Operations::getSingleton(automaton);

    if (singleton != nullptr) {
      // matches a fixed string
      type = AUTOMATON_TYPE::SINGLE;
      commonSuffixRef.reset();
      runAutomaton.reset();
      this->automaton.reset();
      this->finite = nullopt;

      if (isBinary) {
        term = StringHelper::intsRefToBytesRef(singleton);
      } else {
        term = make_shared<BytesRef>(UnicodeUtil::newString(
            singleton->ints, singleton->offset, singleton->length));
      }
      sinkState = -1;
      return;
    }
  }

  type = AUTOMATON_TYPE::NORMAL;
  term.reset();

  if (!finite) {
    this->finite = Operations::isFinite(automaton);
  } else {
    this->finite = finite;
  }

  shared_ptr<Automaton> binary;
  if (isBinary) {
    // Caller already built binary automaton themselves, e.g. PrefixQuery
    // does this since it can be provided with a binary (not necessarily
    // UTF8!) term:
    binary = automaton;
  } else {
    // Incoming automaton is unicode, and we must convert to UTF8 to match
    // what's in the index:
    binary = (make_shared<UTF32ToUTF8>())->convert(automaton);
  }

  if (this->finite) {
    commonSuffixRef.reset();
  } else {
    // NOTE: this is a very costly operation!  We should test if it's really
    // warranted in practice... we could do a fast match by looking for a sink
    // state (which means it has no common suffix).  Or maybe we shouldn't do it
    // when simplify is false?:
    shared_ptr<BytesRef> suffix =
        Operations::getCommonSuffixBytesRef(binary, maxDeterminizedStates);
    if (suffix->length == 0) {
      commonSuffixRef.reset();
    } else {
      commonSuffixRef = suffix;
    }
  }

  // This will determinize the binary automaton for us:
  runAutomaton =
      make_shared<ByteRunAutomaton>(binary, true, maxDeterminizedStates);

  this->automaton = runAutomaton->automaton;

  // TODO: this is a bit fragile because if the automaton is not minimized there
  // could be more than 1 sink state but auto-prefix will fail to run for those:
  sinkState = findSinkState(this->automaton);
}

shared_ptr<BytesRef>
CompiledAutomaton::addTail(int state, shared_ptr<BytesRefBuilder> term, int idx,
                           int leadLabel)
{
  // System.out.println("addTail state=" + state + " term=" +
  // term.utf8ToString() + " idx=" + idx + " leadLabel=" + (char) leadLabel);
  // System.out.println(automaton.toDot());
  // Find biggest transition that's < label
  // TODO: use binary search here
  int maxIndex = -1;
  int numTransitions = automaton->initTransition(state, transition);
  for (int i = 0; i < numTransitions; i++) {
    automaton->getNextTransition(transition);
    if (transition->min < leadLabel) {
      maxIndex = i;
    } else {
      // Transitions are alway sorted
      break;
    }
  }

  // System.out.println("  maxIndex=" + maxIndex);

  assert(maxIndex != -1);
  automaton->getTransition(state, maxIndex, transition);

  // Append floorLabel
  constexpr int floorLabel;
  if (transition->max > leadLabel - 1) {
    floorLabel = leadLabel - 1;
  } else {
    floorLabel = transition->max;
  }
  // System.out.println("  floorLabel=" + (char) floorLabel);
  term->grow(1 + idx);
  // if (DEBUG) System.out.println("  add floorLabel=" + (char) floorLabel + "
  // idx=" + idx);
  term->setByteAt(idx, static_cast<char>(floorLabel));

  state = transition->dest;
  // System.out.println("  dest: " + state);
  idx++;

  // Push down to last accept state
  while (true) {
    numTransitions = automaton->getNumTransitions(state);
    if (numTransitions == 0) {
      // System.out.println("state=" + state + " 0 trans");
      assert(runAutomaton->isAccept(state));
      term->setLength(idx);
      // if (DEBUG) System.out.println("  return " + term.utf8ToString());
      return term->get();
    } else {
      // We are pushing "top" -- so get last label of
      // last transition:
      // System.out.println("get state=" + state + " numTrans=" +
      // numTransitions);
      automaton->getTransition(state, numTransitions - 1, transition);
      term->grow(1 + idx);
      // if (DEBUG) System.out.println("  push maxLabel=" + (char)
      // lastTransition.max + " idx=" + idx); System.out.println("  add trans
      // dest=" + scratch.dest + " label=" + (char) scratch.max);
      term->setByteAt(idx, static_cast<char>(transition->max));
      state = transition->dest;
      idx++;
    }
  }
}

shared_ptr<TermsEnum>
CompiledAutomaton::getTermsEnum(shared_ptr<Terms> terms) 
{
  switch (type) {
  case org::apache::lucene::util::automaton::CompiledAutomaton::AUTOMATON_TYPE::
      NONE:
    return TermsEnum::EMPTY;
  case org::apache::lucene::util::automaton::CompiledAutomaton::AUTOMATON_TYPE::
      ALL:
    return terms->begin();
  case org::apache::lucene::util::automaton::CompiledAutomaton::AUTOMATON_TYPE::
      SINGLE:
    return make_shared<SingleTermsEnum>(terms->begin(), term);
  case org::apache::lucene::util::automaton::CompiledAutomaton::AUTOMATON_TYPE::
      NORMAL:
    return terms->intersect(shared_from_this(), nullptr);
  default:
    // unreachable
    throw runtime_error(L"unhandled case");
  }
}

shared_ptr<BytesRef>
CompiledAutomaton::floor(shared_ptr<BytesRef> input,
                         shared_ptr<BytesRefBuilder> output)
{

  // if (DEBUG) System.out.println("CA.floor input=" + input.utf8ToString());

  int state = 0;

  // Special case empty string:
  if (input->length == 0) {
    if (runAutomaton->isAccept(state)) {
      output->clear();
      return output->get();
    } else {
      return nullptr;
    }
  }

  const deque<int> stack = deque<int>();

  int idx = 0;
  while (true) {
    int label = input->bytes[input->offset + idx] & 0xff;
    int nextState = runAutomaton->step(state, label);
    // if (DEBUG) System.out.println("  cycle label=" + (char) label + "
    // nextState=" + nextState);

    if (idx == input->length - 1) {
      if (nextState != -1 && runAutomaton->isAccept(nextState)) {
        // Input string is accepted
        output->grow(1 + idx);
        output->setByteAt(idx, static_cast<char>(label));
        output->setLength(input->length);
        // if (DEBUG) System.out.println("  input is accepted; return term=" +
        // output.utf8ToString());
        return output->get();
      } else {
        nextState = -1;
      }
    }

    if (nextState == -1) {

      // Pop back to a state that has a transition
      // <= our label:
      while (true) {
        int numTransitions = automaton->getNumTransitions(state);
        if (numTransitions == 0) {
          assert(runAutomaton->isAccept(state));
          output->setLength(idx);
          // if (DEBUG) System.out.println("  return " + output.utf8ToString());
          return output->get();
        } else {
          automaton->getTransition(state, 0, transition);

          if (label - 1 < transition->min) {

            if (runAutomaton->isAccept(state)) {
              output->setLength(idx);
              // if (DEBUG) System.out.println("  return " +
              // output.utf8ToString());
              return output->get();
            }
            // pop
            if (stack.empty()) {
              // if (DEBUG) System.out.println("  pop ord=" + idx + " return
              // null");
              return nullptr;
            } else {
              state = stack.pop_back();
              idx--;
              // if (DEBUG) System.out.println("  pop ord=" + (idx+1) + "
              // label=" + (char) label + " first trans.min=" + (char)
              // transitions[0].min);
              label = input->bytes[input->offset + idx] & 0xff;
            }
          } else {
            // if (DEBUG) System.out.println("  stop pop ord=" + idx + " first
            // trans.min=" + (char) transitions[0].min);
            break;
          }
        }
      }

      // if (DEBUG) System.out.println("  label=" + (char) label + " idx=" +
      // idx);

      return addTail(state, output, idx, label);

    } else {
      output->grow(1 + idx);
      output->setByteAt(idx, static_cast<char>(label));
      stack.push_back(state);
      state = nextState;
      idx++;
    }
  }
}

int CompiledAutomaton::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result +
           ((runAutomaton == nullptr) ? 0 : runAutomaton->hashCode());
  result = prime * result + ((term == nullptr) ? 0 : term->hashCode());
  result = prime * result + ((type == nullptr) ? 0 : type.hashCode());
  return result;
}

bool CompiledAutomaton::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<CompiledAutomaton> other =
      any_cast<std::shared_ptr<CompiledAutomaton>>(obj);
  if (type != other->type) {
    return false;
  }
  if (type == AUTOMATON_TYPE::SINGLE) {
    if (!term->equals(other->term)) {
      return false;
    }
  } else if (type == AUTOMATON_TYPE::NORMAL) {
    if (!runAutomaton->equals(other->runAutomaton)) {
      return false;
    }
  }

  return true;
}
} // namespace org::apache::lucene::util::automaton