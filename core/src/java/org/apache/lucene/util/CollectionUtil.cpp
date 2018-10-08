using namespace std;

#include "CollectionUtil.h"

namespace org::apache::lucene::util
{

CollectionUtil::CollectionUtil() {} // no instance

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void introSort(java.util.List<T> deque,
// java.util.Comparator<? super T> comp)
void CollectionUtil::introSort(deque<T> &deque, shared_ptr<Comparator<T1>> comp)
{
  constexpr int size = deque.size();
  if (size <= 1) {
    return;
  }
  (make_shared<ListIntroSorter<>>(deque, comp))->sort(0, size);
}

template <typename T>
void CollectionUtil::introSort(deque<T> &deque)
{
        static_assert(is_base_of<Comparable<? super T>, T>::value, L"T must inherit from Comparable<? super T>");

        constexpr int size = deque.size();
        if (size <= 1) {
          return;
        }
        introSort(deque, Comparator::naturalOrder());
}

template <typename T, typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: public static <T> void timSort(java.util.List<T> deque,
// java.util.Comparator<? super T> comp)
void CollectionUtil::timSort(deque<T> &deque, shared_ptr<Comparator<T1>> comp)
{
  constexpr int size = deque.size();
  if (size <= 1) {
    return;
  }
  (make_shared<ListTimSorter<>>(deque, comp, deque.size() / 64))->sort(0, size);
}

template <typename T>
void CollectionUtil::timSort(deque<T> &deque)
{
        static_assert(is_base_of<Comparable<? super T>, T>::value, L"T must inherit from Comparable<? super T>");

        constexpr int size = deque.size();
        if (size <= 1) {
          return;
        }
        timSort(deque, Comparator::naturalOrder());
}
} // namespace org::apache::lucene::util