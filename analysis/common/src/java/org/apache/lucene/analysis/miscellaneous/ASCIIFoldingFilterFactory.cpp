using namespace std;

#include "ASCIIFoldingFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/AbstractAnalysisFactory.h"
#include "ASCIIFoldingFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using ASCIIFoldingFilter =
    org::apache::lucene::analysis::miscellaneous::ASCIIFoldingFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
const wstring ASCIIFoldingFilterFactory::PRESERVE_ORIGINAL =
    L"preserveOriginal";

ASCIIFoldingFilterFactory::ASCIIFoldingFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      preserveOriginal(getBoolean(args, PRESERVE_ORIGINAL, false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ASCIIFoldingFilter>
ASCIIFoldingFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ASCIIFoldingFilter>(input, preserveOriginal);
}

shared_ptr<AbstractAnalysisFactory>
ASCIIFoldingFilterFactory::getMultiTermComponent()
{
  if (preserveOriginal) {
    // The main use-case for using preserveOriginal is to match regardless of
    // case but to give better scores to exact matches. Since most multi-term
    // queries return constant scores anyway, the multi-term component only
    // emits the folded token
    unordered_map<wstring, wstring> args =
        unordered_map<wstring, wstring>(getOriginalArgs());
    args.erase(PRESERVE_ORIGINAL);
    return make_shared<ASCIIFoldingFilterFactory>(args);
  } else {
    return shared_from_this();
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous