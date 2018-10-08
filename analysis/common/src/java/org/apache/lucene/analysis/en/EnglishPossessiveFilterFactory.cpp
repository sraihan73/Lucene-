using namespace std;

#include "EnglishPossessiveFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "EnglishPossessiveFilter.h"

namespace org::apache::lucene::analysis::en
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using EnglishPossessiveFilter =
    org::apache::lucene::analysis::en::EnglishPossessiveFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

EnglishPossessiveFilterFactory::EnglishPossessiveFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
EnglishPossessiveFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<EnglishPossessiveFilter>(input);
}
} // namespace org::apache::lucene::analysis::en