using namespace std;

#include "TestFileSwitchDirectory.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using CompressingStoredFieldsWriter =
    org::apache::lucene::codecs::compressing::CompressingStoredFieldsWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexNotFoundException =
    org::apache::lucene::index::IndexNotFoundException;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using TestIndexWriterReader = org::apache::lucene::index::TestIndexWriterReader;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestFileSwitchDirectory::testBasic() 
{
  shared_ptr<Set<wstring>> fileExtensions = unordered_set<wstring>();
  fileExtensions->add(CompressingStoredFieldsWriter::FIELDS_EXTENSION);
  fileExtensions->add(CompressingStoredFieldsWriter::FIELDS_INDEX_EXTENSION);

  shared_ptr<MockDirectoryWrapper> primaryDir =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  primaryDir->setCheckIndexOnClose(false); // only part of an index
  shared_ptr<MockDirectoryWrapper> secondaryDir =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  secondaryDir->setCheckIndexOnClose(false); // only part of an index

  shared_ptr<FileSwitchDirectory> fsd = make_shared<FileSwitchDirectory>(
      fileExtensions, primaryDir, secondaryDir, true);
  // for now we wire the default codec because we rely upon its specific impl
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      fsd, (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
               ->setMergePolicy(newLogMergePolicy(false))
               ->setCodec(TestUtil::getDefaultCodec())
               ->setUseCompoundFile(false));
  TestIndexWriterReader::createIndexNoClose(true, L"ram", writer);
  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  TestUtil::assertEquals(100, reader->maxDoc());
  writer->commit();
  // we should see only fdx,fdt files here
  std::deque<wstring> files = primaryDir->listAll();
  assertTrue(files.size() > 0);
  for (int x = 0; x < files.size(); x++) {
    wstring ext = FileSwitchDirectory::getExtension(files[x]);
    assertTrue(fileExtensions->contains(ext));
  }
  files = secondaryDir->listAll();
  assertTrue(files.size() > 0);
  // we should not see fdx,fdt files here
  for (int x = 0; x < files.size(); x++) {
    wstring ext = FileSwitchDirectory::getExtension(files[x]);
    assertFalse(fileExtensions->contains(ext));
  }
  delete reader;
  delete writer;

  files = fsd->listAll();
  for (int i = 0; i < files.size(); i++) {
    assertNotNull(files[i]);
  }
  delete fsd;
}

shared_ptr<Directory> TestFileSwitchDirectory::newFSSwitchDirectory(
    shared_ptr<Set<wstring>> primaryExtensions) 
{
  shared_ptr<Path> primDir = createTempDir(L"foo");
  shared_ptr<Path> secondDir = createTempDir(L"bar");
  return newFSSwitchDirectory(primDir, secondDir, primaryExtensions);
}

shared_ptr<Directory> TestFileSwitchDirectory::newFSSwitchDirectory(
    shared_ptr<Path> aDir, shared_ptr<Path> bDir,
    shared_ptr<Set<wstring>> primaryExtensions) 
{
  shared_ptr<Directory> a = make_shared<SimpleFSDirectory>(aDir);
  shared_ptr<Directory> b = make_shared<SimpleFSDirectory>(bDir);
  return make_shared<FileSwitchDirectory>(primaryExtensions, a, b, true);
}

void TestFileSwitchDirectory::testNoDir() 
{
  shared_ptr<Path> primDir = createTempDir(L"foo");
  shared_ptr<Path> secondDir = createTempDir(L"bar");
  shared_ptr<Directory> dir = newFSSwitchDirectory(
      primDir, secondDir, Collections::emptySet<wstring>());
  expectThrows(IndexNotFoundException::typeid,
               [&]() { DirectoryReader::open(dir); });

  delete dir;
}

shared_ptr<Directory>
TestFileSwitchDirectory::getDirectory(shared_ptr<Path> path) 
{
  shared_ptr<Set<wstring>> extensions = unordered_set<wstring>();
  if (random()->nextBoolean()) {
    extensions->add(L"cfs");
  }
  if (random()->nextBoolean()) {
    extensions->add(L"prx");
  }
  if (random()->nextBoolean()) {
    extensions->add(L"frq");
  }
  if (random()->nextBoolean()) {
    extensions->add(L"tip");
  }
  if (random()->nextBoolean()) {
    extensions->add(L"tim");
  }
  if (random()->nextBoolean()) {
    extensions->add(L"del");
  }
  return newFSSwitchDirectory(extensions);
}
} // namespace org::apache::lucene::store