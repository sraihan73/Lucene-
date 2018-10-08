using namespace std;

#include "TestPointValues.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using StringField = org::apache::lucene::document::StringField;
using Store = org::apache::lucene::document::Field::Store;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using PointValues = org::apache::lucene::index::PointValues;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPointValues::testUpgradeFieldToPoints() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"dim", L"foo", Field::Store::NO));
  w->addDocument(doc);
  delete w;

  iwc = newIndexWriterConfig();
  w = make_shared<IndexWriter>(dir, iwc);
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  delete w;
  delete dir;
}

void TestPointValues::testIllegalDimChangeOneDoc() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                          std::deque<char>(4)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 1 to 2 for field=\"dim\"",
      expected.what());
  delete w;
  delete dir;
}

void TestPointValues::testIllegalDimChangeTwoDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                           std::deque<char>(4)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 1 to 2 for field=\"dim\"",
      expected.what());

  delete w;
  delete dir;
}

void TestPointValues::testIllegalDimChangeTwoSegments() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  w->commit();

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                           std::deque<char>(4)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 1 to 2 for field=\"dim\"",
      expected.what());

  delete w;
  delete dir;
}

void TestPointValues::testIllegalDimChangeTwoWriters() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));

  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                           std::deque<char>(4)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w2->addDocument(doc2); });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 1 to 2 for field=\"dim\"",
      expected.what());

  delete w2;
  delete dir;
}

void TestPointValues::testIllegalDimChangeViaAddIndexesDirectory() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2,
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                          std::deque<char>(4)));
  w2->addDocument(doc);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    w2->addIndexes(std::deque<std::shared_ptr<Directory>>{dir});
  });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 2 to 1 for field=\"dim\"",
      expected.what());

  IOUtils::close({w2, dir, dir2});
}

void TestPointValues::testIllegalDimChangeViaAddIndexesCodecReader() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2,
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                          std::deque<char>(4)));
  w2->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    w2->addIndexes(std::deque<std::shared_ptr<CodecReader>>{
        std::static_pointer_cast<CodecReader>(getOnlyLeafReader(r))});
  });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 2 to 1 for field=\"dim\"",
      expected.what());

  IOUtils::close({r, w2, dir, dir2});
}

void TestPointValues::testIllegalDimChangeViaAddIndexesSlowCodecReader() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4),
                                          std::deque<char>(4)));
  w2->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { TestUtil::addIndexesSlowly(w2, {r}); });
  TestUtil::assertEquals(
      L"cannot change point dimension count from 2 to 1 for field=\"dim\"",
      expected.what());

  IOUtils::close({r, w2, dir, dir2});
}

void TestPointValues::testIllegalNumBytesChangeOneDoc() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 4 to 6 for field=\"dim\"",
      expected.what());

  delete w;
  delete dir;
}

void TestPointValues::testIllegalNumBytesChangeTwoDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 4 to 6 for field=\"dim\"",
      expected.what());

  delete w;
  delete dir;
}

void TestPointValues::testIllegalNumBytesChangeTwoSegments() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  w->commit();

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 4 to 6 for field=\"dim\"",
      expected.what());

  delete w;
  delete dir;
}

void TestPointValues::testIllegalNumBytesChangeTwoWriters() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w2->addDocument(doc2); });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 4 to 6 for field=\"dim\"",
      expected.what());

  delete w2;
  delete dir;
}

void TestPointValues::testIllegalNumBytesChangeViaAddIndexesDirectory() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));
  w2->addDocument(doc);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    w2->addIndexes(std::deque<std::shared_ptr<Directory>>{dir});
  });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 6 to 4 for field=\"dim\"",
      expected.what());

  IOUtils::close({w2, dir, dir2});
}

void TestPointValues::testIllegalNumBytesChangeViaAddIndexesCodecReader() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));
  w2->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    w2->addIndexes(std::deque<std::shared_ptr<CodecReader>>{
        std::static_pointer_cast<CodecReader>(getOnlyLeafReader(r))});
  });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 6 to 4 for field=\"dim\"",
      expected.what());

  IOUtils::close({r, w2, dir, dir2});
}

