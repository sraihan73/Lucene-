using namespace std;

#include "BaseDirectoryTestCase.h"

namespace org::apache::lucene::store
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexNotFoundException =
    org::apache::lucene::index::IndexNotFoundException;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void BaseDirectoryTestCase::testCopyFrom() 
{
  shared_ptr<Directory> source = getDirectory(createTempDir(L"testCopy"));
  shared_ptr<Directory> dest = newDirectory();

  shared_ptr<IndexOutput> output =
      source->createOutput(L"foobar", newIOContext(random()));
  int numBytes = random()->nextInt(20000);
  std::deque<char> bytes(numBytes);
  random()->nextBytes(bytes);
  output->writeBytes(bytes, bytes.size());
  delete output;

  dest->copyFrom(source, L"foobar", L"foobaz", newIOContext(random()));
  assertTrue(slowFileExists(dest, L"foobaz"));

  shared_ptr<IndexInput> input =
      dest->openInput(L"foobaz", newIOContext(random()));
  std::deque<char> bytes2(numBytes);
  input->readBytes(bytes2, 0, bytes2.size());
  TestUtil::assertEquals(input->length(), numBytes);
  delete input;

  assertArrayEquals(bytes, bytes2);

  IOUtils::close({source, dest});
}

void BaseDirectoryTestCase::testCopyFromDestination() 
{
  shared_ptr<Directory> source = newDirectory();
  shared_ptr<Directory> dest =
      getDirectory(createTempDir(L"testCopyDestination"));

  shared_ptr<IndexOutput> output =
      source->createOutput(L"foobar", newIOContext(random()));
  int numBytes = random()->nextInt(20000);
  std::deque<char> bytes(numBytes);
  random()->nextBytes(bytes);
  output->writeBytes(bytes, bytes.size());
  delete output;

  dest->copyFrom(source, L"foobar", L"foobaz", newIOContext(random()));
  assertTrue(slowFileExists(dest, L"foobaz"));

  shared_ptr<IndexInput> input =
      dest->openInput(L"foobaz", newIOContext(random()));
  std::deque<char> bytes2(numBytes);
  input->readBytes(bytes2, 0, bytes2.size());
  TestUtil::assertEquals(input->length(), numBytes);
  delete input;

  assertArrayEquals(bytes, bytes2);

  IOUtils::close({source, dest});
}

void BaseDirectoryTestCase::testRename() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testRename"));

  shared_ptr<IndexOutput> output =
      dir->createOutput(L"foobar", newIOContext(random()));
  int numBytes = random()->nextInt(20000);
  std::deque<char> bytes(numBytes);
  random()->nextBytes(bytes);
  output->writeBytes(bytes, bytes.size());
  delete output;

  dir->rename(L"foobar", L"foobaz");

  shared_ptr<IndexInput> input =
      dir->openInput(L"foobaz", newIOContext(random()));
  std::deque<char> bytes2(numBytes);
  input->readBytes(bytes2, 0, bytes2.size());
  TestUtil::assertEquals(input->length(), numBytes);
  delete input;

  assertArrayEquals(bytes, bytes2);

  delete dir;
}

void BaseDirectoryTestCase::testDeleteFile() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testDeleteFile"));
  int count = dir->listAll().size();
  delete dir->createOutput(L"foo.txt", IOContext::DEFAULT);
  TestUtil::assertEquals(count + 1, dir->listAll().size());
  dir->deleteFile(L"foo.txt");
  TestUtil::assertEquals(count, dir->listAll().size());
  delete dir;
}

