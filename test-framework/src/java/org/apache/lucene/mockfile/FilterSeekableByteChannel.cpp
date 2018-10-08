using namespace std;

#include "FilterSeekableByteChannel.h"

namespace org::apache::lucene::mockfile
{

FilterSeekableByteChannel::FilterSeekableByteChannel(
    shared_ptr<SeekableByteChannel> delegate_)
    : delegate_(delegate_)
{
}

bool FilterSeekableByteChannel::isOpen() { return delegate_->isOpen(); }

FilterSeekableByteChannel::~FilterSeekableByteChannel() { delegate_->close(); }

int FilterSeekableByteChannel::read(shared_ptr<ByteBuffer> dst) throw(
    IOException)
{
  return delegate_->read(dst);
}

int FilterSeekableByteChannel::write(shared_ptr<ByteBuffer> src) throw(
    IOException)
{
  return delegate_->write(src);
}

int64_t FilterSeekableByteChannel::position() 
{
  return delegate_->position();
}

shared_ptr<SeekableByteChannel>
FilterSeekableByteChannel::position(int64_t newPosition) 
{
  delegate_->position(newPosition);
  return shared_from_this();
}

int64_t FilterSeekableByteChannel::size() 
{
  return delegate_->size();
}

shared_ptr<SeekableByteChannel>
FilterSeekableByteChannel::truncate(int64_t size) 
{
  delegate_->truncate(size);
  return shared_from_this();
}
} // namespace org::apache::lucene::mockfile