void TestPointValues::
    testIllegalNumBytesChangeViaAddIndexesSlowCodecReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(4)));
  w->addDocument(doc);
  delete w;

  shared_ptr<Directory> dir2 = newDirectory();
  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"dim", std::deque<char>(6)));
  w2->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { TestUtil::addIndexesSlowly(w2, {r}); });
  TestUtil::assertEquals(
      L"cannot change point numBytes from 6 to 4 for field=\"dim\"",
      expected.what());

  IOUtils::close({r, w2, dir, dir2});
}

void TestPointValues::testIllegalTooManyBytes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  expectThrows(invalid_argument::typeid, [&]() {
    doc->add(make_shared<BinaryPoint>(
        L"dim", std::deque<char>(PointValues::MAX_NUM_BYTES + 1)));
  });

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<IntPoint>(L"dim", 17));
  w->addDocument(doc2);
  delete w;
  delete dir;
}

void TestPointValues::testIllegalTooManyDimensions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<std::deque<char>> values(PointValues::MAX_DIMENSIONS + 1);
  for (int i = 0; i < values.size(); i++) {
    values[i] = std::deque<char>(4);
  }
  expectThrows(invalid_argument::typeid,
               [&]() { doc->add(make_shared<BinaryPoint>(L"dim", values)); });

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<IntPoint>(L"dim", 17));
  w->addDocument(doc2);
  delete w;
  delete dir;
}

void TestPointValues::testDifferentCodecs1() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(Codec::forName(L"Lucene70"));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 1));
  w->addDocument(doc);
  delete w;

  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(Codec::forName(L"SimpleText"));
  w = make_shared<IndexWriter>(dir, iwc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 1));
  w->addDocument(doc);

  w->forceMerge(1);
  delete w;
  delete dir;
}

void TestPointValues::testDifferentCodecs2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(Codec::forName(L"SimpleText"));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 1));
  w->addDocument(doc);
  delete w;

  iwc = make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(Codec::forName(L"Lucene70"));
  w = make_shared<IndexWriter>(dir, iwc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 1));
  w->addDocument(doc);

  w->forceMerge(1);
  delete w;
  delete dir;
}

void TestPointValues::testInvalidIntPointUsage() 
{
  shared_ptr<IntPoint> field = make_shared<IntPoint>(L"field", 17, 42);

  expectThrows(invalid_argument::typeid, [&]() { field->setIntValue(14); });

  expectThrows(IllegalStateException::typeid, [&]() { field->numericValue(); });
}

void TestPointValues::testInvalidLongPointUsage() 
{
  shared_ptr<LongPoint> field = make_shared<LongPoint>(L"field", 17, 42);

  expectThrows(invalid_argument::typeid, [&]() { field->setLongValue(14); });

  expectThrows(IllegalStateException::typeid, [&]() { field->numericValue(); });
}

void TestPointValues::testInvalidFloatPointUsage() 
{
  shared_ptr<FloatPoint> field = make_shared<FloatPoint>(L"field", 17, 42);

  expectThrows(invalid_argument::typeid, [&]() { field->setFloatValue(14); });

  expectThrows(IllegalStateException::typeid, [&]() { field->numericValue(); });
}

void TestPointValues::testInvalidDoublePointUsage() 
{
  shared_ptr<DoublePoint> field = make_shared<DoublePoint>(L"field", 17, 42);

  expectThrows(invalid_argument::typeid, [&]() { field->setDoubleValue(14); });

  expectThrows(IllegalStateException::typeid, [&]() { field->numericValue(); });
}

void TestPointValues::testTieBreakByDocID() 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 17));
  for (int i = 0; i < 300000; i++) {
    w->addDocument(doc);
    if (random()->nextInt(1000) == 17) {
      w->commit();
    }
  }

  shared_ptr<IndexReader> r = DirectoryReader::open(w);

  for (auto ctx : r->leaves()) {
    shared_ptr<PointValues> points = ctx->reader()->getPointValues(L"int");
    points->intersect(
        make_shared<IntersectVisitorAnonymousInnerClass>(shared_from_this()));
  }

  delete r;
  delete w;
  delete dir;
}

TestPointValues::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<TestPointValues> outerInstance)
{
  this->outerInstance = outerInstance;
  lastDocID = -1;
}

void TestPointValues::IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  if (docID < lastDocID) {
    fail(L"docs out of order: docID=" + to_wstring(docID) + L" but lastDocID=" +
         lastDocID);
  }
  lastDocID = docID;
}

