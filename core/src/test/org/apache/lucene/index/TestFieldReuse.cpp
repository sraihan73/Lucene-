using namespace std;

#include "TestFieldReuse.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestFieldReuse::testStringField() 
{
  shared_ptr<StringField> stringField =
      make_shared<StringField>(L"foo", L"bar", Field::Store::NO);

  // passing null
  shared_ptr<TokenStream> ts = stringField->tokenStream(nullptr, nullptr);
  assertTokenStreamContents(ts, std::deque<wstring>{L"bar"},
                            std::deque<int>{0}, std::deque<int>{3});

  // now reuse previous stream
  stringField = make_shared<StringField>(L"foo", L"baz", Field::Store::NO);
  shared_ptr<TokenStream> ts2 = stringField->tokenStream(nullptr, ts);
  assertSame(ts, ts);
  assertTokenStreamContents(ts, std::deque<wstring>{L"baz"},
                            std::deque<int>{0}, std::deque<int>{3});

  // pass a bogus stream and ensure it's still ok
  stringField = make_shared<StringField>(L"foo", L"beer", Field::Store::NO);
  shared_ptr<TokenStream> bogus = make_shared<CannedTokenStream>();
  ts = stringField->tokenStream(nullptr, bogus);
  assertNotSame(ts, bogus);
  assertTokenStreamContents(ts, std::deque<wstring>{L"beer"},
                            std::deque<int>{0}, std::deque<int>{4});
}

wstring TestFieldReuse::MyField::name() { return L"foo"; }

shared_ptr<IndexableFieldType> TestFieldReuse::MyField::fieldType()
{
  return StringField::TYPE_NOT_STORED;
}

shared_ptr<TokenStream>
TestFieldReuse::MyField::tokenStream(shared_ptr<Analyzer> analyzer,
                                     shared_ptr<TokenStream> reuse)
{
  lastSeen = reuse;
  return lastReturned = make_shared<CannedTokenStream>(
             make_shared<Token>(L"unimportant", 0, 10));
}

shared_ptr<BytesRef> TestFieldReuse::MyField::binaryValue() { return nullptr; }

wstring TestFieldReuse::MyField::stringValue() { return L""; }

shared_ptr<Reader> TestFieldReuse::MyField::readerValue() { return nullptr; }

shared_ptr<Number> TestFieldReuse::MyField::numericValue() { return nullptr; }

void TestFieldReuse::testIndexWriterActuallyReuses() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<MyField> *const field1 = make_shared<MyField>();
  iw->addDocument(Collections::singletonList(field1));
  shared_ptr<TokenStream> previous = field1->lastReturned;
  assertNotNull(previous);

  shared_ptr<MyField> *const field2 = make_shared<MyField>();
  iw->addDocument(Collections::singletonList(field2));
  assertSame(previous, field2->lastSeen);
  delete iw;
  delete dir;
}
} // namespace org::apache::lucene::index