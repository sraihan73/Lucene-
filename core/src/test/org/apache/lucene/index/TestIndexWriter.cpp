using namespace std;

#include "TestIndexWriter.h"

namespace org::apache::lucene::index
{
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Codec = org::apache::lucene::codecs::Codec;
using SimpleTextCodec =
    org::apache::lucene::codecs::simpletext::SimpleTextCodec;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using FilterPath = org::apache::lucene::mockfile::FilterPath;
using WindowsFS = org::apache::lucene::mockfile::WindowsFS;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using MMapDirectory = org::apache::lucene::store::MMapDirectory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using NIOFSDirectory = org::apache::lucene::store::NIOFSDirectory;
using NoLockFactory = org::apache::lucene::store::NoLockFactory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using SimpleFSDirectory = org::apache::lucene::store::SimpleFSDirectory;
using SimpleFSLockFactory = org::apache::lucene::store::SimpleFSLockFactory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using Constants = org::apache::lucene::util::Constants;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using SetOnce = org::apache::lucene::util::SetOnce;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using Version = org::apache::lucene::util::Version;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using org::junit::Ignore;
using org::junit::Test;
const shared_ptr<org::apache::lucene::document::FieldType>
    TestIndexWriter::storedTextType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_NOT_STORED);

void TestIndexWriter::testDocCount() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = nullptr;
  shared_ptr<IndexReader> reader = nullptr;
  int i;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // add 100 documents
  for (i = 0; i < 100; i++) {
    addDocWithIndex(writer, i);
  }
  TestUtil::assertEquals(100, writer->maxDoc());
  delete writer;

  // delete 40 documents
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(NoMergePolicy::INSTANCE));
  for (i = 0; i < 40; i++) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  delete writer;

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(60, reader->numDocs());
  delete reader;

  // merge the index down and check that the new doc count is correct
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  TestUtil::assertEquals(60, writer->numDocs());
  writer->forceMerge(1);
  TestUtil::assertEquals(60, writer->maxDoc());
  TestUtil::assertEquals(60, writer->numDocs());
  delete writer;

  // check that the index reader gives the same numbers.
  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(60, reader->maxDoc());
  TestUtil::assertEquals(60, reader->numDocs());
  delete reader;

  // make sure opening a new index for create over
  // this existing one works correctly:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::CREATE));
  TestUtil::assertEquals(0, writer->maxDoc());
  TestUtil::assertEquals(0, writer->numDocs());
  delete writer;
  delete dir;
}

void TestIndexWriter::addDoc(shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}

void TestIndexWriter::addDocWithIndex(shared_ptr<IndexWriter> writer,
                                      int index) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newField(L"content", L"aaa " + to_wstring(index), storedTextType));
  doc->push_back(newField(L"id", L"" + to_wstring(index), storedTextType));
  writer->addDocument(doc);
}

void TestIndexWriter::assertNoUnreferencedFiles(
    shared_ptr<Directory> dir, const wstring &message) 
{
  std::deque<wstring> startFiles = dir->listAll();
  (make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(
                                     make_shared<MockAnalyzer>(random()))))
      ->rollback();
  std::deque<wstring> endFiles = dir->listAll();

  Arrays::sort(startFiles);
  Arrays::sort(endFiles);

  if (!Arrays::equals(startFiles, endFiles)) {
    fail(message + L": before delete:\n    " + arrayToString(startFiles) +
         L"\n  after delete:\n    " + arrayToString(endFiles));
  }
}

wstring TestIndexWriter::arrayToString(std::deque<wstring> &l)
{
  wstring s = L"";
  for (int i = 0; i < l.size(); i++) {
    if (i > 0) {
      s += L"\n    ";
    }
    s += l[i];
  }
  return s;
}

void TestIndexWriter::testCreateWithReader() 
{
  shared_ptr<Directory> dir = newDirectory();

  // add one document & close writer
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  addDoc(writer);
  delete writer;

  // now open reader:
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertEquals(L"should be one document", reader->numDocs(), 1);

  // now open index for create:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::CREATE));
  assertEquals(L"should be zero documents", writer->maxDoc(), 0);
  addDoc(writer);
  delete writer;

  assertEquals(L"should be one document", reader->numDocs(), 1);
  shared_ptr<IndexReader> reader2 = DirectoryReader::open(dir);
  assertEquals(L"should be one document", reader2->numDocs(), 1);
  delete reader;
  delete reader2;

  delete dir;
}

void TestIndexWriter::testChangesAfterClose() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  addDoc(writer);

  // close
  delete writer;
  expectThrows(AlreadyClosedException::typeid, [&]() { addDoc(writer); });

  delete dir;
}

void TestIndexWriter::testIndexNoDocuments() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->maxDoc());
  TestUtil::assertEquals(0, reader->numDocs());
  delete reader;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND));
  writer->commit();
  delete writer;

  reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, reader->maxDoc());
  TestUtil::assertEquals(0, reader->numDocs());
  delete reader;
  delete dir;
}

void TestIndexWriter::testSmallRAMBuffer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setRAMBufferSizeMB(0.000001)
               ->setMergePolicy(newLogMergePolicy(10)));
  int lastNumSegments = getSegmentCount(dir);
  for (int j = 0; j < 9; j++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(L"field", L"aaa" + to_wstring(j), storedTextType));
    writer->addDocument(doc);
    // Verify that with a tiny RAM buffer we see new
    // segment after every doc
    int numSegments = getSegmentCount(dir);
    assertTrue(numSegments > lastNumSegments);
    lastNumSegments = numSegments;
  }
  delete writer;
  delete dir;
}

int TestIndexWriter::getSegmentCount(shared_ptr<Directory> dir) throw(
    IOException)
{
  shared_ptr<Set<wstring>> segments = unordered_set<wstring>();
  for (auto file : dir->listAll()) {
    segments->add(IndexFileNames::parseSegmentName(file));
  }

  return segments->size();
}

void TestIndexWriter::testChangingRAMBuffer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->getConfig()->setMaxBufferedDocs(10);
  writer->getConfig()->setRAMBufferSizeMB(
      IndexWriterConfig::DISABLE_AUTO_FLUSH);

  int lastFlushCount = -1;
  for (int j = 1; j < 52; j++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<Field>(L"field", L"aaa" + to_wstring(j), storedTextType));
    writer->addDocument(doc);
    TestUtil::syncConcurrentMerges(writer);
    int flushCount = writer->getFlushCount();
    if (j == 1) {
      lastFlushCount = flushCount;
    } else if (j < 10) {
      // No new files should be created
      TestUtil::assertEquals(flushCount, lastFlushCount);
    } else if (10 == j) {
      assertTrue(flushCount > lastFlushCount);
      lastFlushCount = flushCount;
      writer->getConfig()->setRAMBufferSizeMB(0.000001);
      writer->getConfig()->setMaxBufferedDocs(
          IndexWriterConfig::DISABLE_AUTO_FLUSH);
    } else if (j < 20) {
      assertTrue(flushCount > lastFlushCount);
      lastFlushCount = flushCount;
    } else if (20 == j) {
      writer->getConfig()->setRAMBufferSizeMB(16);
      writer->getConfig()->setMaxBufferedDocs(
          IndexWriterConfig::DISABLE_AUTO_FLUSH);
      lastFlushCount = flushCount;
    } else if (j < 30) {
      TestUtil::assertEquals(flushCount, lastFlushCount);
    } else if (30 == j) {
      writer->getConfig()->setRAMBufferSizeMB(0.000001);
      writer->getConfig()->setMaxBufferedDocs(
          IndexWriterConfig::DISABLE_AUTO_FLUSH);
    } else if (j < 40) {
      assertTrue(flushCount > lastFlushCount);
      lastFlushCount = flushCount;
    } else if (40 == j) {
      writer->getConfig()->setMaxBufferedDocs(10);
      writer->getConfig()->setRAMBufferSizeMB(
          IndexWriterConfig::DISABLE_AUTO_FLUSH);
      lastFlushCount = flushCount;
    } else if (j < 50) {
      TestUtil::assertEquals(flushCount, lastFlushCount);
      writer->getConfig()->setMaxBufferedDocs(10);
      writer->getConfig()->setRAMBufferSizeMB(
          IndexWriterConfig::DISABLE_AUTO_FLUSH);
    } else if (50 == j) {
      assertTrue(flushCount > lastFlushCount);
    }
  }
  delete writer;
  delete dir;
}

void TestIndexWriter::testEnablingNorms() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10));
  // Enable norms for only 1 doc, pre flush
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setOmitNorms(true);
  for (int j = 0; j < 10; j++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> f = nullptr;
    if (j != 8) {
      f = newField(L"field", L"aaa", customType);
    } else {
      f = newField(L"field", L"aaa", storedTextType);
    }
    doc->push_back(f);
    writer->addDocument(doc);
  }
  delete writer;

  shared_ptr<Term> searchTerm = make_shared<Term>(L"field", L"aaa");

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  TestUtil::assertEquals(10, hits.size());
  delete reader;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::CREATE)
               ->setMaxBufferedDocs(10));
  // Enable norms for only 1 doc, post flush
  for (int j = 0; j < 27; j++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> f = nullptr;
    if (j != 26) {
      f = newField(L"field", L"aaa", customType);
    } else {
      f = newField(L"field", L"aaa", storedTextType);
    }
    doc->push_back(f);
    writer->addDocument(doc);
  }
  delete writer;
  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  TestUtil::assertEquals(27, hits.size());
  delete reader;

  reader = DirectoryReader::open(dir);
  delete reader;

  delete dir;
}

void TestIndexWriter::testHighFreqTerm() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setRAMBufferSizeMB(0.01));
  // Massive doc that has 128 K a's
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>(1024 * 1024);
  for (int i = 0; i < 4096; i++) {
    b->append(L" a a a a a a a a");
    b->append(L" a a a a a a a a");
    b->append(L" a a a a a a a a");
    b->append(L" a a a a a a a a");
  }
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(newField(L"field", b->toString(), customType));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, reader->maxDoc());
  TestUtil::assertEquals(1, reader->numDocs());
  shared_ptr<Term> t = make_shared<Term>(L"field", L"a");
  TestUtil::assertEquals(1, reader->docFreq(t));
  shared_ptr<PostingsEnum> td =
      TestUtil::docs(random(), reader, L"field", make_shared<BytesRef>(L"a"),
                     nullptr, PostingsEnum::FREQS);
  td->nextDoc();
  TestUtil::assertEquals(128 * 1024, td->freq());
  delete reader;
  delete dir;
}

void TestIndexWriter::testFlushWithNoMerging() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy(10)));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(newField(L"field", L"aaa", customType));
  for (int i = 0; i < 19; i++) {
    writer->addDocument(doc);
  }
  writer->flush(false, true);
  delete writer;
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  // Since we flushed w/o allowing merging we should now
  // have 10 segments
  TestUtil::assertEquals(10, sis->size());
  delete dir;
}

void TestIndexWriter::testEmptyDocAfterFlushingRealDoc() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(newField(L"field", L"aaa", customType));
  writer->addDocument(doc);
  writer->commit();
  if (VERBOSE) {
    wcout << L"\nTEST: now add empty doc" << endl;
  }
  writer->addDocument(make_shared<Document>());
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(2, reader->numDocs());
  delete reader;
  delete dir;
}

void TestIndexWriter::testBadSegment() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  document->push_back(newField(L"tvtest", L"", customType));
  iw->addDocument(document);
  delete iw;
  delete dir;
}

void TestIndexWriter::testMaxThreadPriority() 
{
  int pri = Thread::currentThread().getPriority();
  try {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setMaxBufferedDocs(2)
            ->setMergePolicy(newLogMergePolicy());
    (std::static_pointer_cast<LogMergePolicy>(conf->getMergePolicy()))
        ->setMergeFactor(2);
    shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, conf);
    shared_ptr<Document> document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    document->push_back(newField(L"tvtest", L"a b c", customType));
    Thread::currentThread().setPriority(Thread::MAX_PRIORITY);
    for (int i = 0; i < 4; i++) {
      iw->addDocument(document);
    }
    delete iw;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    Thread::currentThread().setPriority(pri);
  }
}

void TestIndexWriter::testVariableSchema() 
{
  shared_ptr<Directory> dir = newDirectory();
  for (int i = 0; i < 20; i++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << i << endl;
    }
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2)
                 ->setMergePolicy(newLogMergePolicy()));
    // LogMergePolicy lmp = (LogMergePolicy)
    // writer.getConfig().getMergePolicy(); lmp.setMergeFactor(2);
    // lmp.setNoCFSRatio(0.0);
    shared_ptr<Document> doc = make_shared<Document>();
    wstring contents = L"aa bb cc dd ee ff gg hh ii jj kk";

    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_STORED);
    shared_ptr<FieldType> type = nullptr;
    if (i == 7) {
      // Add empty docs here
      doc->push_back(newTextField(L"content3", L"", Field::Store::NO));
    } else {
      if (i % 2 == 0) {
        doc->push_back(newField(L"content4", contents, customType));
        type = customType;
      } else {
        type = TextField::TYPE_NOT_STORED;
      }
      doc->push_back(newTextField(L"content1", contents, Field::Store::NO));
      doc->push_back(newField(L"content3", L"", customType));
      doc->push_back(newField(L"content5", L"", type));
    }

    for (int j = 0; j < 4; j++) {
      writer->addDocument(doc);
    }

    delete writer;

    if (0 == i % 4) {
      writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
      // LogMergePolicy lmp2 = (LogMergePolicy)
      // writer.getConfig().getMergePolicy(); lmp2.setNoCFSRatio(0.0);
      writer->forceMerge(1);
      delete writer;
    }
  }
  delete dir;
}

void TestIndexWriter::testUnlimitedMaxFieldLength() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < 10000; i++) {
    b->append(L" a");
  }
  b->append(L" x");
  doc->push_back(newTextField(L"field", b->toString(), Field::Store::NO));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<Term> t = make_shared<Term>(L"field", L"x");
  TestUtil::assertEquals(1, reader->docFreq(t));
  delete reader;
  delete dir;
}

void TestIndexWriter::testEmptyFieldName() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"", L"a b c", Field::Store::NO));
  writer->addDocument(doc);
  delete writer;
  delete dir;
}

void TestIndexWriter::testEmptyFieldNameTerms() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"", L"a b c", Field::Store::NO));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> subreader = getOnlyLeafReader(reader);
  shared_ptr<TermsEnum> te = subreader->terms(L"")->begin();
  TestUtil::assertEquals(make_shared<BytesRef>(L"a"), te->next());
  TestUtil::assertEquals(make_shared<BytesRef>(L"b"), te->next());
  TestUtil::assertEquals(make_shared<BytesRef>(L"c"), te->next());
  assertNull(te->next());
  reader->close();
  delete dir;
}

void TestIndexWriter::testEmptyFieldNameWithEmptyTerm() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"", L"", Field::Store::NO));
  doc->push_back(newStringField(L"", L"a", Field::Store::NO));
  doc->push_back(newStringField(L"", L"b", Field::Store::NO));
  doc->push_back(newStringField(L"", L"c", Field::Store::NO));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> subreader = getOnlyLeafReader(reader);
  shared_ptr<TermsEnum> te = subreader->terms(L"")->begin();
  TestUtil::assertEquals(make_shared<BytesRef>(L""), te->next());
  TestUtil::assertEquals(make_shared<BytesRef>(L"a"), te->next());
  TestUtil::assertEquals(make_shared<BytesRef>(L"b"), te->next());
  TestUtil::assertEquals(make_shared<BytesRef>(L"c"), te->next());
  assertNull(te->next());
  reader->close();
  delete dir;
}

TestIndexWriter::MockIndexWriter::MockIndexWriter(
    shared_ptr<Directory> dir,
    shared_ptr<IndexWriterConfig> conf) 
    : IndexWriter(dir, conf)
{
}

void TestIndexWriter::MockIndexWriter::doAfterFlush() { afterWasCalled = true; }