void TestPointValues::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue)
{
  visit(docID);
}

Relation TestPointValues::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  if (LuceneTestCase::random()->nextBoolean()) {
    return Relation::CELL_CROSSES_QUERY;
  } else {
    return Relation::CELL_INSIDE_QUERY;
  }
}

void TestPointValues::testDeleteAllPointDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  doc->push_back(make_shared<IntPoint>(L"int", 17));
  w->addDocument(doc);
  w->addDocument(make_shared<Document>());
  w->commit();

  w->deleteDocuments({make_shared<Term>(L"id", L"0")});

  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = w->getReader();
  assertNull(r->leaves()->get(0).reader().getPointValues(L"int"));
  delete w;
  r->close();
  delete dir;
}

void TestPointValues::testPointsFieldMissingFromOneSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = FSDirectory::open(createTempDir());
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  doc->push_back(make_shared<IntPoint>(L"int0", 0));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int1", 17));
  w->addDocument(doc);
  w->forceMerge(1);

  delete w;
  delete dir;
}

void TestPointValues::testSparsePoints() 
{
  shared_ptr<Directory> dir = newDirectory();
  int numDocs = atLeast(1000);
  int numFields = TestUtil::nextInt(random(), 1, 10);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  std::deque<int> fieldDocCounts(numFields);
  std::deque<int> fieldSizes(numFields);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    for (int field = 0; field < numFields; field++) {
      wstring fieldName = L"int" + to_wstring(field);
      if (random()->nextInt(100) == 17) {
        doc->push_back(make_shared<IntPoint>(fieldName, random()->nextInt()));
        fieldDocCounts[field]++;
        fieldSizes[field]++;

        if (random()->nextInt(10) == 5) {
          // add same field again!
          doc->push_back(make_shared<IntPoint>(fieldName, random()->nextInt()));
          fieldSizes[field]++;
        }
      }
    }
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  for (int field = 0; field < numFields; field++) {
    int docCount = 0;
    int size = 0;
    wstring fieldName = L"int" + to_wstring(field);
    for (auto ctx : r->leaves()) {
      shared_ptr<PointValues> points = ctx->reader()->getPointValues(fieldName);
      if (points != nullptr) {
        docCount += points->getDocCount();
        size += points->size();
      }
    }
    TestUtil::assertEquals(fieldDocCounts[field], docCount);
    TestUtil::assertEquals(fieldSizes[field], size);
  }
  delete r;
  delete w;
  delete dir;
}

void TestPointValues::testCheckIndexIncludesPoints() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int1", 17));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int1", 44));
  doc->push_back(make_shared<IntPoint>(L"int2", -17));
  w->addDocument(doc);
  delete w;

  shared_ptr<ByteArrayOutputStream> output =
      make_shared<ByteArrayOutputStream>();
  shared_ptr<CheckIndex::Status> status =
      TestUtil::checkIndex(dir, false, true, output);
  TestUtil::assertEquals(1, status->segmentInfos.size());
  shared_ptr<CheckIndex::Status::SegmentInfoStatus> segStatus =
      status->segmentInfos[0];
  // total 3 point values were index:
  TestUtil::assertEquals(3, segStatus->pointsStatus->totalValuePoints);
  // ... across 2 fields:
  TestUtil::assertEquals(2, segStatus->pointsStatus->totalValueFields);

  // Make sure CheckIndex in fact declares that it is testing points!
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(output->toString(IOUtils::UTF_8)->contains(L"test: points..."));
  delete dir;
}

void TestPointValues::testMergedStatsEmptyReader() 
{
  shared_ptr<IndexReader> reader = make_shared<MultiReader>();
  assertNull(PointValues::getMinPackedValue(reader, L"field"));
  assertNull(PointValues::getMaxPackedValue(reader, L"field"));
  TestUtil::assertEquals(0, PointValues::getDocCount(reader, L"field"));
  TestUtil::assertEquals(0, PointValues::size(reader, L"field"));
}

