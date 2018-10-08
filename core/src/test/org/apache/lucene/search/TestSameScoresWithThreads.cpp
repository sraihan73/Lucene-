using namespace std;

#include "TestSameScoresWithThreads.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSameScoresWithThreads::test() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  int charsToIndex = atLeast(100000);
  int charsIndexed = 0;
  // System.out.println("bytesToIndex=" + charsToIndex);
  while (charsIndexed < charsToIndex) {
    shared_ptr<Document> doc = docs->nextDoc();
    charsIndexed += doc[L"body"]->length();
    w->addDocument(doc);
    // System.out.println("  bytes=" + charsIndexed + " add: " + doc);
  }
  shared_ptr<IndexReader> r = w->getReader();
  // System.out.println("numDocs=" + r.numDocs());
  delete w;

  shared_ptr<IndexSearcher> *const s = newSearcher(r);
  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"body");
  int termCount = 0;
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  while (termsEnum->next() != nullptr) {
    termCount++;
  }
  assertTrue(termCount > 0);

  // Target ~10 terms to search:
  double chance = 10.0 / termCount;
  termsEnum = terms->begin();
  const unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<TopDocs>>
      answers =
          unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<TopDocs>>();
  while (termsEnum->next() != nullptr) {
    if (random()->nextDouble() <= chance) {
      shared_ptr<BytesRef> term = BytesRef::deepCopyOf(termsEnum->term());
      answers.emplace(term, s->search(make_shared<TermQuery>(
                                          make_shared<Term>(L"body", term)),
                                      100));
    }
  }

  if (!answers.empty()) {
    shared_ptr<CountDownLatch> *const startingGun =
        make_shared<CountDownLatch>(1);
    int numThreads = TestUtil::nextInt(random(), 2, 5);
    std::deque<std::shared_ptr<Thread>> threads(numThreads);
    for (int threadID = 0; threadID < numThreads; threadID++) {
      shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), s, answers, startingGun);
      threads[threadID] = thread;
      thread->start();
    }
    startingGun->countDown();
    for (auto thread : threads) {
      thread->join();
    }
  }
  delete r;
  delete dir;
}

TestSameScoresWithThreads::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestSameScoresWithThreads> outerInstance,
    shared_ptr<org::apache::lucene::search::IndexSearcher> s,
    unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<TopDocs>> &answers,
    shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->s = s;
  this->answers = answers;
  this->startingGun = startingGun;
}

void TestSameScoresWithThreads::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    for (int i = 0; i < 20; i++) {
      deque<unordered_map::Entry<std::shared_ptr<BytesRef>,
                                  std::shared_ptr<TopDocs>>>
          shuffled = deque<unordered_map::Entry<std::shared_ptr<BytesRef>,
                                                 std::shared_ptr<TopDocs>>>(
              answers.entrySet());
      Collections::shuffle(shuffled, LuceneTestCase::random());
      for (auto ent : shuffled) {
        shared_ptr<TopDocs> actual = s->search(
            make_shared<TermQuery>(make_shared<Term>(L"body", ent.first)), 100);
        shared_ptr<TopDocs> expected = ent.second;
        TestUtil::assertEquals(expected->totalHits, actual->totalHits);
        assertEquals(L"query=" + ent.first::utf8ToString(),
                     expected->scoreDocs.size(), actual->scoreDocs.size());
        for (int hit = 0; hit < expected->scoreDocs.size(); hit++) {
          TestUtil::assertEquals(expected->scoreDocs[hit]->doc,
                                 actual->scoreDocs[hit]->doc);
          // Floats really should be identical:
          assertTrue(expected->scoreDocs[hit]->score ==
                     actual->scoreDocs[hit]->score);
        }
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::search