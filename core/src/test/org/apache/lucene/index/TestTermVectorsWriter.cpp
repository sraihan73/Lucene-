using namespace std;

#include "TestTermVectorsWriter.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestTermVectorsWriter::testDoubleOffsetCounting() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> f = newField(L"field", L"abcd", customType);
  doc->push_back(f);
  doc->push_back(f);
  shared_ptr<Field> f2 = newField(L"field", L"", customType);
  doc->push_back(f2);
  doc->push_back(f);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<Terms> deque = r->getTermVectors(0)->terms(L"field");
  assertNotNull(deque);
  shared_ptr<TermsEnum> termsEnum = deque->begin();
  assertNotNull(termsEnum->next());
  TestUtil::assertEquals(L"", termsEnum->term()->utf8ToString());

  // Token "" occurred once
  TestUtil::assertEquals(1, termsEnum->totalTermFreq());

  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(8, dpEnum->startOffset());
  TestUtil::assertEquals(8, dpEnum->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

  // Token "abcd" occurred three times
  TestUtil::assertEquals(make_shared<BytesRef>(L"abcd"), termsEnum->next());
  dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
  TestUtil::assertEquals(3, termsEnum->totalTermFreq());

  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  dpEnum->nextPosition();
  TestUtil::assertEquals(4, dpEnum->startOffset());
  TestUtil::assertEquals(8, dpEnum->endOffset());

  dpEnum->nextPosition();
  TestUtil::assertEquals(8, dpEnum->startOffset());
  TestUtil::assertEquals(12, dpEnum->endOffset());

  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());
  assertNull(termsEnum->next());
  delete r;
  delete dir;
}

void TestTermVectorsWriter::testDoubleOffsetCounting2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> f = newField(L"field", L"abcd", customType);
  doc->push_back(f);
  doc->push_back(f);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  TestUtil::assertEquals(2, termsEnum->totalTermFreq());

  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  dpEnum->nextPosition();
  TestUtil::assertEquals(5, dpEnum->startOffset());
  TestUtil::assertEquals(9, dpEnum->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testEndOffsetPositionCharAnalyzer() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> f = newField(L"field", L"abcd   ", customType);
  doc->push_back(f);
  doc->push_back(f);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  TestUtil::assertEquals(2, termsEnum->totalTermFreq());

  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  dpEnum->nextPosition();
  TestUtil::assertEquals(8, dpEnum->startOffset());
  TestUtil::assertEquals(12, dpEnum->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testEndOffsetPositionWithCachingTokenFilter() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // stream = new
  // org.apache.lucene.analysis.CachingTokenFilter(analyzer.tokenStream("field",
  // "abcd   ")))
  {
    org::apache::lucene::analysis::TokenStream stream =
        org::apache::lucene::analysis::CachingTokenFilter(
            analyzer->tokenStream(L"field", L"abcd   "));
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectorOffsets(true);
    shared_ptr<Field> f = make_shared<Field>(L"field", stream, customType);
    doc->push_back(f);
    doc->push_back(f);
    w->addDocument(doc);
  }
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  TestUtil::assertEquals(2, termsEnum->totalTermFreq());

  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  dpEnum->nextPosition();
  TestUtil::assertEquals(8, dpEnum->startOffset());
  TestUtil::assertEquals(12, dpEnum->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testEndOffsetPositionStopFilter() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(
               make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                         MockTokenFilter::ENGLISH_STOPSET)));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> f = newField(L"field", L"abcd the", customType);
  doc->push_back(f);
  doc->push_back(f);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  TestUtil::assertEquals(2, termsEnum->totalTermFreq());

  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  dpEnum->nextPosition();
  TestUtil::assertEquals(9, dpEnum->startOffset());
  TestUtil::assertEquals(13, dpEnum->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dpEnum->nextDoc());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testEndOffsetPositionStandard() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> f = newField(L"field", L"abcd the  ", customType);
  shared_ptr<Field> f2 = newField(L"field", L"crunch man", customType);
  doc->push_back(f);
  doc->push_back(f2);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);

  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  assertNotNull(termsEnum->next());
  dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(11, dpEnum->startOffset());
  TestUtil::assertEquals(17, dpEnum->endOffset());

  assertNotNull(termsEnum->next());
  dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(18, dpEnum->startOffset());
  TestUtil::assertEquals(21, dpEnum->endOffset());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testEndOffsetPositionStandardEmptyField() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> f = newField(L"field", L"", customType);
  shared_ptr<Field> f2 = newField(L"field", L"crunch man", customType);
  doc->push_back(f);
  doc->push_back(f2);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);

  TestUtil::assertEquals(1, static_cast<int>(termsEnum->totalTermFreq()));
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(1, dpEnum->startOffset());
  TestUtil::assertEquals(7, dpEnum->endOffset());

  assertNotNull(termsEnum->next());
  dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(8, dpEnum->startOffset());
  TestUtil::assertEquals(11, dpEnum->endOffset());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testEndOffsetPositionStandardEmptyField2() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);

  shared_ptr<Field> f = newField(L"field", L"abcd", customType);
  doc->push_back(f);
  doc->push_back(newField(L"field", L"", customType));

  shared_ptr<Field> f2 = newField(L"field", L"crunch", customType);
  doc->push_back(f2);

  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termsEnum =
      r->getTermVectors(0)->terms(L"field")->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);

  TestUtil::assertEquals(1, static_cast<int>(termsEnum->totalTermFreq()));
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(0, dpEnum->startOffset());
  TestUtil::assertEquals(4, dpEnum->endOffset());

  assertNotNull(termsEnum->next());
  dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  dpEnum->nextPosition();
  TestUtil::assertEquals(6, dpEnum->startOffset());
  TestUtil::assertEquals(12, dpEnum->endOffset());

  delete r;
  delete dir;
}

