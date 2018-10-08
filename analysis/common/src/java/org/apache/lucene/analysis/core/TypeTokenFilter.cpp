using namespace std;

#include "TypeTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

namespace org::apache::lucene::analysis::core
{
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

TypeTokenFilter::TypeTokenFilter(shared_ptr<TokenStream> input,
                                 shared_ptr<Set<wstring>> stopTypes,
                                 bool useWhiteList)
    : org::apache::lucene::analysis::FilteringTokenFilter(input),
      stopTypes(stopTypes), useWhiteList(useWhiteList)
{
}

TypeTokenFilter::TypeTokenFilter(shared_ptr<TokenStream> input,
                                 shared_ptr<Set<wstring>> stopTypes)
    : TypeTokenFilter(input, stopTypes, false)
{
}

bool TypeTokenFilter::accept()
{
  return useWhiteList == stopTypes->contains(typeAttribute->type());
}
} // namespace org::apache::lucene::analysis::core