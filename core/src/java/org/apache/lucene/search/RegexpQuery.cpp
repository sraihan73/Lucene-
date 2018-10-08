using namespace std;

#include "RegexpQuery.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonProvider =
    org::apache::lucene::util::automaton::AutomatonProvider;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;
shared_ptr<org::apache::lucene::util::automaton::AutomatonProvider>
    RegexpQuery::defaultProvider =
        make_shared<AutomatonProviderAnonymousInnerClass>();

RegexpQuery::AutomatonProviderAnonymousInnerClass::
    AutomatonProviderAnonymousInnerClass()
{
}

shared_ptr<Automaton>
RegexpQuery::AutomatonProviderAnonymousInnerClass::getAutomaton(
    const wstring &name)
{
  return nullptr;
}

RegexpQuery::RegexpQuery(shared_ptr<Term> term) : RegexpQuery(term, RegExp::ALL)
{
}

RegexpQuery::RegexpQuery(shared_ptr<Term> term, int flags)
    : RegexpQuery(term, flags, defaultProvider,
                  Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

RegexpQuery::RegexpQuery(shared_ptr<Term> term, int flags,
                         int maxDeterminizedStates)
    : RegexpQuery(term, flags, defaultProvider, maxDeterminizedStates)
{
}

RegexpQuery::RegexpQuery(shared_ptr<Term> term, int flags,
                         shared_ptr<AutomatonProvider> provider,
                         int maxDeterminizedStates)
    : AutomatonQuery(term,
                     new RegExp(term->text(), flags)
                         .toAutomaton(provider, maxDeterminizedStates),
                     maxDeterminizedStates)
{
}

shared_ptr<Term> RegexpQuery::getRegexp() { return term; }

wstring RegexpQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (term->field() != field) {
    buffer->append(term->field());
    buffer->append(L":");
  }
  buffer->append(L'/');
  buffer->append(term->text());
  buffer->append(L'/');
  return buffer->toString();
}
} // namespace org::apache::lucene::search