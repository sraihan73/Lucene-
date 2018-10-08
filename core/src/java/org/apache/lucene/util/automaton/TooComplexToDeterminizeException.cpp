using namespace std;

#include "TooComplexToDeterminizeException.h"

namespace org::apache::lucene::util::automaton
{

TooComplexToDeterminizeException::TooComplexToDeterminizeException(
    shared_ptr<RegExp> regExp,
    shared_ptr<TooComplexToDeterminizeException> cause)
    : RuntimeException(L"Determinizing " + regExp->getOriginalString() +
                           L" would result in more than " +
                           cause->maxDeterminizedStates + L" states.",
                       cause),
      automaton(cause->automaton), regExp(regExp),
      maxDeterminizedStates(cause->maxDeterminizedStates)
{
}

TooComplexToDeterminizeException::TooComplexToDeterminizeException(
    shared_ptr<Automaton> automaton, int maxDeterminizedStates)
    : RuntimeException(L"Determinizing automaton with " +
                       automaton->getNumStates() + L" states and " +
                       automaton->getNumTransitions() +
                       L" transitions would result in more than " +
                       maxDeterminizedStates + L" states."),
      automaton(automaton), regExp(this->regExp.reset()),
      maxDeterminizedStates(maxDeterminizedStates)
{
}

shared_ptr<Automaton> TooComplexToDeterminizeException::getAutomaton()
{
  return automaton;
}

shared_ptr<RegExp> TooComplexToDeterminizeException::getRegExp()
{
  return regExp;
}

int TooComplexToDeterminizeException::getMaxDeterminizedStates()
{
  return maxDeterminizedStates;
}
} // namespace org::apache::lucene::util::automaton