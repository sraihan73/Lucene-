using namespace std;

#include "RegexCompletionQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;

RegexCompletionQuery::RegexCompletionQuery(shared_ptr<Term> term)
    : RegexCompletionQuery(term, nullptr)
{
}

RegexCompletionQuery::RegexCompletionQuery(shared_ptr<Term> term,
                                           shared_ptr<BitsProducer> filter)
    : RegexCompletionQuery(term, RegExp::ALL,
                           Operations::DEFAULT_MAX_DETERMINIZED_STATES, filter)
{
}

RegexCompletionQuery::RegexCompletionQuery(shared_ptr<Term> term, int flags,
                                           int maxDeterminizedStates)
    : RegexCompletionQuery(term, flags, maxDeterminizedStates, nullptr)
{
}

RegexCompletionQuery::RegexCompletionQuery(shared_ptr<Term> term, int flags,
                                           int maxDeterminizedStates,
                                           shared_ptr<BitsProducer> filter)
    : CompletionQuery(term, filter), flags(flags),
      maxDeterminizedStates(maxDeterminizedStates)
{
}

shared_ptr<Weight>
RegexCompletionQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                   bool needsScores,
                                   float boost) 
{
  // If an empty regex is provided, we return an automaton that matches nothing.
  // This ensures consistency with PrefixCompletionQuery, which returns no
  // results for an empty term.
  shared_ptr<Automaton> automaton =
      getTerm()->text().isEmpty()
          ? Automata::makeEmpty()
          : (make_shared<RegExp>(getTerm()->text(), flags))
                ->toAutomaton(maxDeterminizedStates);
  return make_shared<CompletionWeight>(shared_from_this(), automaton);
}

int RegexCompletionQuery::getFlags() { return flags; }

int RegexCompletionQuery::getMaxDeterminizedStates()
{
  return maxDeterminizedStates;
}

bool RegexCompletionQuery::equals(any o)
{
  throw make_shared<UnsupportedOperationException>();
}

int RegexCompletionQuery::hashCode()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::search::suggest::document