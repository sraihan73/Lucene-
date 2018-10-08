using namespace std;

#include "PrintStreamInfoStream.h"

namespace org::apache::lucene::util
{

const shared_ptr<java::util::concurrent::atomic::AtomicInteger>
    PrintStreamInfoStream::MESSAGE_ID =
        make_shared<java::util::concurrent::atomic::AtomicInteger>();

PrintStreamInfoStream::PrintStreamInfoStream(shared_ptr<PrintStream> stream)
    : PrintStreamInfoStream(stream, MESSAGE_ID->getAndIncrement())
{
}

PrintStreamInfoStream::PrintStreamInfoStream(shared_ptr<PrintStream> stream,
                                             int messageID)
    : messageID(messageID), stream(stream)
{
}

void PrintStreamInfoStream::message(const wstring &component,
                                    const wstring &message)
{
  stream->println(component + L" " + to_wstring(messageID) + L" [" +
                  getTimestamp() + L"; " + Thread::currentThread().getName() +
                  L"]: " + message);
}

bool PrintStreamInfoStream::isEnabled(const wstring &component) { return true; }

PrintStreamInfoStream::~PrintStreamInfoStream()
{
  if (!isSystemStream()) {
    stream->close();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out/err detection") public
// bool isSystemStream()
bool PrintStreamInfoStream::isSystemStream()
{
  return stream == System::out || stream == System::err;
}

wstring PrintStreamInfoStream::getTimestamp()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Instant::now()->toString();
}
} // namespace org::apache::lucene::util