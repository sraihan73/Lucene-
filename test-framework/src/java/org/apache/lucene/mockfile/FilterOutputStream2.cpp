using namespace std;

#include "FilterOutputStream2.h"

namespace org::apache::lucene::mockfile
{

FilterOutputStream2::FilterOutputStream2(shared_ptr<OutputStream> delegate_)
    : delegate_(Objects::requireNonNull(delegate_))
{
}

void FilterOutputStream2::write(std::deque<char> &b) 
{
  delegate_->write(b);
}

void FilterOutputStream2::write(std::deque<char> &b, int off,
                                int len) 
{
  delegate_->write(b, off, len);
}

void FilterOutputStream2::flush()  { delegate_->flush(); }

FilterOutputStream2::~FilterOutputStream2() { delegate_->close(); }

void FilterOutputStream2::write(int b) 
{
  delegate_->write(b);
}
} // namespace org::apache::lucene::mockfile