void TestIndexWriter::MockIndexWriter::doBeforeFlush()
{
  beforeWasCalled = true;
}

void TestIndexWriter::testDoBeforeAfterFlush() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockIndexWriter> w = make_shared<MockIndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  doc->push_back(newField(L"field", L"a field", customType));
  w->addDocument(doc);
  w->commit();
  assertTrue(w->beforeWasCalled);
  assertTrue(w->afterWasCalled);
  w->beforeWasCalled = false;
  w->afterWasCalled = false;
  w->deleteDocuments({make_shared<Term>(L"field", L"field")});
  w->commit();
  assertTrue(w->beforeWasCalled);
  assertTrue(w->afterWasCalled);
  delete w;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, ir->numDocs());
  delete ir;

  delete dir;
}

void TestIndexWriter::testNegativePositions() 
{
  shared_ptr<TokenStream> *const tokens =
      make_shared<TokenStreamAnonymousInnerClass>(shared_from_this());

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"field", tokens));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });

  delete w;
  delete dir;
}

TestIndexWriter::TokenStreamAnonymousInnerClass::TokenStreamAnonymousInnerClass(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  terms = Arrays::asList(L"a", L"b", L"c").begin();
  first = true;
}

bool TestIndexWriter::TokenStreamAnonymousInnerClass::incrementToken()
{
  if (!terms->hasNext()) {
    return false;
  }
  clearAttributes();
  termAtt->append(terms->next());
  posIncrAtt::setPositionIncrement(first ? 0 : 1);
  first = false;
  return true;
}

void TestIndexWriter::testPositionIncrementGapEmptyField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setPositionIncrementGap(100);
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  shared_ptr<Field> f = newField(L"field", L"", customType);
  shared_ptr<Field> f2 = newField(L"field", L"crunch man", customType);
  doc->push_back(f);
  doc->push_back(f2);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<Terms> tpv = r->getTermVectors(0)->terms(L"field");
  shared_ptr<TermsEnum> termsEnum = tpv->begin();
  assertNotNull(termsEnum->next());
  shared_ptr<PostingsEnum> dpEnum =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertNotNull(dpEnum);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(1, dpEnum->freq());
  TestUtil::assertEquals(100, dpEnum->nextPosition());

  assertNotNull(termsEnum->next());
  dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
  assertNotNull(dpEnum);
  assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(1, dpEnum->freq());
  TestUtil::assertEquals(101, dpEnum->nextPosition());
  assertNull(termsEnum->next());

  delete r;
  delete dir;
}

void TestIndexWriter::testDeadlock() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);

  doc->push_back(
      newField(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii", customType));
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->commit();
  // index has 2 segments

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer2->addDocument(doc);
  delete writer2;

  shared_ptr<DirectoryReader> r1 = DirectoryReader::open(dir2);
  TestUtil::addIndexesSlowly(writer, {r1, r1});
  delete writer;

  shared_ptr<IndexReader> r3 = DirectoryReader::open(dir);
  TestUtil::assertEquals(5, r3->numDocs());
  delete r3;

  r1->close();

  delete dir2;
  delete dir;
}

TestIndexWriter::IndexerThreadInterrupt::IndexerThreadInterrupt(
    shared_ptr<TestIndexWriter> outerInstance, int id) 
    : random(make_shared<Random>(random()->nextLong())),
      adder(make_shared<MockDirectoryWrapper>(random,
                                              make_shared<RAMDirectory>())),
      id(id), outerInstance(outerInstance)
{
  // make a little directory for addIndexes
  // LUCENE-2239: won't work with NIOFS/MMAP
  shared_ptr<IndexWriterConfig> conf = LuceneTestCase::newIndexWriterConfig(
      random, make_shared<MockAnalyzer>(random));
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(
          conf->getMergeScheduler()) != nullptr) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    conf->setMergeScheduler(
        make_shared<SuppressingConcurrentMergeSchedulerAnonymousInnerClass>(
            shared_from_this()));
  }
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(adder, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      LuceneTestCase::newStringField(random, L"id", L"500", Field::Store::NO));
  doc->push_back(LuceneTestCase::newField(
      random, L"field", L"some prepackaged text contents", storedTextType));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"binarydv", make_shared<BytesRef>(L"500")));
  doc->push_back(make_shared<NumericDocValuesField>(L"numericdv", 500));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"sorteddv", make_shared<BytesRef>(L"500")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sortedsetdv", make_shared<BytesRef>(L"one")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sortedsetdv", make_shared<BytesRef>(L"two")));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sortednumericdv", 4));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sortednumericdv", 3));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      LuceneTestCase::newStringField(random, L"id", L"501", Field::Store::NO));
  doc->push_back(LuceneTestCase::newField(
      random, L"field", L"some more contents", storedTextType));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"binarydv", make_shared<BytesRef>(L"501")));
  doc->push_back(make_shared<NumericDocValuesField>(L"numericdv", 501));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"sorteddv", make_shared<BytesRef>(L"501")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sortedsetdv", make_shared<BytesRef>(L"two")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sortedsetdv", make_shared<BytesRef>(L"three")));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sortednumericdv", 6));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sortednumericdv", 1));
  w->addDocument(doc);
  w->deleteDocuments({make_shared<Term>(L"id", L"500")});
  delete w;
}

TestIndexWriter::IndexerThreadInterrupt::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::
        SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
            shared_ptr<IndexerThreadInterrupt> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriter::IndexerThreadInterrupt::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::isOK(
        runtime_error th)
{
  return std::dynamic_pointer_cast<AlreadyClosedException>(th) != nullptr ||
         (std::dynamic_pointer_cast<IllegalStateException>(th) != nullptr &&
          th.what()->contains(L"this writer hit an unrecoverable error"));
}

void TestIndexWriter::IndexerThreadInterrupt::run()
{
  // LUCENE-2239: won't work with NIOFS/MMAP
  shared_ptr<MockDirectoryWrapper> dir =
      make_shared<MockDirectoryWrapper>(random, make_shared<RAMDirectory>());

  // open/close slowly sometimes
  dir->setUseSlowOpenClosers(true);

  // throttle a little
  dir->setThrottling(MockDirectoryWrapper::Throttling::SOMETIMES);

  shared_ptr<IndexWriter> w = nullptr;
  while (!finish) {
    try {

      while (!finish) {
        if (w != nullptr) {
          // If interrupt arrives inside here, it's
          // fine: we will cycle back and the first
          // thing we do is try to close again,
          // i.e. we'll never try to open a new writer
          // until this one successfully closes:
          // w.rollback();
          try {
            delete w;
          } catch (const AlreadyClosedException &ace) {
            // OK
          }
          w.reset();
        }
        shared_ptr<IndexWriterConfig> conf =
            LuceneTestCase::newIndexWriterConfig(
                random, make_shared<MockAnalyzer>(random))
                ->setMaxBufferedDocs(2);
        if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(
                conf->getMergeScheduler()) != nullptr) {
          conf->setMergeScheduler(
              make_shared<
                  SuppressingConcurrentMergeSchedulerAnonymousInnerClass2>(
                  shared_from_this()));
        }
        // conf.setInfoStream(log);
        w = make_shared<IndexWriter>(dir, conf);

        shared_ptr<Document> doc = make_shared<Document>();
        shared_ptr<Field> idField = LuceneTestCase::newStringField(
            random, L"id", L"", Field::Store::NO);
        shared_ptr<Field> binaryDVField = make_shared<BinaryDocValuesField>(
            L"binarydv", make_shared<BytesRef>());
        shared_ptr<Field> numericDVField =
            make_shared<NumericDocValuesField>(L"numericdv", 0);
        shared_ptr<Field> sortedDVField = make_shared<SortedDocValuesField>(
            L"sorteddv", make_shared<BytesRef>());
        shared_ptr<Field> sortedSetDVField =
            make_shared<SortedSetDocValuesField>(L"sortedsetdv",
                                                 make_shared<BytesRef>());
        doc->push_back(idField);
        doc->push_back(LuceneTestCase::newField(
            random, L"field", L"some text contents", storedTextType));
        doc->push_back(binaryDVField);
        doc->push_back(numericDVField);
        doc->push_back(sortedDVField);
        doc->push_back(sortedSetDVField);
        for (int i = 0; i < 100; i++) {
          // log.println("\nTEST: i=" + i);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          idField->setStringValue(Integer::toString(i));
          binaryDVField->setBytesValue(
              make_shared<BytesRef>(idField->stringValue()));
          numericDVField->setLongValue(i);
          sortedDVField->setBytesValue(
              make_shared<BytesRef>(idField->stringValue()));
          sortedSetDVField->setBytesValue(
              make_shared<BytesRef>(idField->stringValue()));
          int action = random->nextInt(100);
          if (action == 17) {
            w->addIndexes({adder});
          } else if (action % 30 == 0) {
            w->deleteAll();
          } else if (action % 2 == 0) {
            w->updateDocument(make_shared<Term>(L"id", idField->stringValue()),
                              doc);
          } else {
            w->addDocument(doc);
          }
          if (random->nextInt(3) == 0) {
            shared_ptr<IndexReader> r = nullptr;
            try {
              r = DirectoryReader::open(w, random->nextBoolean(), false);
              if (random->nextBoolean() && r->maxDoc() > 0) {
                int docid = random->nextInt(r->maxDoc());
                w->tryDeleteDocument(r, docid);
              }
            }
            // C++ TODO: There is no native C++ equivalent to the exception
            // 'finally' clause:
            finally {
              IOUtils::closeWhileHandlingException({r});
            }
          }
          if (i % 10 == 0) {
            w->commit();
          }
          if (random->nextInt(50) == 0) {
            w->forceMerge(1);
          }
        }
        delete w;
        w.reset();
        DirectoryReader::open(dir)->close();

        // Strangely, if we interrupt a thread before
        // all classes are loaded, the class loader
        // seems to do scary things with the interrupt
        // status.  In java 1.5, it'll throw an
        // incorrect ClassNotFoundException.  In java
        // 1.6, it'll silently clear the interrupt.
        // So, on first iteration through here we
        // don't open ourselves up for interrupts
        // until we've done the above loop.
        allowInterrupt = true;
      }
    } catch (const ThreadInterruptedException &re) {
      // NOTE: important to leave this verbosity/noise
      // on!!  This test doesn't repro easily so when
      // Jenkins hits a fail we need to study where the
      // interrupts struck!
      log->println(L"TEST thread " + to_wstring(id) + L": got interrupt");
      re->printStackTrace(log);
      runtime_error e = re->getCause();
      assertTrue(std::dynamic_pointer_cast<InterruptedException>(e) != nullptr);
      if (finish) {
        break;
      }
    } catch (const runtime_error &t) {
      log->println(L"thread " + to_wstring(id) +
                   L" FAILED; unexpected exception");
      t.printStackTrace(log);
      listIndexFiles(log, dir);
      failed = true;
      break;
    }
  }

  if (VERBOSE) {
    log->println(L"TEST: thread " + to_wstring(id) + L": now finish failed=" +
                 StringHelper::toString(failed));
  }
  if (!failed) {
    if (VERBOSE) {
      log->println(L"TEST: thread " + to_wstring(id) + L": now rollback");
    }
    // clear interrupt state:
    Thread::interrupted();
    if (w != nullptr) {
      try {
        w->rollback();
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }
    }

    try {
      TestUtil::checkIndex(dir);
    } catch (const runtime_error &e) {
      failed = true;
      log->println(L"thread " + to_wstring(id) +
                   L": CheckIndex FAILED: unexpected exception");
      e.printStackTrace(log);
      listIndexFiles(log, dir);
    }
    try {
      shared_ptr<IndexReader> r = DirectoryReader::open(dir);
      // System.out.println("doc count=" + r.numDocs());
      delete r;
    } catch (const runtime_error &e) {
      failed = true;
      log->println(L"thread " + to_wstring(id) +
                   L": DirectoryReader.open FAILED: unexpected exception");
      e.printStackTrace(log);
      listIndexFiles(log, dir);
    }
  }
  try {
    IOUtils::close({dir});
  } catch (const IOException &e) {
    failed = true;
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("thread " + id, e);
    throw runtime_error(L"thread " + to_wstring(id));
  }
  try {
    IOUtils::close({adder});
  } catch (const IOException &e) {
    failed = true;
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("thread " + id, e);
    throw runtime_error(L"thread " + to_wstring(id));
  }
}

TestIndexWriter::IndexerThreadInterrupt::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass2::
        SuppressingConcurrentMergeSchedulerAnonymousInnerClass2(
            shared_ptr<IndexerThreadInterrupt> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriter::IndexerThreadInterrupt::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass2::isOK(
        runtime_error th)
{
  return std::dynamic_pointer_cast<AlreadyClosedException>(th) != nullptr ||
         (std::dynamic_pointer_cast<IllegalStateException>(th) != nullptr &&
          th.what()->contains(L"this writer hit an unrecoverable error"));
}

void TestIndexWriter::IndexerThreadInterrupt::listIndexFiles(
    shared_ptr<PrintStream> log, shared_ptr<Directory> dir)
{
  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    log->println(L"index files: " + Arrays->toString(dir->listAll()));
  } catch (const IOException &ioe) {
    // Suppress
    log->println(L"failed to index files:");
    ioe->printStackTrace(log);
  }
}

void TestIndexWriter::testThreadInterruptDeadlock() 
{
  shared_ptr<IndexerThreadInterrupt> t =
      make_shared<IndexerThreadInterrupt>(shared_from_this(), 1);
  t->setDaemon(true);
  t->start();

  // Force class loader to load ThreadInterruptedException
  // up front... else we can see a false failure if 2nd
  // interrupt arrives while class loader is trying to
  // init this class (in servicing a first interrupt):
  assertTrue(std::dynamic_pointer_cast<InterruptedException>(
                 (make_shared<ThreadInterruptedException>(
                      make_shared<InterruptedException>()))
                     ->getCause()) != nullptr);

  // issue 100 interrupts to child thread
  constexpr int numInterrupts = atLeast(100);
  int i = 0;
  while (i < numInterrupts) {
    // TODO: would be nice to also sometimes interrupt the
    // CMS merge threads too ...
    delay(10);
    if (t->allowInterrupt) {
      i++;
      t->interrupt();
    }
    if (!t->isAlive()) {
      break;
    }
  }
  t->finish = true;
  t->join();
  if (t->failed) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    fail(t->bytesLog->toString(L"UTF-8"));
  }
}

