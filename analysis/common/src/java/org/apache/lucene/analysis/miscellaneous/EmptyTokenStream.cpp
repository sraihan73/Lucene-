using namespace std;

#include "EmptyTokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;

bool EmptyTokenStream::incrementToken() { return false; }
} // namespace org::apache::lucene::analysis::miscellaneous