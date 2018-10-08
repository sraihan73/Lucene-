using namespace std;

#include "TestDocumentWriter.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestDocumentWriter::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
}

void TestDocumentWriter::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}

void TestDocumentWriter::test() { assertTrue(dir != nullptr); }

void TestDocumentWriter::testAddDocument() 
{
  shared_ptr<Document> testDoc = make_shared<Document>();
  DocHelper::setupDoc(testDoc);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->addDocument(testDoc);
  writer->commit();
  shared_ptr<SegmentCommitInfo> info = writer->newestSegment();
  delete writer;
  // After adding the document, we should be able to read it back in
  shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
      info, Version::LATEST->major, newIOContext(random()));
  assertTrue(reader != nullptr);
  shared_ptr<Document> doc = reader->document(0);
  assertTrue(doc->size() > 0);

  // System.out.println("Document: " + doc);
  std::deque<std::shared_ptr<IndexableField>> fields =
      doc->getFields(L"textField2");
  assertTrue(fields.size() > 0 && fields.size() == 1);
  assertTrue(fields[0]->stringValue() == DocHelper::FIELD_2_TEXT);
  assertTrue(fields[0]->fieldType().storeTermVectors());

  fields = doc->getFields(L"textField1");
  assertTrue(fields.size() > 0 && fields.size() == 1);
  assertTrue(fields[0]->stringValue() == DocHelper::FIELD_1_TEXT);
  assertFalse(fields[0]->fieldType().storeTermVectors());

  fields = doc->getFields(L"keyField");
  assertTrue(fields.size() > 0 && fields.size() == 1);
  assertTrue(fields[0]->stringValue() == DocHelper::KEYWORD_TEXT);

  fields = doc->getFields(DocHelper::NO_NORMS_KEY);
  assertTrue(fields.size() > 0 && fields.size() == 1);
  assertTrue(fields[0]->stringValue() == DocHelper::NO_NORMS_TEXT);

  fields = doc->getFields(DocHelper::TEXT_FIELD_3_KEY);
  assertTrue(fields.size() > 0 && fields.size() == 1);
  assertTrue(fields[0]->stringValue() == DocHelper::FIELD_3_TEXT);

  // test that the norms are not present in the segment if
  // omitNorms is true
  for (auto fi : reader->getFieldInfos()) {
    if (fi->getIndexOptions() != IndexOptions::NONE) {
      assertTrue(fi->omitsNorms() ==
                 (reader->getNormValues(fi->name) == nullptr));
    }
  }
  delete reader;
}

void TestDocumentWriter::testPositionIncrementGap() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"repeated", L"repeated one", Field::Store::YES));
  doc->push_back(newTextField(L"repeated", L"repeated two", Field::Store::YES));

  writer->addDocument(doc);
  writer->commit();
  shared_ptr<SegmentCommitInfo> info = writer->newestSegment();
  delete writer;
  shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
      info, Version::LATEST->major, newIOContext(random()));

  shared_ptr<PostingsEnum> termPositions = MultiFields::getTermPositionsEnum(
      reader, L"repeated", make_shared<BytesRef>(L"repeated"));
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  int freq = termPositions->freq();
  TestUtil::assertEquals(2, freq);
  TestUtil::assertEquals(0, termPositions->nextPosition());
  TestUtil::assertEquals(502, termPositions->nextPosition());
  delete reader;
}

TestDocumentWriter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestDocumentWriter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestDocumentWriter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

int TestDocumentWriter::AnalyzerAnonymousInnerClass::getPositionIncrementGap(
    const wstring &fieldName)
{
  return 500;
}

void TestDocumentWriter::testTokenReuse() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"f1", L"a 5 a a", Field::Store::YES));

  writer->addDocument(doc);
  writer->commit();
  shared_ptr<SegmentCommitInfo> info = writer->newestSegment();
  delete writer;
  shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
      info, Version::LATEST->major, newIOContext(random()));

  shared_ptr<PostingsEnum> termPositions = MultiFields::getTermPositionsEnum(
      reader, L"f1", make_shared<BytesRef>(L"a"));
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  int freq = termPositions->freq();
  TestUtil::assertEquals(3, freq);
  TestUtil::assertEquals(0, termPositions->nextPosition());
  assertNotNull(termPositions->getPayload());
  TestUtil::assertEquals(6, termPositions->nextPosition());
  assertNull(termPositions->getPayload());
  TestUtil::assertEquals(7, termPositions->nextPosition());
  assertNull(termPositions->getPayload());
  delete reader;
}

TestDocumentWriter::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestDocumentWriter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestDocumentWriter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<TokenFilterAnonymousInnerClass>(shared_from_this(),
                                                             tokenizer));
}