void TestTermVectorsWriter::testTermVectorCorruption() 
{

  shared_ptr<Directory> dir = newDirectory();
  for (int iter = 0; iter < 2; iter++) {
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2)
                 ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
                 ->setMergeScheduler(make_shared<SerialMergeScheduler>())
                 ->setMergePolicy(make_shared<LogDocMergePolicy>()));

    shared_ptr<Document> document = make_shared<Document>();
    shared_ptr<FieldType> customType = make_shared<FieldType>();
    customType->setStored(true);

    shared_ptr<Field> storedField = newField(L"stored", L"stored", customType);
    document->push_back(storedField);
    writer->addDocument(document);
    writer->addDocument(document);

    document = make_shared<Document>();
    document->push_back(storedField);
    shared_ptr<FieldType> customType2 =
        make_shared<FieldType>(StringField::TYPE_NOT_STORED);
    customType2->setStoreTermVectors(true);
    customType2->setStoreTermVectorPositions(true);
    customType2->setStoreTermVectorOffsets(true);
    shared_ptr<Field> termVectorField =
        newField(L"termVector", L"termVector", customType2);

    document->push_back(termVectorField);
    writer->addDocument(document);
    writer->forceMerge(1);
    delete writer;

    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    for (int i = 0; i < reader->numDocs(); i++) {
      reader->document(i);
      reader->getTermVectors(i);
    }
    delete reader;

    writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2)
                 ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
                 ->setMergeScheduler(make_shared<SerialMergeScheduler>())
                 ->setMergePolicy(make_shared<LogDocMergePolicy>()));

    std::deque<std::shared_ptr<Directory>> indexDirs = {
        make_shared<MockDirectoryWrapper>(random(), TestUtil::ramCopyOf(dir))};
    writer->addIndexes(indexDirs);
    writer->forceMerge(1);
    delete writer;
  }
  delete dir;
}

void TestTermVectorsWriter::testTermVectorCorruption2() 
{
  shared_ptr<Directory> dir = newDirectory();
  for (int iter = 0; iter < 2; iter++) {
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2)
                 ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
                 ->setMergeScheduler(make_shared<SerialMergeScheduler>())
                 ->setMergePolicy(make_shared<LogDocMergePolicy>()));

    shared_ptr<Document> document = make_shared<Document>();

    shared_ptr<FieldType> customType = make_shared<FieldType>();
    customType->setStored(true);

    shared_ptr<Field> storedField = newField(L"stored", L"stored", customType);
    document->push_back(storedField);
    writer->addDocument(document);
    writer->addDocument(document);

    document = make_shared<Document>();
    document->push_back(storedField);
    shared_ptr<FieldType> customType2 =
        make_shared<FieldType>(StringField::TYPE_NOT_STORED);
    customType2->setStoreTermVectors(true);
    customType2->setStoreTermVectorPositions(true);
    customType2->setStoreTermVectorOffsets(true);
    shared_ptr<Field> termVectorField =
        newField(L"termVector", L"termVector", customType2);
    document->push_back(termVectorField);
    writer->addDocument(document);
    writer->forceMerge(1);
    delete writer;

    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    assertNull(reader->getTermVectors(0));
    assertNull(reader->getTermVectors(1));
    assertNotNull(reader->getTermVectors(2));
    delete reader;
  }
  delete dir;
}

void TestTermVectorsWriter::testTermVectorCorruption3() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               ->setMergeScheduler(make_shared<SerialMergeScheduler>())
               ->setMergePolicy(make_shared<LogDocMergePolicy>()));

  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<Field> storedField = newField(L"stored", L"stored", customType);
  document->push_back(storedField);
  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType2->setStoreTermVectors(true);
  customType2->setStoreTermVectorPositions(true);
  customType2->setStoreTermVectorOffsets(true);
  shared_ptr<Field> termVectorField =
      newField(L"termVector", L"termVector", customType2);
  document->push_back(termVectorField);
  for (int i = 0; i < 10; i++) {
    writer->addDocument(document);
  }
  delete writer;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               ->setMergeScheduler(make_shared<SerialMergeScheduler>())
               ->setMergePolicy(make_shared<LogDocMergePolicy>()));
  for (int i = 0; i < 6; i++) {
    writer->addDocument(document);
  }

  writer->forceMerge(1);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  for (int i = 0; i < 10; i++) {
    reader->getTermVectors(i);
    reader->document(i);
  }
  delete reader;
  delete dir;
}

