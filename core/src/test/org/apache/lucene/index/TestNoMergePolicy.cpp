using namespace std;

#include "TestNoMergePolicy.h"

namespace org::apache::lucene::index
{
using org::junit::Test;

shared_ptr<MergePolicy> TestNoMergePolicy::mergePolicy()
{
  return NoMergePolicy::INSTANCE;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoMergePolicy() throws Exception
void TestNoMergePolicy::testNoMergePolicy() 
{
  shared_ptr<MergePolicy> mp = mergePolicy();
  assertNull(mp->findMerges(
      nullptr, std::static_pointer_cast<SegmentInfos>(nullptr), nullptr));
  assertNull(mp->findForcedMerges(nullptr, 0, nullptr, nullptr));
  assertNull(mp->findForcedDeletesMerges(nullptr, nullptr));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFinalSingleton() throws Exception
void TestNoMergePolicy::testFinalSingleton() 
{
  assertTrue(Modifier::isFinal(NoMergePolicy::typeid->getModifiers()));
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Constructor<?>[] ctors =
  // NoMergePolicy.class.getDeclaredConstructors();
  std::deque < Constructor <
      ? >> ctors = NoMergePolicy::typeid->getDeclaredConstructors();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"expected 1 private ctor only: " + Arrays->toString(ctors), 1,
               ctors.size());
  assertTrue(L"that 1 should be private: " + ctors[0],
             Modifier::isPrivate(ctors[0]->getModifiers()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMethodsOverridden() throws Exception
void TestNoMergePolicy::testMethodsOverridden() 
{
  // Ensures that all methods of MergePolicy are overridden. That's important
  // to ensure that NoMergePolicy overrides everything, so that no unexpected
  // behavior/error occurs
  for (shared_ptr<Method> m : NoMergePolicy::typeid->getMethods()) {
    // getDeclaredMethods() returns just those methods that are declared on
    // NoMergePolicy. getMethods() returns those that are visible in that
    // context, including ones from Object. So just filter out Object. If in
    // the future MergePolicy will extend a different class than Object, this
    // will need to change.
    if (m->getName().equals(L"clone")) {
      continue;
    }
    if (m->getDeclaringClass() != any::typeid &&
        !Modifier::isFinal(m->getModifiers())) {
      assertTrue(m + L" is not overridden ! ",
                 m->getDeclaringClass() == NoMergePolicy::typeid);
    }
  }
}
} // namespace org::apache::lucene::index