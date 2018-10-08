using namespace std;

#include "TestTwoPhaseCommitTool.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
bool TestTwoPhaseCommitTool::TwoPhaseCommitImpl::commitCalled = false;

TestTwoPhaseCommitTool::TwoPhaseCommitImpl::TwoPhaseCommitImpl(
    bool failOnPrepare, bool failOnCommit, bool failOnRollback)
    : failOnPrepare(failOnPrepare), failOnCommit(failOnCommit),
      failOnRollback(failOnRollback)
{
}

int64_t
TestTwoPhaseCommitTool::TwoPhaseCommitImpl::prepareCommit() 
{
  return prepareCommit(nullptr);
}

int64_t TestTwoPhaseCommitTool::TwoPhaseCommitImpl::prepareCommit(
    unordered_map<wstring, wstring> &commitData) 
{
  this->prepareCommitData = commitData;
  assertFalse(
      L"commit should not have been called before all prepareCommit were",
      commitCalled);
  if (failOnPrepare) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"failOnPrepare");
  }
  return 1;
}

int64_t
TestTwoPhaseCommitTool::TwoPhaseCommitImpl::commit() 
{
  return commit(nullptr);
}

int64_t TestTwoPhaseCommitTool::TwoPhaseCommitImpl::commit(
    unordered_map<wstring, wstring> &commitData) 
{
  this->commitData = commitData;
  commitCalled = true;
  if (failOnCommit) {
    throw runtime_error(L"failOnCommit");
  }
  return 1;
}

void TestTwoPhaseCommitTool::TwoPhaseCommitImpl::rollback() 
{
  rollbackCalled = true;
  if (failOnRollback) {
    throw make_shared<Error>(L"failOnRollback");
  }
}

void TestTwoPhaseCommitTool::setUp() 
{
  LuceneTestCase::setUp();
  TwoPhaseCommitImpl::commitCalled = false; // reset count before every test
}

void TestTwoPhaseCommitTool::testPrepareThenCommit() 
{
  // tests that prepareCommit() is called on all objects before commit()
  std::deque<std::shared_ptr<TwoPhaseCommitImpl>> objects(2);
  for (int i = 0; i < objects.size(); i++) {
    objects[i] = make_shared<TwoPhaseCommitImpl>(false, false, false);
  }

  // following call will fail if commit() is called before all prepare() were
  TwoPhaseCommitTool::execute(objects);
}

void TestTwoPhaseCommitTool::testRollback() 
{
  // tests that rollback is called if failure occurs at any stage
  int numObjects = random()->nextInt(8) + 3; // between [3, 10]
  std::deque<std::shared_ptr<TwoPhaseCommitImpl>> objects(numObjects);
  for (int i = 0; i < objects.size(); i++) {
    bool failOnPrepare = random()->nextBoolean();
    // we should not hit failures on commit usually
    bool failOnCommit = random()->nextDouble() < 0.05;
    bool railOnRollback = random()->nextBoolean();
    objects[i] = make_shared<TwoPhaseCommitImpl>(failOnPrepare, failOnCommit,
                                                 railOnRollback);
  }

  bool anyFailure = false;
  try {
    TwoPhaseCommitTool::execute(objects);
  } catch (const runtime_error &t) {
    anyFailure = true;
  }

  if (anyFailure) {
    // if any failure happened, ensure that rollback was called on all.
    for (auto tpc : objects) {
      assertTrue(L"rollback was not called while a failure occurred during the "
                 L"2-phase commit",
                 tpc->rollbackCalled);
    }
  }
}

void TestTwoPhaseCommitTool::testNullTPCs() 
{
  int numObjects = random()->nextInt(4) + 3; // between [3, 6]
  std::deque<std::shared_ptr<TwoPhaseCommit>> tpcs(numObjects);
  bool setNull = false;
  for (int i = 0; i < tpcs.size(); i++) {
    bool isNull = random()->nextDouble() < 0.3;
    if (isNull) {
      setNull = true;
      tpcs[i].reset();
    } else {
      tpcs[i] = make_shared<TwoPhaseCommitImpl>(false, false, false);
    }
  }

  if (!setNull) {
    // none of the TPCs were picked to be null, pick one at random
    int idx = random()->nextInt(numObjects);
    tpcs[idx].reset();
  }

  // following call would fail if TPCTool won't handle null TPCs properly
  TwoPhaseCommitTool::execute(tpcs);
}
} // namespace org::apache::lucene::index