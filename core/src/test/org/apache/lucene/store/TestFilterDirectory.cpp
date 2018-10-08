using namespace std;

#include "TestFilterDirectory.h"

namespace org::apache::lucene::store
{
using org::junit::Test;

shared_ptr<Directory> TestFilterDirectory::getDirectory(shared_ptr<Path> path)
{
  return make_shared<FilterDirectoryAnonymousInnerClass>(
      shared_from_this(), make_shared<RAMDirectory>());
}

TestFilterDirectory::FilterDirectoryAnonymousInnerClass::
    FilterDirectoryAnonymousInnerClass(
        shared_ptr<TestFilterDirectory> outerInstance,
        shared_ptr<RAMDirectory> new) new : FilterDirectory(new RAMDirectory)
{
  this->outerInstance = outerInstance;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOverrides() throws Exception
void TestFilterDirectory::testOverrides() 
{
  // verify that all methods of Directory are overridden by FilterDirectory,
  // except those under the 'exclude' deque
  shared_ptr<Set<std::shared_ptr<Method>>> exclude =
      unordered_set<std::shared_ptr<Method>>();
  exclude->add(Directory::typeid->getMethod(L"copyFrom", Directory::typeid,
                                            wstring::typeid, wstring::typeid,
                                            IOContext::typeid));
  exclude->add(Directory::typeid->getMethod(
      L"openChecksumInput", wstring::typeid, IOContext::typeid));
  for (shared_ptr<Method> m : FilterDirectory::typeid->getMethods()) {
    if (m->getDeclaringClass() == Directory::typeid) {
      assertTrue(L"method " + m->getName() + L" not overridden!",
                 exclude->contains(m));
    }
  }
}

void TestFilterDirectory::testUnwrap() 
{
  shared_ptr<Directory> dir = FSDirectory::open(createTempDir());
  shared_ptr<FilterDirectory> dir2 =
      make_shared<FilterDirectoryAnonymousInnerClass2>(shared_from_this(), dir);
  assertEquals(dir, dir2->getDelegate());
  assertEquals(dir, FilterDirectory::unwrap(dir2));
  delete dir2;
}

TestFilterDirectory::FilterDirectoryAnonymousInnerClass2::
    FilterDirectoryAnonymousInnerClass2(
        shared_ptr<TestFilterDirectory> outerInstance,
        shared_ptr<org::apache::lucene::store::Directory> dir)
    : FilterDirectory(dir)
{
  this->outerInstance = outerInstance;
}
} // namespace org::apache::lucene::store