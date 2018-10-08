using namespace std;

#include "TestTermVectorsReader.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Codec = org::apache::lucene::codecs::Codec;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
int TestTermVectorsReader::TERM_FREQ = 3;

int TestTermVectorsReader::TestToken::compareTo(shared_ptr<TestToken> other)
{
  return pos - other->pos;
}

void TestTermVectorsReader::setUp() 
{
  LuceneTestCase::setUp();
  /*
  for (int i = 0; i < testFields.length; i++) {
    fieldInfos.add(testFields[i], true, true, testFieldsStorePos[i],
  testFieldsStoreOff[i]);
  }
  */

  Arrays::sort(testTerms);
  int tokenUpto = 0;
  shared_ptr<Random> rnd = random();
  for (int i = 0; i < testTerms.size(); i++) {
    positions[i] = std::deque<int>(TERM_FREQ);
    // first position must be 0
    for (int j = 0; j < TERM_FREQ; j++) {
      // positions are always sorted in increasing order
      positions[i][j] = static_cast<int>(j * 10 + rnd->nextDouble() * 10);
      shared_ptr<TestToken> token = tokens[tokenUpto++] =
          make_shared<TestToken>();
      token->text = testTerms[i];
      token->pos = positions[i][j];
      token->startOffset = j * 10;
      token->endOffset = j * 10 + testTerms[i].length();
    }
  }
  Arrays::sort(tokens);

  dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MyAnalyzer>(shared_from_this()))
               ->setMaxBufferedDocs(-1)
               ->setMergePolicy(newLogMergePolicy(false, 10))
               ->setUseCompoundFile(false));

  shared_ptr<Document> doc = make_shared<Document>();
  for (int i = 0; i < testFields.size(); i++) {
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    if (testFieldsStorePos[i] && testFieldsStoreOff[i]) {
      customType->setStoreTermVectors(true);
      customType->setStoreTermVectorPositions(true);
      customType->setStoreTermVectorOffsets(true);
    } else if (testFieldsStorePos[i] && !testFieldsStoreOff[i]) {
      customType->setStoreTermVectors(true);
      customType->setStoreTermVectorPositions(true);
    } else if (!testFieldsStorePos[i] && testFieldsStoreOff[i]) {
      customType->setStoreTermVectors(true);
      customType->setStoreTermVectorPositions(true);
      customType->setStoreTermVectorOffsets(true);
    } else {
      customType->setStoreTermVectors(true);
    }
    doc->push_back(make_shared<Field>(testFields[i], L"", customType));
  }

  // Create 5 documents for testing, they all have the same
  // terms
  for (int j = 0; j < 5; j++) {
    writer->addDocument(doc);
  }
  writer->commit();
  seg = writer->newestSegment();
  delete writer;

  fieldInfos = IndexWriter::readFieldInfos(seg);
}

void TestTermVectorsReader::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}

TestTermVectorsReader::MyTokenizer::MyTokenizer(
    shared_ptr<TestTermVectorsReader> outerInstance)
    : Tokenizer(), termAtt(addAttribute(CharTermAttribute::typeid)),
      posIncrAtt(addAttribute(PositionIncrementAttribute::typeid)),
      offsetAtt(addAttribute(OffsetAttribute::typeid)),
      outerInstance(outerInstance)
{
}

bool TestTermVectorsReader::MyTokenizer::incrementToken()
{
  if (tokenUpto >= outerInstance->tokens.size()) {
    return false;
  } else {
    shared_ptr<TestToken> *const testToken = outerInstance->tokens[tokenUpto++];
    clearAttributes();
    termAtt->append(testToken->text);
    offsetAtt->setOffset(testToken->startOffset, testToken->endOffset);
    if (tokenUpto > 1) {
      posIncrAtt->setPositionIncrement(
          testToken->pos - outerInstance->tokens[tokenUpto - 2]->pos);
    } else {
      posIncrAtt->setPositionIncrement(testToken->pos + 1);
    }
    return true;
  }
}

void TestTermVectorsReader::MyTokenizer::reset() 
{
  Tokenizer::reset();
  this->tokenUpto = 0;
}

TestTermVectorsReader::MyAnalyzer::MyAnalyzer(
    shared_ptr<TestTermVectorsReader> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<TokenStreamComponents>
TestTermVectorsReader::MyAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(
      make_shared<MyTokenizer>(outerInstance));
}

