using namespace std;

#include "CommonGramsQueryFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "CommonGramsFilter.h"
#include "CommonGramsQueryFilter.h"

namespace org::apache::lucene::analysis::commongrams
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CommonGramsFilter =
    org::apache::lucene::analysis::commongrams::CommonGramsFilter;
using CommonGramsQueryFilter =
    org::apache::lucene::analysis::commongrams::CommonGramsQueryFilter;

CommonGramsQueryFilterFactory::CommonGramsQueryFilterFactory(
    unordered_map<wstring, wstring> &args)
    : CommonGramsFilterFactory(args)
{
}

shared_ptr<TokenFilter>
CommonGramsQueryFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<CommonGramsFilter> commonGrams =
      std::static_pointer_cast<CommonGramsFilter>(
          CommonGramsFilterFactory::create(input));
  return make_shared<CommonGramsQueryFilter>(commonGrams);
}
} // namespace org::apache::lucene::analysis::commongrams