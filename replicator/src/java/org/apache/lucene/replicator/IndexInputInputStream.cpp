using namespace std;

#include "IndexInputInputStream.h"

namespace org::apache::lucene::replicator
{
using IndexInput = org::apache::lucene::store::IndexInput;

IndexInputInputStream::IndexInputInputStream(shared_ptr<IndexInput> in_)
    : in_(in_)
{
  remaining = in_->length();
}

int IndexInputInputStream::read() 
{
  if (remaining == 0) {
    return -1;
  } else {
    --remaining;
    return in_->readByte();
  }
}

int IndexInputInputStream::available() 
{
  return static_cast<int>(in_->length());
}

IndexInputInputStream::~IndexInputInputStream() { delete in_; }

int IndexInputInputStream::read(std::deque<char> &b) 
{
  return read(b, 0, b.size());
}

int IndexInputInputStream::read(std::deque<char> &b, int off,
                                int len) 
{
  if (remaining == 0) {
    return -1;
  }
  if (remaining < len) {
    len = static_cast<int>(remaining);
  }
  in_->readBytes(b, off, len);
  remaining -= len;
  return len;
}

int64_t IndexInputInputStream::skip(int64_t n) 
{
  if (remaining == 0) {
    return -1;
  }
  if (remaining < n) {
    n = remaining;
  }
  in_->seek(in_->getFilePointer() + n);
  remaining -= n;
  return n;
}
} // namespace org::apache::lucene::replicator