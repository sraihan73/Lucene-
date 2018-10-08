using namespace std;

#include "TestField.h"

namespace org::apache::lucene::document
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestField::testDoublePoint() 
{
  shared_ptr<Field> field = make_shared<DoublePoint>(L"foo", 5);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  field->setDoubleValue(6); // ok
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6, field->numericValue()->doubleValue(), 0.0);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"DoublePoint <foo:6.0>", field->toString());
}

void TestField::testDoublePoint2D() 
{
  shared_ptr<DoublePoint> field = make_shared<DoublePoint>(L"foo", 5, 4);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  field->setDoubleValues({6, 7}); // ok
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  shared_ptr<IllegalStateException> expected = expectThrows(
      IllegalStateException::typeid, [&]() { field->numericValue(); });
  assertTrue(expected->getMessage()->contains(
      L"cannot convert to a single numeric value"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"DoublePoint <foo:6.0,7.0>", field->toString());
}

void TestField::testDoubleDocValuesField() 
{
  shared_ptr<DoubleDocValuesField> field =
      make_shared<DoubleDocValuesField>(L"foo", 5);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  field->setDoubleValue(6); // ok
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6, Double::longBitsToDouble(field->numericValue()->longValue()),
               0.0);
}

void TestField::testFloatDocValuesField() 
{
  shared_ptr<FloatDocValuesField> field =
      make_shared<FloatDocValuesField>(L"foo", 5.0f);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  field->setFloatValue(6.0f); // ok
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6.0f, Float::intBitsToFloat(field->numericValue()->intValue()),
               0.0f);
}

void TestField::testFloatPoint() 
{
  shared_ptr<Field> field = make_shared<FloatPoint>(L"foo", 5.0f);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  field->setFloatValue(6.0f); // ok
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6.0f, field->numericValue()->floatValue(), 0.0f);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"FloatPoint <foo:6.0>", field->toString());
}

void TestField::testFloatPoint2D() 
{
  shared_ptr<FloatPoint> field = make_shared<FloatPoint>(L"foo", 5.0f, 4.0f);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  field->setFloatValues({6.0f, 7.0f}); // ok
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  shared_ptr<IllegalStateException> expected = expectThrows(
      IllegalStateException::typeid, [&]() { field->numericValue(); });
  assertTrue(expected->getMessage()->contains(
      L"cannot convert to a single numeric value"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"FloatPoint <foo:6.0,7.0>", field->toString());
}

void TestField::testIntPoint() 
{
  shared_ptr<Field> field = make_shared<IntPoint>(L"foo", 5);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  field->setIntValue(6); // ok
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6, field->numericValue()->intValue());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"IntPoint <foo:6>", field->toString());
}

void TestField::testIntPoint2D() 
{
  shared_ptr<IntPoint> field = make_shared<IntPoint>(L"foo", 5, 4);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  field->setIntValues({6, 7}); // ok
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  shared_ptr<IllegalStateException> expected = expectThrows(
      IllegalStateException::typeid, [&]() { field->numericValue(); });
  assertTrue(expected->getMessage()->contains(
      L"cannot convert to a single numeric value"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"IntPoint <foo:6,7>", field->toString());
}

void TestField::testNumericDocValuesField() 
{
  shared_ptr<NumericDocValuesField> field =
      make_shared<NumericDocValuesField>(L"foo", 5LL);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  field->setLongValue(6); // ok
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6LL, field->numericValue()->longValue());
}

void TestField::testLongPoint() 
{
  shared_ptr<Field> field = make_shared<LongPoint>(L"foo", 5);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  field->setLongValue(6); // ok
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(6, field->numericValue()->intValue());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"LongPoint <foo:6>", field->toString());
}

