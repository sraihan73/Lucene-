using namespace std;

#include "TestBagOfPostings.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBagOfPostings::test() 
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

  shared_ptr<Directory> dir = newFSDirectory(createTempDir(L"bagofpostings"));
  shared_ptr<RandomIndexWriter> *const iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int threadCount = TestUtil::nextInt(random(), 1, 5);
  if (VERBOSE) {
    wcout << L"config: " << iw->w->getConfig() << endl;
    wcout << L"threadCount=" << threadCount << endl;
  }

  std::deque<std::shared_ptr<Thread>> threads(threadCount);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);

  for (int threadID = 0; threadID < threadCount; threadID++) {
    threads[threadID] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), maxTermsPerDoc, postings, iw, startingGun);
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
    TestUtil::assertEquals(value, termsEnum->docFreq());
    // don't really need to check more than this, as CheckIndex
    // will verify that docFreq == actual number of documents seen
    // from a postingsEnum.
  }
  ir->close();
  delete iw;
  delete dir;
}

TestBagOfPostings::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestBagOfPostings> outerInstance, int maxTermsPerDoc,
    shared_ptr<ConcurrentLinkedQueue<wstring>> postings,
    shared_ptr<org::apache::lucene::index::RandomIndexWriter> iw,
    shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->maxTermsPerDoc = maxTermsPerDoc;
  this->postings = postings;
  this->iw = iw;
  this->startingGun = startingGun;
}

void TestBagOfPostings::ThreadAnonymousInnerClass::run()
{
  try {
    shared_ptr<Document> document = make_shared<Document>();
    shared_ptr<Field> field =
        LuceneTestCase::newTextField(L"field", L"", Field::Store::NO);
    document->push_back(field);
    startingGun->await();
    while (!postings->isEmpty()) {
      shared_ptr<StringBuilder> text = make_shared<StringBuilder>();
      shared_ptr<Set<wstring>> visited = unordered_set<wstring>();
      for (int i = 0; i < maxTermsPerDoc; i++) {
        wstring token = postings->poll();
        if (token == L"") {
          break;
        }
        if (visited->contains(token)) {
          // Put it back:
          postings->add(token);
          break;
        }
        text->append(L' ');
        text->append(token);
        visited->add(token);
      }
      field->setStringValue(text->toString());
      iw->addDocument(document);
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::index