void BaseDirectoryTestCase::testByte() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testByte"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"byte", newIOContext(random()));
  output->writeByte(static_cast<char>(128));
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"byte", newIOContext(random()));
  TestUtil::assertEquals(1, input->length());
  TestUtil::assertEquals(static_cast<char>(128), input->readByte());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testShort() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testShort"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"short", newIOContext(random()));
  output->writeShort(static_cast<short>(-20));
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"short", newIOContext(random()));
  TestUtil::assertEquals(2, input->length());
  TestUtil::assertEquals(static_cast<short>(-20), input->readShort());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testInt() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testInt"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"int", newIOContext(random()));
  output->writeInt(-500);
  delete output;

  shared_ptr<IndexInput> input = dir->openInput(L"int", newIOContext(random()));
  TestUtil::assertEquals(4, input->length());
  TestUtil::assertEquals(-500, input->readInt());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testLong() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testLong"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"long", newIOContext(random()));
  output->writeLong(-5000);
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"long", newIOContext(random()));
  TestUtil::assertEquals(8, input->length());
  TestUtil::assertEquals(-5000LL, input->readLong());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testString() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testString"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"string", newIOContext(random()));
  output->writeString(L"hello!");
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"string", newIOContext(random()));
  TestUtil::assertEquals(L"hello!", input->readString());
  TestUtil::assertEquals(7, input->length());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testVInt() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testVInt"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"vint", newIOContext(random()));
  output->writeVInt(500);
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"vint", newIOContext(random()));
  TestUtil::assertEquals(2, input->length());
  TestUtil::assertEquals(500, input->readVInt());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testVLong() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testVLong"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"vlong", newIOContext(random()));
  output->writeVLong(numeric_limits<int64_t>::max());
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"vlong", newIOContext(random()));
  TestUtil::assertEquals(9, input->length());
  TestUtil::assertEquals(numeric_limits<int64_t>::max(), input->readVLong());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testZInt() 
{
  const std::deque<int> ints = std::deque<int>(random()->nextInt(10));
  for (int i = 0; i < ints.size(); ++i) {
    switch (random()->nextInt(3)) {
    case 0:
      ints[i] = random()->nextInt();
      break;
    case 1:
      ints[i] = random()->nextBoolean() ? numeric_limits<int>::min()
                                        : numeric_limits<int>::max();
      break;
    case 2:
      ints[i] = (random()->nextBoolean() ? -1 : 1) * random()->nextInt(1024);
      break;
    default:
      throw make_shared<AssertionError>();
    }
  }
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testZInt"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"zint", newIOContext(random()));
  for (auto i : ints) {
    output->writeZInt(i);
  }
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"zint", newIOContext(random()));
  for (auto i : ints) {
    TestUtil::assertEquals(i, input->readZInt());
  }
  TestUtil::assertEquals(input->length(), input->getFilePointer());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testZLong() 
{
  const std::deque<int64_t> longs =
      std::deque<int64_t>(random()->nextInt(10));
  for (int i = 0; i < longs.size(); ++i) {
    switch (random()->nextInt(3)) {
    case 0:
      longs[i] = random()->nextLong();
      break;
    case 1:
      longs[i] = random()->nextBoolean() ? numeric_limits<int64_t>::min()
                                         : numeric_limits<int64_t>::max();
      break;
    case 2:
      longs[i] = (random()->nextBoolean() ? -1 : 1) * random()->nextInt(1024);
      break;
    default:
      throw make_shared<AssertionError>();
    }
  }
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testZLong"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"zlong", newIOContext(random()));
  for (auto l : longs) {
    output->writeZLong(l);
  }
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"zlong", newIOContext(random()));
  for (auto l : longs) {
    TestUtil::assertEquals(l, input->readZLong());
  }
  TestUtil::assertEquals(input->length(), input->getFilePointer());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testSetOfStrings() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testSetOfStrings"));

  shared_ptr<IndexOutput> output =
      dir->createOutput(L"stringset", newIOContext(random()));
  output->writeSetOfStrings(asSet({L"test1", L"test2"}));
  output->writeSetOfStrings(Collections::emptySet());
  output->writeSetOfStrings(asSet({L"test3"}));
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"stringset", newIOContext(random()));
  shared_ptr<Set<wstring>> set1 = input->readSetOfStrings();
  TestUtil::assertEquals(asSet({L"test1", L"test2"}), set1);
  // set should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { set1->add(L"bogus"); });

  shared_ptr<Set<wstring>> set2 = input->readSetOfStrings();
  TestUtil::assertEquals(Collections::emptySet(), set2);
  // set should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { set2->add(L"bogus"); });

  shared_ptr<Set<wstring>> set3 = input->readSetOfStrings();
  TestUtil::assertEquals(Collections::singleton(L"test3"), set3);
  // set should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { set3->add(L"bogus"); });

  TestUtil::assertEquals(input->length(), input->getFilePointer());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testMapOfStrings() 
{
  unordered_map<wstring, wstring> m = unordered_map<wstring, wstring>();
  m.emplace(L"test1", L"value1");
  m.emplace(L"test2", L"value2");

  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testMapOfStrings"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"stringmap", newIOContext(random()));
  output->writeMapOfStrings(m);
  output->writeMapOfStrings(Collections::emptyMap());
  output->writeMapOfStrings(Collections::singletonMap(L"key", L"value"));
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"stringmap", newIOContext(random()));
  unordered_map<wstring, wstring> map1 = input->readMapOfStrings();
  TestUtil::assertEquals(m, map1);
  // map_obj should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { map1.put(L"bogus1", L"bogus2"); });

  unordered_map<wstring, wstring> map2 = input->readMapOfStrings();
  TestUtil::assertEquals(Collections::emptyMap(), map2);
  // map_obj should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { map2.put(L"bogus1", L"bogus2"); });

  unordered_map<wstring, wstring> map3 = input->readMapOfStrings();
  TestUtil::assertEquals(Collections::singletonMap(L"key", L"value"), map3);
  // map_obj should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { map3.put(L"bogus1", L"bogus2"); });

  TestUtil::assertEquals(input->length(), input->getFilePointer());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testChecksum() 
{
  shared_ptr<CRC32> expected = make_shared<CRC32>();
  int numBytes = random()->nextInt(20000);
  std::deque<char> bytes(numBytes);
  random()->nextBytes(bytes);
  expected->update(bytes);

  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testChecksum"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"checksum", newIOContext(random()));
  output->writeBytes(bytes, 0, bytes.size());
  delete output;

  shared_ptr<ChecksumIndexInput> input =
      dir->openChecksumInput(L"checksum", newIOContext(random()));
  input->skipBytes(numBytes);

  TestUtil::assertEquals(expected->getValue(), input->getChecksum());
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testDetectClose() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testDetectClose"));
  delete dir;
  expectThrows(AlreadyClosedException::typeid,
               [&]() { dir->createOutput(L"test", newIOContext(random())); });
}

