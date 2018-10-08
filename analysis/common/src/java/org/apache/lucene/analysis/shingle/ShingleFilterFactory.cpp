using namespace std;

#include "ShingleFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ShingleFilter.h"

namespace org::apache::lucene::analysis::shingle
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ShingleFilterFactory::ShingleFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      minShingleSize(getInt(args, L"minShingleSize",
                            ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE)),
      maxShingleSize(getInt(args, L"maxShingleSize",
                            ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE)),
      outputUnigrams(getBoolean(args, L"outputUnigrams", true)),
      outputUnigramsIfNoShingles(
          getBoolean(args, L"outputUnigramsIfNoShingles", false)),
      tokenSeparator(
          get(args, L"tokenSeparator", ShingleFilter::DEFAULT_TOKEN_SEPARATOR)),
      fillerToken(
          get(args, L"fillerToken", ShingleFilter::DEFAULT_FILLER_TOKEN))
{
  if (maxShingleSize < 2) {
    throw invalid_argument(L"Invalid maxShingleSize (" +
                           to_wstring(maxShingleSize) +
                           L") - must be at least 2");
  }
  if (minShingleSize < 2) {
    throw invalid_argument(L"Invalid minShingleSize (" +
                           to_wstring(minShingleSize) +
                           L") - must be at least 2");
  }
  if (minShingleSize > maxShingleSize) {
    throw invalid_argument(L"Invalid minShingleSize (" +
                           to_wstring(minShingleSize) +
                           L") - must be no greater than maxShingleSize (" +
                           to_wstring(maxShingleSize) + L")");
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ShingleFilter>
ShingleFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<ShingleFilter> r =
      make_shared<ShingleFilter>(input, minShingleSize, maxShingleSize);
  r->setOutputUnigrams(outputUnigrams);
  r->setOutputUnigramsIfNoShingles(outputUnigramsIfNoShingles);
  r->setTokenSeparator(tokenSeparator);
  r->setFillerToken(fillerToken);
  return r;
}
} // namespace org::apache::lucene::analysis::shingle