void TestTermVectorsReader::test() 
{
  // Check to see the files were created properly in setup
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> ctx : reader->leaves()) {
    shared_ptr<SegmentReader> sr =
        std::static_pointer_cast<SegmentReader>(ctx->reader());
    assertTrue(sr->getFieldInfos()->hasVectors());
  }
  reader->close();
}

void TestTermVectorsReader::testReader() 
{
  shared_ptr<TermVectorsReader> reader =
      Codec::getDefault()->termVectorsFormat()->vectorsReader(
          dir, seg->info, fieldInfos, newIOContext(random()));
  for (int j = 0; j < 5; j++) {
    shared_ptr<Terms> deque = reader->get(j).terms(testFields[0]);
    assertNotNull(deque);
    TestUtil::assertEquals(testTerms.size(), deque->size());
    shared_ptr<TermsEnum> termsEnum = deque->begin();
    for (int i = 0; i < testTerms.size(); i++) {
      shared_ptr<BytesRef> *const text = termsEnum->next();
      assertNotNull(text);
      wstring term = text->utf8ToString();
      // System.out.println("Term: " + term);
      TestUtil::assertEquals(testTerms[i], term);
    }
    assertNull(termsEnum->next());
  }
  delete reader;
}

void TestTermVectorsReader::testDocsEnum() 
{
  shared_ptr<TermVectorsReader> reader =
      Codec::getDefault()->termVectorsFormat()->vectorsReader(
          dir, seg->info, fieldInfos, newIOContext(random()));
  for (int j = 0; j < 5; j++) {
    shared_ptr<Terms> deque = reader->get(j).terms(testFields[0]);
    assertNotNull(deque);
    TestUtil::assertEquals(testTerms.size(), deque->size());
    shared_ptr<TermsEnum> termsEnum = deque->begin();
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    for (int i = 0; i < testTerms.size(); i++) {
      shared_ptr<BytesRef> *const text = termsEnum->next();
      assertNotNull(text);
      wstring term = text->utf8ToString();
      // System.out.println("Term: " + term);
      TestUtil::assertEquals(testTerms[i], term);

      postingsEnum =
          TestUtil::docs(random(), termsEnum, postingsEnum, PostingsEnum::NONE);
      assertNotNull(postingsEnum);
      int doc = postingsEnum->docID();
      TestUtil::assertEquals(-1, doc);
      assertTrue(postingsEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
      TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                             postingsEnum->nextDoc());
    }
    assertNull(termsEnum->next());
  }
  delete reader;
}