void BaseDirectoryTestCase::testThreadSafety() 
{
  shared_ptr<Directory> *const dir =
      getDirectory(createTempDir(L"testThreadSafety"));
  if (std::dynamic_pointer_cast<BaseDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<BaseDirectoryWrapper>(dir))
        ->setCheckIndexOnClose(false); // we arent making an index
  }
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(
            MockDirectoryWrapper::Throttling::NEVER); // makes this test really
                                                      // slow
  }

  if (VERBOSE) {
    wcout << dir << endl;
  }

  // C++ TODO: Local classes are not converted by Java to C++ Converter:
  //      class TheThread extends Thread
  //    {
  //      private std::wstring name;
  //
  //      public TheThread(std::wstring name)
  //      {
  //        this.name = name;
  //      }
  //
  //      @@Override public void run()
  //      {
  //        for (int i = 0; i < 1000; i++)
  //        {
  //          std::wstring fileName = this.name + i;
  //          try
  //          {
  //            //System.out.println("create:" + fileName);
  //            IndexOutput output = dir.createOutput(fileName,
  //            newIOContext(random())); output.close();
  //            assertTrue(slowFileExists(dir, fileName));
  //          }
  //          catch (IOException e)
  //          {
  //            throw new RuntimeException(e);
  //          }
  //        }
  //      }
  //    };

  // C++ TODO: Local classes are not converted by Java to C++ Converter:
  //      class TheThread2 extends Thread
  //    {
  //      private std::wstring name;
  //      private volatile bool stop;
  //
  //      public TheThread2(std::wstring name)
  //      {
  //        this.name = name;
  //      }
  //
  //      @@Override public void run()
  //      {
  //        while (stop == false)
  //        {
  //          try
  //          {
  //            std::wstring[] files = dir.listAll();
  //            for (std::wstring file : files)
  //            {
  //              if (!file.startsWith(name))
  //              {
  //                continue;
  //              }
  //              //System.out.println("file:" + file);
  //             try
  //             {
  //              IndexInput input = dir.openInput(file,
  //              newIOContext(random())); input.close();
  //              }
  //              catch (FileNotFoundException | NoSuchFileException e)
  //              {
  //                // ignore
  //              }
  //              catch (IOException e)
  //              {
  //                if (e.getMessage() != nullptr &&
  //                e.getMessage().contains("still open for writing"))
  //                {
  //                  // ignore
  //                }
  //                else
  //                {
  //                  throw new RuntimeException(e);
  //                }
  //              }
  //              if (random().nextBoolean())
  //              {
  //                break;
  //              }
  //            }
  //          }
  //          catch (IOException e)
  //          {
  //            throw new RuntimeException(e);
  //          }
  //        }
  //      }
  //    };

  shared_ptr<TheThread> theThread = make_shared<TheThread>(L"t1");
  shared_ptr<TheThread2> theThread2 = make_shared<TheThread2>(L"t2");
  theThread->start();
  theThread2->start();

  theThread->join();

  // after first thread is done, no sense in waiting on thread 2
  // to listFiles() and loop over and over
  theThread2->stop = true;
  theThread2->join();

  delete dir;
}

void BaseDirectoryTestCase::testDirectoryFilter() 
{
  wstring name = L"file";
  shared_ptr<Directory> dir =
      getDirectory(createTempDir(L"testDirectoryFilter"));
  try {
    delete dir->createOutput(name, newIOContext(random()));
    assertTrue(slowFileExists(dir, name));
    assertTrue(Arrays::asList(dir->listAll())->contains(name));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete dir;
  }
}

void BaseDirectoryTestCase::testSeekToEOFThenBack() 
{
  shared_ptr<Directory> dir =
      getDirectory(createTempDir(L"testSeekToEOFThenBack"));

  shared_ptr<IndexOutput> o = dir->createOutput(L"out", newIOContext(random()));
  std::deque<char> bytes(3 * RAMOutputStream::BUFFER_SIZE);
  o->writeBytes(bytes, 0, bytes.size());
  delete o;

  shared_ptr<IndexInput> i = dir->openInput(L"out", newIOContext(random()));
  i->seek(2 * RAMOutputStream::BUFFER_SIZE - 1);
  i->seek(3 * RAMOutputStream::BUFFER_SIZE);
  i->seek(RAMOutputStream::BUFFER_SIZE);
  i->readBytes(bytes, 0, 2 * RAMOutputStream::BUFFER_SIZE);
  delete i;
  delete dir;
}

void BaseDirectoryTestCase::testIllegalEOF() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testIllegalEOF"));
  shared_ptr<IndexOutput> o = dir->createOutput(L"out", newIOContext(random()));
  std::deque<char> b(1024);
  o->writeBytes(b, 0, 1024);
  delete o;
  shared_ptr<IndexInput> i = dir->openInput(L"out", newIOContext(random()));
  i->seek(1024);
  delete i;
  delete dir;
}

