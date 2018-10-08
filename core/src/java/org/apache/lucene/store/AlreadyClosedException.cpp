using namespace std;

#include "AlreadyClosedException.h"

namespace org::apache::lucene::store
{

AlreadyClosedException::AlreadyClosedException(const wstring &message)
    : IllegalStateException(message)
{
}

AlreadyClosedException::AlreadyClosedException(const wstring &message,
                                               runtime_error cause)
    : IllegalStateException(message, cause)
{
}
} // namespace org::apache::lucene::store