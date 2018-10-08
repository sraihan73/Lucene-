using namespace std;

#include "LockReleaseFailedException.h"

namespace org::apache::lucene::store
{

LockReleaseFailedException::LockReleaseFailedException(const wstring &message)
    : java::io::IOException(message)
{
}

LockReleaseFailedException::LockReleaseFailedException(const wstring &message,
                                                       runtime_error cause)
    : java::io::IOException(message, cause)
{
}
} // namespace org::apache::lucene::store