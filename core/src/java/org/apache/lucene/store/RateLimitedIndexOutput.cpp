using namespace std;

#include "RateLimitedIndexOutput.h"

namespace org::apache::lucene::store
{

RateLimitedIndexOutput::RateLimitedIndexOutput(
    shared_ptr<RateLimiter> rateLimiter, shared_ptr<IndexOutput> delegate_)
    : IndexOutput(L"RateLimitedIndexOutput(" + delegate_ + L")",
                  delegate_->getName()),
      delegate_(delegate_), rateLimiter(rateLimiter)
{
  this->currentMinPauseCheckBytes = rateLimiter->getMinPauseCheckBytes();
}

RateLimitedIndexOutput::~RateLimitedIndexOutput() { delete delegate_; }

int64_t RateLimitedIndexOutput::getFilePointer()
{
  return delegate_->getFilePointer();
}

int64_t RateLimitedIndexOutput::getChecksum() 
{
  return delegate_->getChecksum();
}

void RateLimitedIndexOutput::writeByte(char b) 
{
  bytesSinceLastPause++;
  checkRate();
  delegate_->writeByte(b);
}

void RateLimitedIndexOutput::writeBytes(std::deque<char> &b, int offset,
                                        int length) 
{
  bytesSinceLastPause += length;
  checkRate();
  delegate_->writeBytes(b, offset, length);
}

void RateLimitedIndexOutput::checkRate() 
{
  if (bytesSinceLastPause > currentMinPauseCheckBytes) {
    rateLimiter->pause(bytesSinceLastPause);
    bytesSinceLastPause = 0;
    currentMinPauseCheckBytes = rateLimiter->getMinPauseCheckBytes();
  }
}
} // namespace org::apache::lucene::store