void BaseDirectoryTestCase::testSeekPastEOF() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testSeekPastEOF"));
  shared_ptr<IndexOutput> o = dir->createOutput(L"out", newIOContext(random()));
  constexpr int len = random()->nextInt(2048);
  std::deque<char> b(len);
  o->writeBytes(b, 0, len);
  delete o;
  shared_ptr<IndexInput> i = dir->openInput(L"out", newIOContext(random()));
  expectThrows(EOFException::typeid, [&]() {
    i->seek(len + random()->nextInt(2048));
    i->readByte();
  });

  delete i;
  delete dir;
}

void BaseDirectoryTestCase::testSliceOutOfBounds() 
{
  shared_ptr<Directory> dir =
      getDirectory(createTempDir(L"testSliceOutOfBounds"));
  shared_ptr<IndexOutput> o = dir->createOutput(L"out", newIOContext(random()));
  constexpr int len = random()->nextInt(2040) + 8;
  std::deque<char> b(len);
  o->writeBytes(b, 0, len);
  delete o;
  shared_ptr<IndexInput> i = dir->openInput(L"out", newIOContext(random()));
  expectThrows(invalid_argument::typeid,
               [&]() { i->slice(L"slice1", 0, len + 1); });

  expectThrows(invalid_argument::typeid,
               [&]() { i->slice(L"slice2", -1, len); });

  shared_ptr<IndexInput> slice = i->slice(L"slice3", 4, len / 2);
  expectThrows(invalid_argument::typeid,
               [&]() { slice->slice(L"slice3sub", 1, len / 2); });

  delete i;
  delete dir;
}

void BaseDirectoryTestCase::testNoDir() 
{
  shared_ptr<Path> tempDir = createTempDir(L"doesnotexist");
  IOUtils::rm({tempDir});
  shared_ptr<Directory> dir = getDirectory(tempDir);
  try {
    DirectoryReader::open(dir);
    fail(L"did not hit expected exception");
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | IndexNotFoundException nsde) {
    // expected
  }
  delete dir;
}

void BaseDirectoryTestCase::testCopyBytes() 
{
  testCopyBytes(getDirectory(createTempDir(L"testCopyBytes")));
}

char BaseDirectoryTestCase::value(int idx)
{
  return static_cast<char>((idx % 256) * (1 + (idx / 256)));
}

void BaseDirectoryTestCase::testCopyBytes(shared_ptr<Directory> dir) throw(
    runtime_error)
{

  // make random file
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"test", newIOContext(random()));
  std::deque<char> bytes(TestUtil::nextInt(random(), 1, 77777));
  constexpr int size = TestUtil::nextInt(random(), 1, 1777777);
  int upto = 0;
  int byteUpto = 0;
  while (upto < size) {
    bytes[byteUpto++] = value(upto);
    upto++;
    if (byteUpto == bytes.size()) {
      out->writeBytes(bytes, 0, bytes.size());
      byteUpto = 0;
    }
  }

  out->writeBytes(bytes, 0, byteUpto);
  TestUtil::assertEquals(size, out->getFilePointer());
  delete out;
  TestUtil::assertEquals(size, dir->fileLength(L"test"));

  // copy from test -> test2
  shared_ptr<IndexInput> *const in_ =
      dir->openInput(L"test", newIOContext(random()));

  out = dir->createOutput(L"test2", newIOContext(random()));

  upto = 0;
  while (upto < size) {
    if (random()->nextBoolean()) {
      out->writeByte(in_->readByte());
      upto++;
    } else {
      constexpr int chunk =
          min(TestUtil::nextInt(random(), 1, bytes.size()), size - upto);
      out->copyBytes(in_, chunk);
      upto += chunk;
    }
  }
  TestUtil::assertEquals(size, upto);
  delete out;
  delete in_;

  // verify
  shared_ptr<IndexInput> in2 = dir->openInput(L"test2", newIOContext(random()));
  upto = 0;
  while (upto < size) {
    if (random()->nextBoolean()) {
      constexpr char v = in2->readByte();
      TestUtil::assertEquals(value(upto), v);
      upto++;
    } else {
      constexpr int limit =
          min(TestUtil::nextInt(random(), 1, bytes.size()), size - upto);
      in2->readBytes(bytes, 0, limit);
      for (int byteIdx = 0; byteIdx < limit; byteIdx++) {
        TestUtil::assertEquals(value(upto), bytes[byteIdx]);
        upto++;
      }
    }
  }
  delete in2;

  dir->deleteFile(L"test");
  dir->deleteFile(L"test2");
  delete dir;
}

void BaseDirectoryTestCase::testCopyBytesWithThreads() 
{
  testCopyBytesWithThreads(
      getDirectory(createTempDir(L"testCopyBytesWithThreads")));
}

