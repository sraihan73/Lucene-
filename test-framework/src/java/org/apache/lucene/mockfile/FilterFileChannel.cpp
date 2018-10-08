using namespace std;

#include "FilterFileChannel.h"

namespace org::apache::lucene::mockfile
{

FilterFileChannel::FilterFileChannel(shared_ptr<FileChannel> delegate_)
    : delegate_(Objects::requireNonNull(delegate_))
{
}

int FilterFileChannel::read(shared_ptr<ByteBuffer> dst) 
{
  return delegate_->read(dst);
}

int64_t
FilterFileChannel::read(std::deque<std::shared_ptr<ByteBuffer>> &dsts,
                        int offset, int length) 
{
  return delegate_->read(dsts, offset, length);
}

int FilterFileChannel::write(shared_ptr<ByteBuffer> src) 
{
  return delegate_->write(src);
}

int64_t
FilterFileChannel::write(std::deque<std::shared_ptr<ByteBuffer>> &srcs,
                         int offset, int length) 
{
  return delegate_->write(srcs, offset, length);
}

int64_t FilterFileChannel::position() 
{
  return delegate_->position();
}

shared_ptr<FileChannel>
FilterFileChannel::position(int64_t newPosition) 
{
  delegate_->position(newPosition);
  return shared_from_this();
}

int64_t FilterFileChannel::size() 
{
  return delegate_->size();
}

shared_ptr<FileChannel>
FilterFileChannel::truncate(int64_t size) 
{
  delegate_->truncate(size);
  return shared_from_this();
}

void FilterFileChannel::force(bool metaData) 
{
  delegate_->force(metaData);
}

int64_t FilterFileChannel::transferTo(
    int64_t position, int64_t count,
    shared_ptr<WritableByteChannel> target) 
{
  return delegate_->transferTo(position, count, target);
}

int64_t FilterFileChannel::transferFrom(shared_ptr<ReadableByteChannel> src,
                                          int64_t position,
                                          int64_t count) 
{
  return delegate_->transferFrom(src, position, count);
}

int FilterFileChannel::read(shared_ptr<ByteBuffer> dst,
                            int64_t position) 
{
  return delegate_->read(dst, position);
}

int FilterFileChannel::write(shared_ptr<ByteBuffer> src,
                             int64_t position) 
{
  return delegate_->write(src, position);
}

shared_ptr<MappedByteBuffer>
FilterFileChannel::map_obj(shared_ptr<MapMode> mode, int64_t position,
                       int64_t size) 
{
  return delegate_->map_obj(mode, position, size);
}

shared_ptr<FileLock> FilterFileChannel::lock(int64_t position, int64_t size,
                                             bool shared) 
{
  return delegate_->lock(position, size, shared);
}

shared_ptr<FileLock> FilterFileChannel::tryLock(int64_t position,
                                                int64_t size,
                                                bool shared) 
{
  return delegate_->tryLock(position, size, shared);
}

void FilterFileChannel::implCloseChannel() 
{
  // we can't call implCloseChannel, but calling this instead is "ok":
  // http://mail.openjdk.java.net/pipermail/nio-dev/2015-September/003322.html
  delegate_->close();
}
} // namespace org::apache::lucene::mockfile