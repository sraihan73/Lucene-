using namespace std;

#include "TestRuleLimitSysouts.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::RandomizedTest;
using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
const shared_ptr<java::util::concurrent::atomic::AtomicInteger>
    TestRuleLimitSysouts::bytesWritten =
        make_shared<java::util::concurrent::atomic::AtomicInteger>();
const shared_ptr<DelegateStream> TestRuleLimitSysouts::capturedSystemOut;
const shared_ptr<DelegateStream> TestRuleLimitSysouts::capturedSystemErr;

TestRuleLimitSysouts::StaticConstructor::StaticConstructor()
{
  System::out::flush();
  System::err::flush();

  const wstring csn = Charset::defaultCharset().name();
  capturedSystemOut =
      make_shared<DelegateStream>(System::out, csn, bytesWritten);
  capturedSystemErr =
      make_shared<DelegateStream>(System::err, csn, bytesWritten);

  System::setOut(capturedSystemOut->printStream);
  System::setErr(capturedSystemErr->printStream);
}

TestRuleLimitSysouts::StaticConstructor TestRuleLimitSysouts::staticConstructor;

TestRuleLimitSysouts::DelegateStream::DelegateStream(
    shared_ptr<OutputStream> delegate_, const wstring &charset,
    shared_ptr<AtomicInteger> bytesCounter)
    : java::io::FilterOutputStream(delegate_)
{
  try {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    this->printStream =
        make_shared<PrintStream>(shared_from_this(), true, charset);
    this->bytesCounter = bytesCounter;
  } catch (const UnsupportedEncodingException &e) {
    throw runtime_error(e);
  }
}

void TestRuleLimitSysouts::DelegateStream::write(std::deque<char> &b) throw(
    IOException)
{
  if (b.size() > 0) {
    bytesCounter->addAndGet(b.size());
  }
  FilterOutputStream::write(b);
}

void TestRuleLimitSysouts::DelegateStream::write(std::deque<char> &b, int off,
                                                 int len) 
{
  if (len > 0) {
    bytesCounter->addAndGet(len);
  }
  FilterOutputStream::write(b, off, len);
}

void TestRuleLimitSysouts::DelegateStream::write(int b) 
{
  bytesCounter->incrementAndGet();
  FilterOutputStream::write(b);
}

TestRuleLimitSysouts::TestRuleLimitSysouts(
    shared_ptr<TestRuleMarkFailure> failureMarker)
    : failureMarker(failureMarker)
{
}

void TestRuleLimitSysouts::before() 
{
  if (isEnforced()) {
    checkCaptureStreams();
  }
  resetCaptureState();
  validateClassAnnotations();
}

void TestRuleLimitSysouts::validateClassAnnotations()
{
  type_info target = RandomizedTest::getContext().getTargetClass();
  if (target.isAnnotationPresent(Limit::typeid)) {
    int bytes = target.getAnnotation(Limit::typeid).bytes();
    if (bytes < 0 || bytes > 1 * 1024 * 1024) {
      throw make_shared<AssertionError>(
          wstring(L"The sysout limit is insane. Did you want to use ") + L"@" +
          LuceneTestCase::SuppressSysoutChecks::typeid->getName() +
          L" annotation to " + L"avoid sysout checks entirely?");
    }
  }
}

void TestRuleLimitSysouts::checkCaptureStreams()
{
  // Make sure we still hold the right references to wrapper streams.
  if (System::out != capturedSystemOut->printStream) {
    throw make_shared<AssertionError>(L"Something has changed System.out to: " +
                                      System::out::getClassName());
  }
  if (System::err != capturedSystemErr->printStream) {
    throw make_shared<AssertionError>(L"Something has changed System.err to: " +
                                      System::err::getClassName());
  }
}

bool TestRuleLimitSysouts::isEnforced()
{
  type_info target = RandomizedTest::getContext().getTargetClass();

  if (LuceneTestCase::VERBOSE || LuceneTestCase::INFOSTREAM ||
      target.isAnnotationPresent(Monster::typeid) ||
      target.isAnnotationPresent(SuppressSysoutChecks::typeid)) {
    return false;
  }

  if (!target.isAnnotationPresent(Limit::typeid)) {
    return false;
  }

  return true;
}

void TestRuleLimitSysouts::afterIfSuccessful() 
{
  if (isEnforced()) {
    checkCaptureStreams();

    // Flush any buffers.
    capturedSystemOut->printStream->flush();
    capturedSystemErr->printStream->flush();

    // Check for offenders, but only if everything was successful so far.
    int limit = RandomizedTest::getContext()
                    .getTargetClass()
                    .getAnnotation(Limit::typeid)
                    .bytes();
    if (bytesWritten->get() >= limit && failureMarker->wasSuccessful()) {
      throw make_shared<AssertionError>(wstring::format(
          Locale::ENGLISH,
          wstring(L"The test or suite printed %d bytes to stdout and stderr,") +
              L" even though the limit was set to %d bytes. Increase the limit "
              L"with @%s, ignore it completely" +
              L" with @%s or run with -Dtests.verbose=true",
          bytesWritten->get(), limit, Limit::typeid->getSimpleName(),
          SuppressSysoutChecks::typeid->getSimpleName()));
    }
  }
}

void TestRuleLimitSysouts::afterAlways(deque<runtime_error> &errors) throw(
    runtime_error)
{
  resetCaptureState();
}

void TestRuleLimitSysouts::resetCaptureState()
{
  capturedSystemOut->printStream->flush();
  capturedSystemErr->printStream->flush();
  bytesWritten->set(0);
}
} // namespace org::apache::lucene::util