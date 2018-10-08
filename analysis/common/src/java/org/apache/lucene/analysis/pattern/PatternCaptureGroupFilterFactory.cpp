using namespace std;

#include "PatternCaptureGroupFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "PatternCaptureGroupTokenFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PatternCaptureGroupFilterFactory::PatternCaptureGroupFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  pattern = getPattern(args, L"pattern");
  preserveOriginal =
      args.find(L"preserve_original") != args.end()
          ? StringHelper::fromString<bool>(args[L"preserve_original"])
          : true;
}

shared_ptr<PatternCaptureGroupTokenFilter>
PatternCaptureGroupFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PatternCaptureGroupTokenFilter>(input, preserveOriginal,
                                                     pattern);
}
} // namespace org::apache::lucene::analysis::pattern