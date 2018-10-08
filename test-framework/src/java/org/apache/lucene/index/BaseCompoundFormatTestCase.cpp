using namespace std;

#include "BaseCompoundFormatTestCase.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StoredField = org::apache::lucene::document::StoredField;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using NRTCachingDirectory = org::apache::lucene::store::NRTCachingDirectory;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void BaseCompoundFormatTestCase::testEmpty() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::emptySet());
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  TestUtil::assertEquals(0, cfs->listAll().size());
  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testSingleFile() 
{
  std::deque<int> data = {0, 1, 10, 100};
  for (int i = 0; i < data.size(); i++) {
    wstring testfile = L"_" + to_wstring(i) + L".test";
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_" + to_wstring(i));
    createSequenceFile(dir, testfile, static_cast<char>(0), data[i],
                       si->getId(), L"suffix");

    si->setFiles(Collections::singleton(testfile));
    si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
    shared_ptr<Directory> cfs =
        si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                            IOContext::DEFAULT);

    shared_ptr<IndexInput> expected =
        dir->openInput(testfile, newIOContext(random()));
    shared_ptr<IndexInput> actual =
        cfs->openInput(testfile, newIOContext(random()));
    assertSameStreams(testfile, expected, actual);
    assertSameSeekBehavior(testfile, expected, actual);
    delete expected;
    delete actual;
    delete cfs;
    delete dir;
  }
}

void BaseCompoundFormatTestCase::testTwoFiles() 
{
  std::deque<wstring> files = {L"_123.d1", L"_123.d2"};
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  createSequenceFile(dir, files[0], static_cast<char>(0), 15, si->getId(),
                     L"suffix");
  createSequenceFile(dir, files[1], static_cast<char>(0), 114, si->getId(),
                     L"suffix");

  si->setFiles(Arrays::asList(files));
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);

  for (auto file : files) {
    shared_ptr<IndexInput> expected =
        dir->openInput(file, newIOContext(random()));
    shared_ptr<IndexInput> actual =
        cfs->openInput(file, newIOContext(random()));
    assertSameStreams(file, expected, actual);
    assertSameSeekBehavior(file, expected, actual);
    delete expected;
    delete actual;
  }

  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testDoubleClose() 
{
  const wstring testfile = L"_123.test";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // dir.createOutput(testfile, org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexOutput out = dir->createOutput(
        testfile, org::apache::lucene::store::IOContext::DEFAULT);
    CodecUtil::writeIndexHeader(out, L"Foo", 0, si->getId(), L"suffix");
    out->writeInt(3);
    CodecUtil::writeFooter(out);
  }

  si->setFiles(Collections::singleton(testfile));
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  TestUtil::assertEquals(1, cfs->listAll().size());
  delete cfs;
  delete cfs; // second close should not throw exception
  delete dir;
}

void BaseCompoundFormatTestCase::testPassIOContext() 
{
  const wstring testfile = L"_123.test";
  shared_ptr<IOContext> *const myContext = make_shared<IOContext>();

  shared_ptr<Directory> dir = make_shared<FilterDirectoryAnonymousInnerClass>(
      shared_from_this(), newDirectory(), myContext);
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // dir.createOutput(testfile, myContext))
  {
    org::apache::lucene::store::IndexOutput out =
        dir->createOutput(testfile, myContext);
    CodecUtil::writeIndexHeader(out, L"Foo", 0, si->getId(), L"suffix");
    out->writeInt(3);
    CodecUtil::writeFooter(out);
  }

  si->setFiles(Collections::singleton(testfile));
  si->getCodec()->compoundFormat()->write(dir, si, myContext);
  delete dir;
}

BaseCompoundFormatTestCase::FilterDirectoryAnonymousInnerClass::
    FilterDirectoryAnonymousInnerClass(
        shared_ptr<BaseCompoundFormatTestCase> outerInstance,
        shared_ptr<org::apache::lucene::store::BaseDirectoryWrapper>
            newDirectory,
        shared_ptr<IOContext> myContext)
    : org::apache::lucene::store::FilterDirectory(newDirectory)
{
  this->outerInstance = outerInstance;
  this->myContext = myContext;
}

shared_ptr<IndexOutput>
BaseCompoundFormatTestCase::FilterDirectoryAnonymousInnerClass::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  assertSame(myContext, context);
  return outerInstance->super->createOutput(name, context);
}