void TestPointValues::testMergedStatsOneSegmentWithoutPoints() throw(
    IOException)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(nullptr))
               ->setMergePolicy(NoMergePolicy::INSTANCE));
  w->addDocument(make_shared<Document>());
  DirectoryReader::open(w)->close();
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"field", numeric_limits<int>::min()));
  w->addDocument(doc);
  shared_ptr<IndexReader> reader = DirectoryReader::open(w);

  assertArrayEquals(std::deque<char>(4),
                    PointValues::getMinPackedValue(reader, L"field"));
  assertArrayEquals(std::deque<char>(4),
                    PointValues::getMaxPackedValue(reader, L"field"));
  TestUtil::assertEquals(1, PointValues::getDocCount(reader, L"field"));
  TestUtil::assertEquals(1, PointValues::size(reader, L"field"));

  assertNull(PointValues::getMinPackedValue(reader, L"field2"));
  assertNull(PointValues::getMaxPackedValue(reader, L"field2"));
  TestUtil::assertEquals(0, PointValues::getDocCount(reader, L"field2"));
  TestUtil::assertEquals(0, PointValues::size(reader, L"field2"));
}

void TestPointValues::testMergedStatsAllPointsDeleted() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  w->addDocument(make_shared<Document>());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"field", numeric_limits<int>::min()));
  doc->push_back(make_shared<StringField>(L"delete", L"yes", Field::Store::NO));
  w->addDocument(doc);
  w->forceMerge(1);
  w->deleteDocuments({make_shared<Term>(L"delete", L"yes")});
  w->addDocument(make_shared<Document>());
  w->forceMerge(1);
  shared_ptr<IndexReader> reader = DirectoryReader::open(w);

  assertNull(PointValues::getMinPackedValue(reader, L"field"));
  assertNull(PointValues::getMaxPackedValue(reader, L"field"));
  TestUtil::assertEquals(0, PointValues::getDocCount(reader, L"field"));
  TestUtil::assertEquals(0, PointValues::size(reader, L"field"));
}

void TestPointValues::testMergedStats() 
{
  constexpr int iters = atLeast(3);
  for (int iter = 0; iter < iters; ++iter) {
    doTestMergedStats();
  }
}

std::deque<std::deque<char>>
TestPointValues::randomBinaryValue(int numDims, int numBytesPerDim)
{
  std::deque<std::deque<char>> bytes(numDims);
  for (int i = 0; i < numDims; ++i) {
    bytes[i] = std::deque<char>(numBytesPerDim);
    random()->nextBytes(bytes[i]);
  }
  return bytes;
}

void TestPointValues::doTestMergedStats() 
{
  constexpr int numDims = TestUtil::nextInt(random(), 1, 8);
  constexpr int numBytesPerDim = TestUtil::nextInt(random(), 1, 16);
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  constexpr int numDocs = TestUtil::nextInt(random(), 10, 20);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    constexpr int numPoints = random()->nextInt(3);
    for (int j = 0; j < numPoints; ++j) {
      doc->push_back(make_shared<BinaryPoint>(
          L"field", randomBinaryValue(numDims, numBytesPerDim)));
    }
    w->addDocument(doc);
    if (random()->nextBoolean()) {
      DirectoryReader::open(w)->close();
    }
  }

  shared_ptr<IndexReader> *const reader1 = DirectoryReader::open(w);
  w->forceMerge(1);
  shared_ptr<IndexReader> *const reader2 = DirectoryReader::open(w);
  shared_ptr<PointValues> *const expected =
      getOnlyLeafReader(reader2)->getPointValues(L"field");
  if (expected == nullptr) {
    assertNull(PointValues::getMinPackedValue(reader1, L"field"));
    assertNull(PointValues::getMaxPackedValue(reader1, L"field"));
    TestUtil::assertEquals(0, PointValues::getDocCount(reader1, L"field"));
    TestUtil::assertEquals(0, PointValues::size(reader1, L"field"));
  } else {
    assertArrayEquals(expected->getMinPackedValue(),
                      PointValues::getMinPackedValue(reader1, L"field"));
    assertArrayEquals(expected->getMaxPackedValue(),
                      PointValues::getMaxPackedValue(reader1, L"field"));
    TestUtil::assertEquals(expected->getDocCount(),
                           PointValues::getDocCount(reader1, L"field"));
    TestUtil::assertEquals(expected->size(),
                           PointValues::size(reader1, L"field"));
  }
  IOUtils::close({w, reader1, reader2, dir});
}
} // namespace org::apache::lucene::index