TestDocumentWriter::AnalyzerAnonymousInnerClass2::
    TokenFilterAnonymousInnerClass::TokenFilterAnonymousInnerClass(
        shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
        shared_ptr<org::apache::lucene::analysis::Tokenizer> tokenizer)
    : TokenFilter(tokenizer)
{
  this->outerInstance = outerInstance;
  first = true;
  termAtt = addAttribute(CharTermAttribute::typeid);
  payloadAtt = addAttribute(PayloadAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
}

bool TestDocumentWriter::AnalyzerAnonymousInnerClass2::
    TokenFilterAnonymousInnerClass::incrementToken() 
{
  if (state != nullptr) {
    restoreState(state);
    payloadAtt::setPayload(nullptr);
    posIncrAtt::setPositionIncrement(0);
    termAtt::setEmpty()->append(L"b");
    state = nullptr;
    return true;
  }

  bool hasNext = input::incrementToken();
  if (!hasNext) {
    return false;
  }
  if (isdigit(termAtt::buffer()[0])) {
    posIncrAtt::setPositionIncrement(termAtt::buffer()[0] - L'0');
  }
  if (first) {
    // set payload on first position only
    payloadAtt::setPayload(make_shared<BytesRef>(std::deque<char>{100}));
    first = false;
  }

  // index a "synonym" for every token
  state = captureState();
  return true;
}

void TestDocumentWriter::AnalyzerAnonymousInnerClass2::
    TokenFilterAnonymousInnerClass::reset() 
{
  outerInstance->outerInstance.super.reset();
  first = true;
  state = nullptr;
}

void TestDocumentWriter::testPreAnalyzedField() 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(make_shared<TextField>(
      L"preanalyzed",
      make_shared<TokenStreamAnonymousInnerClass>(shared_from_this())));

  writer->addDocument(doc);
  writer->commit();
  shared_ptr<SegmentCommitInfo> info = writer->newestSegment();
  delete writer;
  shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
      info, Version::LATEST->major, newIOContext(random()));

  shared_ptr<PostingsEnum> termPositions = reader->postings(
      make_shared<Term>(L"preanalyzed", L"term1"), PostingsEnum::ALL);
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(1, termPositions->freq());
  TestUtil::assertEquals(0, termPositions->nextPosition());

  termPositions = reader->postings(make_shared<Term>(L"preanalyzed", L"term2"),
                                   PostingsEnum::ALL);
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(2, termPositions->freq());
  TestUtil::assertEquals(1, termPositions->nextPosition());
  TestUtil::assertEquals(3, termPositions->nextPosition());

  termPositions = reader->postings(make_shared<Term>(L"preanalyzed", L"term3"),
                                   PostingsEnum::ALL);
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(1, termPositions->freq());
  TestUtil::assertEquals(2, termPositions->nextPosition());
  delete reader;
}

TestDocumentWriter::TokenStreamAnonymousInnerClass::
    TokenStreamAnonymousInnerClass(shared_ptr<TestDocumentWriter> outerInstance)
{
  this->outerInstance = outerInstance;
  tokens = std::deque<wstring>{L"term1", L"term2", L"term3", L"term2"};
  index = 0;
  termAtt = addAttribute(CharTermAttribute::typeid);
}

bool TestDocumentWriter::TokenStreamAnonymousInnerClass::incrementToken()
{
  if (index == tokens->length) {
    return false;
  } else {
    clearAttributes();
    termAtt::setEmpty()->append(tokens[index++]);
    return true;
  }
}

void TestDocumentWriter::testLUCENE_1590() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  // f1 has no norms
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<FieldType> customType2 = make_shared<FieldType>();
  customType2->setStored(true);
  doc->push_back(newField(L"f1", L"v1", customType));
  doc->push_back(newField(L"f1", L"v2", customType2));
  // f2 has no TF
  shared_ptr<FieldType> customType3 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType3->setIndexOptions(IndexOptions::DOCS);
  shared_ptr<Field> f = newField(L"f2", L"v1", customType3);
  doc->push_back(f);
  doc->push_back(newField(L"f2", L"v2", customType2));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->addDocument(doc);
  writer->forceMerge(1); // be sure to have a single segment
  delete writer;

  TestUtil::checkIndex(dir);

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(dir));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  // f1
  assertFalse(L"f1 should have no norms", fi->fieldInfo(L"f1")->hasNorms());
  assertEquals(L"omitTermFreqAndPositions field bit should not be set for f1",
               IndexOptions::DOCS_AND_FREQS_AND_POSITIONS,
               fi->fieldInfo(L"f1")->getIndexOptions());
  // f2
  assertTrue(L"f2 should have norms", fi->fieldInfo(L"f2")->hasNorms());
  assertEquals(L"omitTermFreqAndPositions field bit should be set for f2",
               IndexOptions::DOCS, fi->fieldInfo(L"f2")->getIndexOptions());
  delete reader;
}
} // namespace org::apache::lucene::index