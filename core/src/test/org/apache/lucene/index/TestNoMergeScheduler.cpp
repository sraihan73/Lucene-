using namespace std;

#include "TestNoMergeScheduler.h"

namespace org::apache::lucene::index
{
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoMergeScheduler() throws Exception
void TestNoMergeScheduler::testNoMergeScheduler() 
{
  shared_ptr<MergeScheduler> ms = NoMergeScheduler::INSTANCE;
  delete ms;
  ms->merge(nullptr, RandomPicks::randomFrom(random(), MergeTrigger::values()),
            random()->nextBoolean());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFinalSingleton() throws Exception
void TestNoMergeScheduler::testFinalSingleton() 
{
  assertTrue(Modifier::isFinal(NoMergeScheduler::typeid->getModifiers()));
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Constructor<?>[] ctors =
  // NoMergeScheduler.class.getDeclaredConstructors();
  std::deque < Constructor <
      ? >> ctors = NoMergeScheduler::typeid->getDeclaredConstructors();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"expected 1 private ctor only: " + Arrays->toString(ctors), 1,
               ctors.size());
  assertTrue(L"that 1 should be private: " + ctors[0],
             Modifier::isPrivate(ctors[0]->getModifiers()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMethodsOverridden() throws Exception
void TestNoMergeScheduler::testMethodsOverridden() 
{
  // Ensures that all methods of MergeScheduler are overridden. That's
  // important to ensure that NoMergeScheduler overrides everything, so that
  // no unexpected behavior/error occurs
  for (shared_ptr<Method> m : NoMergeScheduler::typeid->getMethods()) {
    // getDeclaredMethods() returns just those methods that are declared on
    // NoMergeScheduler. getMethods() returns those that are visible in that
    // context, including ones from Object. So just filter out Object. If in
    // the future MergeScheduler will extend a different class than Object,
    // this will need to change.
    if (m->getDeclaringClass() != any::typeid) {
      assertTrue(m + L" is not overridden !",
                 m->getDeclaringClass() == NoMergeScheduler::typeid);
    }
  }
}
} // namespace org::apache::lucene::index