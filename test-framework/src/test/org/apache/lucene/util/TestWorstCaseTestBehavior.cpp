using namespace std;

#include "TestWorstCaseTestBehavior.h"

namespace org::apache::lucene::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using com::carrotsearch::randomizedtesting::RandomizedTest;
using com::carrotsearch::randomizedtesting::annotations::Timeout;
using org::junit::Ignore;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testThreadLeak()
void TestWorstCaseTestBehavior::testThreadLeak()
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this());
  t->start();

  while (!t->isAlive()) {
    Thread::yield();
  }

  // once alive, leave it to run outside of the test scope.
}

TestWorstCaseTestBehavior::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestWorstCaseTestBehavior> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestWorstCaseTestBehavior::ThreadAnonymousInnerClass::run()
{
  try {
    delay(10000);
  } catch (const InterruptedException &e) {
    // Ignore.
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testLaaaaaargeOutput() throws Exception
void TestWorstCaseTestBehavior::testLaaaaaargeOutput() 
{
  wstring message = L"I will not OOM on large output";
  int howMuch = 250 * 1024 * 1024;
  for (int i = 0; i < howMuch; i++) {
    if (i > 0) {
      wcout << L",\n";
    }
    wcout << message;
    howMuch -= message.length(); // approximately.
  }
  wcout << L"." << endl;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testProgressiveOutput() throws Exception
void TestWorstCaseTestBehavior::testProgressiveOutput() 
{
  for (int i = 0; i < 20; i++) {
    wcout << L"Emitting sysout line: " << i << endl;
    System::err::println(L"Emitting syserr line: " + to_wstring(i));
    System::out::flush();
    System::err::flush();
    RandomizedTest::sleep(1000);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testUncaughtException() throws Exception
void TestWorstCaseTestBehavior::testUncaughtException() 
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass2>(shared_from_this());
  t->start();
  t->join();
}

TestWorstCaseTestBehavior::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<TestWorstCaseTestBehavior> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestWorstCaseTestBehavior::ThreadAnonymousInnerClass2::run()
{
  throw runtime_error(L"foobar");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore @Timeout(millis = 500) public void testTimeout()
// throws Exception
void TestWorstCaseTestBehavior::testTimeout() 
{
  delay(5000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore @Timeout(millis = 1000) public void testZombie()
// throws Exception
void TestWorstCaseTestBehavior::testZombie() 
{
  while (true) {
    try {
      delay(1000);
    } catch (const InterruptedException &e) {
    }
  }
}
} // namespace org::apache::lucene::util