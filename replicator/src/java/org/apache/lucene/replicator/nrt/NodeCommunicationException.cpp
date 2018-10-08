using namespace std;

#include "NodeCommunicationException.h"

namespace org::apache::lucene::replicator::nrt
{

NodeCommunicationException::NodeCommunicationException(const wstring &when,
                                                       runtime_error cause)
    : RuntimeException(when)
{
  assert(cause != nullptr);
  initCause(cause);
}
} // namespace org::apache::lucene::replicator::nrt