void BaseDirectoryTestCase::testCopyBytesWithThreads(
    shared_ptr<Directory> d) 
{
  int datalen = TestUtil::nextInt(random(), 101, 10000);
  std::deque<char> data(datalen);
  random()->nextBytes(data);

  shared_ptr<IndexOutput> output = d->createOutput(L"data", IOContext::DEFAULT);
  output->writeBytes(data, 0, datalen);
  delete output;

  shared_ptr<IndexInput> input = d->openInput(L"data", IOContext::DEFAULT);
  shared_ptr<IndexOutput> outputHeader =
      d->createOutput(L"header", IOContext::DEFAULT);
  // copy our 100-byte header
  outputHeader->copyBytes(input, 100);
  delete outputHeader;

  // now make N copies of the remaining bytes
  std::deque<std::shared_ptr<CopyThread>> copies(10);
  for (int i = 0; i < copies.size(); i++) {
    copies[i] = make_shared<CopyThread>(
        input->clone(),
        d->createOutput(L"copy" + to_wstring(i), IOContext::DEFAULT));
  }

  for (int i = 0; i < copies.size(); i++) {
    copies[i]->start();
  }

  for (int i = 0; i < copies.size(); i++) {
    copies[i]->join();
  }

  for (int i = 0; i < copies.size(); i++) {
    shared_ptr<IndexInput> copiedData =
        d->openInput(L"copy" + to_wstring(i), IOContext::DEFAULT);
    std::deque<char> dataCopy(datalen);
    System::arraycopy(data, 0, dataCopy, 0,
                      100); // copy the header for easy testing
    copiedData->readBytes(dataCopy, 100, datalen - 100);
    assertArrayEquals(data, dataCopy);
    delete copiedData;
  }
  delete input;
  delete d;
}

BaseDirectoryTestCase::CopyThread::CopyThread(shared_ptr<IndexInput> src,
                                              shared_ptr<IndexOutput> dst)
    : src(src), dst(dst)
{
}

void BaseDirectoryTestCase::CopyThread::run()
{
  try {
    dst->copyBytes(src, src->length() - 100);
    delete dst;
  } catch (const IOException &ex) {
    throw runtime_error(ex);
  }
}

