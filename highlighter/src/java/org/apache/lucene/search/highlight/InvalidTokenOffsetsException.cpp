using namespace std;

#include "InvalidTokenOffsetsException.h"

namespace org::apache::lucene::search::highlight
{

InvalidTokenOffsetsException::InvalidTokenOffsetsException(
    const wstring &message)
    : Exception(message)
{
}
} // namespace org::apache::lucene::search::highlight