using namespace std;

#include "StempelPolishStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../pl/PolishAnalyzer.h"
#include "StempelFilter.h"
#include "StempelStemmer.h"

namespace org::apache::lucene::analysis::stempel
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PolishAnalyzer = org::apache::lucene::analysis::pl::PolishAnalyzer;
using StempelFilter = org::apache::lucene::analysis::stempel::StempelFilter;
using StempelStemmer = org::apache::lucene::analysis::stempel::StempelStemmer;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

StempelPolishStemFilterFactory::StempelPolishStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
StempelPolishStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<StempelFilter>(
      input, make_shared<StempelStemmer>(PolishAnalyzer::getDefaultTable()));
}
} // namespace org::apache::lucene::analysis::stempel