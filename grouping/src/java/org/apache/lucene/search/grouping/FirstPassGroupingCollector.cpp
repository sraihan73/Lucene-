using namespace std;

#include "FirstPassGroupingCollector.h"

namespace org::apache::lucene::search::grouping
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;

FirstPassGroupingCollector<T>::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<FirstPassGroupingCollector<std::shared_ptr<T>>>
            outerInstance)
{
  this->outerInstance = outerInstance;
}

template <typename T1, typename T2>
int FirstPassGroupingCollector<T>::ComparatorAnonymousInnerClass::compare(
    shared_ptr<CollectedSearchGroup<T1>> o1,
    shared_ptr<CollectedSearchGroup<T2>> o2)
{
  for (int compIDX = 0;; compIDX++) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: org.apache.lucene.search.FieldComparator<?> fc =
    // comparators[compIDX];
    shared_ptr < FieldComparator < ? >> fc =
                                         outerInstance->comparators[compIDX];
    constexpr int c = outerInstance->reversed[compIDX] *
                      fc->compare(o1->comparatorSlot, o2->comparatorSlot);
    if (c != 0) {
      return c;
    } else if (compIDX == outerInstance->compIDXEnd) {
      return o1->topDoc - o2->topDoc;
    }
  }
}
} // namespace org::apache::lucene::search::grouping