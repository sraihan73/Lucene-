using namespace std;

#include "PrefixQuery.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

PrefixQuery::PrefixQuery(shared_ptr<Term> prefix)
    : AutomatonQuery(prefix, toAutomaton(prefix->bytes()),
                     numeric_limits<int>::max(), true)
{
  // It's OK to pass unlimited maxDeterminizedStates: the automaton is born
  // small and determinized:
  if (prefix == nullptr) {
    throw make_shared<NullPointerException>(L"prefix must not be null");
  }
}

shared_ptr<Automaton> PrefixQuery::toAutomaton(shared_ptr<BytesRef> prefix)
{
  constexpr int numStatesAndTransitions = prefix->length + 1;
  shared_ptr<Automaton> *const automaton =
      make_shared<Automaton>(numStatesAndTransitions, numStatesAndTransitions);
  int lastState = automaton->createState();
  for (int i = 0; i < prefix->length; i++) {
    int state = automaton->createState();
    automaton->addTransition(lastState, state,
                             prefix->bytes[prefix->offset + i] & 0xff);
    lastState = state;
  }
  automaton->setAccept(lastState, true);
  automaton->addTransition(lastState, lastState, 0, 255);
  automaton->finishState();
  assert(automaton->isDeterministic());
  return automaton;
}

shared_ptr<Term> PrefixQuery::getPrefix() { return term; }

wstring PrefixQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (getField() != field) {
    buffer->append(getField());
    buffer->append(L':');
  }
  buffer->append(term->text());
  buffer->append(L'*');
  return buffer->toString();
}

int PrefixQuery::hashCode()
{
  constexpr int prime = 31;
  int result = AutomatonQuery::hashCode();
  result = prime * result + term->hashCode();
  return result;
}

bool PrefixQuery::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!AutomatonQuery::equals(obj)) {
    return false;
  }
  // super.equals() ensures we are the same class
  shared_ptr<PrefixQuery> other = any_cast<std::shared_ptr<PrefixQuery>>(obj);
  if (!term->equals(other->term)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::search