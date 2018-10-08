using namespace std;

#include "TestIndexWriterOnJRECrash.h"

namespace org::apache::lucene::index
{
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Constants = org::apache::lucene::util::Constants;
using org::apache::lucene::util::SuppressForbidden;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::SeedUtils;

void TestIndexWriterOnJRECrash::setUp() 
{
  TestNRTThreads::setUp();
  assumeFalse(L"This test fails on UNIX with Turkish default locale "
              L"(https://issues.apache.org/jira/browse/LUCENE-6036)",
              (make_shared<Locale>(L"tr"))
                  ->getLanguage()
                  .equals(Locale::getDefault().getLanguage()));
  tempDir = createTempDir(L"jrecrash");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Nightly public void testNRTThreads() throws
// Exception
void TestIndexWriterOnJRECrash::testNRTThreads() 
{
  // if we are not the fork
  if (System::getProperty(L"tests.crashmode") == nullptr) {
    // try up to 10 times to create an index
    for (int i = 0; i < 10; i++) {
      forkTest();
      // if we succeeded in finding an index, we are done.
      if (checkIndexes(tempDir)) {
        return;
      }
    }
  } else {
    // note: re-enable this if we create a 4.x impersonator,
    // and if its format is actually different than the real 4.x (unlikely)
    // TODO: the non-fork code could simply enable impersonation?
    // assumeFalse("does not support PreFlex, see LUCENE-3992",
    //    Codec.getDefault().getName().equals("Lucene4x"));

    // we are the fork, setup a crashing thread
    constexpr int crashTime = TestUtil::nextInt(random(), 3000, 4000);
    shared_ptr<Thread> t =
        make_shared<ThreadAnonymousInnerClass>(shared_from_this(), crashTime);
    t->setPriority(Thread::MAX_PRIORITY);
    t->start();
    // run the test until we crash.
    for (int i = 0; i < 1000; i++) {
      TestNRTThreads::testNRTThreads();
    }
  }
}

TestIndexWriterOnJRECrash::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexWriterOnJRECrash> outerInstance, int crashTime)
{
  this->outerInstance = outerInstance;
  this->crashTime = crashTime;
}

void TestIndexWriterOnJRECrash::ThreadAnonymousInnerClass::run()
{
  try {
    delay(crashTime);
  } catch (const InterruptedException &e) {
  }
  outerInstance->crashJRE();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "ProcessBuilder requires
// java.io.File for CWD") public void forkTest() throws Exception
void TestIndexWriterOnJRECrash::forkTest() 
{
  deque<wstring> cmd = deque<wstring>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  cmd.push_back(Paths->get(System::getProperty(L"java.home"), L"bin", L"java")
                    ->toString());
  cmd.push_back(L"-Xmx512m");
  cmd.push_back(L"-Dtests.crashmode=true");
  // passing NIGHTLY to this test makes it run for much longer, easier to catch
  // it in the act...
  cmd.push_back(L"-Dtests.nightly=true");
  cmd.push_back(L"-DtempDir=" + tempDir);
  cmd.push_back(L"-Dtests.seed=" + SeedUtils::formatSeed(random()->nextLong()));
  cmd.push_back(L"-ea");
  cmd.push_back(L"-cp");
  cmd.push_back(System::getProperty(L"java.class.path"));
  cmd.push_back(L"org.junit.runner.JUnitCore");
  cmd.push_back(getClassName());
  shared_ptr<ProcessBuilder> pb = (make_shared<ProcessBuilder>(cmd))
                                      ->directory(tempDir->toFile())
                                      .redirectInput(Redirect::INHERIT)
                                      .redirectErrorStream(true);
  shared_ptr<Process> p = pb->start();

  // We pump everything to stderr.
  shared_ptr<PrintStream> childOut = System::err;
  shared_ptr<Thread> stdoutPumper =
      ThreadPumper::start(p->getInputStream(), childOut);
  if (VERBOSE) {
    childOut->println(L">>> Begin subprocess output");
  }
  p->waitFor();
  stdoutPumper->join();
  if (VERBOSE) {
    childOut->println(L"<<< End subprocess output");
  }
}

shared_ptr<Thread>
TestIndexWriterOnJRECrash::ThreadPumper::start(shared_ptr<InputStream> from,
                                               shared_ptr<OutputStream> to)
{
  shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass2>(from, to);
  t->start();
  return t;
}

TestIndexWriterOnJRECrash::ThreadPumper::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(shared_ptr<InputStream> from,
                               shared_ptr<OutputStream> to)
{
  this->from = from;
  this->to = to;
}

void TestIndexWriterOnJRECrash::ThreadPumper::ThreadAnonymousInnerClass2::run()
{
  try {
    std::deque<char> buffer(1024);
    int len;
    while ((len = from->read(buffer)) != -1) {
      if (VERBOSE) {
        to->write(buffer, 0, len);
      }
    }
  } catch (const IOException &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    System::err::println(L"Couldn't pipe from the forked process: " +
                         e->toString());
  }
}

bool TestIndexWriterOnJRECrash::checkIndexes(shared_ptr<Path> path) throw(
    IOException)
{
  shared_ptr<AtomicBoolean> *const found = make_shared<AtomicBoolean>();
  Files::walkFileTree(path, make_shared<SimpleFileVisitorAnonymousInnerClass>(
                                shared_from_this(), found));
  return found->get();
}

TestIndexWriterOnJRECrash::SimpleFileVisitorAnonymousInnerClass::
    SimpleFileVisitorAnonymousInnerClass(
        shared_ptr<TestIndexWriterOnJRECrash> outerInstance,
        shared_ptr<AtomicBoolean> found)
{
  this->outerInstance = outerInstance;
  this->found = found;
}

shared_ptr<FileVisitResult>
TestIndexWriterOnJRECrash::SimpleFileVisitorAnonymousInnerClass::
    postVisitDirectory(shared_ptr<Path> dirPath,
                       shared_ptr<IOException> exc) 
{
  if (exc != nullptr) {
    throw exc;
  } else {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try
    // (org.apache.lucene.store.BaseDirectoryWrapper dir =
    // newFSDirectory(dirPath))
    {
      org::apache::lucene::store::BaseDirectoryWrapper dir =
          newFSDirectory(dirPath);
      dir->setCheckIndexOnClose(false); // don't double-checkindex
      if (DirectoryReader::indexExists(dir)) {
        if (VERBOSE) {
          System::err::println(L"Checking index: " + dirPath);
        }
        // LUCENE-4738: if we crashed while writing first
        // commit it's possible index will be corrupt (by
        // design we don't try to be smart about this case
        // since that too risky):
        if (SegmentInfos::getLastCommitGeneration(dir) > 1) {
          TestUtil::checkIndex(dir);
        }
        found->set(true);
      }
    }
    return FileVisitResult::CONTINUE;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "We need Unsafe to actually crush
// :-)") public void crashJRE()
void TestIndexWriterOnJRECrash::crashJRE()
{
  const wstring vendor = Constants::JAVA_VENDOR;
  constexpr bool supportsUnsafeNpeDereference =
      StringHelper::startsWith(vendor, L"Oracle") ||
      StringHelper::startsWith(vendor, L"Sun") ||
      StringHelper::startsWith(vendor, L"Apple");

  try {
    if (supportsUnsafeNpeDereference) {
      try {
        type_info clazz = type_info::forName(L"sun.misc.Unsafe");
        shared_ptr<Field> field = clazz.getDeclaredField(L"theUnsafe");
        field->setAccessible(true);
        any o = field->get(nullptr);
        shared_ptr<Method> m = clazz.getMethod(
            L"putAddress", int64_t ::typeid, int64_t ::typeid);
        m->invoke(o, 0LL, 0LL);
      } catch (const runtime_error &e) {
        wcout << L"Couldn't kill the JVM via Unsafe." << endl;
        e.printStackTrace(System::out);
      }
    }

    // Fallback attempt to Runtime.halt();
    Runtime::getRuntime().halt(-1);
  } catch (const runtime_error &e) {
    wcout << L"Couldn't kill the JVM." << endl;
    e.printStackTrace(System::out);
  }

  // We couldn't get the JVM to crash for some reason.
  fail();
}
} // namespace org::apache::lucene::index