using namespace std;

#include "CompletionAnalyzer.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;

CompletionAnalyzer::CompletionAnalyzer(shared_ptr<Analyzer> analyzer,
                                       bool preserveSep,
                                       bool preservePositionIncrements,
                                       int maxGraphExpansions)
    : org::apache::lucene::analysis::AnalyzerWrapper(PER_FIELD_REUSE_STRATEGY),
      analyzer(analyzer), preserveSep(preserveSep),
      preservePositionIncrements(preservePositionIncrements),
      maxGraphExpansions(maxGraphExpansions)
{
}

CompletionAnalyzer::CompletionAnalyzer(shared_ptr<Analyzer> analyzer)
    : CompletionAnalyzer(
          analyzer, ConcatenateGraphFilter::DEFAULT_PRESERVE_SEP,
          ConcatenateGraphFilter::DEFAULT_PRESERVE_POSITION_INCREMENTS,
          ConcatenateGraphFilter::DEFAULT_MAX_GRAPH_EXPANSIONS)
{
}

CompletionAnalyzer::CompletionAnalyzer(shared_ptr<Analyzer> analyzer,
                                       bool preserveSep,
                                       bool preservePositionIncrements)
    : CompletionAnalyzer(analyzer, preserveSep, preservePositionIncrements,
                         ConcatenateGraphFilter::DEFAULT_MAX_GRAPH_EXPANSIONS)
{
}

CompletionAnalyzer::CompletionAnalyzer(shared_ptr<Analyzer> analyzer,
                                       int maxGraphExpansions)
    : CompletionAnalyzer(
          analyzer, ConcatenateGraphFilter::DEFAULT_PRESERVE_SEP,
          ConcatenateGraphFilter::DEFAULT_PRESERVE_POSITION_INCREMENTS,
          maxGraphExpansions)
{
}

bool CompletionAnalyzer::preserveSep() { return preserveSep_; }

bool CompletionAnalyzer::preservePositionIncrements()
{
  return preservePositionIncrements_;
}

shared_ptr<Analyzer>
CompletionAnalyzer::getWrappedAnalyzer(const wstring &fieldName)
{
  return analyzer;
}

shared_ptr<Analyzer::TokenStreamComponents> CompletionAnalyzer::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  shared_ptr<CompletionTokenStream> tokenStream =
      make_shared<CompletionTokenStream>(
          components->getTokenStream(), preserveSep_,
          preservePositionIncrements_, maxGraphExpansions);
  return make_shared<Analyzer::TokenStreamComponents>(
      components->getTokenizer(), tokenStream);
}
} // namespace org::apache::lucene::search::suggest::document