using namespace std;

#include "PrefixCompletionQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;

PrefixCompletionQuery::PrefixCompletionQuery(shared_ptr<Analyzer> analyzer,
                                             shared_ptr<Term> term)
    : PrefixCompletionQuery(analyzer, term, nullptr)
{
}

PrefixCompletionQuery::PrefixCompletionQuery(shared_ptr<Analyzer> analyzer,
                                             shared_ptr<Term> term,
                                             shared_ptr<BitsProducer> filter)
    : CompletionQuery(term, filter)
{
  if (!(std::dynamic_pointer_cast<CompletionAnalyzer>(analyzer) != nullptr)) {
    this->analyzer = make_shared<CompletionAnalyzer>(analyzer);
  } else {
    this->analyzer = std::static_pointer_cast<CompletionAnalyzer>(analyzer);
  }
}

shared_ptr<Weight>
PrefixCompletionQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (CompletionTokenStream stream =
  // (CompletionTokenStream) analyzer.tokenStream(getField(), getTerm().text()))
  {
    CompletionTokenStream stream =
        std::static_pointer_cast<CompletionTokenStream>(
            analyzer->tokenStream(getField(), getTerm()->text()));
    return make_shared<CompletionWeight>(shared_from_this(),
                                         stream->toAutomaton());
  }
}

shared_ptr<Analyzer> PrefixCompletionQuery::getAnalyzer() { return analyzer; }

bool PrefixCompletionQuery::equals(any o)
{
  throw make_shared<UnsupportedOperationException>();
}

int PrefixCompletionQuery::hashCode()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::search::suggest::document