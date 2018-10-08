using namespace std;

#include "ConcatenateGraphFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ConcatenateGraphFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TooComplexToDeterminizeException =
    org::apache::lucene::util::automaton::TooComplexToDeterminizeException;

ConcatenateGraphFilterFactory::ConcatenateGraphFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{

  preserveSep = getBoolean(args, L"preserveSep",
                           ConcatenateGraphFilter::DEFAULT_PRESERVE_SEP);
  preservePositionIncrements =
      getBoolean(args, L"preservePositionIncrements",
                 ConcatenateGraphFilter::DEFAULT_PRESERVE_POSITION_INCREMENTS);
  maxGraphExpansions =
      getInt(args, L"maxGraphExpansions",
             ConcatenateGraphFilter::DEFAULT_MAX_GRAPH_EXPANSIONS);

  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
ConcatenateGraphFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ConcatenateGraphFilter>(
      input, preserveSep, preservePositionIncrements, maxGraphExpansions);
}
} // namespace org::apache::lucene::analysis::miscellaneous