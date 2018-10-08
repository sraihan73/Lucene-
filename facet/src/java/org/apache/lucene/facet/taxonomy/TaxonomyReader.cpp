using namespace std;

#include "TaxonomyReader.h"

namespace org::apache::lucene::facet::taxonomy
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;

TaxonomyReader::ChildrenIterator::ChildrenIterator(int child,
                                                   std::deque<int> &siblings)
    : siblings(siblings)
{
  this->child = child;
}

int TaxonomyReader::ChildrenIterator::next()
{
  int res = child;
  if (child != TaxonomyReader::INVALID_ORDINAL) {
    child = siblings[child];
  }
  return res;
}

TaxonomyReader::TaxonomyReader() {}

template <typename T>
T TaxonomyReader::openIfChanged(T oldTaxoReader) 
{
  static_assert(is_base_of<TaxonomyReader, T>::value,
                L"T must inherit from TaxonomyReader");

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") final T newTaxoReader = (T)
  // oldTaxoReader.doOpenIfChanged();
  constexpr T newTaxoReader = static_cast<T>(oldTaxoReader->doOpenIfChanged());
  assert(newTaxoReader != oldTaxoReader);
  return newTaxoReader;
}

void TaxonomyReader::ensureOpen() 
{
  if (getRefCount() <= 0) {
    throw make_shared<AlreadyClosedException>(L"this TaxonomyReader is closed");
  }
}

TaxonomyReader::~TaxonomyReader()
{
  if (!closed) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      if (!closed) {
        decRef();
        closed = true;
      }
    }
  }
}

void TaxonomyReader::decRef() 
{
  ensureOpen();
  constexpr int rc = refCount->decrementAndGet();
  if (rc == 0) {
    bool success = false;
    try {
      doClose();
      closed = true;
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        // Put reference back on failure
        refCount->incrementAndGet();
      }
    }
  } else if (rc < 0) {
    throw make_shared<IllegalStateException>(
        L"too many decRef calls: refCount is " + to_wstring(rc) +
        L" after decrement");
  }
}

shared_ptr<ChildrenIterator>
TaxonomyReader::getChildren(int const ordinal) 
{
  shared_ptr<ParallelTaxonomyArrays> arrays = getParallelTaxonomyArrays();
  int child = ordinal >= 0 ? arrays->children()[ordinal] : INVALID_ORDINAL;
  return make_shared<ChildrenIterator>(child, arrays->siblings());
}

int TaxonomyReader::getOrdinal(const wstring &dim,
                               std::deque<wstring> &path) 
{
  std::deque<wstring> fullPath(path.size() + 1);
  fullPath[0] = dim;
  System::arraycopy(path, 0, fullPath, 1, path.size());
  return getOrdinal(make_shared<FacetLabel>(fullPath));
}

int TaxonomyReader::getRefCount() { return refCount->get(); }

void TaxonomyReader::incRef()
{
  ensureOpen();
  refCount->incrementAndGet();
}

bool TaxonomyReader::tryIncRef()
{
  int count;
  while ((count = refCount->get()) > 0) {
    if (refCount->compareAndSet(count, count + 1)) {
      return true;
    }
  }
  return false;
}
} // namespace org::apache::lucene::facet::taxonomy