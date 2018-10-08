using namespace std;

#include "TestMaxFailuresRule.h"

namespace org::apache::lucene::util
{
using WithNestedTests = org::apache::lucene::util::WithNestedTests;
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakAction;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakLingering;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakScope;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakZombies;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakScope::Scope;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakZombies::
    Consequence;
using org::junit::Assert;
using org::junit::BeforeClass;
using org::junit::Test;
using org::junit::runner::Description;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;
using org::junit::runner::notification::RunListener;

TestMaxFailuresRule::TestMaxFailuresRule()
    : org::apache::lucene::util::WithNestedTests(true)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = TOTAL_ITERS) public void
// testFailSometimes()
void TestMaxFailuresRule::Nested::testFailSometimes()
{
  numIters++;
  bool fail = random()->nextInt(5) == 0;
  if (fail) {
    numFails++;
  }
  // some seeds are really lucky ... so cheat.
  if (numFails < DESIRED_FAILURES &&
      DESIRED_FAILURES <= TOTAL_ITERS - numIters) {
    fail = true;
  }
  assertFalse(fail);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMaxFailures()
void TestMaxFailuresRule::testMaxFailures()
{
  LuceneTestCase::replaceMaxFailureRule(
      make_shared<TestRuleIgnoreAfterMaxFailures>(2));
  shared_ptr<JUnitCore> core = make_shared<JUnitCore>();
  shared_ptr<StringBuilder> *const results = make_shared<StringBuilder>();
  core->addListener(
      make_shared<RunListenerAnonymousInnerClass>(shared_from_this(), results));

  shared_ptr<Result> result = core->run(Nested::typeid);
  Assert::assertEquals(500, result->getRunCount());
  Assert::assertEquals(0, result->getIgnoreCount());
  Assert::assertEquals(2, result->getFailureCount());

  // Make sure we had exactly two failures followed by assumption-failures
  // resulting from ignored tests.
  Assert::assertTrue(results->toString(),
                     results->toString().matches(L"(S*F){2}A+"));
}

TestMaxFailuresRule::RunListenerAnonymousInnerClass::
    RunListenerAnonymousInnerClass(
        shared_ptr<TestMaxFailuresRule> outerInstance,
        shared_ptr<StringBuilder> results)
{
  this->outerInstance = outerInstance;
  this->results = results;
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testStarted(
    shared_ptr<Description> description) 
{
  lastTest = L'S'; // success.
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testAssumptionFailure(
    shared_ptr<Failure> failure)
{
  lastTest = L'A'; // assumption failure.
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testFailure(
    shared_ptr<Failure> failure) 
{
  lastTest = L'F'; // failure
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testFinished(
    shared_ptr<Description> description) 
{
  results->append(lastTest);
}

shared_ptr<java::util::concurrent::CountDownLatch>
    TestMaxFailuresRule::Nested2::die;
shared_ptr<Thread> TestMaxFailuresRule::Nested2::zombie;
int TestMaxFailuresRule::Nested2::testNum = 0;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setup()
void TestMaxFailuresRule::Nested2::setup()
{
  assert(zombie == nullptr);
  die = make_shared<CountDownLatch>(1);
  testNum = 0;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = TOTAL_ITERS) public void
// testLeaveZombie()
void TestMaxFailuresRule::Nested2::testLeaveZombie()
{
  if (++testNum == 2) {
    zombie = make_shared<ThreadAnonymousInnerClass>(shared_from_this());
    zombie->start();
  }
}

TestMaxFailuresRule::Nested2::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<Nested2> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestMaxFailuresRule::Nested2::ThreadAnonymousInnerClass::run()
{
  while (true) {
    try {
      die->await();
      return;
    } catch (const runtime_error &e) {
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testZombieThreadFailures() throws Exception
void TestMaxFailuresRule::testZombieThreadFailures() 
{
  LuceneTestCase::replaceMaxFailureRule(
      make_shared<TestRuleIgnoreAfterMaxFailures>(1));
  shared_ptr<JUnitCore> core = make_shared<JUnitCore>();
  shared_ptr<StringBuilder> *const results = make_shared<StringBuilder>();
  core->addListener(
      make_shared<RunListenerAnonymousInnerClass>(shared_from_this(), results));

  shared_ptr<Result> result = core->run(Nested2::typeid);
  if (Nested2::die != nullptr) {
    Nested2::die->countDown();
    Nested2::zombie->join();
  }

  WithNestedTests::prevSysOut->println(results->toString());
  Assert::assertEquals(Nested2::TOTAL_ITERS, result->getRunCount());
  Assert::assertEquals(results->toString(), L"SFAAAAAAAA", results->toString());
}

TestMaxFailuresRule::RunListenerAnonymousInnerClass::
    RunListenerAnonymousInnerClass(
        shared_ptr<TestMaxFailuresRule> outerInstance,
        shared_ptr<StringBuilder> results)
{
  this->outerInstance = outerInstance;
  this->results = results;
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testStarted(
    shared_ptr<Description> description) 
{
  lastTest = L'S'; // success.
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testAssumptionFailure(
    shared_ptr<Failure> failure)
{
  lastTest = L'A'; // assumption failure.
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testFailure(
    shared_ptr<Failure> failure) 
{
  lastTest = L'F'; // failure
  wcout << failure->getMessage() << endl;
}

void TestMaxFailuresRule::RunListenerAnonymousInnerClass::testFinished(
    shared_ptr<Description> description) 
{
  results->append(lastTest);
}
} // namespace org::apache::lucene::util