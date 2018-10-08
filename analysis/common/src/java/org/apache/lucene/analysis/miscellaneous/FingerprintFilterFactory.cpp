using namespace std;

#include "FingerprintFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FingerprintFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring FingerprintFilterFactory::MAX_OUTPUT_TOKEN_SIZE_KEY =
    L"maxOutputTokenSize";
const wstring FingerprintFilterFactory::SEPARATOR_KEY = L"separator";

FingerprintFilterFactory::FingerprintFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      maxOutputTokenSize(
          getInt(args, MAX_OUTPUT_TOKEN_SIZE_KEY,
                 FingerprintFilter::DEFAULT_MAX_OUTPUT_TOKEN_SIZE)),
      separator(
          getChar(args, SEPARATOR_KEY, FingerprintFilter::DEFAULT_SEPARATOR))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
FingerprintFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FingerprintFilter>(input, maxOutputTokenSize, separator);
}
} // namespace org::apache::lucene::analysis::miscellaneous