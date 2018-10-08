using namespace std;

#include "JapaneseBaseFormFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "JapaneseBaseFormFilter.h"

namespace org::apache::lucene::analysis::ja
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using JapaneseBaseFormFilter =
    org::apache::lucene::analysis::ja::JapaneseBaseFormFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

JapaneseBaseFormFilterFactory::JapaneseBaseFormFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
JapaneseBaseFormFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<JapaneseBaseFormFilter>(input);
}
} // namespace org::apache::lucene::analysis::ja