using namespace std;

#include "TestFilterMergePolicy.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFilterMergePolicy::testMethodsOverridden()
{
  for (shared_ptr<Method> m : MergePolicy::typeid->getDeclaredMethods()) {
    if (Modifier::isFinal(m->getModifiers()) ||
        Modifier::isPrivate(m->getModifiers())) {
      continue;
    }
    try {
      FilterMergePolicy::typeid->getDeclaredMethod(m->getName(),
                                                   m->getParameterTypes());
    } catch (const NoSuchMethodException &e) {
      fail(L"FilterMergePolicy needs to override '" + m + L"'");
    }
  }
}
} // namespace org::apache::lucene::index