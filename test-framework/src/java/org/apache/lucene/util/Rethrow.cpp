using namespace std;

#include "Rethrow.h"

namespace org::apache::lucene::util
{

Rethrow::Rethrow() {}

void Rethrow::rethrow(runtime_error t)
{
  Rethrow::rethrow0<std::shared_ptr<Error>>(t);
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") private static <T extends
// Throwable> void rethrow0(Throwable t) throws T
void Rethrow::rethrow0(runtime_error t) 
{
  static_assert(is_base_of<runtime_error, T>::value,
                L"T must inherit from std::runtime_error");

  throw static_cast<T>(t);
}
} // namespace org::apache::lucene::util