void TestField::testLongPoint2D() 
{
  shared_ptr<LongPoint> field = make_shared<LongPoint>(L"foo", 5, 4);

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  field->setLongValues({6, 7}); // ok
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  shared_ptr<IllegalStateException> expected = expectThrows(
      IllegalStateException::typeid, [&]() { field->numericValue(); });
  assertTrue(expected->getMessage()->contains(
      L"cannot convert to a single numeric value"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"LongPoint <foo:6,7>", field->toString());
}

void TestField::testSortedBytesDocValuesField() 
{
  shared_ptr<SortedDocValuesField> field =
      make_shared<SortedDocValuesField>(L"foo", make_shared<BytesRef>(L"bar"));

  trySetByteValue(field);
  field->setBytesValue((wstring(L"fubar")).getBytes(StandardCharsets::UTF_8));
  field->setBytesValue(make_shared<BytesRef>(L"baz"));
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(make_shared<BytesRef>(L"baz"), field->binaryValue());
}

void TestField::testBinaryDocValuesField() 
{
  shared_ptr<BinaryDocValuesField> field =
      make_shared<BinaryDocValuesField>(L"foo", make_shared<BytesRef>(L"bar"));

  trySetByteValue(field);
  field->setBytesValue((wstring(L"fubar")).getBytes(StandardCharsets::UTF_8));
  field->setBytesValue(make_shared<BytesRef>(L"baz"));
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(make_shared<BytesRef>(L"baz"), field->binaryValue());
}

void TestField::testStringField() 
{
  std::deque<std::shared_ptr<Field>> fields = {
      make_shared<StringField>(L"foo", L"bar", Field::Store::NO),
      make_shared<StringField>(L"foo", L"bar", Field::Store::YES)};

  for (auto field : fields) {
    trySetByteValue(field);
    trySetBytesValue(field);
    trySetBytesRefValue(field);
    trySetDoubleValue(field);
    trySetIntValue(field);
    trySetFloatValue(field);
    trySetLongValue(field);
    trySetReaderValue(field);
    trySetShortValue(field);
    field->setStringValue(L"baz");
    trySetTokenStreamValue(field);

    assertEquals(L"baz", field->stringValue());
  }
}

void TestField::testTextFieldString() 
{
  std::deque<std::shared_ptr<Field>> fields = {
      make_shared<TextField>(L"foo", L"bar", Field::Store::NO),
      make_shared<TextField>(L"foo", L"bar", Field::Store::YES)};

  for (auto field : fields) {
    trySetByteValue(field);
    trySetBytesValue(field);
    trySetBytesRefValue(field);
    trySetDoubleValue(field);
    trySetIntValue(field);
    trySetFloatValue(field);
    trySetLongValue(field);
    trySetReaderValue(field);
    trySetShortValue(field);
    field->setStringValue(L"baz");
    field->setTokenStream(
        make_shared<CannedTokenStream>(make_shared<Token>(L"foo", 0, 3)));

    assertEquals(L"baz", field->stringValue());
  }
}

void TestField::testTextFieldReader() 
{
  shared_ptr<Field> field =
      make_shared<TextField>(L"foo", make_shared<StringReader>(L"bar"));

  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  field->setReaderValue(make_shared<StringReader>(L"foobar"));
  trySetShortValue(field);
  trySetStringValue(field);
  field->setTokenStream(
      make_shared<CannedTokenStream>(make_shared<Token>(L"foo", 0, 3)));

  assertNotNull(field->readerValue());
}

void TestField::testStoredFieldBytes() 
{
  std::deque<std::shared_ptr<Field>> fields = {
      make_shared<StoredField>(
          L"foo", (wstring(L"bar")).getBytes(StandardCharsets::UTF_8)),
      make_shared<StoredField>(
          L"foo", (wstring(L"bar")).getBytes(StandardCharsets::UTF_8), 0, 3),
      make_shared<StoredField>(L"foo", make_shared<BytesRef>(L"bar"))};

  for (auto field : fields) {
    trySetByteValue(field);
    field->setBytesValue((wstring(L"baz")).getBytes(StandardCharsets::UTF_8));
    field->setBytesValue(make_shared<BytesRef>(L"baz"));
    trySetDoubleValue(field);
    trySetIntValue(field);
    trySetFloatValue(field);
    trySetLongValue(field);
    trySetReaderValue(field);
    trySetShortValue(field);
    trySetStringValue(field);
    trySetTokenStreamValue(field);

    assertEquals(make_shared<BytesRef>(L"baz"), field->binaryValue());
  }
}

void TestField::testStoredFieldString() 
{
  shared_ptr<Field> field = make_shared<StoredField>(L"foo", L"bar");
  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  field->setStringValue(L"baz");
  trySetTokenStreamValue(field);

  assertEquals(L"baz", field->stringValue());
}

void TestField::testStoredFieldInt() 
{
  shared_ptr<Field> field = make_shared<StoredField>(L"foo", 1);
  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  field->setIntValue(5);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(5, field->numericValue()->intValue());
}

void TestField::testStoredFieldDouble() 
{
  shared_ptr<Field> field = make_shared<StoredField>(L"foo", 1);
  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  field->setDoubleValue(5);
  trySetIntValue(field);
  trySetFloatValue(field);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(5, field->numericValue()->doubleValue(), 0.0);
}

void TestField::testStoredFieldFloat() 
{
  shared_ptr<Field> field = make_shared<StoredField>(L"foo", 1.0F);
  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  field->setFloatValue(5.0f);
  trySetLongValue(field);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(5.0f, field->numericValue()->floatValue(), 0.0f);
}

void TestField::testStoredFieldLong() 
{
  shared_ptr<Field> field = make_shared<StoredField>(L"foo", 1LL);
  trySetByteValue(field);
  trySetBytesValue(field);
  trySetBytesRefValue(field);
  trySetDoubleValue(field);
  trySetIntValue(field);
  trySetFloatValue(field);
  field->setLongValue(5);
  trySetReaderValue(field);
  trySetShortValue(field);
  trySetStringValue(field);
  trySetTokenStreamValue(field);

  assertEquals(5LL, field->numericValue()->longValue());
}

void TestField::testIndexedBinaryField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<BytesRef> br = make_shared<BytesRef>(std::deque<char>(5));
  shared_ptr<Field> field =
      make_shared<StringField>(L"binary", br, Field::Store::YES);
  assertEquals(br, field->binaryValue());
  doc->push_back(field);
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<TopDocs> hits =
      s->search(make_shared<TermQuery>(make_shared<Term>(L"binary", br)), 1);
  assertEquals(1, hits->totalHits);
  shared_ptr<Document> storedDoc = s->doc(hits->scoreDocs[0]->doc);
  assertEquals(br, storedDoc->getField(L"binary")->binaryValue());

  delete r;
  delete w;
  delete dir;
}

