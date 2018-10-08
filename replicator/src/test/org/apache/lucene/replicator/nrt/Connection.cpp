using namespace std;

#include "Connection.h"

namespace org::apache::lucene::replicator::nrt
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;

Connection::Connection(int tcpPort) 
    : in_(make_shared<InputStreamDataInput>(sockIn)),
      out(make_shared<OutputStreamDataOutput>(bos)),
      sockIn(s->getInputStream()),
      bos(make_shared<BufferedOutputStream>(s->getOutputStream())),
      s(make_shared<Socket>(InetAddress::getLoopbackAddress(), tcpPort)),
      destTCPPort(tcpPort)
{
  if (Node::VERBOSE_CONNECTIONS) {
    wcout << L"make new client Connection socket=" << this->s << L" destPort="
          << tcpPort << endl;
  }
}

void Connection::flush()  { bos->flush(); }

Connection::~Connection() { s->close(); }
} // namespace org::apache::lucene::replicator::nrt