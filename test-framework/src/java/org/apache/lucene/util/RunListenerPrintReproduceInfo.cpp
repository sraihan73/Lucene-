using namespace std;

#include "RunListenerPrintReproduceInfo.h"

namespace org::apache::lucene::util
{
using namespace org::apache::lucene::util;
//    import static org.apache.lucene.util.LuceneTestCase.*;
using com::carrotsearch::randomizedtesting::LifecycleScope;
using com::carrotsearch::randomizedtesting::RandomizedContext;
using org::junit::runner::Description;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;
using org::junit::runner::notification::RunListener;
deque<wstring> RunListenerPrintReproduceInfo::testClassesRun =
    deque<wstring>();

void RunListenerPrintReproduceInfo::testRunStarted(
    shared_ptr<Description> description) 
{
  suiteFailed = false;
  testFailed = false;
  scope = LifecycleScope::SUITE;

  type_info targetClass = RandomizedContext::current().getTargetClass();
  suppressReproduceLine = targetClass.isAnnotationPresent(
      LuceneTestCase::SuppressReproduceLine::typeid);
  testClassesRun.push_back(targetClass.name());
}

void RunListenerPrintReproduceInfo::testStarted(
    shared_ptr<Description> description) 
{
  this->testFailed = false;
  this->scope = LifecycleScope::TEST;
}

void RunListenerPrintReproduceInfo::testFailure(
    shared_ptr<Failure> failure) 
{
  if (scope == LifecycleScope::TEST) {
    testFailed = true;
  } else {
    suiteFailed = true;
  }
  printDiagnosticsAfterClass = true;
}

void RunListenerPrintReproduceInfo::testFinished(
    shared_ptr<Description> description) 
{
  if (testFailed) {
    reportAdditionalFailureInfo(
        stripTestNameAugmentations(description->getMethodName()));
  }
  scope = LifecycleScope::SUITE;
  testFailed = false;
}

wstring RunListenerPrintReproduceInfo::stripTestNameAugmentations(
    const wstring &methodName)
{
  if (methodName != L"") {
    methodName = methodName.replaceAll(L"\\s*\\{.+?\\}", L"");
  }
  return methodName;
}

void RunListenerPrintReproduceInfo::testRunFinished(
    shared_ptr<Result> result) 
{
  if (printDiagnosticsAfterClass || LuceneTestCase::VERBOSE) {
    RunListenerPrintReproduceInfo::printDebuggingInformation();
  }

  if (suiteFailed) {
    reportAdditionalFailureInfo(L"");
  }
}

void RunListenerPrintReproduceInfo::printDebuggingInformation()
{
  if (LuceneTestCase::classEnvRule != nullptr &&
      LuceneTestCase::classEnvRule->isInitialized()) {
    System::err::println(
        L"NOTE: test params are: codec=" + LuceneTestCase::classEnvRule->codec +
        L", sim=" + LuceneTestCase::classEnvRule->similarity + L", locale=" +
        LuceneTestCase::classEnvRule->locale->toLanguageTag() + L", timezone=" +
        StringHelper::toString(
            LuceneTestCase::classEnvRule->timeZone == nullptr
                ? L"(null)"
                : LuceneTestCase::classEnvRule->timeZone->getID()));
  }
  System::err::println(L"NOTE: " + System::getProperty(L"os.name") + L" " +
                       System::getProperty(L"os.version") + L" " +
                       System::getProperty(L"os.arch") + L"/" +
                       System::getProperty(L"java.vendor") + L" " +
                       System::getProperty(L"java.version") + L" " +
                       (Constants::JRE_IS_64BIT ? L"(64-bit)" : L"(32-bit)") +
                       L"/" + L"cpus=" +
                       Runtime::getRuntime().availableProcessors() + L"," +
                       L"threads=" + Thread::activeCount() + L"," + L"free=" +
                       Runtime::getRuntime().freeMemory() + L"," + L"total=" +
                       Runtime::getRuntime().totalMemory());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  System::err::println(L"NOTE: All tests run in this JVM: " +
                       Arrays->toString(testClassesRun.toArray()));
}

void RunListenerPrintReproduceInfo::reportAdditionalFailureInfo(
    const wstring &testName)
{
  if (suppressReproduceLine) {
    return;
  }
  if (StringHelper::endsWith(LuceneTestCase::TEST_LINE_DOCS_FILE,
                             LuceneTestCase::JENKINS_LARGE_LINE_DOCS_FILE)) {
    System::err::println(
        wstring(
            L"NOTE: download the large Jenkins line-docs file by running ") +
        L"'ant get-jenkins-line-docs' in the lucene directory.");
  }

  shared_ptr<StringBuilder> *const b = make_shared<StringBuilder>();
  b->append(L"NOTE: reproduce with: ant test ");

  // Test case, method, seed.
  addVmOpt(b, L"testcase",
           RandomizedContext::current().getTargetClass().getSimpleName());
  addVmOpt(b, L"tests.method", testName);
  addVmOpt(b, L"tests.seed",
           RandomizedContext::current().getRunnerSeedAsString());

  // Test groups and multipliers.
  if (LuceneTestCase::RANDOM_MULTIPLIER > 1) {
    addVmOpt(b, L"tests.multiplier", LuceneTestCase::RANDOM_MULTIPLIER);
  }
  if (LuceneTestCase::TEST_NIGHTLY) {
    addVmOpt(b, LuceneTestCase::SYSPROP_NIGHTLY, LuceneTestCase::TEST_NIGHTLY);
  }
  if (LuceneTestCase::TEST_WEEKLY) {
    addVmOpt(b, LuceneTestCase::SYSPROP_WEEKLY, LuceneTestCase::TEST_WEEKLY);
  }
  if (LuceneTestCase::TEST_SLOW) {
    addVmOpt(b, LuceneTestCase::SYSPROP_SLOW, LuceneTestCase::TEST_SLOW);
  }
  if (LuceneTestCase::TEST_MONSTER) {
    addVmOpt(b, LuceneTestCase::SYSPROP_MONSTER, LuceneTestCase::TEST_MONSTER);
  }
  if (LuceneTestCase::TEST_AWAITSFIX) {
    addVmOpt(b, LuceneTestCase::SYSPROP_AWAITSFIX,
             LuceneTestCase::TEST_AWAITSFIX);
  }
  if (LuceneTestCase::TEST_BADAPPLES) {
    addVmOpt(b, LuceneTestCase::SYSPROP_BADAPPLES,
             LuceneTestCase::TEST_BADAPPLES);
  }

  // Codec, postings, directories.
  if (LuceneTestCase::TEST_CODEC != L"random") {
    addVmOpt(b, L"tests.codec", LuceneTestCase::TEST_CODEC);
  }
  if (LuceneTestCase::TEST_POSTINGSFORMAT != L"random") {
    addVmOpt(b, L"tests.postingsformat", LuceneTestCase::TEST_POSTINGSFORMAT);
  }
  if (LuceneTestCase::TEST_DOCVALUESFORMAT != L"random") {
    addVmOpt(b, L"tests.docvaluesformat", LuceneTestCase::TEST_DOCVALUESFORMAT);
  }
  if (LuceneTestCase::TEST_DIRECTORY != L"random") {
    addVmOpt(b, L"tests.directory", LuceneTestCase::TEST_DIRECTORY);
  }

  // Environment.
  if (LuceneTestCase::TEST_LINE_DOCS_FILE !=
      LuceneTestCase::DEFAULT_LINE_DOCS_FILE) {
    addVmOpt(b, L"tests.linedocsfile", LuceneTestCase::TEST_LINE_DOCS_FILE);
  }
  if (LuceneTestCase::classEnvRule != nullptr &&
      LuceneTestCase::classEnvRule->isInitialized()) {
    addVmOpt(b, L"tests.locale",
             LuceneTestCase::classEnvRule->locale->toLanguageTag());
    if (LuceneTestCase::classEnvRule->timeZone != nullptr) {
      addVmOpt(b, L"tests.timezone",
               LuceneTestCase::classEnvRule->timeZone->getID());
    }
  }

  if (LuceneTestCase::assertsAreEnabled) {
    addVmOpt(b, L"tests.asserts", L"true");
  } else {
    addVmOpt(b, L"tests.asserts", L"false");
  }

  addVmOpt(b, L"tests.file.encoding", System::getProperty(L"file.encoding"));

  System::err::println(b->toString());
}

void RunListenerPrintReproduceInfo::addVmOpt(shared_ptr<StringBuilder> b,
                                             const wstring &key, any value)
{
  if (value == nullptr) {
    return;
  }

  b->append(L" -D")->append(key)->append(L"=");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring v = value.toString();
  // Add simplistic quoting. This varies a lot from system to system and between
  // shells... ANT should have some code for doing it properly.
  if (Pattern::compile(L"[\\s=']").matcher(v).find()) {
    v = StringHelper::toString(L'"') + v + StringHelper::toString(L'"');
  }
  b->append(v);
}
} // namespace org::apache::lucene::util