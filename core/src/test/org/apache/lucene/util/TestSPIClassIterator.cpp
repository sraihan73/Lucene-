using namespace std;

#include "TestSPIClassIterator.h"

namespace org::apache::lucene::util
{

void TestSPIClassIterator::testParentChild() 
{
  shared_ptr<ClassLoader> *const parent = getClass().getClassLoader();
  shared_ptr<ClassLoader> *const child =
      URLClassLoader::newInstance(std::deque<std::shared_ptr<URL>>(0), parent);
  assertTrue(checkNoPerms(parent, parent));
  assertTrue(checkNoPerms(child, child));
  assertTrue(checkNoPerms(parent, child));
  assertFalse(checkNoPerms(child, parent));
}

bool TestSPIClassIterator::checkNoPerms(
    shared_ptr<ClassLoader> parent,
    shared_ptr<ClassLoader> child) 
{
  return runWithRestrictedPermissions(
      [&]() { SPIClassIterator::isParentClassLoader(parent, child); });
}
} // namespace org::apache::lucene::util