void BaseCompoundFormatTestCase::testLargeCFS() 
{
  const wstring testfile = L"_123.test";
  shared_ptr<IOContext> context =
      make_shared<IOContext>(make_shared<FlushInfo>(0, 512 * 1024 * 1024));

  shared_ptr<Directory> dir = make_shared<NRTCachingDirectory>(
      newFSDirectory(createTempDir()), 2.0, 25.0);

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // dir.createOutput(testfile, context))
  {
    org::apache::lucene::store::IndexOutput out =
        dir->createOutput(testfile, context);
    CodecUtil::writeIndexHeader(out, L"Foo", 0, si->getId(), L"suffix");
    std::deque<char> bytes(512);
    for (int i = 0; i < 1024 * 1024; i++) {
      out->writeBytes(bytes, 0, bytes.size());
    }
    CodecUtil::writeFooter(out);
  }

  si->setFiles(Collections::singleton(testfile));
  si->getCodec()->compoundFormat()->write(dir, si, context);

  delete dir;
}

void BaseCompoundFormatTestCase::testListAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  // riw should sometimes create docvalues fields, etc
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  // these fields should sometimes get term vectors, etc
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::NO);
  shared_ptr<Field> bodyField = newTextField(L"body", L"", Field::Store::NO);
  doc->push_back(idField);
  doc->push_back(bodyField);
  for (int i = 0; i < 100; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    bodyField->setStringValue(TestUtil::randomUnicodeString(random()));
    riw->addDocument(doc);
    if (random()->nextInt(7) == 0) {
      riw->commit();
    }
  }
  delete riw;
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
  for (auto si : infos) {
    if (si->info->getUseCompoundFile()) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory
      // cfsDir = si.info.getCodec().compoundFormat().getCompoundReader(dir,
      // si.info, newIOContext(random())))
      {
        org::apache::lucene::store::Directory cfsDir =
            si->info->getCodec()->compoundFormat()->getCompoundReader(
                dir, si->info, newIOContext(random()));
        for (auto cfsFile : cfsDir->listAll()) {
          // C++ NOTE: The following 'try with resources' block is replaced by
          // its C++ equivalent: ORIGINAL LINE: try
          // (org.apache.lucene.store.IndexInput cfsIn =
          // cfsDir.openInput(cfsFile,
          // org.apache.lucene.store.IOContext.DEFAULT))
          {
            org::apache::lucene::store::IndexInput cfsIn = cfsDir->openInput(
                cfsFile, org::apache::lucene::store::IOContext::DEFAULT);
            assert(cfsIn != nullptr);
          }
        }
      }
    }
  }
  delete dir;
}

