using namespace std;

#include "TestPayloadsOnVectors.h"

namespace org::apache::lucene::index
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPayloadsOnVectors::testMixupDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorPayloads(true);
  customType->setStoreTermVectorOffsets(random()->nextBoolean());
  shared_ptr<Field> field = make_shared<Field>(L"field", L"", customType);
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"here we go"));
  field->setTokenStream(ts);
  doc->push_back(field);
  writer->addDocument(doc);

  shared_ptr<Token> withPayload = make_shared<Token>(L"withPayload", 0, 11);
  withPayload->setPayload(make_shared<BytesRef>(L"test"));
  ts = make_shared<CannedTokenStream>(withPayload);
  assertTrue(ts->hasAttribute(PayloadAttribute::typeid));
  field->setTokenStream(ts);
  writer->addDocument(doc);

  ts = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"another"));
  field->setTokenStream(ts);
  writer->addDocument(doc);

  shared_ptr<DirectoryReader> reader = writer->getReader();
  shared_ptr<Terms> terms = reader->getTermVector(1, L"field");
  assert(terms != nullptr);
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"withPayload")));
  shared_ptr<PostingsEnum> de = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertEquals(0, de->nextDoc());
  assertEquals(0, de->nextPosition());
  assertEquals(make_shared<BytesRef>(L"test"), de->getPayload());
  delete writer;
  reader->close();
  delete dir;
}

void TestPayloadsOnVectors::testMixupMultiValued() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorPayloads(true);
  customType->setStoreTermVectorOffsets(random()->nextBoolean());
  shared_ptr<Field> field = make_shared<Field>(L"field", L"", customType);
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"here we go"));
  field->setTokenStream(ts);
  doc->push_back(field);
  shared_ptr<Field> field2 = make_shared<Field>(L"field", L"", customType);
  shared_ptr<Token> withPayload = make_shared<Token>(L"withPayload", 0, 11);
  withPayload->setPayload(make_shared<BytesRef>(L"test"));
  ts = make_shared<CannedTokenStream>(withPayload);
  assertTrue(ts->hasAttribute(PayloadAttribute::typeid));
  field2->setTokenStream(ts);
  doc->push_back(field2);
  shared_ptr<Field> field3 = make_shared<Field>(L"field", L"", customType);
  ts = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"nopayload"));
  field3->setTokenStream(ts);
  doc->push_back(field3);
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  shared_ptr<Terms> terms = reader->getTermVector(0, L"field");
  assert(terms != nullptr);
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"withPayload")));
  shared_ptr<PostingsEnum> de = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertEquals(0, de->nextDoc());
  assertEquals(3, de->nextPosition());
  assertEquals(make_shared<BytesRef>(L"test"), de->getPayload());
  delete writer;
  reader->close();
  delete dir;
}

void TestPayloadsOnVectors::testPayloadsWithoutPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(false);
  customType->setStoreTermVectorPayloads(true);
  customType->setStoreTermVectorOffsets(random()->nextBoolean());
  doc->push_back(make_shared<Field>(L"field", L"foo", customType));

  expectThrows(invalid_argument::typeid, [&]() { writer->addDocument(doc); });

  delete writer;
  delete dir;
}
} // namespace org::apache::lucene::index