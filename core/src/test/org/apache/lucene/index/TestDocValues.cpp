using namespace std;

#include "TestDocValues.h"

namespace org::apache::lucene::index
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDocValues::testEmptyIndex() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  iw->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // ok
  assertNotNull(DocValues::getBinary(r, L"bogus"));
  assertNotNull(DocValues::getNumeric(r, L"bogus"));
  assertNotNull(DocValues::getSorted(r, L"bogus"));
  assertNotNull(DocValues::getSortedSet(r, L"bogus"));
  assertNotNull(DocValues::getSortedNumeric(r, L"bogus"));

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testMisconfiguredField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // errors
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getBinary(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getNumeric(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSorted(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedSet(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedNumeric(r, L"foo"); });

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testNumericField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 3));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // ok
  assertNotNull(DocValues::getNumeric(r, L"foo"));
  assertNotNull(DocValues::getSortedNumeric(r, L"foo"));

  // errors
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getBinary(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSorted(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedSet(r, L"foo"); });

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testBinaryField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"foo", make_shared<BytesRef>(L"bar")));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // ok
  assertNotNull(DocValues::getBinary(r, L"foo"));

  // errors
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getNumeric(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSorted(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedSet(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedNumeric(r, L"foo"); });

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testSortedField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"bar")));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // ok
  assertNotNull(DocValues::getBinary(r, L"foo"));
  assertNotNull(DocValues::getSorted(r, L"foo"));
  assertNotNull(DocValues::getSortedSet(r, L"foo"));

  // errors
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getNumeric(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedNumeric(r, L"foo"); });

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testSortedSetField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"bar")));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // ok
  assertNotNull(DocValues::getSortedSet(r, L"foo"));

  // errors
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getBinary(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getNumeric(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSorted(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedNumeric(r, L"foo"); });

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testSortedNumericField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"foo", 3));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(iw);
  shared_ptr<LeafReader> r = getOnlyLeafReader(dr);

  // ok
  assertNotNull(DocValues::getSortedNumeric(r, L"foo"));

  // errors
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getBinary(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getNumeric(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSorted(r, L"foo"); });
  expectThrows(IllegalStateException::typeid,
               [&]() { DocValues::getSortedSet(r, L"foo"); });

  dr->close();
  delete iw;
  delete dir;
}

void TestDocValues::testAddNullNumericDocValues() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  if (random()->nextBoolean()) {
    doc->push_back(make_shared<NumericDocValuesField>(L"foo", nullptr));
  } else {
    doc->push_back(make_shared<BinaryDocValuesField>(L"foo", nullptr));
  }
  invalid_argument iae =
      expectThrows(invalid_argument::typeid, [&]() { iw->addDocument(doc); });
  assertEquals(L"field=\"foo\": null value not allowed", iae.what());
  IOUtils::close({iw, dir});
}
} // namespace org::apache::lucene::index