void BaseCompoundFormatTestCase::testCreateOutputDisabled() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::emptyList());
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { cfs->createOutput(L"bogus", IOContext::DEFAULT); });

  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testDeleteFileDisabled() 
{
  const wstring testfile = L"_123.test";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(testfile, IOContext::DEFAULT);
  out->writeInt(3);
  delete out;

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::emptyList());
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { cfs->deleteFile(testfile); });

  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testRenameFileDisabled() 
{
  const wstring testfile = L"_123.test";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(testfile, IOContext::DEFAULT);
  out->writeInt(3);
  delete out;

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::emptyList());
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { cfs->rename(testfile, L"bogus"); });

  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testSyncDisabled() 
{
  const wstring testfile = L"_123.test";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(testfile, IOContext::DEFAULT);
  out->writeInt(3);
  delete out;

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::emptyList());
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { cfs->sync(Collections::singleton(testfile)); });

  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testMakeLockDisabled() 
{
  const wstring testfile = L"_123.test";

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(testfile, IOContext::DEFAULT);
  out->writeInt(3);
  delete out;

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::emptyList());
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { cfs->obtainLock(L"foobar"); });

  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testRandomFiles() 
{
  shared_ptr<Directory> dir = newDirectory();
  // Setup the test segment
  wstring segment = L"_123";
  int chunk = 1024; // internal buffer size used by the stream
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  std::deque<char> segId = si->getId();
  createRandomFile(dir, segment + L".zero", 0, segId);
  createRandomFile(dir, segment + L".one", 1, segId);
  createRandomFile(dir, segment + L".ten", 10, segId);
  createRandomFile(dir, segment + L".hundred", 100, segId);
  createRandomFile(dir, segment + L".big1", chunk, segId);
  createRandomFile(dir, segment + L".big2", chunk - 1, segId);
  createRandomFile(dir, segment + L".big3", chunk + 1, segId);
  createRandomFile(dir, segment + L".big4", 3 * chunk, segId);
  createRandomFile(dir, segment + L".big5", 3 * chunk - 1, segId);
  createRandomFile(dir, segment + L".big6", 3 * chunk + 1, segId);
  createRandomFile(dir, segment + L".big7", 1000 * chunk, segId);

  deque<wstring> files = deque<wstring>();
  for (auto file : dir->listAll()) {
    if (file.startsWith(segment)) {
      files.push_back(file);
    }
  }

  si->setFiles(files);
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);

  for (auto file : files) {
    shared_ptr<IndexInput> check = dir->openInput(file, newIOContext(random()));
    shared_ptr<IndexInput> test = cfs->openInput(file, newIOContext(random()));
    assertSameStreams(file, check, test);
    assertSameSeekBehavior(file, check, test);
    delete test;
    delete check;
  }
  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testManySubFiles() 
{
  shared_ptr<MockDirectoryWrapper> *const dir =
      newMockFSDirectory(createTempDir(L"CFSManySubFiles"));

  constexpr int FILE_COUNT = atLeast(500);

  deque<wstring> files = deque<wstring>();
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  for (int fileIdx = 0; fileIdx < FILE_COUNT; fileIdx++) {
    wstring file = L"_123." + to_wstring(fileIdx);
    files.push_back(file);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
    // dir.createOutput(file, newIOContext(random())))
    {
      org::apache::lucene::store::IndexOutput out =
          dir->createOutput(file, newIOContext(random()));
      CodecUtil::writeIndexHeader(out, L"Foo", 0, si->getId(), L"suffix");
      out->writeByte(static_cast<char>(fileIdx));
      CodecUtil::writeFooter(out);
    }
  }

  TestUtil::assertEquals(0, dir->getFileHandleCount());

  si->setFiles(files);
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);

  std::deque<std::shared_ptr<IndexInput>> ins(FILE_COUNT);
  for (int fileIdx = 0; fileIdx < FILE_COUNT; fileIdx++) {
    ins[fileIdx] =
        cfs->openInput(L"_123." + to_wstring(fileIdx), newIOContext(random()));
    CodecUtil::checkIndexHeader(ins[fileIdx], L"Foo", 0, 0, si->getId(),
                                L"suffix");
  }

  TestUtil::assertEquals(1, dir->getFileHandleCount());

  for (int fileIdx = 0; fileIdx < FILE_COUNT; fileIdx++) {
    TestUtil::assertEquals(static_cast<char>(fileIdx),
                           ins[fileIdx]->readByte());
  }

  TestUtil::assertEquals(1, dir->getFileHandleCount());

  for (int fileIdx = 0; fileIdx < FILE_COUNT; fileIdx++) {
    delete ins[fileIdx];
  }
  delete cfs;

  delete dir;
}

void BaseCompoundFormatTestCase::testClonedStreamsClosing() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> cr = createLargeCFS(dir);

  // basic clone
  shared_ptr<IndexInput> expected =
      dir->openInput(L"_123.f11", newIOContext(random()));

  shared_ptr<IndexInput> one =
      cr->openInput(L"_123.f11", newIOContext(random()));

  shared_ptr<IndexInput> two = one->clone();

  assertSameStreams(L"basic clone one", expected, one);
  expected->seek(0);
  assertSameStreams(L"basic clone two", expected, two);

  // Now close the compound reader
  delete cr;
  delete expected;
  delete dir;
}

