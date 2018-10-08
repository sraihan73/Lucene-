using namespace std;

#include "TestSetOnce.h"

namespace org::apache::lucene::util
{
using AlreadySetException =
    org::apache::lucene::util::SetOnce::AlreadySetException;
using org::junit::Test;

TestSetOnce::SetOnceThread::SetOnceThread(shared_ptr<Random> random)
    : RAND(make_shared<Random>(random->nextLong()))
{
}

void TestSetOnce::SetOnceThread::run()
{
  try {
    sleep(RAND->nextInt(10)); // sleep for a short time
    set->set(optional<int>(stoi(getName()->substr(2))));
    success = true;
  } catch (const InterruptedException &e) {
    // ignore
  } catch (const runtime_error &e) {
    // TODO: change exception type
    // expected.
    success = false;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyCtor() throws Exception
void TestSetOnce::testEmptyCtor() 
{
  shared_ptr<SetOnce<int>> set = make_shared<SetOnce<int>>();
  assertNull(set->get());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE:
// @Test(expected=org.apache.lucene.util.SetOnce.AlreadySetException.class)
// public void testSettingCtor() throws Exception
void TestSetOnce::testSettingCtor() 
{
  shared_ptr<SetOnce<int>> set = make_shared<SetOnce<int>>(optional<int>(5));
  assertEquals(5, set->get().intValue());
  set->set(optional<int>(7));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE:
// @Test(expected=org.apache.lucene.util.SetOnce.AlreadySetException.class)
// public void testSetOnce() throws Exception
void TestSetOnce::testSetOnce() 
{
  shared_ptr<SetOnce<int>> set = make_shared<SetOnce<int>>();
  set->set(optional<int>(5));
  assertEquals(5, set->get().intValue());
  set->set(optional<int>(7));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSetMultiThreaded() throws Exception
void TestSetOnce::testSetMultiThreaded() 
{
  shared_ptr<SetOnce<int>> *const set = make_shared<SetOnce<int>>();
  std::deque<std::shared_ptr<SetOnceThread>> threads(10);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<SetOnceThread>(random());
    threads[i]->setName(L"t-" + to_wstring(i + 1));
    threads[i]->set = set;
  }

  for (auto t : threads) {
    t->start();
  }

  for (auto t : threads) {
    t->join();
  }

  for (auto t : threads) {
    if (t->success) {
      int expectedVal = stoi(t->getName()->substr(2));
      assertEquals(L"thread " + t->getName(), expectedVal,
                   t->set->get().intValue());
    }
  }
}
} // namespace org::apache::lucene::util