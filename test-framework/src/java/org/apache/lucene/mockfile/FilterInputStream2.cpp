using namespace std;

#include "FilterInputStream2.h"

namespace org::apache::lucene::mockfile
{

FilterInputStream2::FilterInputStream2(shared_ptr<InputStream> delegate_)
    : delegate_(Objects::requireNonNull(delegate_))
{
}

int FilterInputStream2::read()  { return delegate_->read(); }

int FilterInputStream2::read(std::deque<char> &b) 
{
  return delegate_->read(b);
}

int FilterInputStream2::read(std::deque<char> &b, int off,
                             int len) 
{
  return delegate_->read(b, off, len);
}

int64_t FilterInputStream2::skip(int64_t n) 
{
  return delegate_->skip(n);
}

int FilterInputStream2::available() 
{
  return delegate_->available();
}

FilterInputStream2::~FilterInputStream2() { delegate_->close(); }

// C++ WARNING: The following method was originally marked 'synchronized':
void FilterInputStream2::mark(int readlimit) { delegate_->mark(readlimit); }

// C++ WARNING: The following method was originally marked 'synchronized':
void FilterInputStream2::reset()  { delegate_->reset(); }

bool FilterInputStream2::markSupported() { return delegate_->markSupported(); }
} // namespace org::apache::lucene::mockfile