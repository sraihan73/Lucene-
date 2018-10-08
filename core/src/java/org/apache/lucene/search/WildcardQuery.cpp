using namespace std;

#include "WildcardQuery.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Operations = org::apache::lucene::util::automaton::Operations;

WildcardQuery::WildcardQuery(shared_ptr<Term> term)
    : AutomatonQuery(term, toAutomaton(term))
{
}

WildcardQuery::WildcardQuery(shared_ptr<Term> term, int maxDeterminizedStates)
    : AutomatonQuery(term, toAutomaton(term), maxDeterminizedStates)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public static
// org.apache.lucene.util.automaton.Automaton
// toAutomaton(org.apache.lucene.index.Term wildcardquery)
shared_ptr<Automaton> WildcardQuery::toAutomaton(shared_ptr<Term> wildcardquery)
{
  deque<std::shared_ptr<Automaton>> automata =
      deque<std::shared_ptr<Automaton>>();

  wstring wildcardText = wildcardquery->text();

  for (int i = 0; i < wildcardText.length();) {
    constexpr int c = wildcardText.codePointAt(i);
    int length = Character::charCount(c);
    switch (c) {
    case WILDCARD_STRING:
      automata.push_back(Automata::makeAnyString());
      break;
    case WILDCARD_CHAR:
      automata.push_back(Automata::makeAnyChar());
      break;
    case WILDCARD_ESCAPE:
      // add the next codepoint instead, if it exists
      if (i + length < wildcardText.length()) {
        constexpr int nextChar = wildcardText.codePointAt(i + length);
        length += Character::charCount(nextChar);
        automata.push_back(Automata::makeChar(nextChar));
        break;
      } // else fallthru, lenient parsing with a trailing \
          default:
      automata.push_back(Automata::makeChar(c));
    }
    i += length;
  }

  return Operations::concatenate(automata);
}

shared_ptr<Term> WildcardQuery::getTerm() { return term; }

wstring WildcardQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (getField() != field) {
    buffer->append(getField());
    buffer->append(L":");
  }
  buffer->append(term->text());
  return buffer->toString();
}
} // namespace org::apache::lucene::search