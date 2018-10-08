using namespace std;

#include "ThrottledIndexOutput.h"

namespace org::apache::lucene::util
{
using DataInput = org::apache::lucene::store::DataInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

shared_ptr<ThrottledIndexOutput>
ThrottledIndexOutput::newFromDelegate(shared_ptr<IndexOutput> output)
{
  return make_shared<ThrottledIndexOutput>(bytesPerSecond, flushDelayMillis,
                                           closeDelayMillis, seekDelayMillis,
                                           minBytesWritten, output);
}

ThrottledIndexOutput::ThrottledIndexOutput(int bytesPerSecond,
                                           int64_t delayInMillis,
                                           shared_ptr<IndexOutput> delegate_)
    : ThrottledIndexOutput(bytesPerSecond, delayInMillis, delayInMillis,
                           delayInMillis, DEFAULT_MIN_WRITTEN_BYTES, delegate_)
{
}

ThrottledIndexOutput::ThrottledIndexOutput(int bytesPerSecond, int64_t delays,
                                           int minBytesWritten,
                                           shared_ptr<IndexOutput> delegate_)
    : ThrottledIndexOutput(bytesPerSecond, delays, delays, delays,
                           minBytesWritten, delegate_)
{
}

int ThrottledIndexOutput::mBitsToBytes(int mbits) { return mbits * 125000000; }

ThrottledIndexOutput::ThrottledIndexOutput(int bytesPerSecond,
                                           int64_t flushDelayMillis,
                                           int64_t closeDelayMillis,
                                           int64_t seekDelayMillis,
                                           int64_t minBytesWritten,
                                           shared_ptr<IndexOutput> delegate_)
    : org::apache::lucene::store::IndexOutput(
          L"ThrottledIndexOutput(" + delegate_ + L")",
          delegate_ == nullptr ? L"n/a" : delegate_->getName()),
      bytesPerSecond(bytesPerSecond)
{
  assert(bytesPerSecond > 0);
  this->delegate_ = delegate_;
  this->flushDelayMillis = flushDelayMillis;
  this->closeDelayMillis = closeDelayMillis;
  this->seekDelayMillis = seekDelayMillis;
  this->minBytesWritten = minBytesWritten;
}

ThrottledIndexOutput::~ThrottledIndexOutput()
{
  try {
    sleep(closeDelayMillis + getDelay(true));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete delegate_;
  }
}

int64_t ThrottledIndexOutput::getFilePointer()
{
  return delegate_->getFilePointer();
}

void ThrottledIndexOutput::writeByte(char b) 
{
  bytes[0] = b;
  writeBytes(bytes, 0, 1);
}

void ThrottledIndexOutput::writeBytes(std::deque<char> &b, int offset,
                                      int length) 
{
  constexpr int64_t before = System::nanoTime();
  // TODO: sometimes, write only half the bytes, then
  // sleep, then 2nd half, then sleep, so we sometimes
  // interrupt having only written not all bytes
  delegate_->writeBytes(b, offset, length);
  timeElapsed += System::nanoTime() - before;
  pendingBytes += length;
  sleep(getDelay(false));
}

int64_t ThrottledIndexOutput::getDelay(bool closing)
{
  if (pendingBytes > 0 && (closing || pendingBytes > minBytesWritten)) {
    int64_t actualBps =
        (timeElapsed / pendingBytes) * 1000000000LL; // nano to sec
    if (actualBps > bytesPerSecond) {
      int64_t expected = (pendingBytes * 1000LL / bytesPerSecond);
      constexpr int64_t delay = expected - (timeElapsed / 1000000LL);
      pendingBytes = 0;
      timeElapsed = 0;
      return delay;
    }
  }
  return 0;
}

void ThrottledIndexOutput::sleep(int64_t ms)
{
  if (ms <= 0) {
    return;
  }
  try {
    delay(ms);
  } catch (const InterruptedException &e) {
    throw make_shared<ThreadInterruptedException>(e);
  }
}

void ThrottledIndexOutput::copyBytes(shared_ptr<DataInput> input,
                                     int64_t numBytes) 
{
  delegate_->copyBytes(input, numBytes);
}

int64_t ThrottledIndexOutput::getChecksum() 
{
  return delegate_->getChecksum();
}
} // namespace org::apache::lucene::util