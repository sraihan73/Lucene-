using namespace std;

#include "Completion50PostingsFormat.h"

namespace org::apache::lucene::search::suggest::document
{
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;

Completion50PostingsFormat::Completion50PostingsFormat()
    : CompletionPostingsFormat()
{
}

shared_ptr<PostingsFormat> Completion50PostingsFormat::delegatePostingsFormat()
{
  return PostingsFormat::forName(L"Lucene50");
}
} // namespace org::apache::lucene::search::suggest::document