void TestIndexWriter::testIndexStoreCombos() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  std::deque<char> b(50);
  for (int i = 0; i < 50; i++) {
    b[i] = static_cast<char>(i + 77);
  }

  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType = make_shared<FieldType>(StoredField::TYPE);
  customType->setTokenized(true);

  shared_ptr<Field> f = make_shared<Field>(L"binary", b, 10, 17, customType);
  // TODO: this is evil, changing the type after creating the field:
  customType->setIndexOptions(IndexOptions::DOCS);
  shared_ptr<MockTokenizer> *const doc1field1 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  doc1field1->setReader(make_shared<StringReader>(L"doc1field1"));
  f->setTokenStream(doc1field1);

  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_STORED);

  shared_ptr<Field> f2 = newField(L"string", L"value", customType2);
  shared_ptr<MockTokenizer> *const doc1field2 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  doc1field2->setReader(make_shared<StringReader>(L"doc1field2"));
  f2->setTokenStream(doc1field2);
  doc->push_back(f);
  doc->push_back(f2);
  w->addDocument(doc);

  // add 2 docs to test in-memory merging
  shared_ptr<MockTokenizer> *const doc2field1 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  doc2field1->setReader(make_shared<StringReader>(L"doc2field1"));
  f->setTokenStream(doc2field1);
  shared_ptr<MockTokenizer> *const doc2field2 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  doc2field2->setReader(make_shared<StringReader>(L"doc2field2"));
  f2->setTokenStream(doc2field2);
  w->addDocument(doc);

  // force segment flush so we can force a segment merge with doc3 later.
  w->commit();

  shared_ptr<MockTokenizer> *const doc3field1 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  doc3field1->setReader(make_shared<StringReader>(L"doc3field1"));
  f->setTokenStream(doc3field1);
  shared_ptr<MockTokenizer> *const doc3field2 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  doc3field2->setReader(make_shared<StringReader>(L"doc3field2"));
  f2->setTokenStream(doc3field2);

  w->addDocument(doc);
  w->commit();
  w->forceMerge(1); // force segment merge.
  delete w;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Document> doc2 = ir->document(0);
  shared_ptr<IndexableField> f3 = doc2->getField(L"binary");
  b = f3->binaryValue()->bytes;
  assertTrue(b.size() > 0);
  assertEquals(17, b.size(), 17);
  TestUtil::assertEquals(87, b[0]);

  assertTrue(ir->document(0)->getField(L"binary")->binaryValue() != nullptr);
  assertTrue(ir->document(1)->getField(L"binary")->binaryValue() != nullptr);
  assertTrue(ir->document(2)->getField(L"binary")->binaryValue() != nullptr);

  TestUtil::assertEquals(L"value", ir->document(0)[L"string"]);
  TestUtil::assertEquals(L"value", ir->document(1)[L"string"]);
  TestUtil::assertEquals(L"value", ir->document(2)[L"string"]);

  // test that the terms were indexed.
  assertTrue(TestUtil::docs(random(), ir, L"binary",
                            make_shared<BytesRef>(L"doc1field1"), nullptr,
                            PostingsEnum::NONE)
                 ->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(TestUtil::docs(random(), ir, L"binary",
                            make_shared<BytesRef>(L"doc2field1"), nullptr,
                            PostingsEnum::NONE)
                 ->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(TestUtil::docs(random(), ir, L"binary",
                            make_shared<BytesRef>(L"doc3field1"), nullptr,
                            PostingsEnum::NONE)
                 ->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(TestUtil::docs(random(), ir, L"string",
                            make_shared<BytesRef>(L"doc1field2"), nullptr,
                            PostingsEnum::NONE)
                 ->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(TestUtil::docs(random(), ir, L"string",
                            make_shared<BytesRef>(L"doc2field2"), nullptr,
                            PostingsEnum::NONE)
                 ->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(TestUtil::docs(random(), ir, L"string",
                            make_shared<BytesRef>(L"doc3field2"), nullptr,
                            PostingsEnum::NONE)
                 ->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  delete ir;
  delete dir;
}

void TestIndexWriter::testNoDocsIndex() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->addDocument(make_shared<Document>());
  delete writer;

  delete dir;
}

void TestIndexWriter::testDeleteUnusedFiles() 
{
  assumeFalse(L"test relies on exact filenames",
              std::dynamic_pointer_cast<SimpleTextCodec>(Codec::getDefault()) !=
                  nullptr);
  assumeWorkingMMapOnWindows();

  for (int iter = 0; iter < 2; iter++) {
    // relies on windows semantics
    shared_ptr<Path> path = createTempDir();
    shared_ptr<FileSystem> fs = (make_shared<WindowsFS>(path->getFileSystem()))
                                    ->getFileSystem(URI::create(L"file:///"));
    shared_ptr<Path> indexPath = make_shared<FilterPath>(path, fs);

    // NOTE: on Unix, we cannot use MMapDir, because WindowsFS doesn't see/think
    // it keeps file handles open.  Yet, on Windows, we MUST use MMapDir because
    // the windows OS will in fact prevent file deletion for us, and fails
    // otherwise:
    shared_ptr<FSDirectory> dir;
    if (Constants::WINDOWS) {
      dir = make_shared<MMapDirectory>(indexPath);
    } else {
      dir = make_shared<NIOFSDirectory>(indexPath);
    }

    shared_ptr<MergePolicy> mergePolicy = newLogMergePolicy(true);

    // This test expects all of its segments to be in CFS
    mergePolicy->setNoCFSRatio(1.0);
    mergePolicy->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());

    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMergePolicy(mergePolicy)
                 ->setUseCompoundFile(true));
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"field", L"go", Field::Store::NO));
    w->addDocument(doc);
    shared_ptr<DirectoryReader> r;
    if (iter == 0) {
      // use NRT
      r = w->getReader();
    } else {
      // don't use NRT
      w->commit();
      r = DirectoryReader::open(dir);
    }

    assertTrue(Files::exists(indexPath->resolve(L"_0.cfs")));
    assertTrue(Files::exists(indexPath->resolve(L"_0.cfe")));
    assertTrue(Files::exists(indexPath->resolve(L"_0.si")));
    if (iter == 1) {
      // we run a full commit so there should be a segments file etc.
      assertTrue(Files::exists(indexPath->resolve(L"segments_1")));
    } else {
      // this is an NRT reopen - no segments files yet
      assertFalse(Files::exists(indexPath->resolve(L"segments_1")));
    }
    w->addDocument(doc);
    w->forceMerge(1);
    if (iter == 1) {
      w->commit();
    }
    shared_ptr<IndexReader> r2 = DirectoryReader::openIfChanged(r);
    assertNotNull(r2);
    assertTrue(r != r2);

    // NOTE: here we rely on "Windows" behavior, ie, even
    // though IW wanted to delete _0.cfs since it was
    // merged away, because we have a reader open
    // against this file, it should still be here:
    assertTrue(Files::exists(indexPath->resolve(L"_0.cfs")));
    // forceMerge created this
    // assertTrue(files.contains("_2.cfs"));
    w->deleteUnusedFiles();

    // r still holds this file open
    assertTrue(Files::exists(indexPath->resolve(L"_0.cfs")));
    // assertTrue(files.contains("_2.cfs"));

    r->close();
    if (iter == 0) {
      // on closing NRT reader, it calls writer.deleteUnusedFiles
      assertFalse(Files::exists(indexPath->resolve(L"_0.cfs")));
    } else {
      // now FSDir can remove it
      dir->deletePendingFiles();
      assertFalse(Files::exists(indexPath->resolve(L"_0.cfs")));
    }

    delete w;
    delete r2;

    delete dir;
  }
}

void TestIndexWriter::testDeleteUnusedFiles2() 
{
  // Validates that iw.deleteUnusedFiles() also deletes unused index commits
  // in case a deletion policy which holds onto commits is used.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(make_shared<SnapshotDeletionPolicy>(
                   make_shared<KeepOnlyLastCommitDeletionPolicy>())));
  shared_ptr<SnapshotDeletionPolicy> sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());

  // First commit
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);

  doc->push_back(newField(L"c", L"val", customType));
  writer->addDocument(doc);
  writer->commit();
  TestUtil::assertEquals(1, DirectoryReader::listCommits(dir).size());

  // Keep that commit
  shared_ptr<IndexCommit> id = sdp->snapshot();

  // Second commit - now KeepOnlyLastCommit cannot delete the prev commit.
  doc = make_shared<Document>();
  doc->push_back(newField(L"c", L"val", customType));
  writer->addDocument(doc);
  writer->commit();
  TestUtil::assertEquals(2, DirectoryReader::listCommits(dir).size());

  // Should delete the unreferenced commit
  sdp->release(id);
  writer->deleteUnusedFiles();
  TestUtil::assertEquals(1, DirectoryReader::listCommits(dir).size());

  delete writer;
  delete dir;
}

void TestIndexWriter::testEmptyFSDirWithNoLock() 
{
  // Tests that if FSDir is opened w/ a NoLockFactory (or SingleInstanceLF),
  // then IndexWriter ctor succeeds. Previously (LUCENE-2386) it failed
  // when listAll() was called in IndexFileDeleter.
  shared_ptr<Directory> dir = newFSDirectory(createTempDir(L"emptyFSDirNoLock"),
                                             NoLockFactory::INSTANCE);
  delete (make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))));
  delete dir;
}

void TestIndexWriter::testEmptyDirRollback() 
{
  // TODO: generalize this test
  assumeFalse(L"test makes assumptions about file counts",
              std::dynamic_pointer_cast<SimpleTextCodec>(Codec::getDefault()) !=
                  nullptr);
  // Tests that if IW is created over an empty Directory, some documents are
  // indexed, flushed (but not committed) and then IW rolls back, then no
  // files are left in the Directory.
  shared_ptr<Directory> dir = newDirectory();

  std::deque<wstring> origFiles = dir->listAll();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy())
               ->setUseCompoundFile(false));
  std::deque<wstring> files = dir->listAll();

  // Creating over empty dir should not create any files,
  // or, at most the write.lock file
  constexpr int extraFileCount = files.size() - origFiles.size();
  if (extraFileCount == 1) {
    assertTrue(Arrays::asList(files)->contains(IndexWriter::WRITE_LOCK_NAME));
  } else {
    Arrays::sort(origFiles);
    Arrays::sort(files);
    assertArrayEquals(origFiles, files);
  }

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  // create as many files as possible
  doc->push_back(newField(L"c", L"val", customType));
  writer->addDocument(doc);
  // Adding just one document does not call flush yet.
  int computedExtraFileCount = 0;
  for (auto file : dir->listAll()) {
    if (IndexWriter::WRITE_LOCK_NAME == file ||
        file.startsWith(IndexFileNames::SEGMENTS) ||
        IndexFileNames::CODEC_FILE_PATTERN->matcher(file).matches()) {
      if (file.rfind(L'.') < 0 ||
          !Arrays::asList(L"fdx", L"fdt", L"tvx", L"tvd", L"tvf")
               ->contains(file.substr(file.rfind(L'.') + 1))) {
        ++computedExtraFileCount;
      }
    }
  }
  assertEquals(
      L"only the stored and term deque files should exist in the directory",
      extraFileCount, computedExtraFileCount);

  doc = make_shared<Document>();
  doc->push_back(newField(L"c", L"val", customType));
  writer->addDocument(doc);

  // The second document should cause a flush.
  assertTrue(L"flush should have occurred and files should have been created",
             dir->listAll().size() > 5 + extraFileCount);

  // After rollback, IW should remove all files
  writer->rollback();
  std::deque<wstring> allFiles = dir->listAll();
  assertEquals(L"no files should exist in the directory after rollback",
               origFiles.size() + extraFileCount, allFiles.size());

  // Since we rolled-back above, that close should be a no-op
  delete writer;
  allFiles = dir->listAll();
  assertEquals(L"expected a no-op close after IW.rollback()",
               origFiles.size() + extraFileCount, allFiles.size());
  delete dir;
}

void TestIndexWriter::testNoUnwantedTVFiles() 
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setRAMBufferSizeMB(0.01)
               ->setMergePolicy(newLogMergePolicy()));
  indexWriter->getConfig()->getMergePolicy()->setNoCFSRatio(0.0);

  wstring BIG = L"alskjhlaksjghlaksjfhalksvjepgjioefgjnsdfjgefgjhelkgjhqewlrkhg"
                L"wlekgrhwelkgjhwelkgrhwlkejg";
  BIG = BIG + BIG + BIG + BIG;

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setOmitNorms(true);
  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType2->setTokenized(false);
  shared_ptr<FieldType> customType3 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType3->setTokenized(false);
  customType3->setOmitNorms(true);

  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<Field>(L"id", Integer::toString(i) + BIG, customType3));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<Field>(L"str", Integer::toString(i) + BIG, customType2));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<Field>(L"str2", Integer::toString(i) + BIG,
                                      storedTextType));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<Field>(L"str3", Integer::toString(i) + BIG, customType));
    indexWriter->addDocument(doc);
  }

  delete indexWriter;

  TestUtil::checkIndex(dir);

  assertNoUnreferencedFiles(dir, L"no tv files");
  shared_ptr<DirectoryReader> r0 = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> ctx : r0->leaves()) {
    shared_ptr<SegmentReader> sr =
        std::static_pointer_cast<SegmentReader>(ctx->reader());
    assertFalse(sr->getFieldInfos()->hasVectors());
  }

  r0->close();
  delete dir;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriter::StringSplitAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<StringSplitTokenizer>());
}

TestIndexWriter::StringSplitTokenizer::StringSplitTokenizer()
    : org::apache::lucene::analysis::Tokenizer()
{
}

bool TestIndexWriter::StringSplitTokenizer::incrementToken()
{
  clearAttributes();
  if (upto < tokens.size()) {
    termAtt->setEmpty();
    termAtt->append(tokens[upto]);
    upto++;
    return true;
  } else {
    return false;
  }
}

void TestIndexWriter::StringSplitTokenizer::reset() 
{
  Tokenizer::reset();
  this->upto = 0;
  shared_ptr<StringBuilder> *const b = make_shared<StringBuilder>();
  const std::deque<wchar_t> buffer = std::deque<wchar_t>(1024);
  int n;
  while ((n = input->read(buffer)) != -1) {
    b->append(buffer, 0, n);
  }
  this->tokens = b->toString()->split(L" ");
}

void TestIndexWriter::testWickedLongTerm() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir, make_shared<StringSplitAnalyzer>());

  std::deque<wchar_t> chars(DocumentsWriterPerThread::MAX_TERM_LENGTH_UTF8);
  Arrays::fill(chars, L'x');
  shared_ptr<Document> hugeDoc = make_shared<Document>();
  const wstring bigTerm = wstring(chars);

  // This contents produces a too-long term:
  wstring contents = L"abc xyz x" + bigTerm + L" another term";
  hugeDoc->push_back(
      make_shared<TextField>(L"content", contents, Field::Store::NO));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(hugeDoc); });

  // Make sure we can add another normal document
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"content", L"abc bbb ccc", Field::Store::NO));
  w->addDocument(doc);

  // So we remove the deleted doc:
  w->forceMerge(1);

  shared_ptr<IndexReader> reader = w->getReader();
  delete w;

  // Make sure all terms < max size were indexed
  TestUtil::assertEquals(
      1, reader->docFreq(make_shared<Term>(L"content", L"abc")));
  TestUtil::assertEquals(
      1, reader->docFreq(make_shared<Term>(L"content", L"bbb")));
  TestUtil::assertEquals(
      0, reader->docFreq(make_shared<Term>(L"content", L"term")));

  // Make sure the doc that has the massive term is NOT in
  // the index:
  assertEquals(L"document with wicked long term is in the index!", 1,
               reader->numDocs());

  delete reader;
  delete dir;
  dir = newDirectory();

  // Make sure we can add a document with exactly the
  // maximum length term, and search on that term:
  doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setTokenized(false);
  shared_ptr<Field> contentField =
      make_shared<Field>(L"content", L"", customType);
  doc->push_back(contentField);

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(TestUtil::getDefaultCodec());

  shared_ptr<RandomIndexWriter> w2 =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  contentField->setStringValue(L"other");
  w2->addDocument(doc);

  contentField->setStringValue(L"term");
  w2->addDocument(doc);

  contentField->setStringValue(bigTerm);
  w2->addDocument(doc);

  contentField->setStringValue(L"zzz");
  w2->addDocument(doc);

  reader = w2->getReader();
  delete w2;
  TestUtil::assertEquals(
      1, reader->docFreq(make_shared<Term>(L"content", bigTerm)));

  delete reader;
  delete dir;
}

void TestIndexWriter::testDeleteAllNRTLeftoverFiles() 
{

  shared_ptr<MockDirectoryWrapper> d =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 100; ++j) {
      w->addDocument(doc);
    }
    w->commit();
    DirectoryReader::open(w)->close();

    w->deleteAll();
    w->commit();
    // Make sure we accumulate no files except for empty
    // segments_N and segments.gen:
    assertTrue(d->listAll().size() <= 2);
  }

  delete w;
  delete d;
}

void TestIndexWriter::testNRTReaderVersion() 
{
  shared_ptr<Directory> d =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r = w->getReader();
  int64_t version = r->getVersion();
  r->close();

  w->addDocument(doc);
  r = w->getReader();
  int64_t version2 = r->getVersion();
  r->close();
  assert(version2 > version);

  w->deleteDocuments({make_shared<Term>(L"id", L"0")});
  r = w->getReader();
  delete w;
  int64_t version3 = r->getVersion();
  r->close();
  assert(version3 > version2);
  delete d;
}

