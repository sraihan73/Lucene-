using namespace std;

#include "TestInfoStream.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestInfoStream::testTestPointsOff() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  iwc->setInfoStream(
      make_shared<InfoStreamAnonymousInnerClass>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  iw->addDocument(make_shared<Document>());
  delete iw;
  delete dir;
}

TestInfoStream::InfoStreamAnonymousInnerClass::InfoStreamAnonymousInnerClass(
    shared_ptr<TestInfoStream> outerInstance)
{
  this->outerInstance = outerInstance;
}

TestInfoStream::InfoStreamAnonymousInnerClass::~InfoStreamAnonymousInnerClass()
{
}

void TestInfoStream::InfoStreamAnonymousInnerClass::message(
    const wstring &component, const wstring &message)
{
  assertFalse(L"TP" == component);
}

bool TestInfoStream::InfoStreamAnonymousInnerClass::isEnabled(
    const wstring &component)
{
  assertFalse(L"TP" == component);
  return true;
}

void TestInfoStream::testTestPointsOn() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<AtomicBoolean> seenTestPoint = make_shared<AtomicBoolean>();
  iwc->setInfoStream(make_shared<InfoStreamAnonymousInnerClass2>(
      shared_from_this(), seenTestPoint));
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriterAnonymousInnerClass>(shared_from_this(), dir, iwc);
  iw->addDocument(make_shared<Document>());
  delete iw;
  delete dir;
  assertTrue(seenTestPoint->get());
}

TestInfoStream::InfoStreamAnonymousInnerClass2::InfoStreamAnonymousInnerClass2(
    shared_ptr<TestInfoStream> outerInstance,
    shared_ptr<AtomicBoolean> seenTestPoint)
{
  this->outerInstance = outerInstance;
  this->seenTestPoint = seenTestPoint;
}

TestInfoStream::InfoStreamAnonymousInnerClass2::
    ~InfoStreamAnonymousInnerClass2()
{
}

void TestInfoStream::InfoStreamAnonymousInnerClass2::message(
    const wstring &component, const wstring &message)
{
  if (L"TP" == component) {
    seenTestPoint->set(true);
  }
}

bool TestInfoStream::InfoStreamAnonymousInnerClass2::isEnabled(
    const wstring &component)
{
  return true;
}

TestInfoStream::IndexWriterAnonymousInnerClass::IndexWriterAnonymousInnerClass(
    shared_ptr<TestInfoStream> outerInstance, shared_ptr<Directory> dir,
    shared_ptr<org::apache::lucene::index::IndexWriterConfig> iwc)
    : IndexWriter(dir, iwc)
{
  this->outerInstance = outerInstance;
}

bool TestInfoStream::IndexWriterAnonymousInnerClass::isEnableTestPoints()
{
  return true;
}
} // namespace org::apache::lucene::index