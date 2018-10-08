using namespace std;

#include "SessionExpiredException.h"

namespace org::apache::lucene::replicator
{

SessionExpiredException::SessionExpiredException(const wstring &message,
                                                 runtime_error cause)
    : java::io::IOException(message, cause)
{
}

SessionExpiredException::SessionExpiredException(const wstring &message)
    : java::io::IOException(message)
{
}

SessionExpiredException::SessionExpiredException(runtime_error cause)
    : java::io::IOException(cause)
{
}
} // namespace org::apache::lucene::replicator