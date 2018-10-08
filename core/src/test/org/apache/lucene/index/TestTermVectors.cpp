using namespace std;

#include "TestTermVectors.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::index::IndexReader> TestTermVectors::reader;
shared_ptr<org::apache::lucene::store::Directory> TestTermVectors::directory;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestTermVectors::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true))
          ->setMergePolicy(newLogMergePolicy()));
  // writer.setNoCFSRatio(1.0);
  // writer.infoStream = System.out;
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
    int mod3 = i % 3;
    int mod2 = i % 2;
    if (mod2 == 0 && mod3 == 0) {
      ft->setStoreTermVectors(true);
      ft->setStoreTermVectorOffsets(true);
      ft->setStoreTermVectorPositions(true);
    } else if (mod2 == 0) {
      ft->setStoreTermVectors(true);
      ft->setStoreTermVectorPositions(true);
    } else if (mod3 == 0) {
      ft->setStoreTermVectors(true);
      ft->setStoreTermVectorOffsets(true);
    } else {
      ft->setStoreTermVectors(true);
    }
    doc->push_back(make_shared<Field>(L"field", English::intToEnglish(i), ft));
    // test no term vectors too
    doc->push_back(make_shared<TextField>(L"noTV", English::intToEnglish(i),
                                          Field::Store::YES));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  delete writer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestTermVectors::afterClass() 
{
  delete reader;
  delete directory;
  reader.reset();
  directory.reset();
}

shared_ptr<IndexWriter>
TestTermVectors::createWriter(shared_ptr<Directory> dir) 
{
  return make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
}

void TestTermVectors::createDir(shared_ptr<Directory> dir) 
{
  shared_ptr<IndexWriter> writer = createWriter(dir);
  writer->addDocument(createDoc());
  delete writer;
}

shared_ptr<Document> TestTermVectors::createDoc()
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> *const ft =
      make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  ft->setStoreTermVectorPositions(true);
  doc->push_back(newField(L"c", L"aaa", ft));
  return doc;
}

void TestTermVectors::verifyIndex(shared_ptr<Directory> dir) 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  int numDocs = r->numDocs();
  for (int i = 0; i < numDocs; i++) {
    assertNotNull(L"term vectors should not have been null for document " +
                      to_wstring(i),
                  r->getTermVectors(i)->terms(L"c"));
  }
  delete r;
}

void TestTermVectors::testFullMergeAddDocs() 
{
  shared_ptr<Directory> target = newDirectory();
  shared_ptr<IndexWriter> writer = createWriter(target);
  // with maxBufferedDocs=2, this results in two segments, so that forceMerge
  // actually does something.
  for (int i = 0; i < 4; i++) {
    writer->addDocument(createDoc());
  }
  writer->forceMerge(1);
  delete writer;

  verifyIndex(target);
  delete target;
}

void TestTermVectors::testFullMergeAddIndexesDir() 
{
  std::deque<std::shared_ptr<Directory>> input = {newDirectory(),
                                                   newDirectory()};
  shared_ptr<Directory> target = newDirectory();

  for (auto dir : input) {
    createDir(dir);
  }

  shared_ptr<IndexWriter> writer = createWriter(target);
  writer->addIndexes(input);
  writer->forceMerge(1);
  delete writer;

  verifyIndex(target);

  IOUtils::close({target, input[0], input[1]});
}

void TestTermVectors::testFullMergeAddIndexesReader() 
{
  std::deque<std::shared_ptr<Directory>> input = {newDirectory(),
                                                   newDirectory()};
  shared_ptr<Directory> target = newDirectory();

  for (auto dir : input) {
    createDir(dir);
  }

  shared_ptr<IndexWriter> writer = createWriter(target);
  for (auto dir : input) {
    shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
    TestUtil::addIndexesSlowly(writer, {r});
    r->close();
  }
  writer->forceMerge(1);
  delete writer;

  verifyIndex(target);
  IOUtils::close({target, input[0], input[1]});
}
} // namespace org::apache::lucene::index