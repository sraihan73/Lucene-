using namespace std;

#include "BufferedChecksum.h"

namespace org::apache::lucene::store
{

BufferedChecksum::BufferedChecksum(shared_ptr<Checksum> in_)
    : BufferedChecksum(in_, DEFAULT_BUFFERSIZE)
{
}

BufferedChecksum::BufferedChecksum(shared_ptr<Checksum> in_, int bufferSize)
    : in_(in_), buffer(std::deque<char>(bufferSize))
{
}

void BufferedChecksum::update(int b)
{
  if (upto == buffer.size()) {
    flush();
  }
  buffer[upto++] = static_cast<char>(b);
}

void BufferedChecksum::update(std::deque<char> &b, int off, int len)
{
  if (len >= buffer.size()) {
    flush();
    in_->update(b, off, len);
  } else {
    if (upto + len > buffer.size()) {
      flush();
    }
    System::arraycopy(b, off, buffer, upto, len);
    upto += len;
  }
}

int64_t BufferedChecksum::getValue()
{
  flush();
  return in_->getValue();
}

void BufferedChecksum::reset()
{
  upto = 0;
  in_->reset();
}

void BufferedChecksum::flush()
{
  if (upto > 0) {
    in_->update(buffer, 0, upto);
  }
  upto = 0;
}
} // namespace org::apache::lucene::store