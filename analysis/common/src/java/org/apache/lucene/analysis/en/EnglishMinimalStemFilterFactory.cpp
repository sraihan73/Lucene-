using namespace std;

#include "EnglishMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "EnglishMinimalStemFilter.h"

namespace org::apache::lucene::analysis::en
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using EnglishMinimalStemFilter =
    org::apache::lucene::analysis::en::EnglishMinimalStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

EnglishMinimalStemFilterFactory::EnglishMinimalStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
EnglishMinimalStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<EnglishMinimalStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::en