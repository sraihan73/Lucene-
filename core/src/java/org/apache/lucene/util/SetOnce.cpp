using namespace std;

#include "SetOnce.h"

namespace org::apache::lucene::util
{

SetOnce<T>::AlreadySetException::AlreadySetException()
    : IllegalStateException(L"The object cannot be set twice!")
{
}
} // namespace org::apache::lucene::util