void BaseDirectoryTestCase::testFsyncDoesntCreateNewFiles() 
{
  shared_ptr<Path> path = createTempDir(L"nocreate");
  shared_ptr<Directory> fsdir = getDirectory(path);

  // this test backdoors the directory via the filesystem. so it must be an
  // FSDir (for now)
  // TODO: figure a way to test this better/clean it up. E.g. we should be
  // testing for FileSwitchDir, if it's using two FSdirs and so on
  if (std::dynamic_pointer_cast<FSDirectory>(fsdir) != nullptr == false) {
    delete fsdir;
    assumeTrue(L"test only works for FSDirectory subclasses", false);
  }

  // write a file
  shared_ptr<IndexOutput> out =
      fsdir->createOutput(L"afile", newIOContext(random()));
  out->writeString(L"boo");
  delete out;

  // delete it
  Files::delete (path->resolve(L"afile"));

  int fileCount = fsdir->listAll().size();

  // fsync it
  try {
    fsdir->sync(Collections::singleton(L"afile"));
    fail(L"didn't get expected exception, instead fsync created new files: " +
         Arrays::asList(fsdir->listAll()));
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (FileNotFoundException | NoSuchFileException expected) {
    // ok
  }

  // no new files created
  TestUtil::assertEquals(fileCount, fsdir->listAll().size());

  delete fsdir;
}

void BaseDirectoryTestCase::testRandomLong() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testLongs"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"longs", newIOContext(random()));
  int num = TestUtil::nextInt(random(), 50, 3000);
  std::deque<int64_t> longs(num);
  for (int i = 0; i < longs.size(); i++) {
    longs[i] = TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                                  numeric_limits<int64_t>::max());
    output->writeLong(longs[i]);
  }
  delete output;

  // slice
  shared_ptr<IndexInput> input =
      dir->openInput(L"longs", newIOContext(random()));
  shared_ptr<RandomAccessInput> slice =
      input->randomAccessSlice(0, input->length());
  for (int i = 0; i < longs.size(); i++) {
    TestUtil::assertEquals(longs[i], slice->readLong(i * 8));
  }

  // subslices
  for (int i = 1; i < longs.size(); i++) {
    int64_t offset = i * 8;
    shared_ptr<RandomAccessInput> subslice =
        input->randomAccessSlice(offset, input->length() - offset);
    for (int j = i; j < longs.size(); j++) {
      TestUtil::assertEquals(longs[j], subslice->readLong((j - i) * 8));
    }
  }

  // with padding
  for (int i = 0; i < 7; i++) {
    wstring name = L"longs-" + to_wstring(i);
    shared_ptr<IndexOutput> o = dir->createOutput(name, newIOContext(random()));
    std::deque<char> junk(i);
    random()->nextBytes(junk);
    o->writeBytes(junk, junk.size());
    input->seek(0);
    o->copyBytes(input, input->length());
    delete o;
    shared_ptr<IndexInput> padded =
        dir->openInput(name, newIOContext(random()));
    shared_ptr<RandomAccessInput> whole =
        padded->randomAccessSlice(i, padded->length() - i);
    for (int j = 0; j < longs.size(); j++) {
      TestUtil::assertEquals(longs[j], whole->readLong(j * 8));
    }
    delete padded;
  }

  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testRandomInt() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testInts"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"ints", newIOContext(random()));
  int num = TestUtil::nextInt(random(), 50, 3000);
  std::deque<int> ints(num);
  for (int i = 0; i < ints.size(); i++) {
    ints[i] = random()->nextInt();
    output->writeInt(ints[i]);
  }
  delete output;

  // slice
  shared_ptr<IndexInput> input =
      dir->openInput(L"ints", newIOContext(random()));
  shared_ptr<RandomAccessInput> slice =
      input->randomAccessSlice(0, input->length());
  for (int i = 0; i < ints.size(); i++) {
    TestUtil::assertEquals(ints[i], slice->readInt(i * 4));
  }

  // subslices
  for (int i = 1; i < ints.size(); i++) {
    int64_t offset = i * 4;
    shared_ptr<RandomAccessInput> subslice =
        input->randomAccessSlice(offset, input->length() - offset);
    for (int j = i; j < ints.size(); j++) {
      TestUtil::assertEquals(ints[j], subslice->readInt((j - i) * 4));
    }
  }

  // with padding
  for (int i = 0; i < 7; i++) {
    wstring name = L"ints-" + to_wstring(i);
    shared_ptr<IndexOutput> o = dir->createOutput(name, newIOContext(random()));
    std::deque<char> junk(i);
    random()->nextBytes(junk);
    o->writeBytes(junk, junk.size());
    input->seek(0);
    o->copyBytes(input, input->length());
    delete o;
    shared_ptr<IndexInput> padded =
        dir->openInput(name, newIOContext(random()));
    shared_ptr<RandomAccessInput> whole =
        padded->randomAccessSlice(i, padded->length() - i);
    for (int j = 0; j < ints.size(); j++) {
      TestUtil::assertEquals(ints[j], whole->readInt(j * 4));
    }
    delete padded;
  }
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testRandomShort() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testShorts"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"shorts", newIOContext(random()));
  int num = TestUtil::nextInt(random(), 50, 3000);
  std::deque<short> shorts(num);
  for (int i = 0; i < shorts.size(); i++) {
    shorts[i] = static_cast<short>(random()->nextInt());
    output->writeShort(shorts[i]);
  }
  delete output;

  // slice
  shared_ptr<IndexInput> input =
      dir->openInput(L"shorts", newIOContext(random()));
  shared_ptr<RandomAccessInput> slice =
      input->randomAccessSlice(0, input->length());
  for (int i = 0; i < shorts.size(); i++) {
    TestUtil::assertEquals(shorts[i], slice->readShort(i * 2));
  }

  // subslices
  for (int i = 1; i < shorts.size(); i++) {
    int64_t offset = i * 2;
    shared_ptr<RandomAccessInput> subslice =
        input->randomAccessSlice(offset, input->length() - offset);
    for (int j = i; j < shorts.size(); j++) {
      TestUtil::assertEquals(shorts[j], subslice->readShort((j - i) * 2));
    }
  }

  // with padding
  for (int i = 0; i < 7; i++) {
    wstring name = L"shorts-" + to_wstring(i);
    shared_ptr<IndexOutput> o = dir->createOutput(name, newIOContext(random()));
    std::deque<char> junk(i);
    random()->nextBytes(junk);
    o->writeBytes(junk, junk.size());
    input->seek(0);
    o->copyBytes(input, input->length());
    delete o;
    shared_ptr<IndexInput> padded =
        dir->openInput(name, newIOContext(random()));
    shared_ptr<RandomAccessInput> whole =
        padded->randomAccessSlice(i, padded->length() - i);
    for (int j = 0; j < shorts.size(); j++) {
      TestUtil::assertEquals(shorts[j], whole->readShort(j * 2));
    }
    delete padded;
  }
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testRandomByte() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"testBytes"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"bytes", newIOContext(random()));
  int num = TestUtil::nextInt(random(), 50, 3000);
  std::deque<char> bytes(num);
  random()->nextBytes(bytes);
  for (int i = 0; i < bytes.size(); i++) {
    output->writeByte(bytes[i]);
  }
  delete output;

  // slice
  shared_ptr<IndexInput> input =
      dir->openInput(L"bytes", newIOContext(random()));
  shared_ptr<RandomAccessInput> slice =
      input->randomAccessSlice(0, input->length());
  for (int i = 0; i < bytes.size(); i++) {
    TestUtil::assertEquals(bytes[i], slice->readByte(i));
  }

  // subslices
  for (int i = 1; i < bytes.size(); i++) {
    int64_t offset = i;
    shared_ptr<RandomAccessInput> subslice =
        input->randomAccessSlice(offset, input->length() - offset);
    for (int j = i; j < bytes.size(); j++) {
      TestUtil::assertEquals(bytes[j], subslice->readByte(j - i));
    }
  }

  // with padding
  for (int i = 0; i < 7; i++) {
    wstring name = L"bytes-" + to_wstring(i);
    shared_ptr<IndexOutput> o = dir->createOutput(name, newIOContext(random()));
    std::deque<char> junk(i);
    random()->nextBytes(junk);
    o->writeBytes(junk, junk.size());
    input->seek(0);
    o->copyBytes(input, input->length());
    delete o;
    shared_ptr<IndexInput> padded =
        dir->openInput(name, newIOContext(random()));
    shared_ptr<RandomAccessInput> whole =
        padded->randomAccessSlice(i, padded->length() - i);
    for (int j = 0; j < bytes.size(); j++) {
      TestUtil::assertEquals(bytes[j], whole->readByte(j));
    }
    delete padded;
  }
  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testSliceOfSlice() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"sliceOfSlice"));
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"bytes", newIOContext(random()));
  constexpr int num;
  if (TEST_NIGHTLY) {
    num = TestUtil::nextInt(random(), 250, 2500);
  } else {
    num = TestUtil::nextInt(random(), 50, 250);
  }
  std::deque<char> bytes(num);
  random()->nextBytes(bytes);
  for (int i = 0; i < bytes.size(); i++) {
    output->writeByte(bytes[i]);
  }
  delete output;

  shared_ptr<IndexInput> input =
      dir->openInput(L"bytes", newIOContext(random()));
  // seek to a random spot shouldnt impact slicing.
  input->seek(TestUtil::nextLong(random(), 0, input->length()));
  for (int i = 0; i < num; i += 16) {
    shared_ptr<IndexInput> slice1 = input->slice(L"slice1", i, num - i);
    TestUtil::assertEquals(0, slice1->getFilePointer());
    TestUtil::assertEquals(num - i, slice1->length());

    // seek to a random spot shouldnt impact slicing.
    slice1->seek(TestUtil::nextLong(random(), 0, slice1->length()));
    for (int j = 0; j < slice1->length(); j += 16) {
      shared_ptr<IndexInput> slice2 = slice1->slice(L"slice2", j, num - i - j);
      TestUtil::assertEquals(0, slice2->getFilePointer());
      TestUtil::assertEquals(num - i - j, slice2->length());
      std::deque<char> data(num);
      System::arraycopy(bytes, 0, data, 0, i + j);
      if (random()->nextBoolean()) {
        // read the bytes for this slice-of-slice
        slice2->readBytes(data, i + j, num - i - j);
      } else {
        // seek to a random spot in between, read some, seek back and read the
        // rest
        int64_t seek = TestUtil::nextLong(random(), 0, slice2->length());
        slice2->seek(seek);
        slice2->readBytes(data, static_cast<int>(i + j + seek),
                          static_cast<int>(num - i - j - seek));
        slice2->seek(0);
        slice2->readBytes(data, i + j, static_cast<int>(seek));
      }
      assertArrayEquals(bytes, data);
    }
  }

  delete input;
  delete dir;
}

