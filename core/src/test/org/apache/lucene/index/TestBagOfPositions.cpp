using namespace std;

#include "TestBagOfPositions.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBagOfPositions::test() 
{
  deque<wstring> postingsList = deque<wstring>();
  int numTerms = atLeast(300);
  constexpr int maxTermsPerDoc = TestUtil::nextInt(random(), 10, 20);
  bool isSimpleText = L"SimpleText" == TestUtil::getPostingsFormat(L"field");

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(random(), make_shared<MockAnalyzer>(random()));

  if ((isSimpleText || std::dynamic_pointer_cast<MockRandomMergePolicy>(
                           iwc->getMergePolicy()) != nullptr) &&
      (TEST_NIGHTLY || RANDOM_MULTIPLIER > 1)) {
    // Otherwise test can take way too long (> 2 hours)
    numTerms /= 2;
  }
  if (VERBOSE) {
    wcout << L"maxTermsPerDoc=" << maxTermsPerDoc << endl;
    wcout << L"numTerms=" << numTerms << endl;
  }
  for (int i = 0; i < numTerms; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring term = Integer::toString(i);
    for (int j = 0; j < i; j++) {
      postingsList.push_back(term);
    }
  }
  Collections::shuffle(postingsList, random());

  shared_ptr<ConcurrentLinkedQueue<wstring>> *const postings =
      make_shared<ConcurrentLinkedQueue<wstring>>(postingsList);

  shared_ptr<Directory> dir = newFSDirectory(createTempDir(L"bagofpositions"));

  shared_ptr<RandomIndexWriter> *const iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int threadCount = TestUtil::nextInt(random(), 1, 5);
  if (VERBOSE) {
    wcout << L"config: " << iw->w->getConfig() << endl;
    wcout << L"threadCount=" << threadCount << endl;
  }

  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  if (random()->nextBoolean()) {
    fieldType->setOmitNorms(true);
  }
  int options = random()->nextInt(3);
  if (options == 0) {
    fieldType->setIndexOptions(
        IndexOptions::DOCS_AND_FREQS); // we dont actually need positions
    fieldType->setStoreTermVectors(
        true); // but enforce term vectors when we do this so we check SOMETHING
  } else if (options == 1) {
    fieldType->setIndexOptions(
        IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  }
  // else just positions

  std::deque<std::shared_ptr<Thread>> threads(threadCount);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);

  for (int threadID = 0; threadID < threadCount; threadID++) {
    shared_ptr<Random> *const threadRandom =
        make_shared<Random>(random()->nextLong());
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<Field> *const field =
        make_shared<Field>(L"field", L"", fieldType);
    document->push_back(field);
    threads[threadID] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), numTerms, maxTermsPerDoc, postings, iw, startingGun,
        threadRandom, document, field);
    threads[threadID]->start();
  }
  startingGun->countDown();
  for (auto t : threads) {
    t->join();
  }

  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  TestUtil::assertEquals(1, ir->leaves()->size());
  shared_ptr<LeafReader> air = ir->leaves()->get(0).reader();
  shared_ptr<Terms> terms = air->terms(L"field");
  // numTerms-1 because there cannot be a term 0 with 0 postings:
  TestUtil::assertEquals(numTerms - 1, terms->size());
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  shared_ptr<BytesRef> term;
  while ((term = termsEnum->next()) != nullptr) {
    int value = stoi(term->utf8ToString());
    TestUtil::assertEquals(value, termsEnum->totalTermFreq());
    // don't really need to check more than this, as CheckIndex
    // will verify that totalTermFreq == total number of positions seen
    // from a postingsEnum.
  }
  ir->close();
  delete iw;
  delete dir;
}

TestBagOfPositions::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestBagOfPositions> outerInstance, int numTerms,
    int maxTermsPerDoc, shared_ptr<ConcurrentLinkedQueue<wstring>> postings,
    shared_ptr<org::apache::lucene::index::RandomIndexWriter> iw,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<Random> threadRandom,
    shared_ptr<Document> document, shared_ptr<Field> field)
{
  this->outerInstance = outerInstance;
  this->numTerms = numTerms;
  this->maxTermsPerDoc = maxTermsPerDoc;
  this->postings = postings;
  this->iw = iw;
  this->startingGun = startingGun;
  this->threadRandom = threadRandom;
  this->document = document;
  this->field = field;
}

void TestBagOfPositions::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    while (!postings->isEmpty()) {
      shared_ptr<StringBuilder> text = make_shared<StringBuilder>();
      int numTerms = threadRandom->nextInt(maxTermsPerDoc);
      for (int i = 0; i < numTerms; i++) {
        wstring token = postings->poll();
        if (token == L"") {
          break;
        }
        text->append(L' ');
        text->append(token);
      }
      field->setStringValue(text->toString());
      iw->addDocument(document);
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::index