void TestField::trySetByteValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setByteValue(static_cast<char>(10)); });
}

void TestField::trySetBytesValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid, [&]() {
    f->setBytesValue(std::deque<char>{5, 5});
  });
}

void TestField::trySetBytesRefValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setBytesValue(make_shared<BytesRef>(L"bogus")); });
}

void TestField::trySetDoubleValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setDoubleValue(numeric_limits<double>::max()); });
}

void TestField::trySetIntValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setIntValue(numeric_limits<int>::max()); });
}

void TestField::trySetLongValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setLongValue(numeric_limits<int64_t>::max()); });
}

void TestField::trySetFloatValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setFloatValue(numeric_limits<float>::max()); });
}

void TestField::trySetReaderValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid, [&]() {
    f->setReaderValue(make_shared<StringReader>(L"BOO!"));
  });
}

void TestField::trySetShortValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid,
               [&]() { f->setShortValue(numeric_limits<short>::max()); });
}

void TestField::trySetStringValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid, [&]() { f->setStringValue(L"BOO!"); });
}

void TestField::trySetTokenStreamValue(shared_ptr<Field> f)
{
  expectThrows(invalid_argument::typeid, [&]() {
    f->setTokenStream(
        make_shared<CannedTokenStream>(make_shared<Token>(L"foo", 0, 3)));
  });
}
} // namespace org::apache::lucene::document