void BaseCompoundFormatTestCase::testRandomAccess() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> cr = createLargeCFS(dir);

  // Open two files
  shared_ptr<IndexInput> e1 =
      dir->openInput(L"_123.f11", newIOContext(random()));
  shared_ptr<IndexInput> e2 =
      dir->openInput(L"_123.f3", newIOContext(random()));

  shared_ptr<IndexInput> a1 =
      cr->openInput(L"_123.f11", newIOContext(random()));
  shared_ptr<IndexInput> a2 =
      dir->openInput(L"_123.f3", newIOContext(random()));

  // Seek the first pair
  e1->seek(100);
  a1->seek(100);
  TestUtil::assertEquals(100, e1->getFilePointer());
  TestUtil::assertEquals(100, a1->getFilePointer());
  char be1 = e1->readByte();
  char ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  // Now seek the second pair
  e2->seek(1027);
  a2->seek(1027);
  TestUtil::assertEquals(1027, e2->getFilePointer());
  TestUtil::assertEquals(1027, a2->getFilePointer());
  char be2 = e2->readByte();
  char ba2 = a2->readByte();
  TestUtil::assertEquals(be2, ba2);

  // Now make sure the first one didn't move
  TestUtil::assertEquals(101, e1->getFilePointer());
  TestUtil::assertEquals(101, a1->getFilePointer());
  be1 = e1->readByte();
  ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  // Now more the first one again, past the buffer length
  e1->seek(1910);
  a1->seek(1910);
  TestUtil::assertEquals(1910, e1->getFilePointer());
  TestUtil::assertEquals(1910, a1->getFilePointer());
  be1 = e1->readByte();
  ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  // Now make sure the second set didn't move
  TestUtil::assertEquals(1028, e2->getFilePointer());
  TestUtil::assertEquals(1028, a2->getFilePointer());
  be2 = e2->readByte();
  ba2 = a2->readByte();
  TestUtil::assertEquals(be2, ba2);

  // Move the second set back, again cross the buffer size
  e2->seek(17);
  a2->seek(17);
  TestUtil::assertEquals(17, e2->getFilePointer());
  TestUtil::assertEquals(17, a2->getFilePointer());
  be2 = e2->readByte();
  ba2 = a2->readByte();
  TestUtil::assertEquals(be2, ba2);

  // Finally, make sure the first set didn't move
  // Now make sure the first one didn't move
  TestUtil::assertEquals(1911, e1->getFilePointer());
  TestUtil::assertEquals(1911, a1->getFilePointer());
  be1 = e1->readByte();
  ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  delete e1;
  delete e2;
  delete a1;
  delete a2;
  delete cr;
  delete dir;
}

void BaseCompoundFormatTestCase::testRandomAccessClones() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> cr = createLargeCFS(dir);

  // Open two files
  shared_ptr<IndexInput> e1 =
      cr->openInput(L"_123.f11", newIOContext(random()));
  shared_ptr<IndexInput> e2 = cr->openInput(L"_123.f3", newIOContext(random()));

  shared_ptr<IndexInput> a1 = e1->clone();
  shared_ptr<IndexInput> a2 = e2->clone();

  // Seek the first pair
  e1->seek(100);
  a1->seek(100);
  TestUtil::assertEquals(100, e1->getFilePointer());
  TestUtil::assertEquals(100, a1->getFilePointer());
  char be1 = e1->readByte();
  char ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  // Now seek the second pair
  e2->seek(1027);
  a2->seek(1027);
  TestUtil::assertEquals(1027, e2->getFilePointer());
  TestUtil::assertEquals(1027, a2->getFilePointer());
  char be2 = e2->readByte();
  char ba2 = a2->readByte();
  TestUtil::assertEquals(be2, ba2);

  // Now make sure the first one didn't move
  TestUtil::assertEquals(101, e1->getFilePointer());
  TestUtil::assertEquals(101, a1->getFilePointer());
  be1 = e1->readByte();
  ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  // Now more the first one again, past the buffer length
  e1->seek(1910);
  a1->seek(1910);
  TestUtil::assertEquals(1910, e1->getFilePointer());
  TestUtil::assertEquals(1910, a1->getFilePointer());
  be1 = e1->readByte();
  ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  // Now make sure the second set didn't move
  TestUtil::assertEquals(1028, e2->getFilePointer());
  TestUtil::assertEquals(1028, a2->getFilePointer());
  be2 = e2->readByte();
  ba2 = a2->readByte();
  TestUtil::assertEquals(be2, ba2);

  // Move the second set back, again cross the buffer size
  e2->seek(17);
  a2->seek(17);
  TestUtil::assertEquals(17, e2->getFilePointer());
  TestUtil::assertEquals(17, a2->getFilePointer());
  be2 = e2->readByte();
  ba2 = a2->readByte();
  TestUtil::assertEquals(be2, ba2);

  // Finally, make sure the first set didn't move
  // Now make sure the first one didn't move
  TestUtil::assertEquals(1911, e1->getFilePointer());
  TestUtil::assertEquals(1911, a1->getFilePointer());
  be1 = e1->readByte();
  ba1 = a1->readByte();
  TestUtil::assertEquals(be1, ba1);

  delete e1;
  delete e2;
  delete a1;
  delete a2;
  delete cr;
  delete dir;
}

void BaseCompoundFormatTestCase::testFileNotFound() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> cr = createLargeCFS(dir);

  // Open bogus file
  expectThrows(IOException::typeid,
               [&]() { cr->openInput(L"bogus", newIOContext(random())); });

  delete cr;
  delete dir;
}