void TestIndexWriter::testWhetherDeleteAllDeletesWriteLock() throw(
    runtime_error)
{
  // Must use SimpleFSLockFactory... NativeFSLockFactory
  // somehow "knows" a lock is held against write.lock
  // even if you remove that file:
  shared_ptr<Directory> d = newFSDirectory(
      createTempDir(L"TestIndexWriter.testWhetherDeleteAllDeletesWriteLock"),
      SimpleFSLockFactory::INSTANCE);
  shared_ptr<RandomIndexWriter> w1 =
      make_shared<RandomIndexWriter>(random(), d);
  w1->deleteAll();
  expectThrows(LockObtainFailedException::typeid, [&]() {
    make_shared<RandomIndexWriter>(random(), d, newIndexWriterConfig(nullptr));
  });

  delete w1;
  delete d;
}

void TestIndexWriter::testChangeIndexOptions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<FieldType> docsAndFreqs =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  docsAndFreqs->setIndexOptions(IndexOptions::DOCS_AND_FREQS);

  shared_ptr<FieldType> docsOnly =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  docsOnly->setIndexOptions(IndexOptions::DOCS);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"a b c", docsAndFreqs));
  w->addDocument(doc);
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", L"a b c", docsOnly));
  w->addDocument(doc);
  delete w;
  delete dir;
}

void TestIndexWriter::testOnlyUpdateDocuments() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();
  docs.push_back(make_shared<Document>());
  w->updateDocuments(make_shared<Term>(L"foo", L"bar"), docs);
  delete w;
  delete dir;
}

void TestIndexWriter::testPrepareCommitThenClose() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  w->prepareCommit();
  expectThrows(IllegalStateException::typeid, [&]() { delete w; });
  w->commit();
  delete w;
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, r->maxDoc());
  delete r;
  delete dir;
}

void TestIndexWriter::testPrepareCommitThenRollback() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  w->prepareCommit();
  w->rollback();
  assertFalse(DirectoryReader::indexExists(dir));
  delete dir;
}

void TestIndexWriter::testPrepareCommitThenRollback2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  w->commit();
  w->addDocument(make_shared<Document>());
  w->prepareCommit();
  w->rollback();
  assertTrue(DirectoryReader::indexExists(dir));
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(0, r->maxDoc());
  delete r;
  delete dir;
}

void TestIndexWriter::testDontInvokeAnalyzerForUnAnalyzedFields() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
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
  delete dir;
}

TestIndexWriter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  throw make_shared<IllegalStateException>(L"don't invoke me!");
}

int TestIndexWriter::AnalyzerAnonymousInnerClass::getPositionIncrementGap(
    const wstring &fieldName)
{
  throw make_shared<IllegalStateException>(L"don't invoke me!");
}

int TestIndexWriter::AnalyzerAnonymousInnerClass::getOffsetGap(
    const wstring &fieldName)
{
  throw make_shared<IllegalStateException>(L"don't invoke me!");
}

void TestIndexWriter::testOtherFiles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  iw->addDocument(make_shared<Document>());
  delete iw;
  try {
    // Create my own random file:
    shared_ptr<IndexOutput> out =
        dir->createOutput(L"myrandomfile", newIOContext(random()));
    out->writeByte(static_cast<char>(42));
    delete out;

    delete (make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))));

    assertTrue(slowFileExists(dir, L"myrandomfile"));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete dir;
  }
}

void TestIndexWriter::testStopwordsPosIncHole() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"body", L"just a", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"body", L"test of gaps", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"body", L"just"), 0);
  builder->add(make_shared<Term>(L"body", L"test"), 2);
  shared_ptr<PhraseQuery> pq = builder->build();
  // body:"just ? test"
  TestUtil::assertEquals(1, is->search(pq, 5)->totalHits);
  delete ir;
  delete dir;
}

TestIndexWriter::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenFilter>(tokenizer, MockTokenFilter::ENGLISH_STOPSET);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestIndexWriter::testStopwordsPosIncHole2() 
{
  // use two stopfilters for testing here
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Automaton> *const secondSet = Automata::makeString(L"foobar");
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), secondSet);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, a);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"body", L"just a foobar", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"body", L"test of gaps", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"body", L"just"), 0);
  builder->add(make_shared<Term>(L"body", L"test"), 3);
  shared_ptr<PhraseQuery> pq = builder->build();
  // body:"just ? ? test"
  TestUtil::assertEquals(1, is->search(pq, 5)->totalHits);
  delete ir;
  delete dir;
}

TestIndexWriter::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<TestIndexWriter> outerInstance, shared_ptr<Automaton> secondSet)
{
  this->outerInstance = outerInstance;
  this->secondSet = secondSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenFilter>(tokenizer, MockTokenFilter::ENGLISH_STOPSET);
  stream = make_shared<MockTokenFilter>(
      stream, make_shared<CharacterRunAutomaton>(secondSet));
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestIndexWriter::testCommitWithUserDataOnly() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  writer->commit(); // first commit to complete IW create transaction.

  // this should store the commit data, even though no other changes were made
  writer->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass>(shared_from_this()))
          ->entrySet());
  writer->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(L"value", r->getIndexCommit()->getUserData()[L"key"]);
  r->close();

  // now check setCommitData and prepareCommit/commit sequence
  writer->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass2>(shared_from_this()))
          ->entrySet());
  writer->prepareCommit();
  writer->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass3>(shared_from_this()))
          ->entrySet());
  writer->commit(); // should commit the first commitData only, per protocol

  r = DirectoryReader::open(dir);
  TestUtil::assertEquals(L"value1", r->getIndexCommit()->getUserData()[L"key"]);
  r->close();

  // now should commit the second commitData - there was a bug where
  // IndexWriter.finishCommit overrode the second commitData
  writer->commit();
  r = DirectoryReader::open(dir);
  assertEquals(
      L"IndexWriter.finishCommit may have overridden the second commitData",
      L"value2", r->getIndexCommit()->getUserData()[L"key"]);
  r->close();

  delete writer;
  delete dir;
}

TestIndexWriter::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"key", L"value");
}

TestIndexWriter::HashMapAnonymousInnerClass2::HashMapAnonymousInnerClass2(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"key", L"value1");
}

TestIndexWriter::HashMapAnonymousInnerClass3::HashMapAnonymousInnerClass3(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"key", L"value2");
}

unordered_map<wstring, wstring>
TestIndexWriter::getLiveCommitData(shared_ptr<IndexWriter> writer)
{
  unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
  deque<unordered_map::Entry<wstring, wstring>> iter =
      writer->getLiveCommitData();
  if (iter.size() > 0) {
    for (auto ent : iter) {
      data.emplace(ent.first, ent.second);
    }
  }
  return data;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetCommitData() throws Exception
void TestIndexWriter::testGetCommitData() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  writer->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass4>(shared_from_this()))
          ->entrySet());
  TestUtil::assertEquals(L"value", getLiveCommitData(writer)[L"key"]);
  delete writer;

  // validate that it's also visible when opening a new IndexWriter
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(nullptr)->setOpenMode(OpenMode::APPEND));
  TestUtil::assertEquals(L"value", getLiveCommitData(writer)[L"key"]);
  delete writer;

  delete dir;
}

TestIndexWriter::HashMapAnonymousInnerClass4::HashMapAnonymousInnerClass4(
    shared_ptr<TestIndexWriter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"key", L"value");
}

void TestIndexWriter::testNullAnalyzer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf = newIndexWriterConfig(nullptr);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConf);

  // add 3 good docs
  for (int i = 0; i < 3; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    iw->addDocument(doc);
  }

  // add broken doc
  expectThrows(NullPointerException::typeid, [&]() {
    shared_ptr<Document> broke = make_shared<Document>();
    broke->add(newTextField(L"test", L"broken", Field::Store::NO));
    iw->addDocument(broke);
  });

  // ensure good docs are still ok
  shared_ptr<IndexReader> ir = iw->getReader();
  TestUtil::assertEquals(3, ir->numDocs());
  delete ir;
  delete iw;
  delete dir;
}

void TestIndexWriter::testNullDocument() 
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
  expectThrows(NullPointerException::typeid,
               [&]() { iw->addDocument(nullptr); });

  // ensure good docs are still ok
  shared_ptr<IndexReader> ir = iw->getReader();
  TestUtil::assertEquals(3, ir->numDocs());

  delete ir;
  delete iw;
  delete dir;
}

void TestIndexWriter::testNullDocuments() 
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

  // add broken doc block
  expectThrows(NullPointerException::typeid,
               [&]() { iw->addDocuments(nullptr); });

  // ensure good docs are still ok
  shared_ptr<IndexReader> ir = iw->getReader();
  TestUtil::assertEquals(3, ir->numDocs());

  delete ir;
  delete iw;
  delete dir;
}

void TestIndexWriter::testIterableFieldThrowsException() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  int iters = atLeast(100);
  int docCount = 0;
  int docId = 0;
  shared_ptr<Set<wstring>> liveIds = unordered_set<wstring>();
  for (int i = 0; i < iters; i++) {
    int numDocs = atLeast(4);
    for (int j = 0; j < numDocs; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring id = Integer::toString(docId++);
      const deque<std::shared_ptr<IndexableField>> fields =
          deque<std::shared_ptr<IndexableField>>();
      fields.push_back(make_shared<StringField>(L"id", id, Field::Store::YES));
      fields.push_back(make_shared<StringField>(
          L"foo", TestUtil::randomSimpleString(random()), Field::Store::NO));
      docId++;

      bool success = false;
      try {
        w->addDocument(
            make_shared<RandomFailingIterable<std::shared_ptr<IndexableField>>>(
                fields, random()));
        success = true;
      } catch (const runtime_error &e) {
        TestUtil::assertEquals(L"boom", e.what());
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (success) {
          docCount++;
          liveIds->add(id);
        }
      }
    }
  }
  shared_ptr<DirectoryReader> reader = w->getReader();
  TestUtil::assertEquals(docCount, reader->numDocs());
  deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  for (auto leafReaderContext : leaves) {
    shared_ptr<LeafReader> ar = leafReaderContext->reader();
    shared_ptr<Bits> liveDocs = ar->getLiveDocs();
    int maxDoc = ar->maxDoc();
    for (int i = 0; i < maxDoc; i++) {
      if (liveDocs == nullptr || liveDocs->get(i)) {
        assertTrue(liveIds->remove(ar->document(i)[L"id"]));
      }
    }
  }
  assertTrue(liveIds->isEmpty());
  delete w;
  IOUtils::close({reader, dir});
}

void TestIndexWriter::testIterableThrowsException() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  int iters = atLeast(100);
  int docCount = 0;
  int docId = 0;
  shared_ptr<Set<wstring>> liveIds = unordered_set<wstring>();
  for (int i = 0; i < iters; i++) {
    int numDocs = atLeast(4);
    for (int j = 0; j < numDocs; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring id = Integer::toString(docId++);
      const deque<std::shared_ptr<IndexableField>> fields =
          deque<std::shared_ptr<IndexableField>>();
      fields.push_back(make_shared<StringField>(L"id", id, Field::Store::YES));
      fields.push_back(make_shared<StringField>(
          L"foo", TestUtil::randomSimpleString(random()), Field::Store::NO));
      docId++;

      bool success = false;
      try {
        w->addDocument(
            make_shared<RandomFailingIterable<std::shared_ptr<IndexableField>>>(
                fields, random()));
        success = true;
      } catch (const runtime_error &e) {
        TestUtil::assertEquals(L"boom", e.what());
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (success) {
          docCount++;
          liveIds->add(id);
        }
      }
    }
  }
  shared_ptr<DirectoryReader> reader = w->getReader();
  TestUtil::assertEquals(docCount, reader->numDocs());
  deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  for (auto leafReaderContext : leaves) {
    shared_ptr<LeafReader> ar = leafReaderContext->reader();
    shared_ptr<Bits> liveDocs = ar->getLiveDocs();
    int maxDoc = ar->maxDoc();
    for (int i = 0; i < maxDoc; i++) {
      if (liveDocs == nullptr || liveDocs->get(i)) {
        assertTrue(liveIds->remove(ar->document(i)[L"id"]));
      }
    }
  }
  assertTrue(liveIds->isEmpty());
  delete w;
  IOUtils::close({reader, dir});
}

