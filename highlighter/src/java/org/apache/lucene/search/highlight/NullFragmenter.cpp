using namespace std;

#include "NullFragmenter.h"

namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;

void NullFragmenter::start(const wstring &s,
                           shared_ptr<TokenStream> tokenStream)
{
}

bool NullFragmenter::isNewFragment() { return false; }
} // namespace org::apache::lucene::search::highlight