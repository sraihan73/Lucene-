using namespace std;

#include "TestNoDeletionPolicy.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoDeletionPolicy() throws Exception
void TestNoDeletionPolicy::testNoDeletionPolicy() 
{
  shared_ptr<IndexDeletionPolicy> idp = NoDeletionPolicy::INSTANCE;
  idp->onInit(nullptr);
  idp->onCommit(nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFinalSingleton() throws Exception
void TestNoDeletionPolicy::testFinalSingleton() 
{
  assertTrue(Modifier::isFinal(NoDeletionPolicy::typeid->getModifiers()));
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Constructor<?>[] ctors =
  // NoDeletionPolicy.class.getDeclaredConstructors();
  std::deque < Constructor <
      ? >> ctors = NoDeletionPolicy::typeid->getDeclaredConstructors();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"expected 1 private ctor only: " + Arrays->toString(ctors), 1,
               ctors.size());
  assertTrue(L"that 1 should be private: " + ctors[0],
             Modifier::isPrivate(ctors[0]->getModifiers()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMethodsOverridden() throws Exception
void TestNoDeletionPolicy::testMethodsOverridden() 
{
  // Ensures that all methods of IndexDeletionPolicy are
  // overridden/implemented. That's important to ensure that NoDeletionPolicy
  // overrides everything, so that no unexpected behavior/error occurs.
  // NOTE: even though IndexDeletionPolicy is an interface today, and so all
  // methods must be implemented by NoDeletionPolicy, this test is important
  // in case one day IDP becomes an abstract class.
  for (shared_ptr<Method> m : NoDeletionPolicy::typeid->getMethods()) {
    // getDeclaredMethods() returns just those methods that are declared on
    // NoDeletionPolicy. getMethods() returns those that are visible in that
    // context, including ones from Object. So just filter out Object. If in
    // the future IndexDeletionPolicy will become a class that extends a
    // different class than Object, this will need to change.
    if (m->getDeclaringClass() != any::typeid) {
      assertTrue(m + L" is not overridden !",
                 m->getDeclaringClass() == NoDeletionPolicy::typeid);
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllCommitsRemain() throws Exception
void TestNoDeletionPolicy::testAllCommitsRemain() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE));
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"c", L"a" + to_wstring(i), Field::Store::YES));
    writer->addDocument(doc);
    writer->commit();
    assertEquals(L"wrong number of commits !", i + 1,
                 DirectoryReader::listCommits(dir).size());
  }
  delete writer;
  delete dir;
}
} // namespace org::apache::lucene::index