using namespace std;

#include "FilterAsynchronousFileChannel.h"

namespace org::apache::lucene::mockfile
{

FilterAsynchronousFileChannel::FilterAsynchronousFileChannel(
    shared_ptr<AsynchronousFileChannel> delegate_)
    : delegate_(Objects::requireNonNull(delegate_))
{
}

FilterAsynchronousFileChannel::~FilterAsynchronousFileChannel()
{
  delegate_->close();
}

bool FilterAsynchronousFileChannel::isOpen() { return delegate_->isOpen(); }

int64_t FilterAsynchronousFileChannel::size() 
{
  return delegate_->size();
}

shared_ptr<AsynchronousFileChannel>
FilterAsynchronousFileChannel::truncate(int64_t size) 
{
  delegate_->truncate(size);
  return shared_from_this();
}

void FilterAsynchronousFileChannel::force(bool metaData) 
{
  delegate_->force(metaData);
}

template <typename A, typename T1>
void FilterAsynchronousFileChannel::lock(
    int64_t position, int64_t size, bool shared, A attachment,
    shared_ptr<CompletionHandler<T1>> handler)
{
  delegate_->lock(position, size, shared, attachment, handler);
}

shared_ptr<Future<std::shared_ptr<FileLock>>>
FilterAsynchronousFileChannel::lock(int64_t position, int64_t size,
                                    bool shared)
{
  return delegate_->lock(position, size, shared);
}

shared_ptr<FileLock>
FilterAsynchronousFileChannel::tryLock(int64_t position, int64_t size,
                                       bool shared) 
{
  return delegate_->tryLock(position, size, shared);
}

template <typename A, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: @Override public <A> void read(java.nio.ByteBuffer dst, long
// position, A attachment, java.nio.channels.CompletionHandler<int,? super A>
// handler)
void FilterAsynchronousFileChannel::read(
    shared_ptr<ByteBuffer> dst, int64_t position, A attachment,
    shared_ptr<CompletionHandler<T1>> handler)
{
  delegate_->read(dst, position, attachment, handler);
}

shared_ptr<Future<int>>
FilterAsynchronousFileChannel::read(shared_ptr<ByteBuffer> dst,
                                    int64_t position)
{
  return delegate_->read(dst, position);
}

template <typename A, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: @Override public <A> void write(java.nio.ByteBuffer src, long
// position, A attachment, java.nio.channels.CompletionHandler<int,? super A>
// handler)
void FilterAsynchronousFileChannel::write(
    shared_ptr<ByteBuffer> src, int64_t position, A attachment,
    shared_ptr<CompletionHandler<T1>> handler)
{
  delegate_->write(src, position, attachment, handler);
}

shared_ptr<Future<int>>
FilterAsynchronousFileChannel::write(shared_ptr<ByteBuffer> src,
                                     int64_t position)
{
  return delegate_->write(src, position);
}
} // namespace org::apache::lucene::mockfile