void BaseDirectoryTestCase::testLargeWrites() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir(L"largeWrites"));
  shared_ptr<IndexOutput> os =
      dir->createOutput(L"testBufferStart.txt", newIOContext(random()));

  std::deque<char> largeBuf(2048);
  random()->nextBytes(largeBuf);

  int64_t currentPos = os->getFilePointer();
  os->writeBytes(largeBuf, largeBuf.size());

  try {
    TestUtil::assertEquals(currentPos + largeBuf.size(), os->getFilePointer());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete os;
  }
  delete dir;
}

void BaseDirectoryTestCase::testIndexOutputToString() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"camelCase.txt", newIOContext(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(out->toString(),
             out->toString()->find(L"camelCase.txt") != wstring::npos);
  delete out;
  delete dir;
}

void BaseDirectoryTestCase::testDoubleCloseDirectory() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"foobar", newIOContext(random()));
  out->writeString(L"testing");
  delete out;
  delete dir;
  delete dir; // close again
}

void BaseDirectoryTestCase::testDoubleCloseOutput() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"foobar", newIOContext(random()));
  out->writeString(L"testing");
  delete out;
  delete out; // close again
  delete dir;
}

void BaseDirectoryTestCase::testDoubleCloseInput() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  shared_ptr<IndexOutput> out =
      dir->createOutput(L"foobar", newIOContext(random()));
  out->writeString(L"testing");
  delete out;
  shared_ptr<IndexInput> in_ =
      dir->openInput(L"foobar", newIOContext(random()));
  TestUtil::assertEquals(L"testing", in_->readString());
  delete in_;
  delete in_; // close again
  delete dir;
}

void BaseDirectoryTestCase::testCreateTempOutput() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  deque<wstring> names = deque<wstring>();
  int iters = atLeast(50);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<IndexOutput> out =
        dir->createTempOutput(L"foo", L"bar", newIOContext(random()));
    names.push_back(out->getName());
    out->writeVInt(iter);
    delete out;
  }
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<IndexInput> in_ =
        dir->openInput(names[iter], newIOContext(random()));
    TestUtil::assertEquals(iter, in_->readVInt());
    delete in_;
  }
  shared_ptr<Set<wstring>> files =
      unordered_set<wstring>(Arrays::asList(dir->listAll()));
  // In case ExtrasFS struck:
  files->remove(L"extra0");
  TestUtil::assertEquals(unordered_set<wstring>(names), files);
  delete dir;
}

