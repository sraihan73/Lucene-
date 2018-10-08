using namespace std;

#include "CompletionWeight.h"

namespace org::apache::lucene::search::suggest::document
{
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using Explanation = org::apache::lucene::search::Explanation;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
using Bits = org::apache::lucene::util::Bits;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

CompletionWeight::CompletionWeight(
    shared_ptr<CompletionQuery> query,
    shared_ptr<Automaton> automaton) 
    : org::apache::lucene::search::Weight(query), completionQuery(query),
      automaton(automaton)
{
}

shared_ptr<Automaton> CompletionWeight::getAutomaton() { return automaton; }

shared_ptr<BulkScorer> CompletionWeight::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> *const reader = context->reader();
  shared_ptr<Terms> *const terms;
  shared_ptr<NRTSuggester> *const suggester;
  if ((terms = reader->terms(completionQuery->getField())) == nullptr) {
    return nullptr;
  }
  if (std::dynamic_pointer_cast<CompletionTerms>(terms) != nullptr) {
    shared_ptr<CompletionTerms> completionTerms =
        std::static_pointer_cast<CompletionTerms>(terms);
    if ((suggester = completionTerms->suggester()) == nullptr) {
      // a segment can have a null suggester
      // i.e. no FST was built
      return nullptr;
    }
  } else {
    throw invalid_argument(completionQuery->getField() +
                           L" is not a SuggestField");
  }

  shared_ptr<BitsProducer> filter = completionQuery->getFilter();
  shared_ptr<Bits> filteredDocs = nullptr;
  if (filter != nullptr) {
    filteredDocs = filter->getBits(context);
    if (filteredDocs->getClass() == Bits::MatchNoBits::typeid) {
      return nullptr;
    }
  }
  return make_shared<CompletionScorer>(shared_from_this(), suggester, reader,
                                       filteredDocs, filter != nullptr,
                                       automaton);
}

void CompletionWeight::setNextMatch(shared_ptr<IntsRef> pathPrefix) {}

float CompletionWeight::boost() { return 0; }

shared_ptr<std::wstring> CompletionWeight::context() { return nullptr; }

shared_ptr<Scorer> CompletionWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

bool CompletionWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

void CompletionWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  // no-op
}

shared_ptr<Explanation>
CompletionWeight::explain(shared_ptr<LeafReaderContext> context,
                          int doc) 
{
  // TODO
  return nullptr;
}
} // namespace org::apache::lucene::search::suggest::document