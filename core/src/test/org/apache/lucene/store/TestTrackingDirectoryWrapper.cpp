using namespace std;

#include "TestTrackingDirectoryWrapper.h"

namespace org::apache::lucene::store
{

shared_ptr<Directory> TestTrackingDirectoryWrapper::getDirectory(
    shared_ptr<Path> path) 
{
  return make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
}

void TestTrackingDirectoryWrapper::testTrackEmpty() 
{
  shared_ptr<TrackingDirectoryWrapper> dir =
      make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
  assertEquals(Collections::emptySet(), dir->getCreatedFiles());
}

void TestTrackingDirectoryWrapper::testTrackCreate() 
{
  shared_ptr<TrackingDirectoryWrapper> dir =
      make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
  delete dir->createOutput(L"foo", newIOContext(random()));
  assertEquals(asSet({L"foo"}), dir->getCreatedFiles());
}

void TestTrackingDirectoryWrapper::testTrackDelete() 
{
  shared_ptr<TrackingDirectoryWrapper> dir =
      make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
  delete dir->createOutput(L"foo", newIOContext(random()));
  assertEquals(asSet({L"foo"}), dir->getCreatedFiles());
  dir->deleteFile(L"foo");
  assertEquals(Collections::emptySet(), dir->getCreatedFiles());
}

void TestTrackingDirectoryWrapper::testTrackRename() 
{
  shared_ptr<TrackingDirectoryWrapper> dir =
      make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
  delete dir->createOutput(L"foo", newIOContext(random()));
  assertEquals(asSet({L"foo"}), dir->getCreatedFiles());
  dir->rename(L"foo", L"bar");
  assertEquals(asSet({L"bar"}), dir->getCreatedFiles());
}

void TestTrackingDirectoryWrapper::testTrackCopyFrom() 
{
  shared_ptr<TrackingDirectoryWrapper> source =
      make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
  shared_ptr<TrackingDirectoryWrapper> dest =
      make_shared<TrackingDirectoryWrapper>(make_shared<RAMDirectory>());
  delete source->createOutput(L"foo", newIOContext(random()));
  assertEquals(asSet({L"foo"}), source->getCreatedFiles());
  dest->copyFrom(source, L"foo", L"bar", newIOContext(random()));
  assertEquals(asSet({L"bar"}), dest->getCreatedFiles());
  assertEquals(asSet({L"foo"}), source->getCreatedFiles());
}
} // namespace org::apache::lucene::store