void BaseDirectoryTestCase::testSeekToEndOfFile() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Directory dir =
  // getDirectory(createTempDir()))
  {
    Directory dir = getDirectory(createTempDir());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexOutput out = dir.createOutput("a",
    // IOContext.DEFAULT))
    {
      IndexOutput out = dir->createOutput(L"a", IOContext::DEFAULT);
      for (int i = 0; i < 1024; ++i) {
        out->writeByte(static_cast<char>(0));
      }
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexInput in = dir.openInput("a",
    // IOContext.DEFAULT))
    {
      IndexInput in_ = dir->openInput(L"a", IOContext::DEFAULT);
      in_->seek(100);
      TestUtil::assertEquals(100, in_->getFilePointer());
      in_->seek(1024);
      TestUtil::assertEquals(1024, in_->getFilePointer());
    }
  }
}

void BaseDirectoryTestCase::testSeekBeyondEndOfFile() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Directory dir =
  // getDirectory(createTempDir()))
  {
    Directory dir = getDirectory(createTempDir());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexOutput out = dir.createOutput("a",
    // IOContext.DEFAULT))
    {
      IndexOutput out = dir->createOutput(L"a", IOContext::DEFAULT);
      for (int i = 0; i < 1024; ++i) {
        out->writeByte(static_cast<char>(0));
      }
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexInput in = dir.openInput("a",
    // IOContext.DEFAULT))
    {
      IndexInput in_ = dir->openInput(L"a", IOContext::DEFAULT);
      in_->seek(100);
      TestUtil::assertEquals(100, in_->getFilePointer());
      expectThrows(EOFException::typeid, [&]() { in_->seek(1025); });
    }
  }
}

void BaseDirectoryTestCase::testPendingDeletions() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Directory dir =
  // getDirectory(addVirusChecker(createTempDir())))
  {
    Directory dir = getDirectory(addVirusChecker(createTempDir()));
    assumeTrue(L"we can only install VirusCheckingFS on an FSDirectory",
               std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr);
    shared_ptr<FSDirectory> fsDir = std::static_pointer_cast<FSDirectory>(dir);

    // Keep trying until virus checker refuses to delete:
    const wstring fileName;
    while (true) {
      // create a random filename (segment file name style), so it cannot hit
      // windows problem with special filenames ("con", "com1",...):
      wstring candidate = IndexFileNames::segmentFileName(
          TestUtil::randomSimpleString(random(), 1, 6),
          TestUtil::randomSimpleString(random()), L"test");
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (IndexOutput out =
      // dir.createOutput(candidate, IOContext.DEFAULT))
      {
        IndexOutput out = dir->createOutput(candidate, IOContext::DEFAULT);
        out->getFilePointer(); // just fake access to prevent compiler warning
      }
      fsDir->deleteFile(candidate);
      if (fsDir->getPendingDeletions()->size() > 0) {
        // good: virus checker struck and prevented deletion of fileName
        fileName = candidate;
        break;
      }
    }

    // Make sure listAll does NOT include the file:
    assertFalse(Arrays::asList(fsDir->listAll())->contains(fileName));

    // Make sure fileLength claims it's deleted:
    expectThrows(NoSuchFileException::typeid,
                 [&]() { fsDir->fileLength(fileName); });

    // Make sure rename fails:
    expectThrows(NoSuchFileException::typeid,
                 [&]() { fsDir->rename(fileName, L"file2"); });

    // Make sure delete fails:
    expectThrows(NoSuchFileException::typeid,
                 [&]() { fsDir->deleteFile(fileName); });

    // Make sure we cannot open it for reading:
    expectThrows(NoSuchFileException::typeid,
                 [&]() { fsDir->openInput(fileName, IOContext::DEFAULT); });
  }
}

void BaseDirectoryTestCase::testListAllIsSorted() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Directory dir =
  // getDirectory(createTempDir()))
  {
    Directory dir = getDirectory(createTempDir());
    int count = atLeast(20);
    shared_ptr<Set<wstring>> names = unordered_set<wstring>();
    while (names->size() < count) {
      // create a random filename (segment file name style), so it cannot hit
      // windows problem with special filenames ("con", "com1",...):
      wstring name = IndexFileNames::segmentFileName(
          TestUtil::randomSimpleString(random(), 1, 6),
          TestUtil::randomSimpleString(random()), L"test");
      if (random()->nextInt(5) == 1) {
        shared_ptr<IndexOutput> out =
            dir->createTempOutput(name, L"foo", IOContext::DEFAULT);
        names->add(out->getName());
        delete out;
      } else if (names->contains(name) == false) {
        shared_ptr<IndexOutput> out =
            dir->createOutput(name, IOContext::DEFAULT);
        names->add(out->getName());
        delete out;
      }
    }
    std::deque<wstring> actual = dir->listAll();
    std::deque<wstring> expected = actual.clone();
    Arrays::sort(expected);
    TestUtil::assertEquals(expected, actual);
  }
}
} // namespace org::apache::lucene::store