void TestTermVectorsReader::testPositionReader() 
{
  shared_ptr<TermVectorsReader> reader =
      Codec::getDefault()->termVectorsFormat()->vectorsReader(
          dir, seg->info, fieldInfos, newIOContext(random()));
  std::deque<std::shared_ptr<BytesRef>> terms;
  shared_ptr<Terms> deque = reader->get(0).terms(testFields[0]);
  assertNotNull(deque);
  TestUtil::assertEquals(testTerms.size(), deque->size());
  shared_ptr<TermsEnum> termsEnum = deque->begin();
  shared_ptr<PostingsEnum> dpEnum = nullptr;
  for (int i = 0; i < testTerms.size(); i++) {
    shared_ptr<BytesRef> *const text = termsEnum->next();
    assertNotNull(text);
    wstring term = text->utf8ToString();
    // System.out.println("Term: " + term);
    TestUtil::assertEquals(testTerms[i], term);

    dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
    assertNotNull(dpEnum);
    int doc = dpEnum->docID();
    TestUtil::assertEquals(-1, doc);
    assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    TestUtil::assertEquals(dpEnum->freq(), positions[i].length);
    for (int j = 0; j < positions[i].length; j++) {
      TestUtil::assertEquals(positions[i][j], dpEnum->nextPosition());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

    dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
    doc = dpEnum->docID();
    TestUtil::assertEquals(-1, doc);
    assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    assertNotNull(dpEnum);
    TestUtil::assertEquals(dpEnum->freq(), positions[i].length);
    for (int j = 0; j < positions[i].length; j++) {
      TestUtil::assertEquals(positions[i][j], dpEnum->nextPosition());
      TestUtil::assertEquals(j * 10, dpEnum->startOffset());
      TestUtil::assertEquals(j * 10 + testTerms[i].length(),
                             dpEnum->endOffset());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());
  }

  shared_ptr<Terms> freqVector =
      reader->get(0).terms(testFields[1]); // no pos, no offset
  assertNotNull(freqVector);
  TestUtil::assertEquals(testTerms.size(), freqVector->size());
  termsEnum = freqVector->begin();
  assertNotNull(termsEnum);
  for (int i = 0; i < testTerms.size(); i++) {
    shared_ptr<BytesRef> *const text = termsEnum->next();
    assertNotNull(text);
    wstring term = text->utf8ToString();
    // System.out.println("Term: " + term);
    TestUtil::assertEquals(testTerms[i], term);
    assertNotNull(termsEnum->postings(nullptr));
    assertNotNull(termsEnum->postings(nullptr, PostingsEnum::ALL));
  }
  delete reader;
}

void TestTermVectorsReader::testOffsetReader() 
{
  shared_ptr<TermVectorsReader> reader =
      Codec::getDefault()->termVectorsFormat()->vectorsReader(
          dir, seg->info, fieldInfos, newIOContext(random()));
  shared_ptr<Terms> deque = reader->get(0).terms(testFields[0]);
  assertNotNull(deque);
  shared_ptr<TermsEnum> termsEnum = deque->begin();
  assertNotNull(termsEnum);
  TestUtil::assertEquals(testTerms.size(), deque->size());
  shared_ptr<PostingsEnum> dpEnum = nullptr;
  for (int i = 0; i < testTerms.size(); i++) {
    shared_ptr<BytesRef> *const text = termsEnum->next();
    assertNotNull(text);
    wstring term = text->utf8ToString();
    TestUtil::assertEquals(testTerms[i], term);

    dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
    assertNotNull(dpEnum);
    assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    TestUtil::assertEquals(dpEnum->freq(), positions[i].length);
    for (int j = 0; j < positions[i].length; j++) {
      TestUtil::assertEquals(positions[i][j], dpEnum->nextPosition());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

    dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
    assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    assertNotNull(dpEnum);
    TestUtil::assertEquals(dpEnum->freq(), positions[i].length);
    for (int j = 0; j < positions[i].length; j++) {
      TestUtil::assertEquals(positions[i][j], dpEnum->nextPosition());
      TestUtil::assertEquals(j * 10, dpEnum->startOffset());
      TestUtil::assertEquals(j * 10 + testTerms[i].length(),
                             dpEnum->endOffset());
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());
  }
  delete reader;
}

void TestTermVectorsReader::testIllegalPayloadsWithoutPositions() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPayloads(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot index term deque payloads without term "
                         L"deque positions (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalOffsetsWithoutVectors() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(false);
  ft->setStoreTermVectorOffsets(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot index term deque offsets when term vectors "
                         L"are not indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalPositionsWithoutVectors() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(false);
  ft->setStoreTermVectorPositions(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot index term deque positions when term "
                         L"vectors are not indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalVectorPayloadsWithoutVectors() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(false);
  ft->setStoreTermVectorPayloads(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot index term deque payloads when term vectors "
                         L"are not indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalVectorsWithoutIndexed() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(StoredField::TYPE);
  ft->setStoreTermVectors(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot store term vectors for a field that is not "
                         L"indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalVectorPositionsWithoutIndexed() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(StoredField::TYPE);
  ft->setStoreTermVectorPositions(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot store term deque positions for a field that "
                         L"is not indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalVectorOffsetsWithoutIndexed() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(StoredField::TYPE);
  ft->setStoreTermVectorOffsets(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot store term deque offsets for a field that "
                         L"is not indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}

void TestTermVectorsReader::testIllegalVectorPayloadsWithoutIndexed() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> a = make_shared<MockAnalyzer>(random());
  a->setEnableChecks(false);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<FieldType> ft = make_shared<FieldType>(StoredField::TYPE);
  ft->setStoreTermVectorPayloads(true);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  TestUtil::assertEquals(L"cannot store term deque payloads for a field that "
                         L"is not indexed (field=\"field\")",
                         expected.what());

  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index