void BaseCompoundFormatTestCase::testReadPastEOF() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> cr = createLargeCFS(dir);
  shared_ptr<IndexInput> is = cr->openInput(L"_123.f2", newIOContext(random()));
  is->seek(is->length() - 10);
  std::deque<char> b(100);
  is->readBytes(b, 0, 10);

  // Single byte read past end of file
  expectThrows(IOException::typeid, [&]() { is->readByte(); });

  is->seek(is->length() - 10);

  // Block read past end of file
  expectThrows(IOException::typeid, [&]() { is->readBytes(b, 0, 50); });

  delete is;
  delete cr;
  delete dir;
}

shared_ptr<SegmentInfo>
BaseCompoundFormatTestCase::newSegmentInfo(shared_ptr<Directory> dir,
                                           const wstring &name)
{
  shared_ptr<Version> minVersion =
      random()->nextBoolean() ? nullptr : Version::LATEST;
  return make_shared<SegmentInfo>(
      dir, Version::LATEST, minVersion, name, 10000, false, Codec::getDefault(),
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);
}

void BaseCompoundFormatTestCase::createRandomFile(
    shared_ptr<Directory> dir, const wstring &name, int size,
    std::deque<char> &segId) 
{
  shared_ptr<Random> rnd = random();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput os =
  // dir.createOutput(name, newIOContext(random())))
  {
    org::apache::lucene::store::IndexOutput os =
        dir->createOutput(name, newIOContext(random()));
    CodecUtil::writeIndexHeader(os, L"Foo", 0, segId, L"suffix");
    for (int i = 0; i < size; i++) {
      char b = static_cast<char>(rnd->nextInt(256));
      os->writeByte(b);
    }
    CodecUtil::writeFooter(os);
  }
}

void BaseCompoundFormatTestCase::createSequenceFile(
    shared_ptr<Directory> dir, const wstring &name, char start, int size,
    std::deque<char> &segID, const wstring &segSuffix) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput os =
  // dir.createOutput(name, newIOContext(random())))
  {
    org::apache::lucene::store::IndexOutput os =
        dir->createOutput(name, newIOContext(random()));
    CodecUtil::writeIndexHeader(os, L"Foo", 0, segID, segSuffix);
    for (int i = 0; i < size; i++) {
      os->writeByte(start);
      start++;
    }
    CodecUtil::writeFooter(os);
  }
}

void BaseCompoundFormatTestCase::assertSameStreams(
    const wstring &msg, shared_ptr<IndexInput> expected,
    shared_ptr<IndexInput> test) 
{
  assertNotNull(msg + L" null expected", expected);
  assertNotNull(msg + L" null test", test);
  assertEquals(msg + L" length", expected->length(), test->length());
  assertEquals(msg + L" position", expected->getFilePointer(),
               test->getFilePointer());

  std::deque<char> expectedBuffer(512);
  std::deque<char> testBuffer(expectedBuffer.size());

  int64_t remainder = expected->length() - expected->getFilePointer();
  while (remainder > 0) {
    int readLen = static_cast<int>(min(remainder, expectedBuffer.size()));
    expected->readBytes(expectedBuffer, 0, readLen);
    test->readBytes(testBuffer, 0, readLen);
    assertEqualArrays(msg + L", remainder " + to_wstring(remainder),
                      expectedBuffer, testBuffer, 0, readLen);
    remainder -= readLen;
  }
}

void BaseCompoundFormatTestCase::assertSameStreams(
    const wstring &msg, shared_ptr<IndexInput> expected,
    shared_ptr<IndexInput> actual, int64_t seekTo) 
{
  if (seekTo >= 0 && seekTo < expected->length()) {
    expected->seek(seekTo);
    actual->seek(seekTo);
    assertSameStreams(msg + L", seek(mid)", expected, actual);
  }
}

void BaseCompoundFormatTestCase::assertSameSeekBehavior(
    const wstring &msg, shared_ptr<IndexInput> expected,
    shared_ptr<IndexInput> actual) 
{
  // seek to 0
  int64_t point = 0;
  assertSameStreams(msg + L", seek(0)", expected, actual, point);

  // seek to middle
  point = expected->length() / 2LL;
  assertSameStreams(msg + L", seek(mid)", expected, actual, point);

  // seek to end - 2
  point = expected->length() - 2;
  assertSameStreams(msg + L", seek(end-2)", expected, actual, point);

  // seek to end - 1
  point = expected->length() - 1;
  assertSameStreams(msg + L", seek(end-1)", expected, actual, point);

  // seek to the end
  point = expected->length();
  assertSameStreams(msg + L", seek(end)", expected, actual, point);

  // seek past end
  point = expected->length() + 1;
  assertSameStreams(msg + L", seek(end+1)", expected, actual, point);
}

