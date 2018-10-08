using namespace std;

#include "TestDocIDMerger.h"

namespace org::apache::lucene::index
{
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

TestDocIDMerger::TestSubUnsorted::TestSubUnsorted(
    shared_ptr<MergeState::DocMap> docMap, int maxDoc, int valueStart)
    : DocIDMerger::Sub(docMap), valueStart(valueStart), maxDoc(maxDoc)
{
}

int TestDocIDMerger::TestSubUnsorted::nextDoc()
{
  docID++;
  if (docID == maxDoc) {
    return NO_MORE_DOCS;
  } else {
    return docID;
  }
}

int TestDocIDMerger::TestSubUnsorted::getValue() { return valueStart + docID; }

void TestDocIDMerger::testNoSort() 
{

  int subCount = TestUtil::nextInt(random(), 1, 20);
  deque<std::shared_ptr<TestSubUnsorted>> subs =
      deque<std::shared_ptr<TestSubUnsorted>>();
  int valueStart = 0;
  for (int i = 0; i < subCount; i++) {
    int maxDoc = TestUtil::nextInt(random(), 1, 1000);
    constexpr int docBase = valueStart;
    subs.push_back(make_shared<TestSubUnsorted>(
        make_shared<DocMapAnonymousInnerClass>(shared_from_this(), docBase),
        maxDoc, valueStart));
    valueStart += maxDoc;
  }

  shared_ptr<DocIDMerger<std::shared_ptr<TestSubUnsorted>>> merger =
      DocIDMerger::of(subs, false);

  int count = 0;
  while (true) {
    shared_ptr<TestSubUnsorted> sub = merger->next();
    if (sub == nullptr) {
      break;
    }
    TestUtil::assertEquals(count, sub->mappedDocID);
    TestUtil::assertEquals(count, sub->getValue());
    count++;
  }

  TestUtil::assertEquals(valueStart, count);
}

TestDocIDMerger::DocMapAnonymousInnerClass::DocMapAnonymousInnerClass(
    shared_ptr<TestDocIDMerger> outerInstance, int docBase)
{
  this->outerInstance = outerInstance;
  this->docBase = docBase;
}

int TestDocIDMerger::DocMapAnonymousInnerClass::get(int docID)
{
  return docBase + docID;
}

TestDocIDMerger::TestSubSorted::TestSubSorted(
    shared_ptr<MergeState::DocMap> docMap, int maxDoc, int index)
    : DocIDMerger::Sub(docMap), maxDoc(maxDoc), index(index)
{
}

int TestDocIDMerger::TestSubSorted::nextDoc()
{
  docID++;
  if (docID == maxDoc) {
    return NO_MORE_DOCS;
  } else {
    return docID;
  }
}

wstring TestDocIDMerger::TestSubSorted::toString()
{
  return L"TestSubSorted(index=" + to_wstring(index) + L", mappedDocID=" +
         to_wstring(mappedDocID) + L")";
}

void TestDocIDMerger::testWithSort() 
{

  int subCount = TestUtil::nextInt(random(), 1, 20);
  deque<std::deque<int>> oldToNew = deque<std::deque<int>>();
  // how many docs we've written to each sub:
  deque<int> uptos = deque<int>();
  int totDocCount = 0;
  for (int i = 0; i < subCount; i++) {
    int maxDoc = TestUtil::nextInt(random(), 1, 1000);
    uptos.push_back(0);
    oldToNew.push_back(std::deque<int>(maxDoc));
    totDocCount += maxDoc;
  }

  deque<std::deque<int>> completedSubs = deque<std::deque<int>>();

  // randomly distribute target docIDs into the segments:
  for (int docID = 0; docID < totDocCount; docID++) {
    int sub = random()->nextInt(oldToNew.size());
    int upto = uptos[sub];
    std::deque<int> subDocs = oldToNew[sub];
    subDocs[upto] = docID;
    upto++;
    if (upto == subDocs.size()) {
      completedSubs.push_back(subDocs);
      oldToNew.erase(oldToNew.begin() + sub);
      uptos.erase(uptos.begin() + sub);
    } else {
      uptos[sub] = upto;
    }
  }
  TestUtil::assertEquals(0, oldToNew.size());

  // sometimes do some deletions:
  shared_ptr<FixedBitSet> *const liveDocs;
  if (random()->nextBoolean()) {
    liveDocs = make_shared<FixedBitSet>(totDocCount);
    liveDocs->set(0, totDocCount);
    int deleteAttemptCount = TestUtil::nextInt(random(), 1, totDocCount);
    for (int i = 0; i < deleteAttemptCount; i++) {
      liveDocs->clear(random()->nextInt(totDocCount));
    }
  } else {
    liveDocs.reset();
  }

  deque<std::shared_ptr<TestSubSorted>> subs =
      deque<std::shared_ptr<TestSubSorted>>();
  for (int i = 0; i < subCount; i++) {
    const std::deque<int> docMap = completedSubs[i];
    subs.push_back(
        make_shared<TestSubSorted>(make_shared<DocMapAnonymousInnerClass>(
                                       shared_from_this(), liveDocs, docMap),
                                   docMap.size(), i));
  }

  shared_ptr<DocIDMerger<std::shared_ptr<TestSubSorted>>> merger =
      DocIDMerger::of(subs, true);

  int count = 0;
  while (true) {
    shared_ptr<TestSubSorted> sub = merger->next();
    if (sub == nullptr) {
      break;
    }
    if (liveDocs != nullptr) {
      count = liveDocs->nextSetBit(count);
    }
    TestUtil::assertEquals(count, sub->mappedDocID);
    count++;
  }

  if (liveDocs != nullptr) {
    if (count < totDocCount) {
      TestUtil::assertEquals(NO_MORE_DOCS, liveDocs->nextSetBit(count));
    } else {
      TestUtil::assertEquals(totDocCount, count);
    }
  } else {
    TestUtil::assertEquals(totDocCount, count);
  }
}

TestDocIDMerger::DocMapAnonymousInnerClass::DocMapAnonymousInnerClass(
    shared_ptr<TestDocIDMerger> outerInstance, shared_ptr<FixedBitSet> liveDocs,
    deque<int> &docMap)
{
  this->outerInstance = outerInstance;
  this->liveDocs = liveDocs;
  this->docMap = docMap;
}

int TestDocIDMerger::DocMapAnonymousInnerClass::get(int docID)
{
  int mapped = docMap[docID];
  if (liveDocs == nullptr || liveDocs->get(mapped)) {
    return mapped;
  } else {
    return -1;
  }
}
} // namespace org::apache::lucene::index