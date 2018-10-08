using namespace std;

#include "TestCodecLoadingDeadlock.h"

namespace org::apache::lucene::codecs
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using com::carrotsearch::randomizedtesting::RandomizedContext;
using com::carrotsearch::randomizedtesting::RandomizedRunner;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::RunWith;
int TestCodecLoadingDeadlock::MAX_TIME_SECONDS = 30;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeadlock() throws Exception
void TestCodecLoadingDeadlock::testDeadlock() 
{
  LuceneTestCase::assumeFalse(
      L"This test fails on UNIX with Turkish default locale "
      L"(https://issues.apache.org/jira/browse/LUCENE-6036)",
      (make_shared<Locale>(L"tr"))
          ->getLanguage()
          .equals(Locale::getDefault().getLanguage()));

  // pick random codec names for stress test in separate process:
  shared_ptr<Random> *const rnd = RandomizedContext::current().getRandom();
  shared_ptr<Set<wstring>> avail;
  const wstring codecName =
      (deque<>(avail = Codec::availableCodecs()))[rnd->nextInt(avail->size())];
  const wstring pfName =
      (deque<>(avail = PostingsFormat::availablePostingsFormats()))
          [rnd->nextInt(avail->size())];
  const wstring dvfName =
      (deque<>(avail = DocValuesFormat::availableDocValuesFormats()))
          [rnd->nextInt(avail->size())];

  wcout << wstring::format(Locale::ROOT, L"codec: %s, pf: %s, dvf: %s",
                           codecName, pfName, dvfName)
        << endl;

  // Fork a separate JVM to reinitialize classes.
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<Process> *const p =
      (make_shared<ProcessBuilder>(
           Paths->get(System::getProperty(L"java.home"), L"bin", L"java")
               ->toString(),
           L"-cp", System::getProperty(L"java.class.path"), getClassName(),
           codecName, pfName, dvfName))
          ->inheritIO()
          .start();
  if (p->waitFor(MAX_TIME_SECONDS * 2, TimeUnit::SECONDS)) {
    assertEquals(L"Process died abnormally?", 0, p->waitFor());
  } else {
    p->destroyForcibly().waitFor();
    fail(L"Process did not exit after 60 secs?");
  }
}

void TestCodecLoadingDeadlock::main(deque<wstring> &args) 
{
  const wstring codecName = args[0];
  const wstring pfName = args[1];
  const wstring dvfName = args[2];

  constexpr int numThreads =
      14; // two times the modulo in switch statement below
  shared_ptr<CopyOnWriteArrayList<std::shared_ptr<Thread>>> *const allThreads =
      make_shared<CopyOnWriteArrayList<std::shared_ptr<Thread>>>();
  shared_ptr<ExecutorService> *const pool = Executors::newFixedThreadPool(
      numThreads,
      make_shared<NamedThreadFactoryAnonymousInnerClass>(allThreads));

  shared_ptr<CyclicBarrier> *const barrier =
      make_shared<CyclicBarrier>(numThreads);
  IntStream::range(0, numThreads).forEach([&](any taskNo) {
    pool->execute([&]() {
      try {
        barrier->await();
        switch (taskNo % 7) {
        case 0:
          Codec::getDefault();
          break;
        case 1:
          Codec::forName(codecName);
          break;
        case 2:
          PostingsFormat::forName(pfName);
          break;
        case 3:
          DocValuesFormat::forName(dvfName);
          break;
        case 4:
          Codec::availableCodecs();
          break;
        case 5:
          PostingsFormat::availablePostingsFormats();
          break;
        case 6:
          DocValuesFormat::availableDocValuesFormats();
          break;
        default:
          throw make_shared<AssertionError>();
        }
      } catch (const runtime_error &t) {
        {
          lock_guard<mutex> lock(args);
          System::err::println(Thread::currentThread().getName() +
                               L" failed to lookup codec service:");
          t.printStackTrace(System::err);
        }
        Runtime::getRuntime().halt(1);
      }
    });
  });

  pool->shutdown();

  if (!pool->awaitTermination(MAX_TIME_SECONDS, TimeUnit::SECONDS)) {
    // Try to collect stacks so that we can better diagnose the failure.
    System::err::println(
        L"Pool didn't return after " + to_wstring(MAX_TIME_SECONDS) +
        L" seconds, classloader deadlock? Dumping stack traces.");

    for (auto t : allThreads) {
      System::err::println(L"# Thread: " + t + L", " + L"state: " +
                           t->getState() + L", " + L"stack:\n\t" +
                           Arrays::stream(t->getStackTrace())
                               .map_obj(any::toString)
                               .collect(Collectors::joining(L"\t")) +
                           L"\n");
    }
    Runtime::getRuntime().halt(1); // signal failure to caller
  }
}

TestCodecLoadingDeadlock::NamedThreadFactoryAnonymousInnerClass::
    NamedThreadFactoryAnonymousInnerClass(
        shared_ptr<CopyOnWriteArrayList<std::shared_ptr<Thread>>> allThreads)
    : org::apache::lucene::util::NamedThreadFactory(L"deadlockchecker")
{
  this->allThreads = allThreads;
}

shared_ptr<Thread>
TestCodecLoadingDeadlock::NamedThreadFactoryAnonymousInnerClass::newThread(
    Runnable r)
{
  shared_ptr<Thread> t = outerInstance->super.newThread(r);
  allThreads->add(t);
  return t;
}
} // namespace org::apache::lucene::codecs