void BaseCompoundFormatTestCase::assertEqualArrays(const wstring &msg,
                                                   std::deque<char> &expected,
                                                   std::deque<char> &test,
                                                   int start, int len)
{
  assertNotNull(msg + L" null expected", expected);
  assertNotNull(msg + L" null test", test);

  for (int i = start; i < len; i++) {
    assertEquals(msg + L" " + to_wstring(i), expected[i], test[i]);
  }
}

shared_ptr<Directory> BaseCompoundFormatTestCase::createLargeCFS(
    shared_ptr<Directory> dir) 
{
  deque<wstring> files = deque<wstring>();
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  for (int i = 0; i < 20; i++) {
    createSequenceFile(dir, L"_123.f" + to_wstring(i), static_cast<char>(0),
                       2000, si->getId(), L"suffix");
    files.push_back(L"_123.f" + to_wstring(i));
  }

  si->setFiles(files);
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  return cfs;
}

void BaseCompoundFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  doc->push_back(make_shared<StoredField>(
      L"foobar", TestUtil::randomSimpleString(random())));
}

void BaseCompoundFormatTestCase::testMergeStability() 
{
  assumeTrue(L"test does not work with CFS", true);
}

void BaseCompoundFormatTestCase::testResourceNameInsideCompoundFile() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  wstring subFile = L"_123.xyz";
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  createSequenceFile(dir, subFile, static_cast<char>(0), 10, si->getId(),
                     L"suffix");

  si->setFiles(Collections::singletonList(subFile));
  si->getCodec()->compoundFormat()->write(dir, si, IOContext::DEFAULT);
  shared_ptr<Directory> cfs =
      si->getCodec()->compoundFormat()->getCompoundReader(dir, si,
                                                          IOContext::DEFAULT);
  shared_ptr<IndexInput> in_ = cfs->openInput(subFile, IOContext::DEFAULT);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring desc = in_->toString();
  assertTrue(L"resource description hides that it's inside a compound file: " +
                 desc,
             desc.find(L"[slice=" + subFile + L"]") != wstring::npos);
  delete cfs;
  delete dir;
}

void BaseCompoundFormatTestCase::testMissingCodecHeadersAreCaught() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  wstring subFile = L"_123.xyz";

  // missing codec header
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput os =
  // dir.createOutput(subFile, newIOContext(random())))
  {
    org::apache::lucene::store::IndexOutput os =
        dir->createOutput(subFile, newIOContext(random()));
    for (int i = 0; i < 1024; i++) {
      os->writeByte(static_cast<char>(i));
    }
  }

  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  si->setFiles(Collections::singletonList(subFile));
  runtime_error e = expectThrows(CorruptIndexException::typeid, [&]() {
    si->getCodec()->compoundFormat()->write(
        dir, si, org::apache::lucene::store::IOContext::DEFAULT);
  });
  assertTrue(e.what()->contains(L"codec header mismatch"));
  delete dir;
}

void BaseCompoundFormatTestCase::testCorruptFilesAreCaught() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  wstring subFile = L"_123.xyz";

  // wrong checksum
  shared_ptr<SegmentInfo> si = newSegmentInfo(dir, L"_123");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput os =
  // dir.createOutput(subFile, newIOContext(random())))
  {
    org::apache::lucene::store::IndexOutput os =
        dir->createOutput(subFile, newIOContext(random()));
    CodecUtil::writeIndexHeader(os, L"Foo", 0, si->getId(), L"suffix");
    for (int i = 0; i < 1024; i++) {
      os->writeByte(static_cast<char>(i));
    }

    // write footer w/ wrong checksum
    os->writeInt(CodecUtil::FOOTER_MAGIC);
    os->writeInt(0);

    int64_t checksum = os->getChecksum();
    os->writeLong(checksum + 1);
  }

  si->setFiles(Collections::singletonList(subFile));
  runtime_error e = expectThrows(CorruptIndexException::typeid, [&]() {
    si->getCodec()->compoundFormat()->write(
        dir, si, org::apache::lucene::store::IOContext::DEFAULT);
  });
  assertTrue(e.what()->contains(L"checksum failed (hardware problem?)"));
  delete dir;
}
} // namespace org::apache::lucene::index