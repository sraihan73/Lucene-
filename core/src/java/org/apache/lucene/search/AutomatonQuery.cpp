using namespace std;

#include "AutomatonQuery.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using Operations = org::apache::lucene::util::automaton::Operations;

AutomatonQuery::AutomatonQuery(shared_ptr<Term> term,
                               shared_ptr<Automaton> automaton)
    : AutomatonQuery(term, automaton,
                     Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

AutomatonQuery::AutomatonQuery(shared_ptr<Term> term,
                               shared_ptr<Automaton> automaton,
                               int maxDeterminizedStates)
    : AutomatonQuery(term, automaton, maxDeterminizedStates, false)
{
}

AutomatonQuery::AutomatonQuery(shared_ptr<Term> term,
                               shared_ptr<Automaton> automaton,
                               int maxDeterminizedStates, bool isBinary)
    : MultiTermQuery(term->field()), automaton(automaton),
      compiled(make_shared<CompiledAutomaton>(automaton, nullptr, true,
                                              maxDeterminizedStates, isBinary)),
      term(term), automatonIsBinary(isBinary)
{
  // TODO: we could take isFinite too, to save a bit of CPU in CompiledAutomaton
  // ctor?:
}

shared_ptr<TermsEnum> AutomatonQuery::getTermsEnum(
    shared_ptr<Terms> terms,
    shared_ptr<AttributeSource> atts) 
{
  return compiled->getTermsEnum(terms);
}

int AutomatonQuery::hashCode()
{
  constexpr int prime = 31;
  int result = MultiTermQuery::hashCode();
  result = prime * result + compiled->hashCode();
  result = prime * result + ((term == nullptr) ? 0 : term->hashCode());
  return result;
}

bool AutomatonQuery::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!MultiTermQuery::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<AutomatonQuery> other =
      any_cast<std::shared_ptr<AutomatonQuery>>(obj);
  if (!compiled->equals(other->compiled)) {
    return false;
  }
  if (term == nullptr) {
    if (other->term != nullptr) {
      return false;
    }
  } else if (!term->equals(other->term)) {
    return false;
  }
  return true;
}

wstring AutomatonQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (term->field() != field) {
    buffer->append(term->field());
    buffer->append(L":");
  }
  buffer->append(getClass().getSimpleName());
  buffer->append(L" {");
  buffer->append(L'\n');
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(automaton->toString());
  buffer->append(L"}");
  return buffer->toString();
}

shared_ptr<Automaton> AutomatonQuery::getAutomaton() { return automaton; }

bool AutomatonQuery::isAutomatonBinary() { return automatonIsBinary; }
} // namespace org::apache::lucene::search