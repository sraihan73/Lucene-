using namespace std;

#include "CompletionScorer.h"

namespace org::apache::lucene::search::suggest::document
{
using LeafReader = org::apache::lucene::index::LeafReader;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Bits = org::apache::lucene::util::Bits;
using Automaton = org::apache::lucene::util::automaton::Automaton;

CompletionScorer::CompletionScorer(
    shared_ptr<CompletionWeight> weight, shared_ptr<NRTSuggester> suggester,
    shared_ptr<LeafReader> reader, shared_ptr<Bits> filterDocs,
    bool const filtered, shared_ptr<Automaton> automaton) 
    : suggester(suggester), filterDocs(filterDocs), weight(weight),
      reader(reader), filtered(filtered), automaton(automaton)
{
}

int CompletionScorer::score(shared_ptr<LeafCollector> collector,
                            shared_ptr<Bits> acceptDocs, int min,
                            int max) 
{
  if (!(std::dynamic_pointer_cast<TopSuggestDocsCollector>(collector) !=
        nullptr)) {
    throw invalid_argument(L"collector is not of type TopSuggestDocsCollector");
  }
  suggester->lookup(
      shared_from_this(), acceptDocs,
      (std::static_pointer_cast<TopSuggestDocsCollector>(collector)));
  return max;
}

int64_t CompletionScorer::cost() { return 0; }

bool CompletionScorer::accept(int docID, shared_ptr<Bits> liveDocs)
{
  return (filterDocs == nullptr || filterDocs->get(docID)) &&
         (liveDocs == nullptr || liveDocs->get(docID));
}

float CompletionScorer::score(float weight, float boost)
{
  if (boost == 0.0f) {
    return weight;
  }
  if (weight == 0.0f) {
    return boost;
  }
  return weight * boost;
}
} // namespace org::apache::lucene::search::suggest::document