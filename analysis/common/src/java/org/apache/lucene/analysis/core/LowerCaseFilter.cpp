using namespace std;

#include "LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::core
{
using TokenStream = org::apache::lucene::analysis::TokenStream;

LowerCaseFilter::LowerCaseFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::LowerCaseFilter(in_)
{
}
} // namespace org::apache::lucene::analysis::core