void TestIndexWriter::testIterableThrowsException2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  // C++ TODO: The following anonymous inner class could not be converted:
  //      Exception expected = expectThrows(Exception.class, () => {
  //      w.addDocuments(new Iterable<org.apache.lucene.document.Document>()
  //    {
  //        @@Override public Iterator<Document> iterator()
  //        {
  //          return new Iterator<Document>()
  //          {
  //
  //            @@Override public bool hasNext()
  //            {
  //              return true;
  //            }
  //
  //            @@Override public Document next()
  //            {
  //              throw new RuntimeException("boom");
  //            }
  //
  //            @@Override public void remove()
  //            {
  //                assert false;
  //            }
  //          };
  //        }
  //      }
       );
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: public IteratorAnonymousInnerClass(RandomFailingIterable<T>
// outerInstance, std::deque<? extends T>::const_iterator docIter)
    RandomFailingIterable<T>::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(shared_ptr<RandomFailingIterable<std::shared_ptr<T>>> outerInstance, deque<? extends T>::const_iterator docIter)
    {
      this->outerInstance = outerInstance;
      this->docIter = docIter;
      count = 0;
    }

    bool RandomFailingIterable<T>::IteratorAnonymousInnerClass::hasNext()
    {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      return docIter.hasNext();
    }

    shared_ptr<T> RandomFailingIterable<T>::IteratorAnonymousInnerClass::next()
    {
      if (count == outerInstance->failOn) {
        throw runtime_error(L"boom");
      }
      count++;
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      return docIter.next();
    }

    void RandomFailingIterable<T>::IteratorAnonymousInnerClass::remove()
    {
      throw make_shared<UnsupportedOperationException>();
    }

    void<missing_class_definition>::testCorruptFirstCommit() throw(
        runtime_error)
    {
      for (int i = 0; i < 6; i++) {
        shared_ptr<BaseDirectoryWrapper> dir = LuceneTestCase::newDirectory();

        // Create a corrupt first commit:
        delete dir->createOutput(IndexFileNames::fileNameFromGeneration(
                                     IndexFileNames::PENDING_SEGMENTS, L"", 0),
                                 IOContext::DEFAULT);

        shared_ptr<IndexWriterConfig> iwc =
            LuceneTestCase::newIndexWriterConfig(
                make_shared<MockAnalyzer>(LuceneTestCase::random()));
        int mode = i / 2;
        if (mode == 0) {
          iwc->setOpenMode(OpenMode::CREATE);
        } else if (mode == 1) {
          iwc->setOpenMode(OpenMode::APPEND);
        } else if (mode == 2) {
          iwc->setOpenMode(OpenMode::CREATE_OR_APPEND);
        }

        if (LuceneTestCase::VERBOSE) {
          wcout << L"\nTEST: i=" << i << endl;
        }

        try {
          if ((i & 1) == 0) {
            delete (make_shared<IndexWriter>(dir, iwc));
          } else {
            (make_shared<IndexWriter>(dir, iwc))->rollback();
          }
        } catch (const IOException &ioe) {
          // OpenMode.APPEND should throw an exception since no
          // index exists:
          if (mode == 0) {
            // Unexpected
            throw ioe;
          }
        }

        if (LuceneTestCase::VERBOSE) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"  at close: " << Arrays->toString(dir->listAll()) << endl;
        }

        if (mode != 0) {
          dir->setCheckIndexOnClose(false);
        }

        delete dir;
      }
    }

    void<missing_class_definition>::testHasUncommittedChanges() throw(
        IOException)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, LuceneTestCase::newIndexWriterConfig(
                   make_shared<MockAnalyzer>(LuceneTestCase::random())));
      assertTrue(
          writer->hasUncommittedChanges()); // this will be true because a
                                            // commit will create an empty index
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          LuceneTestCase::newTextField(L"myfield", L"a b c", Field::Store::NO));
      writer->addDocument(doc);
      assertTrue(writer->hasUncommittedChanges());

      // Must commit, waitForMerges, commit again, to be
      // certain that hasUncommittedChanges returns false:
      writer->commit();
      writer->waitForMerges();
      writer->commit();
      assertFalse(writer->hasUncommittedChanges());
      writer->addDocument(doc);
      assertTrue(writer->hasUncommittedChanges());
      writer->commit();
      doc = make_shared<Document>();
      doc->push_back(
          LuceneTestCase::newStringField(L"id", L"xyz", Field::Store::YES));
      writer->addDocument(doc);
      assertTrue(writer->hasUncommittedChanges());

      // Must commit, waitForMerges, commit again, to be
      // certain that hasUncommittedChanges returns false:
      writer->commit();
      writer->waitForMerges();
      writer->commit();
      assertFalse(writer->hasUncommittedChanges());
      writer->deleteDocuments({make_shared<Term>(L"id", L"xyz")});
      assertTrue(writer->hasUncommittedChanges());

      // Must commit, waitForMerges, commit again, to be
      // certain that hasUncommittedChanges returns false:
      writer->commit();
      writer->waitForMerges();
      writer->commit();
      assertFalse(writer->hasUncommittedChanges());
      delete writer;

      writer = make_shared<IndexWriter>(
          dir, LuceneTestCase::newIndexWriterConfig(
                   make_shared<MockAnalyzer>(LuceneTestCase::random())));
      assertFalse(writer->hasUncommittedChanges());
      writer->addDocument(doc);
      assertTrue(writer->hasUncommittedChanges());

      delete writer;
      delete dir;
    }

    void<missing_class_definition>::testMergeAllDeleted() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = LuceneTestCase::newIndexWriterConfig(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<AtomicBoolean> keepFullyDeletedSegments =
          make_shared<AtomicBoolean>();
      iwc->setMergePolicy(make_shared<FilterMergePolicyAnonymousInnerClass>(
          shared_from_this(), iwc->getMergePolicy(), keepFullyDeletedSegments));
      shared_ptr<SetOnce<std::shared_ptr<IndexWriter>>> *const iwRef =
          make_shared<SetOnce<std::shared_ptr<IndexWriter>>>();
      shared_ptr<IndexWriter> evilWriter = RandomIndexWriter::mockIndexWriter(
          LuceneTestCase::random(), dir, iwc,
          make_shared<TestPointAnonymousInnerClass>(shared_from_this(),
                                                    keepFullyDeletedSegments));
      iwRef->set(evilWriter);
      for (int i = 0; i < 1000; i++) {
        addDoc(evilWriter);
        if (LuceneTestCase::random()->nextInt(17) == 0) {
          evilWriter->commit();
        }
      }
      evilWriter->deleteDocuments({make_shared<MatchAllDocsQuery>()});
      evilWriter->forceMerge(1);
      delete evilWriter;
      delete dir;
    }

    FilterMergePolicyAnonymousInnerClass::FilterMergePolicyAnonymousInnerClass(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<org::apache::lucene::index::MergePolicy> getMergePolicy,
        shared_ptr<AtomicBoolean> keepFullyDeletedSegments)
        : FilterMergePolicy(getMergePolicy)
    {
      this->outerInstance = outerInstance;
      this->keepFullyDeletedSegments = keepFullyDeletedSegments;
    }

    bool FilterMergePolicyAnonymousInnerClass::keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>>
            readerIOSupplier) 
    {
      return keepFullyDeletedSegments->get();
    }

    TestPointAnonymousInnerClass::TestPointAnonymousInnerClass(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<AtomicBoolean> keepFullyDeletedSegments)
    {
      this->outerInstance = outerInstance;
      this->keepFullyDeletedSegments = keepFullyDeletedSegments;
    }

    void TestPointAnonymousInnerClass::apply(const wstring &message)
    {
      if (L"startCommitMerge" == message) {
        keepFullyDeletedSegments->set(false);
      } else if (L"startMergeInit" == message) {
        keepFullyDeletedSegments->set(true);
      }
    }

    void<missing_class_definition>::testDeleteSameTermAcrossFields() throw(
        runtime_error)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<TextField>(L"a", L"foo", Field::Store::NO));
      w->addDocument(doc);

      // Should not delete the document; with LUCENE-5239 the
      // "foo" from the 2nd delete term would incorrectly
      // match field a's "foo":
      w->deleteDocuments({make_shared<Term>(L"a", L"xxx")});
      w->deleteDocuments({make_shared<Term>(L"b", L"foo")});
      shared_ptr<IndexReader> r = w->getReader();
      delete w;

      // Make sure document was not (incorrectly) deleted:
      TestUtil::assertEquals(1, r->numDocs());
      delete r;
      delete dir;
    }

    void<missing_class_definition>::
        testHasUncommittedChangesAfterException() 
    {
      shared_ptr<Analyzer> analyzer =
          make_shared<MockAnalyzer>(LuceneTestCase::random());

      shared_ptr<Directory> directory = LuceneTestCase::newDirectory();
      // we don't use RandomIndexWriter because it might add more docvalues than
      // we expect !!!!
      shared_ptr<IndexWriterConfig> iwc =
          LuceneTestCase::newIndexWriterConfig(analyzer);
      iwc->setMergePolicy(LuceneTestCase::newLogMergePolicy());
      shared_ptr<IndexWriter> iwriter =
          make_shared<IndexWriter>(directory, iwc);
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv", make_shared<BytesRef>(L"foo!")));
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv", make_shared<BytesRef>(L"bar!")));
      LuceneTestCase::expectThrows(invalid_argument::typeid,
                                   [&]() { iwriter->addDocument(doc); });

      iwriter->commit();
      assertFalse(iwriter->hasUncommittedChanges());
      delete iwriter;
      delete directory;
    }

    void<missing_class_definition>::testDoubleClose() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, LuceneTestCase::newIndexWriterConfig(
                   make_shared<MockAnalyzer>(LuceneTestCase::random())));
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv", make_shared<BytesRef>(L"foo!")));
      w->addDocument(doc);
      delete w;
      // Close again should have no effect
      delete w;
      delete dir;
    }

    void<missing_class_definition>::testRollbackThenClose() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, LuceneTestCase::newIndexWriterConfig(
                   make_shared<MockAnalyzer>(LuceneTestCase::random())));
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv", make_shared<BytesRef>(L"foo!")));
      w->addDocument(doc);
      w->rollback();
      // Close after rollback should have no effect
      delete w;
      delete dir;
    }

    void<missing_class_definition>::testCloseThenRollback() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, LuceneTestCase::newIndexWriterConfig(
                   make_shared<MockAnalyzer>(LuceneTestCase::random())));
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv", make_shared<BytesRef>(L"foo!")));
      w->addDocument(doc);
      delete w;
      // Rollback after close should have no effect
      w->rollback();
      delete dir;
    }

    void<missing_class_definition>::testCloseWhileMergeIsRunning() throw(
        IOException)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();

      shared_ptr<CountDownLatch> *const mergeStarted =
          make_shared<CountDownLatch>(1);
      shared_ptr<CountDownLatch> *const closeStarted =
          make_shared<CountDownLatch>(1);

      shared_ptr<IndexWriterConfig> iwc =
          LuceneTestCase::newIndexWriterConfig(
              LuceneTestCase::random(),
              make_shared<MockAnalyzer>(LuceneTestCase::random()))
              ->setCommitOnClose(false);
      shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
      mp->setMergeFactor(2);
      iwc->setMergePolicy(mp);
      iwc->setInfoStream(make_shared<InfoStreamAnonymousInnerClass>(
          shared_from_this(), closeStarted));

      iwc->setMergeScheduler(
          make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>(
              shared_from_this(), mergeStarted, closeStarted));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv", make_shared<BytesRef>(L"foo!")));
      w->addDocument(doc);
      w->commit();
      w->addDocument(doc);
      w->commit();
      delete w;
      delete dir;
    }

    InfoStreamAnonymousInnerClass::InfoStreamAnonymousInnerClass(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<CountDownLatch> closeStarted)
    {
      this->outerInstance = outerInstance;
      this->closeStarted = closeStarted;
    }

    bool InfoStreamAnonymousInnerClass::isEnabled(const wstring &component)
    {
      return true;
    }

    void InfoStreamAnonymousInnerClass::message(const wstring &component,
                                                const wstring &message)
    {
      if (message == L"rollback") {
        closeStarted->countDown();
      }
    }

    InfoStreamAnonymousInnerClass::~InfoStreamAnonymousInnerClass() {}

    ConcurrentMergeSchedulerAnonymousInnerClass::
        ConcurrentMergeSchedulerAnonymousInnerClass(
            shared_ptr<MissingClass> outerInstance,
            shared_ptr<CountDownLatch> mergeStarted,
            shared_ptr<CountDownLatch> closeStarted)
    {
      this->outerInstance = outerInstance;
      this->mergeStarted = mergeStarted;
      this->closeStarted = closeStarted;
    }

    void ConcurrentMergeSchedulerAnonymousInnerClass::doMerge(
        shared_ptr<IndexWriter> writer,
        shared_ptr<MergePolicy::OneMerge> merge) 
    {
      mergeStarted->countDown();
      try {
        closeStarted->await();
      } catch (const InterruptedException &ie) {
        Thread::currentThread().interrupt();
        throw runtime_error(ie);
      }
      ConcurrentMergeScheduler::doMerge(writer, merge);
    }

    ConcurrentMergeSchedulerAnonymousInnerClass::
        ~ConcurrentMergeSchedulerAnonymousInnerClass()
    {
    }

    void<missing_class_definition>::testCloseDuringCommit() 
    {

      shared_ptr<CountDownLatch> *const startCommit =
          make_shared<CountDownLatch>(1);
      shared_ptr<CountDownLatch> *const finishCommit =
          make_shared<CountDownLatch>(1);

      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc =
          make_shared<IndexWriterConfig>(nullptr);
      // use an infostream that "takes a long time" to commit
      shared_ptr<IndexWriter> *const iw = RandomIndexWriter::mockIndexWriter(
          LuceneTestCase::random(), dir, iwc,
          make_shared<TestPointAnonymousInnerClass2>(shared_from_this(),
                                                     startCommit));
      make_shared<ThreadAnonymousInnerClass>(shared_from_this(), finishCommit,
                                             iw)
          .start();
      startCommit->await();
      try {
        delete iw;
      } catch (const IllegalStateException &ise) {
        // OK, but not required (depends on thread scheduling)
      }
      finishCommit->await();
      delete iw;
      delete dir;
    }

    TestPointAnonymousInnerClass2::TestPointAnonymousInnerClass2(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<CountDownLatch> startCommit)
    {
      this->outerInstance = outerInstance;
      this->startCommit = startCommit;
    }

    void TestPointAnonymousInnerClass2::apply(const wstring &message)
    {
      if (message == L"finishStartCommit") {
        startCommit->countDown();
        try {
          delay(10);
        } catch (const InterruptedException &ie) {
          throw make_shared<ThreadInterruptedException>(ie);
        }
      }
    }

    ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<CountDownLatch> finishCommit,
        shared_ptr<org::apache::lucene::index::IndexWriter> iw)
    {
      this->outerInstance = outerInstance;
      this->finishCommit = finishCommit;
      this->iw = iw;
    }

    void ThreadAnonymousInnerClass::run()
    {
      try {
        iw->commit();
        finishCommit->countDown();
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }
    }

    void<missing_class_definition>::testIds() 
    {
      shared_ptr<Directory> d = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          d, LuceneTestCase::newIndexWriterConfig(
                 make_shared<MockAnalyzer>(LuceneTestCase::random())));
      w->addDocument(make_shared<Document>());
      delete w;

      shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(d);
      std::deque<char> id1 = sis->getId();
      assertNotNull(id1);
      TestUtil::assertEquals(StringHelper::ID_LENGTH, id1.size());

      std::deque<char> id2 = sis->info(0)->info->getId();
      assertNotNull(id2);
      TestUtil::assertEquals(StringHelper::ID_LENGTH, id2.size());

      // Make sure CheckIndex includes id output:
      shared_ptr<ByteArrayOutputStream> bos =
          make_shared<ByteArrayOutputStream>(1024);
      shared_ptr<CheckIndex> checker = make_shared<CheckIndex>(d);
      checker->setCrossCheckTermVectors(false);
      checker->setInfoStream(
          make_shared<PrintStream>(bos, false, IOUtils::UTF_8), false);
      shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex(nullptr);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring s = bos->toString(IOUtils::UTF_8);
      delete checker;
      // Make sure CheckIndex didn't fail
      assertTrue(s, indexStatus != nullptr && indexStatus->clean);

      // Commit id is always stored:
      assertTrue(L"missing id=" + StringHelper::idToString(id1) + L" in:\n" + s,
                 s.find(L"id=" + StringHelper::idToString(id1)) !=
                     wstring::npos);

      assertTrue(L"missing id=" + StringHelper::idToString(id1) + L" in:\n" + s,
                 s.find(L"id=" + StringHelper::idToString(id1)) !=
                     wstring::npos);
      delete d;

      shared_ptr<Set<wstring>> ids = unordered_set<wstring>();
      for (int i = 0; i < 100000; i++) {
        wstring id = StringHelper::idToString(StringHelper::randomId());
        assertFalse(L"id=" + id + L" i=" + to_wstring(i), ids->contains(id));
        ids->add(id);
      }
    }

    void<missing_class_definition>::testEmptyNorm() 
    {
      shared_ptr<Directory> d = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          d, LuceneTestCase::newIndexWriterConfig(
                 make_shared<MockAnalyzer>(LuceneTestCase::random())));
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          make_shared<TextField>(L"foo", make_shared<CannedTokenStream>()));
      w->addDocument(doc);
      w->commit();
      delete w;
      shared_ptr<DirectoryReader> r = DirectoryReader::open(d);
      shared_ptr<NumericDocValues> norms =
          LuceneTestCase::getOnlyLeafReader(r)->getNormValues(L"foo");
      TestUtil::assertEquals(0, norms->nextDoc());
      TestUtil::assertEquals(0, norms->longValue());
      r->close();
      delete d;
    }

    void<missing_class_definition>::testManySeparateThreads() throw(
        runtime_error)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      iwc->setMaxBufferedDocs(1000);
      shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
      // Index 100 docs, each from a new thread, but always only 1 thread is in
      // IW at once:
      for (int i = 0; i < 100; i++) {
        shared_ptr<Thread> thread =
            make_shared<ThreadAnonymousInnerClass2>(shared_from_this(), w);
        thread->start();
        thread->join();
      }
      delete w;

      shared_ptr<IndexReader> r = DirectoryReader::open(dir);
      TestUtil::assertEquals(1, r->leaves().size());
      delete r;
      delete dir;
    }

    ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<org::apache::lucene::index::IndexWriter> w)
    {
      this->outerInstance = outerInstance;
      this->w = w;
    }

    void ThreadAnonymousInnerClass2::run()
    {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          LuceneTestCase::newStringField(L"foo", L"bar", Field::Store::NO));
      try {
        w->addDocument(doc);
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }
    }

    void<missing_class_definition>::testNRTSegmentsFile() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      // creates segments_1
      w->commit();

      // newly opened NRT reader should see gen=1 segments file
      shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
      TestUtil::assertEquals(1, r->getIndexCommit()->getGeneration());
      TestUtil::assertEquals(L"segments_1",
                             r->getIndexCommit()->getSegmentsFileName());

      // newly opened non-NRT reader should see gen=1 segments file
      shared_ptr<DirectoryReader> r2 = DirectoryReader::open(dir);
      TestUtil::assertEquals(1, r2->getIndexCommit()->getGeneration());
      TestUtil::assertEquals(L"segments_1",
                             r2->getIndexCommit()->getSegmentsFileName());
      r2->close();

      // make a change and another commit
      w->addDocument(make_shared<Document>());
      w->commit();
      shared_ptr<DirectoryReader> r3 = DirectoryReader::openIfChanged(r);
      r->close();
      assertNotNull(r3);

      // reopened NRT reader should see gen=2 segments file
      TestUtil::assertEquals(2, r3->getIndexCommit()->getGeneration());
      TestUtil::assertEquals(L"segments_2",
                             r3->getIndexCommit()->getSegmentsFileName());
      r3->close();

      // newly opened non-NRT reader should see gen=2 segments file
      shared_ptr<DirectoryReader> r4 = DirectoryReader::open(dir);
      TestUtil::assertEquals(2, r4->getIndexCommit()->getGeneration());
      TestUtil::assertEquals(L"segments_2",
                             r4->getIndexCommit()->getSegmentsFileName());
      r4->close();

      delete w;
      delete dir;
    }

    void<missing_class_definition>::testNRTAfterCommit() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      w->commit();

      w->addDocument(make_shared<Document>());
      shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
      w->commit();

      // commit even with no other changes counts as a "change" that NRT reader
      // reopen will see:
      shared_ptr<DirectoryReader> r2 = DirectoryReader::open(dir);
      assertNotNull(r2);
      TestUtil::assertEquals(2, r2->getIndexCommit()->getGeneration());
      TestUtil::assertEquals(L"segments_2",
                             r2->getIndexCommit()->getSegmentsFileName());

      IOUtils::close({r, r2, w, dir});
    }

    void<missing_class_definition>::
        testNRTAfterSetUserDataWithoutCommit() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      w->commit();

      shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
      unordered_map<wstring, wstring> m = unordered_map<wstring, wstring>();
      m.emplace(L"foo", L"bar");
      w->setLiveCommitData(m.entrySet());

      // setLiveCommitData with no other changes should count as an NRT change:
      shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
      assertNotNull(r2);

      IOUtils::close({r2, r, w, dir});
    }

    void<missing_class_definition>::testNRTAfterSetUserDataWithCommit() throw(
        runtime_error)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      w->commit();

      shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
      unordered_map<wstring, wstring> m = unordered_map<wstring, wstring>();
      m.emplace(L"foo", L"bar");
      w->setLiveCommitData(m.entrySet());
      w->commit();
      // setLiveCommitData and also commit, with no other changes, should count
      // as an NRT change:
      shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
      assertNotNull(r2);
      IOUtils::close({r, r2, w, dir});
    }

    void<missing_class_definition>::testCommitImmediatelyAfterNRTReopen() throw(
        runtime_error)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      w->commit();

      w->addDocument(make_shared<Document>());

      shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
      w->commit();

      assertFalse(r->isCurrent());

      shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
      assertNotNull(r2);
      // segments_N should have changed:
      assertFalse(r2->getIndexCommit()->getSegmentsFileName() ==
                  r->getIndexCommit()->getSegmentsFileName());
      IOUtils::close({r, r2, w, dir});
    }

    void<missing_class_definition>::testPendingDeleteDVGeneration() throw(
        IOException)
    {
      // irony: currently we don't emulate windows well enough to work on
      // windows!
      LuceneTestCase::assumeFalse(L"windows is not supported",
                                  Constants::WINDOWS);

      shared_ptr<Path> path = LuceneTestCase::createTempDir();

      // Use WindowsFS to prevent open files from being deleted:
      shared_ptr<FileSystem> fs =
          (make_shared<WindowsFS>(path->getFileSystem()))
              ->getFileSystem(URI::create(L"file:///"));
      shared_ptr<Path> root = make_shared<FilterPath>(path, fs);

      // MMapDirectory doesn't work because it closes its file handles after
      // mapping!
      deque<std::shared_ptr<Closeable>> toClose =
          deque<std::shared_ptr<Closeable>>();
      {
        org::apache::lucene::store::FSDirectory dir =
            org::apache::lucene::store::SimpleFSDirectory(root);

        java::io::Closeable closeable = [&]() {
          org::apache::lucene::util::IOUtils::close(toClose);
        };
        shared_ptr<IndexWriterConfig> iwc =
            (make_shared<IndexWriterConfig>(
                 make_shared<MockAnalyzer>(LuceneTestCase::random())))
                ->setUseCompoundFile(false)
                ->setMaxBufferedDocs(2)
                ->setRAMBufferSizeMB(-1);
        shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
        shared_ptr<Document> d = make_shared<Document>();
        d->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
        d->push_back(make_shared<NumericDocValuesField>(L"id", 1));
        w->addDocument(d);
        d = make_shared<Document>();
        d->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
        d->push_back(make_shared<NumericDocValuesField>(L"id", 2));
        w->addDocument(d);
        w->flush();
        d = make_shared<Document>();
        d->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
        d->push_back(make_shared<NumericDocValuesField>(L"id", 1));
        w->updateDocument(make_shared<Term>(L"id", L"1"), d);
        w->commit();
        shared_ptr<Set<wstring>> files =
            unordered_set<wstring>(Arrays::asList(dir->listAll()));
        int numIters = 10 + LuceneTestCase::random()->nextInt(50);
        for (int i = 0; i < numIters; i++) {
          if (LuceneTestCase::random()->nextBoolean()) {
            d = make_shared<Document>();
            d->push_back(
                make_shared<StringField>(L"id", L"1", Field::Store::YES));
            d->push_back(make_shared<NumericDocValuesField>(L"id", 1));
            w->updateDocument(make_shared<Term>(L"id", L"1"), d);
          } else if (LuceneTestCase::random()->nextBoolean()) {
            w->deleteDocuments({make_shared<Term>(L"id", L"2")});
          } else {
            w->updateNumericDocValue(make_shared<Term>(L"id", L"1"), L"id", 2);
          }
          w->prepareCommit();
          deque<wstring> newFiles =
              deque<wstring>(Arrays::asList(dir->listAll()));
          newFiles.removeAll(files);
          wstring randomFile =
              RandomPicks::randomFrom(LuceneTestCase::random(), newFiles);
          toClose->add(dir->openInput(randomFile, IOContext::DEFAULT));
          w->rollback();
          iwc = (make_shared<IndexWriterConfig>(
                     make_shared<MockAnalyzer>(LuceneTestCase::random())))
                    ->setUseCompoundFile(false)
                    ->setMaxBufferedDocs(2)
                    ->setRAMBufferSizeMB(-1);
          w = make_shared<IndexWriter>(dir, iwc);
          LuceneTestCase::expectThrows(NoSuchFileException::typeid,
                                       [&]() { dir->deleteFile(randomFile); });
        }
        delete w;
      }
    }

    void<missing_class_definition>::testWithPendingDeletions() throw(
        runtime_error)
    {
      // irony: currently we don't emulate windows well enough to work on
      // windows!
      LuceneTestCase::assumeFalse(L"windows is not supported",
                                  Constants::WINDOWS);

      shared_ptr<Path> path = LuceneTestCase::createTempDir();

      // Use WindowsFS to prevent open files from being deleted:
      shared_ptr<FileSystem> fs =
          (make_shared<WindowsFS>(path->getFileSystem()))
              ->getFileSystem(URI::create(L"file:///"));
      shared_ptr<Path> root = make_shared<FilterPath>(path, fs);
      shared_ptr<IndexCommit> indexCommit;
      shared_ptr<DirectoryReader> reader;
      // MMapDirectory doesn't work because it closes its file handles after
      // mapping!
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.FSDirectory
      // dir = new org.apache.lucene.store.SimpleFSDirectory(root))
      {
        org::apache::lucene::store::FSDirectory dir =
            org::apache::lucene::store::SimpleFSDirectory(root);
        shared_ptr<IndexWriterConfig> iwc =
            (make_shared<IndexWriterConfig>(
                 make_shared<MockAnalyzer>(LuceneTestCase::random())))
                ->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE);
        shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
        w->commit();
        reader = w->getReader();
        // we pull this commit to open it again later to check that we fail if a
        // futur file delete is pending
        indexCommit = reader->getIndexCommit();
        delete w;
        w = make_shared<IndexWriter>(
            dir, (make_shared<IndexWriterConfig>(
                      make_shared<MockAnalyzer>(LuceneTestCase::random())))
                     ->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE));
        w->addDocument(make_shared<Document>());
        delete w;
        shared_ptr<IndexInput> in_ =
            dir->openInput(L"segments_2", IOContext::DEFAULT);
        dir->deleteFile(L"segments_2");
        assertTrue(dir->getPendingDeletions()->size() > 0);

        // make sure we get NFSF if we try to delete and already-pending-delete
        // file:
        LuceneTestCase::expectThrows(NoSuchFileException::typeid,
                                     [&]() { dir->deleteFile(L"segments_2"); });

        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try (IndexWriter writer = new
        // IndexWriter(dir, new IndexWriterConfig(new
        // org.apache.lucene.analysis.MockAnalyzer(random())).setIndexCommit(indexCommit)))
        {
          IndexWriter writer = IndexWriter(
              dir,
              (make_shared<IndexWriterConfig>(
                   make_shared<org::apache::lucene::analysis::MockAnalyzer>(
                       LuceneTestCase::random())))
                  ->setIndexCommit(indexCommit));
          writer->addDocument(make_shared<Document>());
          writer->commit();
          TestUtil::assertEquals(1, writer->maxDoc());
          // now check that we moved to 3
          delete dir->openInput(L"segments_3", IOContext::READ);
        }
        reader->close();
        delete in_;
      }
    }

    void<missing_class_definition>::
        testPendingDeletesAlreadyWrittenFiles() 
    {
      shared_ptr<Path> path = LuceneTestCase::createTempDir();
      // irony: currently we don't emulate windows well enough to work on
      // windows!
      LuceneTestCase::assumeFalse(L"windows is not supported",
                                  Constants::WINDOWS);

      // Use WindowsFS to prevent open files from being deleted:
      shared_ptr<FileSystem> fs =
          (make_shared<WindowsFS>(path->getFileSystem()))
              ->getFileSystem(URI::create(L"file:///"));
      shared_ptr<Path> root = make_shared<FilterPath>(path, fs);
      shared_ptr<DirectoryReader> reader;
      // MMapDirectory doesn't work because it closes its file handles after
      // mapping!
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.FSDirectory
      // dir = new org.apache.lucene.store.SimpleFSDirectory(root))
      {
        org::apache::lucene::store::FSDirectory dir =
            org::apache::lucene::store::SimpleFSDirectory(root);
        shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
            make_shared<MockAnalyzer>(LuceneTestCase::random()));
        shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
        w->commit();
        shared_ptr<IndexInput> in_ =
            dir->openInput(L"segments_1", IOContext::DEFAULT);
        w->addDocument(make_shared<Document>());
        delete w;

        assertTrue(dir->getPendingDeletions()->size() > 0);

        // make sure we get NFSF if we try to delete and already-pending-delete
        // file:
        LuceneTestCase::expectThrows(NoSuchFileException::typeid,
                                     [&]() { dir->deleteFile(L"segments_1"); });
        delete (make_shared<IndexWriter>(
            dir, make_shared<IndexWriterConfig>(
                     make_shared<MockAnalyzer>(LuceneTestCase::random()))));
        delete in_;
      }
    }

    void<missing_class_definition>::testLeftoverTempFiles() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
      delete w;

      shared_ptr<IndexOutput> out =
          dir->createTempOutput(L"_0", L"bkd", IOContext::DEFAULT);
      wstring tempName = out->getName();
      delete out;
      iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      w = make_shared<IndexWriter>(dir, iwc);

      // Make sure IW deleted the unref'd file:
      try {
        dir->openInput(tempName, IOContext::DEFAULT);
        fail(L"did not hit exception");
      }
      // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
      catch (FileNotFoundException | NoSuchFileException e) {
        // expected
      }
      delete w;
      delete dir;
    }

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Ignore("requires running tests with biggish heap") public
    // void testMassiveField() throws Exception
    void<missing_class_definition>::testMassiveField() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
          make_shared<MockAnalyzer>(LuceneTestCase::random()));
      shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

      shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
      while (b->length() <= IndexWriter::MAX_STORED_STRING_LENGTH) {
        b->append(L"x ");
      }

      shared_ptr<Document> *const doc = make_shared<Document>();
      // doc.add(new TextField("big", b.toString(), Field.Store.YES));
      doc->push_back(make_shared<StoredField>(L"big", b->toString()));
      runtime_error e = LuceneTestCase::expectThrows(
          invalid_argument::typeid, [&]() { w->addDocument(doc); });
      TestUtil::assertEquals(L"stored field \"big\" is too large (" +
                                 to_wstring(b->length()) +
                                 L" characters) to store",
                             e.what());

      // make sure writer is still usable:
      shared_ptr<Document> doc2 = make_shared<Document>();
      doc2->push_back(
          make_shared<StringField>(L"id", L"foo", Field::Store::YES));
      w->addDocument(doc2);

      shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
      TestUtil::assertEquals(1, r->numDocs());
      r->close();
      delete w;
      delete dir;
    }

    void<missing_class_definition>::testRecordsIndexCreatedVersion() throw(
        IOException)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w =
          make_shared<IndexWriter>(dir, LuceneTestCase::newIndexWriterConfig());
      w->commit();
      delete w;
      TestUtil::assertEquals(
          Version::LATEST->major,
          SegmentInfos::readLatestCommit(dir)->getIndexCreatedVersionMajor());
      delete dir;
    }

    void<missing_class_definition>::testFlushLargestWriter() throw(
        IOException, InterruptedException)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w =
          make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>());
      int numDocs = indexDocsForMultipleThreadStates(w);
      shared_ptr<DocumentsWriterPerThreadPool::ThreadState>
          largestNonPendingWriter =
              w->docWriter->flushControl->findLargestNonPendingWriter();
      assertFalse(largestNonPendingWriter->flushPending);
      assertNotNull(largestNonPendingWriter->dwpt);

      int numRamDocs = w->numRamDocs();
      int numDocsInDWPT = largestNonPendingWriter->dwpt->getNumDocsInRAM();
      assertTrue(w->flushNextBuffer());
      assertNull(largestNonPendingWriter->dwpt);
      TestUtil::assertEquals(numRamDocs - numDocsInDWPT, w->numRamDocs());

      // make sure it's not locked
      largestNonPendingWriter->lock();
      largestNonPendingWriter->unlock();
      if (LuceneTestCase::random()->nextBoolean()) {
        w->commit();
      }
      shared_ptr<DirectoryReader> reader = DirectoryReader::open(w, true, true);
      TestUtil::assertEquals(numDocs, reader->numDocs());
      reader->close();
      delete w;
      delete dir;
    }

    int<missing_class_definition>::indexDocsForMultipleThreadStates(
        shared_ptr<IndexWriter> w) 
    {
      std::deque<std::shared_ptr<Thread>> threads(3);
      shared_ptr<CountDownLatch> latch =
          make_shared<CountDownLatch>(threads.size());
      int numDocsPerThread = 10 + LuceneTestCase::random()->nextInt(30);
      // ensure we have more than on thread state
      for (int i = 0; i < threads.size(); i++) {
        threads[i] = make_shared<Thread>([&]() {
          latch->countDown();
          try {
            latch->await();
            for (int j = 0; j < numDocsPerThread; j++) {
              shared_ptr<Document> doc = make_shared<Document>();
              doc->add(
                  make_shared<StringField>(L"id", L"foo", Field::Store::YES));
              w->addDocument(doc);
            }
          } catch (const runtime_error &e) {
            throw make_shared<AssertionError>(e);
          }
        });
        threads[i]->start();
      }
      for (auto t : threads) {
        t->join();
      }
      return numDocsPerThread * threads.size();
    }

    void<missing_class_definition>::testNeverCheckOutOnFullFlush() throw(
        IOException, InterruptedException)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w =
          make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>());
      indexDocsForMultipleThreadStates(w);
      shared_ptr<DocumentsWriterPerThreadPool::ThreadState>
          largestNonPendingWriter =
              w->docWriter->flushControl->findLargestNonPendingWriter();
      assertFalse(largestNonPendingWriter->flushPending);
      assertNotNull(largestNonPendingWriter->dwpt);
      int activeThreadStateCount =
          w->docWriter->perThreadPool->getActiveThreadStateCount();
      w->docWriter->flushControl->markForFullFlush();
      shared_ptr<DocumentsWriterPerThread> documentsWriterPerThread =
          w->docWriter->flushControl->checkoutLargestNonPendingWriter();
      assertNull(documentsWriterPerThread);
      TestUtil::assertEquals(activeThreadStateCount,
                             w->docWriter->flushControl->numQueuedFlushes());
      w->docWriter->flushControl->abortFullFlushes();
      assertNull(L"was aborted",
                 w->docWriter->flushControl->checkoutLargestNonPendingWriter());
      TestUtil::assertEquals(0, w->docWriter->flushControl->numQueuedFlushes());
      delete w;
      delete dir;
    }

    void<missing_class_definition>::testHoldLockOnLargestWriter() throw(
        IOException, InterruptedException)
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> w =
          make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>());
      int numDocs = indexDocsForMultipleThreadStates(w);
      shared_ptr<DocumentsWriterPerThreadPool::ThreadState>
          largestNonPendingWriter =
              w->docWriter->flushControl->findLargestNonPendingWriter();
      assertFalse(largestNonPendingWriter->flushPending);
      assertNotNull(largestNonPendingWriter->dwpt);

      shared_ptr<CountDownLatch> wait = make_shared<CountDownLatch>(1);
      shared_ptr<CountDownLatch> locked = make_shared<CountDownLatch>(1);
      shared_ptr<Thread> lockThread = make_shared<Thread>([&]() {
        try {
          largestNonPendingWriter->lock();
          locked->countDown();
          wait->await();
        } catch (const InterruptedException &e) {
          throw make_shared<AssertionError>(e);
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          largestNonPendingWriter->unlock();
        }
      });
      lockThread->start();
      shared_ptr<Thread> flushThread = make_shared<Thread>([&]() {
        try {
          locked->await();
          assertTrue(w->flushNextBuffer());
        } catch (const runtime_error &e) {
          throw make_shared<AssertionError>(e);
        }
      });
      flushThread->start();

      locked->await();
      // access a synced method to ensure we never lock while we hold the flush
      // control monitor
      w->docWriter->flushControl->activeBytes();
      wait->countDown();
      lockThread->join();
      flushThread->join();

      assertNull(L"largest DWPT should be flushed",
                 largestNonPendingWriter->dwpt);
      // make sure it's not locked
      largestNonPendingWriter->lock();
      largestNonPendingWriter->unlock();
      if (LuceneTestCase::random()->nextBoolean()) {
        w->commit();
      }
      shared_ptr<DirectoryReader> reader = DirectoryReader::open(w, true, true);
      TestUtil::assertEquals(numDocs, reader->numDocs());
      reader->close();
      delete w;
      delete dir;
    }

    void<missing_class_definition>::testCheckPendingFlushPostUpdate() throw(
        IOException, InterruptedException)
    {
      shared_ptr<MockDirectoryWrapper> dir = LuceneTestCase::newMockDirectory();
      shared_ptr<Set<wstring>> flushingThreads =
          Collections::synchronizedSet(unordered_set<wstring>());
      dir->failOn(make_shared<FailureAnonymousInnerClass>(
          shared_from_this(), dir, flushingThreads));
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, make_shared<IndexWriterConfig>()
                   .setCheckPendingFlushUpdate(false)
                   .setMaxBufferedDocs(numeric_limits<int>::max())
                   .setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH));
      shared_ptr<AtomicBoolean> done = make_shared<AtomicBoolean>(false);
      int numThreads = 2 + LuceneTestCase::random()->nextInt(3);
      shared_ptr<CountDownLatch> latch =
          make_shared<CountDownLatch>(numThreads);
      shared_ptr<Set<wstring>> indexingThreads = unordered_set<wstring>();
      std::deque<std::shared_ptr<Thread>> threads(numThreads);
      for (int i = 0; i < numThreads; i++) {
        threads[i] = make_shared<Thread>([&]() {
          latch->countDown();
          int numDocs = 0;
          while (done->get() == false) {
            shared_ptr<Document> doc = make_shared<Document>();
            doc->add(
                make_shared<StringField>(L"id", L"foo", Field::Store::YES));
            try {
              w->addDocument(doc);
            } catch (const runtime_error &e) {
              throw make_shared<AssertionError>(e);
            }
            if (numDocs++ % 10 == 0) {
              Thread::yield();
            }
          }
        });
        indexingThreads->add(threads[i]->getName());
        threads[i]->start();
      }
      latch->await();
      try {
        int numIters = LuceneTestCase::rarely()
                           ? 1 + LuceneTestCase::random()->nextInt(5)
                           : 1;
        for (int i = 0; i < numIters; i++) {
          waitForDocsInBuffers(w, min(2, threads.size()));
          w->commit();
          // C++ TODO: There is no native C++ equivalent to 'toString':
          assertTrue(
              flushingThreads->toString(),
              flushingThreads->contains(Thread::currentThread().getName()));
          flushingThreads->retainAll(indexingThreads);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          assertTrue(flushingThreads->toString(), flushingThreads->isEmpty());
        }
        w->getConfig()->setCheckPendingFlushUpdate(true);
        numIters = 0;
        while (true) {
          assertFalse(L"should finish in less than 100 iterations",
                      numIters++ >= 100);
          waitForDocsInBuffers(w, min(2, threads.size()));
          w->flush();
          flushingThreads->retainAll(indexingThreads);
          if (flushingThreads->isEmpty() == false) {
            break;
          }
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        done->set(true);
        for (int i = 0; i < numThreads; i++) {
          threads[i]->join();
        }
        IOUtils::close({w, dir});
      }
    }

    FailureAnonymousInnerClass::FailureAnonymousInnerClass(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<MockDirectoryWrapper> dir,
        shared_ptr<Set<wstring>> flushingThreads)
    {
      this->outerInstance = outerInstance;
      this->dir = dir;
      this->flushingThreads = flushingThreads;
    }

    void FailureAnonymousInnerClass::eval(
        shared_ptr<MockDirectoryWrapper> dir) 
    {
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: StackTraceElement[] trace = new
      // Exception().getStackTrace();
      std::deque<std::shared_ptr<StackTraceElement>> trace =
          (runtime_error())->getStackTrace();
      for (int i = 0; i < trace.size(); i++) {
        if (L"flush" == trace[i]->getMethodName() &&
            L"org.apache.lucene.index.DocumentsWriterPerThread" ==
                trace[i]->getClassName()) {
          flushingThreads->add(Thread::currentThread().getName());
          break;
        }
      }
    }

    void<missing_class_definition>::waitForDocsInBuffers(
        shared_ptr<IndexWriter> w, int buffersWithDocs)
    {
      // wait until at least N threadstates have a doc in order to observe
      // who flushes the segments.
      while (true) {
        int numStatesWithDocs = 0;
        shared_ptr<DocumentsWriterPerThreadPool> perThreadPool =
            w->docWriter->perThreadPool;
        for (int i = 0; i < perThreadPool->getActiveThreadStateCount(); i++) {
          shared_ptr<DocumentsWriterPerThreadPool::ThreadState> threadState =
              perThreadPool->getThreadState(i);
          threadState->lock();
          try {
            shared_ptr<DocumentsWriterPerThread> dwpt = threadState->dwpt;
            if (dwpt != nullptr && dwpt->getNumDocsInRAM() > 1) {
              numStatesWithDocs++;
            }
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            threadState->unlock();
          }
        }
        if (numStatesWithDocs >= buffersWithDocs) {
          return;
        }
      }
    }

    void<missing_class_definition>::testSoftUpdateDocuments() 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                   L"soft_delete"));
      LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
        writer->softUpdateDocument(
            nullptr, make_shared<Document>(),
            {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
      });

      LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
        writer->softUpdateDocument(make_shared<Term>(L"id", L"1"),
                                   make_shared<Document>());
      });

      LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
      writer->softUpdateDocuments(nullptr, deque<? extends deque<? extends IndexableField>> {make_shared<Document>()}, {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
      });

      LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
      writer->softUpdateDocuments(make_shared<Term>(L"id", L"1"), deque<? extends deque<? extends IndexableField>> {make_shared<Document>()});
      });

      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
      doc->push_back(
          make_shared<StringField>(L"version", L"1", Field::Store::YES));
      writer->addDocument(doc);
      doc = make_shared<Document>();
      doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
      doc->push_back(
          make_shared<StringField>(L"version", L"2", Field::Store::YES));
      shared_ptr<Field> field =
          make_shared<NumericDocValuesField>(L"soft_delete", 1);
      writer->softUpdateDocument(make_shared<Term>(L"id", L"1"), doc, {field});
      shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
      TestUtil::assertEquals(2,
                             reader->docFreq(make_shared<Term>(L"id", L"1")));
      shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
      shared_ptr<TopDocs> topDocs = searcher->search(
          make_shared<TermQuery>(make_shared<Term>(L"id", L"1")), 10);
      TestUtil::assertEquals(1, topDocs->totalHits);
      shared_ptr<Document> document =
          reader->document(topDocs->scoreDocs[0]->doc);
      TestUtil::assertEquals(L"2", document[L"version"]);

      // update the on-disk version
      doc = make_shared<Document>();
      doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
      doc->push_back(
          make_shared<StringField>(L"version", L"3", Field::Store::YES));
      field = make_shared<NumericDocValuesField>(L"soft_delete", 1);
      writer->softUpdateDocument(make_shared<Term>(L"id", L"1"), doc, {field});
      shared_ptr<DirectoryReader> oldReader = reader;
      reader = DirectoryReader::openIfChanged(reader, writer);
      assertNotSame(reader, oldReader);
      oldReader->close();
      searcher = make_shared<IndexSearcher>(reader);
      topDocs = searcher->search(
          make_shared<TermQuery>(make_shared<Term>(L"id", L"1")), 10);
      TestUtil::assertEquals(1, topDocs->totalHits);
      document = reader->document(topDocs->scoreDocs[0]->doc);
      TestUtil::assertEquals(L"3", document[L"version"]);

      // now delete it
      writer->updateDocValues(make_shared<Term>(L"id", L"1"), {field});
      oldReader = reader;
      reader = DirectoryReader::openIfChanged(reader, writer);
      assertNotSame(reader, oldReader);
      assertNotNull(reader);
      oldReader->close();
      searcher = make_shared<IndexSearcher>(reader);
      topDocs = searcher->search(
          make_shared<TermQuery>(make_shared<Term>(L"id", L"1")), 10);
      TestUtil::assertEquals(0, topDocs->totalHits);
      int numSoftDeleted = 0;
      for (auto info : writer->segmentInfos) {
        numSoftDeleted += info->getSoftDelCount();
      }
      TestUtil::assertEquals(writer->maxDoc() - writer->numDocs(),
                             numSoftDeleted);
      delete writer;
      reader->close();
      delete dir;
    }

    void<missing_class_definition>::testSoftUpdatesConcurrently() throw(
        IOException, InterruptedException)
    {
      softUpdatesConcurrently(false);
    }

    void<missing_class_definition>::
        testSoftUpdatesConcurrentlyMixedDeletes() throw(IOException,
                                                        InterruptedException)
    {
      softUpdatesConcurrently(true);
    }

    void<missing_class_definition>::softUpdatesConcurrently(
        bool mixDeletes) 
    {
      shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
      shared_ptr<IndexWriterConfig> indexWriterConfig =
          LuceneTestCase::newIndexWriterConfig();
      indexWriterConfig->setSoftDeletesField(L"soft_delete");
      shared_ptr<AtomicBoolean> mergeAwaySoftDeletes =
          make_shared<AtomicBoolean>(LuceneTestCase::random()->nextBoolean());
      if (mixDeletes == false) {
        indexWriterConfig->setMergePolicy(make_shared<OneMergeWrappingMergePolicy>(indexWriterConfig->getMergePolicy(), [&] (any towrap)
        {
      make_shared<MergePolicy::OneMerge>(towrap::segments);
        };
        {
              CodecReader wrapForMerge(CodecReader reader) throws IOException
              {
                if (mergeAwaySoftDeletes->get())
                {
                  return towrap::wrapForMerge(reader);
      } else {
        shared_ptr<CodecReader> wrapped = towrap::wrapForMerge(reader);
        return make_shared<FilterCodecReaderAnonymousInnerClass>(
            shared_from_this(), wrapped);
      }
    }
    } // namespace org::apache::lucene::index
       ));
       }
       shared_ptr<IndexWriter> writer =
           make_shared<IndexWriter>(dir, indexWriterConfig);
       std::deque<std::shared_ptr<Thread>>
           threads(2 + LuceneTestCase::random()->nextInt(3));
       shared_ptr<CountDownLatch> startLatch = make_shared<CountDownLatch>(1);
       shared_ptr<CountDownLatch> started =
           make_shared<CountDownLatch>(threads.size());
       bool updateSeveralDocs = LuceneTestCase::random()->nextBoolean();
       shared_ptr<Set<wstring>> ids =
           Collections::synchronizedSet(unordered_set<wstring>());
       for (int i = 0; i < threads.size(); i++) {
         threads[i] = make_shared<Thread>([&]() {
           try {
             started->countDown();
             startLatch->await();
             for (int d = 0; d < 100; d++) {
               wstring id =
                   wstring::valueOf(LuceneTestCase::random()->nextInt(10));
               if (updateSeveralDocs) {
                 shared_ptr<Document> doc = make_shared<Document>();
                 doc->add(
                     make_shared<StringField>(L"id", id, Field::Store::YES));
                 if (mixDeletes && LuceneTestCase::random()->nextBoolean()) {
                        writer->updateDocuments(make_shared<Term>(L"id", id), deque<? extends deque<? extends IndexableField>> {doc, doc});
                 } else {
                        writer->softUpdateDocuments(make_shared<Term>(L"id", id), deque<? extends deque<? extends IndexableField>> {doc, doc}, {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
                 }
               } else {
                 shared_ptr<Document> doc = make_shared<Document>();
                 doc->add(
                     make_shared<StringField>(L"id", id, Field::Store::YES));
                 if (mixDeletes && LuceneTestCase::random()->nextBoolean()) {
                   writer->updateDocument(make_shared<Term>(L"id", id), doc);
                 } else {
                   writer->softUpdateDocument(
                       make_shared<Term>(L"id", id), doc,
                       {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
                 }
               }
               ids->add(id);
             }
           } catch (IOException | InterruptedException e) {
             throw make_shared<AssertionError>(e);
           }
         });
         threads[i]->start();
       }
       started->await();
       startLatch->countDown();

       for (int i = 0; i < threads.size(); i++) {
         threads[i]->join();
       }
       shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
       shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
       for (auto id : ids) {
         shared_ptr<TopDocs> topDocs = searcher->search(
             make_shared<TermQuery>(make_shared<Term>(L"id", id)), 10);
         if (updateSeveralDocs) {
           TestUtil::assertEquals(2, topDocs->totalHits);
           TestUtil::assertEquals(
               abs(topDocs->scoreDocs[0]->doc - topDocs->scoreDocs[1]->doc), 1);
         } else {
           TestUtil::assertEquals(1, topDocs->totalHits);
         }
       }
       mergeAwaySoftDeletes->set(true);
       writer->addDocument(make_shared<Document>()); // add a dummy doc to
                                                     // trigger a segment here
       writer->flush();
       writer->forceMerge(1);
       shared_ptr<DirectoryReader> oldReader = reader;
       reader = DirectoryReader::openIfChanged(reader, writer);
       if (reader != nullptr) {
         oldReader->close();
         assertNotSame(oldReader, reader);
       } else {
         reader = oldReader;
       }
       for (auto id : ids) {
         if (updateSeveralDocs) {
           TestUtil::assertEquals(
               2, reader->docFreq(make_shared<Term>(L"id", id)));
         } else {
           TestUtil::assertEquals(
               1, reader->docFreq(make_shared<Term>(L"id", id)));
         }
       }
       int numSoftDeleted = 0;
       for (auto info : writer->segmentInfos) {
         numSoftDeleted += info->getSoftDelCount() + info->getDelCount();
       }
       TestUtil::assertEquals(writer->maxDoc() - writer->numDocs(),
                              numSoftDeleted);
       writer->commit();
       // C++ NOTE: The following 'try with resources' block is replaced by its
       // C++ equivalent: ORIGINAL LINE: try (DirectoryReader dirReader =
       // DirectoryReader.open(dir))
       {
         DirectoryReader dirReader = DirectoryReader::open(dir);
         int delCount = 0;
         for (shared_ptr<LeafReaderContext> ctx : dirReader.leaves()) {
           shared_ptr<SegmentCommitInfo> segmentInfo =
               (std::static_pointer_cast<SegmentReader>(ctx->reader()))
                   ->getSegmentInfo();
           delCount +=
               segmentInfo->getSoftDelCount() + segmentInfo->getDelCount();
         }
         TestUtil::assertEquals(numSoftDeleted, delCount);
       }
       IOUtils::close({reader, writer, dir});
       }

       FilterCodecReaderAnonymousInnerClass::
           FilterCodecReaderAnonymousInnerClass(
               shared_ptr<MissingClass> outerInstance,
               shared_ptr<org::apache::lucene::index::CodecReader> wrapped)
           : FilterCodecReader(wrapped)
       {
         this->outerInstance = outerInstance;
       }

       shared_ptr<CacheHelper>
       FilterCodecReaderAnonymousInnerClass::getCoreCacheHelper()
       {
         return in_::getCoreCacheHelper();
       }

       shared_ptr<CacheHelper>
       FilterCodecReaderAnonymousInnerClass::getReaderCacheHelper()
       {
         return in_::getReaderCacheHelper();
       }

       shared_ptr<Bits> FilterCodecReaderAnonymousInnerClass::getLiveDocs()
       {
         return nullptr; // everything is live
       }

       int FilterCodecReaderAnonymousInnerClass::numDocs() { return maxDoc(); }

       void<missing_class_definition>::
           testDeleteHappensBeforeWhileFlush() throw(IOException,
                                                     InterruptedException)
       {
         shared_ptr<CountDownLatch> latch = make_shared<CountDownLatch>(1);
         shared_ptr<CountDownLatch> inFlush = make_shared<CountDownLatch>(1);
         // C++ NOTE: The following 'try with resources' block is replaced by
         // its C++ equivalent: ORIGINAL LINE: try
         // (org.apache.lucene.store.Directory dir = new
         // org.apache.lucene.store.FilterDirectory(newDirectory())
         {
           org::apache::lucene::store::Directory dir =
               FilterDirectoryAnonymousInnerClass(
                   shared_from_this(), LuceneTestCase::newDirectory(), latch,
                   inFlush) IndexWriter writer =
                   make_shared<IndexWriter>(
                       dir, LuceneTestCase::newIndexWriterConfig());
           shared_ptr<Document> document = make_shared<Document>();
           document->push_back(
               make_shared<StringField>(L"id", L"1", Field::Store::YES));
           writer::addDocument(document);
           shared_ptr<Thread> t = make_shared<Thread>([&]() {
             try {
               inFlush->await();
               writer::docWriter::flushControl::setApplyAllDeletes();
               if (LuceneTestCase::random()->nextBoolean()) {
                 writer::updateDocument(make_shared<Term>(L"id", L"1"),
                                        document);
               } else {
                 writer::deleteDocuments(make_shared<Term>(L"id", L"1"));
               }
             } catch (const runtime_error &e) {
               throw make_shared<AssertionError>(e);
             }
             // C++ TODO: There is no native C++ equivalent to the exception
             // 'finally' clause:
             finally {
               latch->countDown();
             }
           });
           t->start();
           // C++ NOTE: The following 'try with resources' block is replaced by
           // its C++ equivalent: ORIGINAL LINE: try (IndexReader reader =
           // writer.getReader())
           {
             IndexReader reader = writer::getReader();
             TestUtil::assertEquals(1, reader->numDocs());
           };
           t->join();
         }
       }

       FilterDirectoryAnonymousInnerClass::FilterDirectoryAnonymousInnerClass(
           shared_ptr<MissingClass> outerInstance,
           shared_ptr<UnknownType> newDirectory,
           shared_ptr<CountDownLatch> latch, shared_ptr<CountDownLatch> inFlush)
           : org::apache::lucene::store::FilterDirectory(newDirectory)
       {
         this->outerInstance = outerInstance;
         this->latch = latch;
         this->inFlush = inFlush;
       }

       shared_ptr<IndexOutput> FilterDirectoryAnonymousInnerClass::createOutput(
           const wstring &name,
           shared_ptr<IOContext> context) 
       {
         // C++ TODO: This exception's constructor requires an argument:
         // ORIGINAL LINE: StackTraceElement[] trace = new
         // Exception().getStackTrace();
         std::deque<std::shared_ptr<StackTraceElement>> trace =
             (runtime_error())->getStackTrace();
         for (int i = 0; i < trace.size(); i++) {
           if (L"flush" == trace[i]->getMethodName() &&
               DefaultIndexingChain::typeid->getName().equals(
                   trace[i]->getClassName())) {
             try {
               inFlush->countDown();
               latch->await();
             } catch (const InterruptedException &e) {
               throw make_shared<AssertionError>(e);
             }
           }
         }
         return FilterDirectory::createOutput(name, context);
       }

       void<missing_class_definition>::assertFiles(
           shared_ptr<IndexWriter> writer) 
       {
         function<bool(const wstring &)> filter = [&](const wstring &file) {
           return file->startsWith(L"segments") == false &&
                  file::equals(L"write.lock") == false;
         };
         // remove segment files we don't know if we have committed and what is
         // kept around
         shared_ptr<Set<wstring>> segFiles =
             (unordered_set<wstring>(writer->segmentInfos->files(true)))
                 ->stream()
                 .filter(filter)
                 .collect(Collectors::toSet());
         shared_ptr<Set<wstring>> dirFiles =
             (unordered_set<wstring>(
                  Arrays::asList(writer->getDirectory()->listAll())))
                 ->stream()
                 .filter(filter)
                 .collect(Collectors::toSet());
         // ExtraFS might add an extra0 file, ignore it
         dirFiles->remove(L"extra0");
         TestUtil::assertEquals(segFiles->size(), dirFiles->size());
       }

       void<missing_class_definition>::
           testFullyDeletedSegmentsReleaseFiles() 
       {
         shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriterConfig> config =
             LuceneTestCase::newIndexWriterConfig();
         config->setRAMBufferSizeMB(numeric_limits<int>::max());
         config->setMaxBufferedDocs(2); // no auto flush
         shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
         shared_ptr<Document> d = make_shared<Document>();
         d->push_back(
             make_shared<StringField>(L"id", L"doc-0", Field::Store::YES));
         writer->addDocument(d);
         writer->flush();
         d = make_shared<Document>();
         d->push_back(
             make_shared<StringField>(L"id", L"doc-1", Field::Store::YES));
         writer->addDocument(d);
         writer->deleteDocuments({make_shared<Term>(L"id", L"doc-1")});
         TestUtil::assertEquals(1, writer->segmentInfos->asList().size());
         writer->flush();
         TestUtil::assertEquals(1, writer->segmentInfos->asList().size());
         writer->commit();
         assertFiles(writer);
         TestUtil::assertEquals(1, writer->segmentInfos->asList().size());
         IOUtils::close({writer, dir});
       }

       void<missing_class_definition>::testSegmentInfoIsSnapshot() throw(
           IOException)
       {
         shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriterConfig> config =
             LuceneTestCase::newIndexWriterConfig();
         config->setRAMBufferSizeMB(numeric_limits<int>::max());
         config->setMaxBufferedDocs(2); // no auto flush
         shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
         shared_ptr<Document> d = make_shared<Document>();
         d->push_back(
             make_shared<StringField>(L"id", L"doc-0", Field::Store::YES));
         writer->addDocument(d);
         d = make_shared<Document>();
         d->push_back(
             make_shared<StringField>(L"id", L"doc-1", Field::Store::YES));
         writer->addDocument(d);
         shared_ptr<DirectoryReader> reader = writer->getReader();
         shared_ptr<SegmentCommitInfo> segmentInfo =
             (std::static_pointer_cast<SegmentReader>(
                  reader->leaves()->get(0).reader()))
                 ->getSegmentInfo();
         shared_ptr<SegmentCommitInfo> originalInfo =
             (std::static_pointer_cast<SegmentReader>(
                  reader->leaves()->get(0).reader()))
                 ->getOriginalSegmentInfo();
         TestUtil::assertEquals(0, originalInfo->getDelCount());
         TestUtil::assertEquals(0, segmentInfo->getDelCount());
         writer->deleteDocuments({make_shared<Term>(L"id", L"doc-0")});
         writer->commit();
         TestUtil::assertEquals(0, segmentInfo->getDelCount());
         TestUtil::assertEquals(1, originalInfo->getDelCount());
         IOUtils::close({reader, writer, dir});
       }

       void<missing_class_definition>::
           testPreventChangingSoftDeletesField() 
       {
         shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
             dir, LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                      L"my_deletes"));
         shared_ptr<Document> v1 = make_shared<Document>();
         v1->push_back(
             make_shared<StringField>(L"id", L"1", Field::Store::YES));
         v1->push_back(
             make_shared<StringField>(L"version", L"1", Field::Store::YES));
         writer->addDocument(v1);
         shared_ptr<Document> v2 = make_shared<Document>();
         v2->push_back(
             make_shared<StringField>(L"id", L"1", Field::Store::YES));
         v2->push_back(
             make_shared<StringField>(L"version", L"2", Field::Store::YES));
         writer->softUpdateDocument(
             make_shared<Term>(L"id", L"1"), v2,
             {make_shared<NumericDocValuesField>(L"my_deletes", 1)});
         writer->commit();
         delete writer;
         for (auto si : SegmentInfos::readLatestCommit(dir)) {
           shared_ptr<FieldInfos> fieldInfos = IndexWriter::readFieldInfos(si);
           TestUtil::assertEquals(L"my_deletes",
                                  fieldInfos->getSoftDeletesField());
           assertTrue(
               fieldInfos->fieldInfo(L"my_deletes")->isSoftDeletesField());
         }

         invalid_argument illegalError =
             LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
               make_shared<IndexWriter>(
                   dir,
                   LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                       L"your_deletes"));
             });
         TestUtil::assertEquals(
             wstring(L"cannot configure [your_deletes] as soft-deletes; ") +
                 L"this index uses [my_deletes] as soft-deletes already",
             illegalError.what());

         shared_ptr<IndexWriterConfig> softDeleteConfig =
             LuceneTestCase::newIndexWriterConfig()
                 ->setSoftDeletesField(L"my_deletes")
                 ->setMergePolicy(make_shared<SoftDeletesRetentionMergePolicy>(
                     L"my_deletes",
                     [&]() {
                       make_shared<
                           org::apache::lucene::search::MatchAllDocsQuery>();
                     },
                     LuceneTestCase::newMergePolicy()));
         writer = make_shared<IndexWriter>(dir, softDeleteConfig);
         shared_ptr<Document> tombstone = make_shared<Document>();
         tombstone->push_back(
             make_shared<StringField>(L"id", L"tombstone", Field::Store::YES));
         tombstone->push_back(
             make_shared<NumericDocValuesField>(L"my_deletes", 1));
         writer->addDocument(tombstone);
         writer->flush();
         for (auto si : writer->segmentInfos) {
           shared_ptr<FieldInfos> fieldInfos = IndexWriter::readFieldInfos(si);
           TestUtil::assertEquals(L"my_deletes",
                                  fieldInfos->getSoftDeletesField());
           assertTrue(
               fieldInfos->fieldInfo(L"my_deletes")->isSoftDeletesField());
         }
         delete writer;
         // reopen writer without soft-deletes field should be prevented
         invalid_argument reopenError =
             LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
               make_shared<IndexWriter>(dir,
                                        LuceneTestCase::newIndexWriterConfig());
             });
         TestUtil::assertEquals(
             wstring(L"this index has [my_deletes] as soft-deletes already") +
                 L" but soft-deletes field is not configured in IWC",
             reopenError.what());
         delete dir;
       }

       void<missing_class_definition>::
           testPreventAddingIndexesWithDifferentSoftDeletesField() throw(
               runtime_error)
       {
         shared_ptr<Directory> dir1 = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(
             dir1, LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                       L"soft_deletes_1"));
         for (int i = 0; i < 2; i++) {
           shared_ptr<Document> d = make_shared<Document>();
           d->push_back(
               make_shared<StringField>(L"id", L"1", Field::Store::YES));
           // C++ TODO: There is no native C++ equivalent to 'toString':
           d->push_back(make_shared<StringField>(
               L"version", Integer::toString(i), Field::Store::YES));
           w1->softUpdateDocument(
               make_shared<Term>(L"id", L"1"), d,
               {make_shared<NumericDocValuesField>(L"soft_deletes_1", 1)});
         }
         w1->commit();
         delete w1;

         shared_ptr<Directory> dir2 = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
             dir2, LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                       L"soft_deletes_2"));
         invalid_argument error = LuceneTestCase::expectThrows(
             invalid_argument::typeid, [&]() { w2->addIndexes({dir1}); });
         TestUtil::assertEquals(
             L"cannot configure [soft_deletes_2] as soft-deletes; this index "
             L"uses [soft_deletes_1] as soft-deletes already",
             error.what());
         delete w2;

         shared_ptr<Directory> dir3 = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriterConfig> config =
             LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                 L"soft_deletes_1");
         shared_ptr<IndexWriter> w3 = make_shared<IndexWriter>(dir3, config);
         w3->addIndexes({dir1});
         for (auto si : w3->segmentInfos) {
           shared_ptr<FieldInfo> softDeleteField =
               IndexWriter::readFieldInfos(si)->fieldInfo(L"soft_deletes_1");
           assertTrue(softDeleteField->isSoftDeletesField());
         }
         delete w3;
         IOUtils::close({dir1, dir2, dir3});
       }

       void<missing_class_definition>::
           testNotAllowUsingExistingFieldAsSoftDeletes() 
       {
         shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
         shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
             dir, LuceneTestCase::newIndexWriterConfig());
         for (int i = 0; i < 2; i++) {
           shared_ptr<Document> d = make_shared<Document>();
           d->push_back(
               make_shared<StringField>(L"id", L"1", Field::Store::YES));
           if (LuceneTestCase::random()->nextBoolean()) {
             d->push_back(make_shared<NumericDocValuesField>(L"dv_field", 1));
             w->updateDocument(make_shared<Term>(L"id", L"1"), d);
           } else {
             w->softUpdateDocument(
                 make_shared<Term>(L"id", L"1"), d,
                 {make_shared<NumericDocValuesField>(L"dv_field", 1)});
           }
         }
         w->commit();
         delete w;
         wstring softDeletesField =
             LuceneTestCase::random()->nextBoolean() ? L"id" : L"dv_field";
         invalid_argument error =
             LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
               shared_ptr<IndexWriterConfig> config =
                   LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                       softDeletesField);
               make_shared<IndexWriter>(dir, config);
             });
         TestUtil::assertEquals(L"cannot configure [" + softDeletesField +
                                    L"] as soft-deletes;" +
                                    L" this index uses [" + softDeletesField +
                                    L"] as non-soft-deletes already",
                                error.what());
         shared_ptr<IndexWriterConfig> config =
             LuceneTestCase::newIndexWriterConfig()->setSoftDeletesField(
                 L"non-existing-field");
         w = make_shared<IndexWriter>(dir, config);
         delete w;
         delete dir;
       }
       }