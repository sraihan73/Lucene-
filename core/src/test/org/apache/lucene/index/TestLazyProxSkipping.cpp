using namespace std;

#include "TestLazyProxSkipping.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::analysis;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

TestLazyProxSkipping::SeekCountingDirectory::SeekCountingDirectory(
    shared_ptr<TestLazyProxSkipping> outerInstance,
    shared_ptr<Directory> delegate_)
    : org::apache::lucene::store::MockDirectoryWrapper(random(), delegate_),
      outerInstance(outerInstance)
{
}

shared_ptr<IndexInput> TestLazyProxSkipping::SeekCountingDirectory::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  shared_ptr<IndexInput> ii = MockDirectoryWrapper::openInput(name, context);
  if (StringHelper::endsWith(name, L".prx") ||
      StringHelper::endsWith(name, L".pos")) {
    // we decorate the proxStream with a wrapper class that allows to count the
    // number of calls of seek()
    ii = make_shared<SeeksCountingStream>(outerInstance, ii);
  }
  return ii;
}

void TestLazyProxSkipping::createIndex(int numHits) 
{
  int numDocs = 500;

  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<Directory> directory = make_shared<SeekCountingDirectory>(
      shared_from_this(), make_shared<RAMDirectory>());
  // note: test explicitly disables payloads
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(10)->setMergePolicy(
          newLogMergePolicy(false)));

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring content;
    if (i % (numDocs / numHits) == 0) {
      // add a document that matches the query "term1 term2"
      content = this->term1 + L" " + this->term2;
    } else if (i % 15 == 0) {
      // add a document that only contains term1
      content = this->term1 + L" " + this->term1;
    } else {
      // add a document that contains term2 but not term 1
      content = this->term3 + L" " + this->term2;
    }

    doc->push_back(newTextField(this->field, content, Field::Store::YES));
    writer->addDocument(doc);
  }

  // make sure the index has only a single segment
  writer->forceMerge(1);
  delete writer;

  shared_ptr<LeafReader> reader =
      getOnlyLeafReader(DirectoryReader::open(directory));

  this->searcher = newSearcher(reader);
}

TestLazyProxSkipping::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestLazyProxSkipping> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestLazyProxSkipping::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true));
}

std::deque<std::shared_ptr<ScoreDoc>>
TestLazyProxSkipping::search() 
{
  // create PhraseQuery "term1 term2" and search
  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(field, term1, term2);
  return this->searcher->search(pq, 1000)->scoreDocs;
}

void TestLazyProxSkipping::performTest(int numHits) 
{
  createIndex(numHits);
  this->seeksCounter = 0;
  std::deque<std::shared_ptr<ScoreDoc>> hits = search();
  // verify that the right number of docs was found
  TestUtil::assertEquals(numHits, hits.size());

  // check if the number of calls of seek() does not exceed the number of hits
  assertTrue(this->seeksCounter > 0);
  assertTrue(L"seeksCounter=" + to_wstring(this->seeksCounter) + L" numHits=" +
                 to_wstring(numHits),
             this->seeksCounter <= numHits + 1);
  delete searcher->getIndexReader();
}

void TestLazyProxSkipping::testLazySkipping() 
{
  const wstring fieldFormat = TestUtil::getPostingsFormat(this->field);
  assumeFalse(L"This test cannot run with Memory postings format",
              fieldFormat == L"Memory");
  assumeFalse(L"This test cannot run with Direct postings format",
              fieldFormat == L"Direct");
  assumeFalse(L"This test cannot run with SimpleText postings format",
              fieldFormat == L"SimpleText");

  // test whether only the minimum amount of seeks()
  // are performed
  performTest(5);
  performTest(10);
}

void TestLazyProxSkipping::testSeek() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(this->field, L"a b", Field::Store::YES));
    writer->addDocument(doc);
  }

  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);

  shared_ptr<PostingsEnum> tp = MultiFields::getTermPositionsEnum(
      reader, this->field, make_shared<BytesRef>(L"b"));

  for (int i = 0; i < 10; i++) {
    tp->nextDoc();
    TestUtil::assertEquals(tp->docID(), i);
    TestUtil::assertEquals(tp->nextPosition(), 1);
  }

  tp = MultiFields::getTermPositionsEnum(reader, this->field,
                                         make_shared<BytesRef>(L"a"));

  for (int i = 0; i < 10; i++) {
    tp->nextDoc();
    TestUtil::assertEquals(tp->docID(), i);
    TestUtil::assertEquals(tp->nextPosition(), 0);
  }
  delete reader;
  delete directory;
}

TestLazyProxSkipping::SeeksCountingStream::SeeksCountingStream(
    shared_ptr<TestLazyProxSkipping> outerInstance,
    shared_ptr<IndexInput> input)
    : org::apache::lucene::store::IndexInput(L"SeekCountingStream(" + input +
                                             L")"),
      outerInstance(outerInstance)
{
  this->input = input;
}

char TestLazyProxSkipping::SeeksCountingStream::readByte() 
{
  return this->input->readByte();
}

void TestLazyProxSkipping::SeeksCountingStream::readBytes(
    std::deque<char> &b, int offset, int len) 
{
  this->input->readBytes(b, offset, len);
}

TestLazyProxSkipping::SeeksCountingStream::~SeeksCountingStream()
{
  delete this->input;
}

int64_t TestLazyProxSkipping::SeeksCountingStream::getFilePointer()
{
  return this->input->getFilePointer();
}

void TestLazyProxSkipping::SeeksCountingStream::seek(int64_t pos) throw(
    IOException)
{
  outerInstance->seeksCounter++;
  this->input->seek(pos);
}

int64_t TestLazyProxSkipping::SeeksCountingStream::length()
{
  return this->input->length();
}

shared_ptr<SeeksCountingStream>
TestLazyProxSkipping::SeeksCountingStream::clone()
{
  return make_shared<SeeksCountingStream>(outerInstance, this->input->clone());
}

shared_ptr<IndexInput> TestLazyProxSkipping::SeeksCountingStream::slice(
    const wstring &sliceDescription, int64_t offset,
    int64_t length) 
{
  return make_shared<SeeksCountingStream>(
      outerInstance, this->input->slice(sliceDescription, offset, length));
}
} // namespace org::apache::lucene::index