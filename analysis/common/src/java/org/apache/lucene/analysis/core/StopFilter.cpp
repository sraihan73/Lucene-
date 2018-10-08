using namespace std;

#include "StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::core
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;

StopFilter::StopFilter(shared_ptr<TokenStream> in_,
                       shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::StopFilter(in_, stopWords)
{
}
} // namespace org::apache::lucene::analysis::core