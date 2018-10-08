using namespace std;

#include "ThreadInterruptedException.h"

namespace org::apache::lucene::util
{

ThreadInterruptedException::ThreadInterruptedException(
    shared_ptr<InterruptedException> ie)
    : RuntimeException(ie)
{
}
} // namespace org::apache::lucene::util