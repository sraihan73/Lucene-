using namespace std;

#include "WithNestedTests.h"

namespace org::apache::lucene::util
{
using FailureMarker = org::apache::lucene::util::FailureMarker;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestRuleIgnoreAfterMaxFailures =
    org::apache::lucene::util::TestRuleIgnoreAfterMaxFailures;
using TestRuleIgnoreTestSuites =
    org::apache::lucene::util::TestRuleIgnoreTestSuites;
using TestRuleMarkFailure = org::apache::lucene::util::TestRuleMarkFailure;
using TestRuleRestoreSystemProperties =
    org::apache::lucene::util::TestRuleRestoreSystemProperties;
using com::carrotsearch::randomizedtesting::RandomizedRunner;
using com::carrotsearch::randomizedtesting::RandomizedTest;
using com::carrotsearch::randomizedtesting::SysGlobals;
using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using org::junit::After;
using org::junit::Assert;
using org::junit::Assume;
using org::junit::Before;
using org::junit::ClassRule;
using org::junit::Rule;
using org::junit::rules::RuleChain;
using org::junit::rules::TestRule;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;

bool WithNestedTests::AbstractNestedTest::isRunningNested()
{
  return TestRuleIgnoreTestSuites::isRunningNested();
}

WithNestedTests::WithNestedTests(bool suppressOutputStreams)
{
  InitializeInstanceFields();
  this->suppressOutputStreams = suppressOutputStreams;
}

const shared_ptr<org::junit::rules::TestRule> WithNestedTests::classRules =
    org::junit::rules::RuleChain::outerRule(
        make_shared<TestRuleAdapterAnonymousInnerClass>());

WithNestedTests::TestRuleAdapterAnonymousInnerClass::
    TestRuleAdapterAnonymousInnerClass()
{
}

void WithNestedTests::TestRuleAdapterAnonymousInnerClass::before() throw(
    runtime_error)
{
  if (!isPropertyEmpty(SysGlobals::SYSPROP_TESTFILTER()) ||
      !isPropertyEmpty(SysGlobals::SYSPROP_TESTCLASS()) ||
      !isPropertyEmpty(SysGlobals::SYSPROP_TESTMETHOD()) ||
      !isPropertyEmpty(SysGlobals::SYSPROP_ITERATIONS())) {
    // We're running with a complex test filter that is properly handled by
    // classes which are executed by RandomizedRunner. The "outer" classes
    // testing LuceneTestCase itself are executed by the default JUnit runner
    // and would be always executed. We thus always skip execution if any
    // filtering is detected.
    Assume::assumeTrue(false);
  }

  // Check zombie threads from previous suites. Don't run if zombies are around.
  RandomizedTest::assumeFalse(RandomizedRunner::hasZombieThreads());

  shared_ptr<TestRuleIgnoreAfterMaxFailures> newRule =
      make_shared<TestRuleIgnoreAfterMaxFailures>(numeric_limits<int>::max());
  prevRule = LuceneTestCase::replaceMaxFailureRule(newRule);
  RandomizedTest::assumeFalse(FailureMarker::hadFailures());
}

void WithNestedTests::TestRuleAdapterAnonymousInnerClass::afterAlways(
    deque<runtime_error> &errors) 
{
  if (prevRule != nullptr) {
    LuceneTestCase::replaceMaxFailureRule(prevRule);
  }
  FailureMarker::resetFailures();
}

bool WithNestedTests::TestRuleAdapterAnonymousInnerClass::isPropertyEmpty(
    const wstring &propertyName)
{
  wstring value = System::getProperty(propertyName);
  return value == L"" || StringHelper::trim(value)->isEmpty();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public final void before()
void WithNestedTests::before()
{
  if (suppressOutputStreams) {
    prevSysOut = System::out;
    prevSysErr = System::err;

    try {
      sysout = make_shared<ByteArrayOutputStream>();
      System::setOut(make_shared<PrintStream>(sysout, true, IOUtils::UTF_8));
      syserr = make_shared<ByteArrayOutputStream>();
      System::setErr(make_shared<PrintStream>(syserr, true, IOUtils::UTF_8));
    } catch (const UnsupportedEncodingException &e) {
      throw runtime_error(e);
    }
  }

  FailureMarker::resetFailures();
  System::setProperty(TestRuleIgnoreTestSuites::PROPERTY_RUN_NESTED, L"true");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public final void after()
void WithNestedTests::after()
{
  if (suppressOutputStreams) {
    System::out::flush();
    System::err::flush();

    System::setOut(prevSysOut);
    System::setErr(prevSysErr);
  }
}

void WithNestedTests::assertFailureCount(int expected,
                                         shared_ptr<Result> result)
{
  if (result->getFailureCount() != expected) {
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (shared_ptr<Failure> f : result->getFailures()) {
      b->append(L"\n\n");
      b->append(f->getMessage());
      b->append(L"\n");
      b->append(f->getTrace());
    }
    Assert::assertFalse(L"Expected failures: " + to_wstring(expected) +
                            L" but was " + result->getFailureCount() +
                            L", failures below: " + b->toString(),
                        true);
  }
}

wstring WithNestedTests::getSysOut()
{
  Assert::assertTrue(suppressOutputStreams);
  System::out::flush();
  return wstring(sysout->toByteArray(), StandardCharsets::UTF_8);
}

wstring WithNestedTests::getSysErr()
{
  Assert::assertTrue(suppressOutputStreams);
  System::err::flush();
  return wstring(syserr->toByteArray(), StandardCharsets::UTF_8);
}

void WithNestedTests::InitializeInstanceFields()
{
  shared_ptr<TestRuleMarkFailure> *const marker =
      make_shared<TestRuleMarkFailure>();
  rules = RuleChain::outerRule(make_shared<TestRuleRestoreSystemProperties>(TestRuleIgnoreTestSuites::PROPERTY_RUN_NESTED)).around(make_shared<TestRuleAdapter>();
     protected:
         void afterAlways(deque<runtime_error> errors) throws runtime_error;
        if (marker->hadFailures() && suppressOutputStreams);
        {
        wcout << L"sysout from nested test: " << getSysOut() << L"\n" << endl;
        }
        wcout << L"syserr from nested test: " << getSysErr() << endl;
        ).around(marker);
}
} // namespace org::apache::lucene::util