void TestTermVectorsWriter::testNoTermVectorAfterTermVector() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType2->setStoreTermVectors(true);
  customType2->setStoreTermVectorPositions(true);
  customType2->setStoreTermVectorOffsets(true);
  document->push_back(newField(L"tvtest", L"a b c", customType2));
  iw->addDocument(document);
  document = make_shared<Document>();
  document->push_back(newTextField(L"tvtest", L"x y z", Field::Store::NO));
  iw->addDocument(document);
  // Make first segment
  iw->commit();

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  document = make_shared<Document>();
  document->push_back(newField(L"tvtest", L"a b c", customType));
  iw->addDocument(document);
  // Make 2nd segment
  iw->commit();

  iw->forceMerge(1);
  delete iw;
  delete dir;
}

void TestTermVectorsWriter::testNoTermVectorAfterTermVectorMerge() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  document->push_back(newField(L"tvtest", L"a b c", customType));
  iw->addDocument(document);
  iw->commit();

  document = make_shared<Document>();
  document->push_back(newTextField(L"tvtest", L"x y z", Field::Store::NO));
  iw->addDocument(document);
  // Make first segment
  iw->commit();

  iw->forceMerge(1);

  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType2->setStoreTermVectors(true);
  document->push_back(newField(L"tvtest", L"a b c", customType2));
  document = make_shared<Document>();
  iw->addDocument(document);
  // Make 2nd segment
  iw->commit();
  iw->forceMerge(1);

  delete iw;
  delete dir;
}

void TestTermVectorsWriter::testInconsistentTermVectorOptions() throw(
    IOException)
{
  shared_ptr<FieldType> a, b;

  // no vectors + vectors
  a = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  b = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  b->setStoreTermVectors(true);
  doTestMixup(a, b);

  // vectors + vectors with pos
  a = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  a->setStoreTermVectors(true);
  b = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  b->setStoreTermVectors(true);
  b->setStoreTermVectorPositions(true);
  doTestMixup(a, b);

  // vectors + vectors with off
  a = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  a->setStoreTermVectors(true);
  b = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  b->setStoreTermVectors(true);
  b->setStoreTermVectorOffsets(true);
  doTestMixup(a, b);

  // vectors with pos + vectors with pos + off
  a = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  a->setStoreTermVectors(true);
  a->setStoreTermVectorPositions(true);
  b = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  b->setStoreTermVectors(true);
  b->setStoreTermVectorPositions(true);
  b->setStoreTermVectorOffsets(true);
  doTestMixup(a, b);

  // vectors with pos + vectors with pos + pay
  a = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  a->setStoreTermVectors(true);
  a->setStoreTermVectorPositions(true);
  b = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  b->setStoreTermVectors(true);
  b->setStoreTermVectorPositions(true);
  b->setStoreTermVectorPayloads(true);
  doTestMixup(a, b);
}

void TestTermVectorsWriter::doTestMixup(
    shared_ptr<FieldType> ft1, shared_ptr<FieldType> ft2) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);

  // add 3 good docs
  for (int i = 0; i < 3; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    iw->addDocument(doc);
  }

  // add broken doc
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"value1", ft1));
  doc->push_back(make_shared<Field>(L"field", L"value2", ft2));

  // ensure broken doc hits exception
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { iw->addDocument(doc); });
  assertNotNull(expected.what());
  assertTrue(
      expected.what()->startsWith(L"all instances of a given field name must "
                                  L"have the same term vectors settings"));

  // ensure good docs are still ok
  shared_ptr<IndexReader> ir = iw->getReader();
  TestUtil::assertEquals(3, ir->numDocs());

  delete ir;
  delete iw;
  delete dir;
}

void TestTermVectorsWriter::testNoAbortOnBadTVSettings() 
{
  shared_ptr<Directory> dir = newDirectory();
  // Don't use RandomIndexWriter because we want to be sure both docs go to 1
  // seg:
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);
  shared_ptr<FieldType> ft = make_shared<FieldType>(StoredField::TYPE);
  ft->setStoreTermVectors(true);
  ft->freeze();
  doc->push_back(make_shared<Field>(L"field", L"value", ft));

  expectThrows(invalid_argument::typeid, [&]() { iw->addDocument(doc); });

  shared_ptr<IndexReader> r = DirectoryReader::open(iw);

  // Make sure the exc didn't lose our first document:
  TestUtil::assertEquals(1, r->numDocs());
  delete iw;
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index