using namespace std;

#include "LockObtainFailedException.h"

namespace org::apache::lucene::store
{

LockObtainFailedException::LockObtainFailedException(const wstring &message)
    : java::io::IOException(message)
{
}

LockObtainFailedException::LockObtainFailedException(const wstring &message,
                                                     runtime_error cause)
    : java::io::IOException(message, cause)
{
}
} // namespace org::apache::lucene::store