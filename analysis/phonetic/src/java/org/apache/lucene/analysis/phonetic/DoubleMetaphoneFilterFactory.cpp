using namespace std;

#include "DoubleMetaphoneFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "DoubleMetaphoneFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using DoubleMetaphoneFilter =
    org::apache::lucene::analysis::phonetic::DoubleMetaphoneFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring DoubleMetaphoneFilterFactory::INJECT = L"inject";
const wstring DoubleMetaphoneFilterFactory::MAX_CODE_LENGTH = L"maxCodeLength";

DoubleMetaphoneFilterFactory::DoubleMetaphoneFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      inject(getBoolean(args, INJECT, true)),
      maxCodeLength(getInt(args, MAX_CODE_LENGTH, DEFAULT_MAX_CODE_LENGTH))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<DoubleMetaphoneFilter>
DoubleMetaphoneFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<DoubleMetaphoneFilter>(input, maxCodeLength, inject);
}
} // namespace org::apache::lucene::analysis::phonetic