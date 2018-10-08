#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/QueryCachingPolicy.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleStoreClassName.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleSetupAndRestoreClassEnv.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleMarkFailure.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleTemporaryFilesCleanup.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleIgnoreAfterMaxFailures.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleSetupTeardownChained.h"
#include  "core/src/java/org/apache/lucene/util/TestRuleThreadAndTestName.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/search/QueryCache.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Store = org::apache::lucene::document::Field::Store;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using namespace org::apache::lucene::index;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using QueryCache = org::apache::lucene::search::QueryCache;
using QueryCachingPolicy = org::apache::lucene::search::QueryCachingPolicy;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using LockFactory = org::apache::lucene::store::LockFactory;
using Throttling = org::apache::lucene::store::MockDirectoryWrapper::Throttling;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using org::junit::Assert;
using org::junit::rules::RuleChain;
using org::junit::rules::TestRule;

using com::carrotsearch::randomizedtesting::annotations::TestGroup;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.systemPropertyAsBoolean;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.systemPropertyAsInt;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Base class for all Lucene unit tests, Junit3 or Junit4 variant.
 *
 * <h3>Class and instance setup.</h3>
 *
 * <p>
 * The preferred way to specify class (suite-level) setup/cleanup is to use
 * static methods annotated with {@link BeforeClass} and {@link AfterClass}. Any
GET_CLASS_NAME((suite-level))
 * code in these methods is executed within the test framework's control and
 * ensure proper setup has been made. <b>Try not to use static initializers
 * (including complex final field initializers).</b> Static initializers are
 * executed before any setup rules are fired and may cause you (or somebody
 * else) headaches.
 *
 * <p>
 * For instance-level setup, use {@link Before} and {@link After} annotated
 * methods. If you override either {@link #setUp()} or {@link #tearDown()} in
 * your subclass, make sure you call <code>super.setUp()</code> and
 * <code>super.tearDown()</code>. This is detected and enforced.
 *
 * <h3>Specifying test cases</h3>
 *
 * <p>
 * Any test method with a <code>testXXX</code> prefix is considered a test case.
 * Any test method annotated with {@link Test} is considered a test case.
 *
 * <h3>Randomized execution and test facilities</h3>
 *
 * <p>
 * {@link LuceneTestCase} uses {@link RandomizedRunner} to execute test cases.
 * {@link RandomizedRunner} has built-in support for tests randomization
 * including access to a repeatable {@link Random} instance. See
 * {@link #random()} method. Any test using {@link Random} acquired from
 * {@link #random()} should be fully reproducible (assuming no race conditions
 * between threads etc.). The initial seed for a test case is reported in many
 * ways:
 * <ul>
 *   <li>as part of any exception thrown from its body (inserted as a dummy
stack
 *   trace entry),</li>
 *   <li>as part of the main thread executing the test case (if your test hangs,
 *   just dump the stack trace of all threads and you'll see the seed),</li>
 *   <li>the master seed can also be accessed manually by getting the current
 *   context ({@link RandomizedContext#current()}) and then calling
 *   {@link RandomizedContext#getRunnerSeedAsString()}.</li>
 * </ul>
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @RunWith(RandomizedRunner.class) @TestMethodProviders({
// LuceneJUnit3MethodProvider.class, JUnit4MethodProvider.class }) @Listeners({
// RunListenerPrintReproduceInfo.class, FailureMarker.class })
// @SeedDecorators({MixWithSuiteName.class}) @ThreadLeakScope(Scope.SUITE)
// @ThreadLeakGroup(Group.MAIN) @ThreadLeakAction({Action.WARN,
// Action.INTERRUPT}) @ThreadLeakLingering(linger = 20000)
// @ThreadLeakZombies(Consequence.IGNORE_REMAINING_TESTS) @TimeoutSuite(millis =
// 2 * TimeUnits.HOUR) @ThreadLeakFilters(defaultFilters = true, filters = {
// QuickPatchThreadsFilter.class }) @TestRuleLimitSysouts.Limit(bytes =
// TestRuleLimitSysouts.DEFAULT_SYSOUT_BYTES_THRESHOLD) public abstract class
// LuceneTestCase extends org.junit.Assert
class LuceneTestCase : public Assert
{

  // --------------------------------------------------------------------
  // Test groups, system properties and other annotations modifying tests
  // --------------------------------------------------------------------

public:
  static const std::wstring SYSPROP_NIGHTLY;
  static const std::wstring SYSPROP_WEEKLY;
  static const std::wstring SYSPROP_MONSTER;
  static const std::wstring SYSPROP_AWAITSFIX;
  static const std::wstring SYSPROP_SLOW;
  static const std::wstring SYSPROP_BADAPPLES;

  /** @see #ignoreAfterMaxFailures*/
  static const std::wstring SYSPROP_MAXFAILURES;

  /** @see #ignoreAfterMaxFailures*/
  static const std::wstring SYSPROP_FAILFAST;

  /**
   * Annotation for tests that should only be run during nightly builds.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false, sysProperty = SYSPROP_NIGHTLY) public
  //      @interface Nightly
  //  {
  //  }
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false, sysProperty = SYSPROP_WEEKLY) public
  //      @interface Weekly
  //  {
  //  }
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false, sysProperty = SYSPROP_MONSTER) public
  //      @interface Monster
  //  {
  //    std::wstring value();
  //  }

  /**
   * Annotation for tests which exhibit a known issue and are temporarily
   * disabled.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false, sysProperty = SYSPROP_AWAITSFIX) public
  //      @interface AwaitsFix
  //  {
  // /** Point to JIRA entry. */
  //    public std::wstring bugUrl();
  //  }

  /**
   * Annotation for tests that are slow. Slow tests do run by default but can be
   * disabled if a quick run is needed.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = true, sysProperty = SYSPROP_SLOW) public
  //      @interface Slow
  //  {
  //  }
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = true, sysProperty = SYSPROP_BADAPPLES) public
  //      @interface BadApple
  //  {
  // /** Point to JIRA entry. */
  //    public std::wstring bugUrl();
  //  }

  /**
   * Annotation for test classes that should avoid certain codec types
   * (because they are expensive, for example).
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public @interface SuppressCodecs
  //  {
  //    std::wstring[] value();
  //  }

  /**
   * Annotation for test classes that should avoid mock filesystem types
   * (because they test a bug that only happens on linux, for example).
   * <p>
   * You can avoid specific names {@link Class#getSimpleName()} or use
   * the special value {@code *} to disable all mock filesystems.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public @interface SuppressFileSystems
  //  {
  //    std::wstring[] value();
  //  }

  /**
   * Annotation for test classes that should avoid always omit
   * actual fsync calls from reaching the filesystem.
   * <p>
   * This can be useful, e.g. if they make many lucene commits.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public @interface SuppressFsync
  //  {
  //  }
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public @interface SuppressTempFileChecks
  //  {
  // /** Point to JIRA entry. */
  //    public std::wstring bugUrl() default "None";
  //  }

  /**
   * Ignore {@link TestRuleLimitSysouts} for any suite which is known to print
   * over the default limit of bytes to {@link System#out} or {@link
   * System#err}.
   *
   * @see TestRuleLimitSysouts
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public @interface SuppressSysoutChecks
  //  {
  // /** Point to JIRA entry. */
  //    public std::wstring bugUrl();
  //  }

  /**
   * Suppress the default {@code reproduce with: ant test...}
   * Your own listener can be added as needed for your build.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public @interface SuppressReproduceLine
  //  {
  //  }

  // -----------------------------------------------------------------
  // Truly immutable fields and constants, initialized once and valid
  // for all suites ever since.
  // -----------------------------------------------------------------

  /**
   * True if and only if tests are run in verbose mode. If this flag is false
   * tests are not expected to print any messages. Enforced with {@link
   * TestRuleLimitSysouts}.
   */
  static const bool VERBOSE = systemPropertyAsBoolean(L"tests.verbose", false);

  /**
   * Enables or disables dumping of {@link InfoStream} messages.
   */
  static const bool INFOSTREAM =
      systemPropertyAsBoolean(L"tests.infostream", VERBOSE);

  /**
   * A random multiplier which you should use when writing random tests:
   * multiply it by the number of iterations to scale your tests (for nightly
   * builds).
   */
  static const int RANDOM_MULTIPLIER =
      systemPropertyAsInt(L"tests.multiplier", 1);

  static const bool TEST_ASSERTS_ENABLED =
      systemPropertyAsBoolean(L"tests.asserts", true);

  /** TODO: javadoc? */
  static const std::wstring DEFAULT_LINE_DOCS_FILE;

  /** TODO: javadoc? */
  static const std::wstring JENKINS_LARGE_LINE_DOCS_FILE;

  /** Gets the codec to run tests with. */
  static const std::wstring TEST_CODEC;

  /** Gets the postingsFormat to run tests with. */
  static const std::wstring TEST_POSTINGSFORMAT;

  /** Gets the docValuesFormat to run tests with */
  static const std::wstring TEST_DOCVALUESFORMAT;

  /** Gets the directory to run tests with */
  static const std::wstring TEST_DIRECTORY;

  /** the line file used by LineFileDocs */
  static const std::wstring TEST_LINE_DOCS_FILE;

  /** Whether or not {@link Nightly} tests should run. */
  static const bool TEST_NIGHTLY = systemPropertyAsBoolean(
      SYSPROP_NIGHTLY,
      Nightly::typeid->getAnnotation(TestGroup::typeid).enabled());

  /** Whether or not {@link Weekly} tests should run. */
  static const bool TEST_WEEKLY = systemPropertyAsBoolean(
      SYSPROP_WEEKLY,
      Weekly::typeid->getAnnotation(TestGroup::typeid).enabled());

  /** Whether or not {@link Monster} tests should run. */
  static const bool TEST_MONSTER = systemPropertyAsBoolean(
      SYSPROP_MONSTER,
      Monster::typeid->getAnnotation(TestGroup::typeid).enabled());

  /** Whether or not {@link AwaitsFix} tests should run. */
  static const bool TEST_AWAITSFIX = systemPropertyAsBoolean(
      SYSPROP_AWAITSFIX,
      AwaitsFix::typeid->getAnnotation(TestGroup::typeid).enabled());

  /** Whether or not {@link BadApple} tests should run. */
  static const bool TEST_BADAPPLES = systemPropertyAsBoolean(
      SYSPROP_BADAPPLES,
      BadApple::typeid->getAnnotation(TestGroup::typeid).enabled());

  /** Whether or not {@link Slow} tests should run. */
  static const bool TEST_SLOW = systemPropertyAsBoolean(
      SYSPROP_SLOW, Slow::typeid->getAnnotation(TestGroup::typeid).enabled());

  /** Throttling, see {@link MockDirectoryWrapper#setThrottling(Throttling)}. */
  static const Throttling TEST_THROTTLING;

  /** Leave temporary files on disk, even on successful runs. */
  static const bool LEAVE_TEMPORARY = false;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();

  private:
    class StaticFieldsInvariantRuleAnonymousInnerClass
        : public StaticFieldsInvariantRule
    {
      GET_CLASS_NAME(StaticFieldsInvariantRuleAnonymousInnerClass)
    private:
      StaticConstructor outerInstance;

    public:
      StaticFieldsInvariantRuleAnonymousInnerClass(
          StaticConstructor outerInstance);

    protected:
      bool accept(std::shared_ptr<java::lang::reflect::Field> field) override;

    protected:
      std::shared_ptr<StaticFieldsInvariantRuleAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            StaticFieldsInvariantRuleAnonymousInnerClass>(
            com.carrotsearch.randomizedtesting.rules
                .StaticFieldsInvariantRule::shared_from_this());
      }
    };

  private:
    class NoInstanceHooksOverridesRuleAnonymousInnerClass
        : public NoInstanceHooksOverridesRule
    {
      GET_CLASS_NAME(NoInstanceHooksOverridesRuleAnonymousInnerClass)
    private:
      StaticConstructor outerInstance;

    public:
      NoInstanceHooksOverridesRuleAnonymousInnerClass(
          StaticConstructor outerInstance);

    protected:
      bool verify(std::shared_ptr<Method> key) override;

    protected:
      std::shared_ptr<NoInstanceHooksOverridesRuleAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            NoInstanceHooksOverridesRuleAnonymousInnerClass>(
            com.carrotsearch.randomizedtesting.rules
                .NoInstanceHooksOverridesRule::shared_from_this());
      }
    };
  };

private:
  static LuceneTestCase::StaticConstructor staticConstructor;

  /** Returns true, if MMapDirectory supports unmapping on this platform
   * (required for Windows), or if we are not on Windows. */
public:
  static bool hasWorkingMMapOnWindows();

  /** Assumes that the current MMapDirectory implementation supports unmapping,
   * so the test will not fail on Windows.
   * @see #hasWorkingMMapOnWindows()
   * */
  static void assumeWorkingMMapOnWindows();

  /** Filesystem-based {@link Directory} implementations. */
private:
  static const std::deque<std::wstring> FS_DIRECTORIES;

  /** All {@link Directory} implementations. */
  static const std::deque<std::wstring> CORE_DIRECTORIES;

  /** A {@link org.apache.lucene.search.QueryCachingPolicy} that randomly
   * caches. */
public:
  static const std::shared_ptr<QueryCachingPolicy> MAYBE_CACHE_POLICY;

private:
  class QueryCachingPolicyAnonymousInnerClass
      : public std::enable_shared_from_this<
            QueryCachingPolicyAnonymousInnerClass>,
        public QueryCachingPolicy
  {
    GET_CLASS_NAME(QueryCachingPolicyAnonymousInnerClass)
  public:
    QueryCachingPolicyAnonymousInnerClass();

    void onUse(std::shared_ptr<Query> query) override;
    bool shouldCache(std::shared_ptr<Query> query)  override;
  };

  // -----------------------------------------------------------------
  // Fields initialized in class or instance rules.
  // -----------------------------------------------------------------

  // -----------------------------------------------------------------
  // Class level (suite) rules.
  // -----------------------------------------------------------------

  /**
   * Stores the currently class under test.
   */
private:
  static const std::shared_ptr<TestRuleStoreClassName> classNameRule;

  /**
   * Class environment setup rule.
   */
public:
  static const std::shared_ptr<TestRuleSetupAndRestoreClassEnv> classEnvRule;

  /**
   * Suite failure marker (any error in the test or suite scope).
   */
protected:
  static std::shared_ptr<TestRuleMarkFailure> suiteFailureMarker;

  /**
   * Temporary files cleanup rule.
   */
private:
  static std::shared_ptr<TestRuleTemporaryFilesCleanup> tempFilesCleanupRule;

  /**
   * Ignore tests after hitting a designated number of initial failures. This
   * is truly a "static" global singleton since it needs to span the lifetime of
   * all test classes running inside this JVM (it cannot be part of a class
   * rule).
   *
   * <p>This poses some problems for the test framework's tests because these
   * sometimes trigger intentional failures which add up to the global count.
   * This field contains a (possibly) changing reference to {@link
   * TestRuleIgnoreAfterMaxFailures} and we dispatch to its current value from
   * the {@link #classRules} chain using {@link TestRuleDelegate}.
   */
  static const std::shared_ptr<
      AtomicReference<std::shared_ptr<TestRuleIgnoreAfterMaxFailures>>>
      ignoreAfterMaxFailuresDelegate;
  static const std::shared_ptr<TestRule> ignoreAfterMaxFailures;

  /**
   * Try to capture streams early so that other classes don't have a chance to
   * steal references to them (as is the case with ju.logging handlers).
   */

  /**
   * Temporarily substitute the global {@link TestRuleIgnoreAfterMaxFailures}.
   * See
   * {@link #ignoreAfterMaxFailuresDelegate} for some explanation why this
   * method is needed.
   */
public:
  static std::shared_ptr<TestRuleIgnoreAfterMaxFailures> replaceMaxFailureRule(
      std::shared_ptr<TestRuleIgnoreAfterMaxFailures> newValue);

  /**
   * Max 10mb of static data stored in a test suite class after the suite is
   * complete. Prevents static data structures leaking and causing OOMs in
   * subsequent tests.
   */
private:
  static constexpr int64_t STATIC_LEAK_THRESHOLD = 10 * 1024 * 1024;

  /** By-name deque of ignored types like loggers etc. */
  static const std::shared_ptr<Set<std::wstring>> STATIC_LEAK_IGNORED_TYPES;

  /**
   * This controls how suite-level rules are nested. It is important that _all_
   * rules declared in {@link LuceneTestCase} are executed in proper order if
   * they depend on each other.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ClassRule public static org.junit.rules.TestRule
  // classRules;
  static std::shared_ptr<TestRule> classRules;

  // -----------------------------------------------------------------
  // Test level rules.
  // -----------------------------------------------------------------

  /** Enforces {@link #setUp()} and {@link #tearDown()} calls are chained. */
private:
  std::shared_ptr<TestRuleSetupTeardownChained> parentChainCallRule =
      std::make_shared<TestRuleSetupTeardownChained>();

  /** Save test thread and name. */
  std::shared_ptr<TestRuleThreadAndTestName> threadAndTestNameRule =
      std::make_shared<TestRuleThreadAndTestName>();

  /** Taint suite result with individual test failures. */
  std::shared_ptr<TestRuleMarkFailure> testFailureMarker =
      std::make_shared<TestRuleMarkFailure>(suiteFailureMarker);

  /**
   * This controls how individual test rules are nested. It is important that
   * _all_ rules declared in {@link LuceneTestCase} are executed in proper order
   * if they depend on each other.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Rule public final org.junit.rules.TestRule ruleChain =
  // org.junit.rules.RuleChain.outerRule(testFailureMarker).around(ignoreAfterMaxFailures).around(threadAndTestNameRule).around(new
  // TestRuleSetupAndRestoreInstanceEnv()).around(parentChainCallRule);
  const std::shared_ptr<TestRule> ruleChain =
      RuleChain::outerRule(testFailureMarker)
          .around(ignoreAfterMaxFailures)
          .around(threadAndTestNameRule)
          .around(std::make_shared<TestRuleSetupAndRestoreInstanceEnv>())
          .around(parentChainCallRule);

private:
  static const std::unordered_map<std::wstring, std::shared_ptr<FieldType>>
      fieldToType;

public:
  enum class LiveIWCFlushMode {
    GET_CLASS_NAME(LiveIWCFlushMode) BY_RAM,
    BY_DOCS,
    EITHER
  };

  /** Set by TestRuleSetupAndRestoreClassEnv */
public:
  static LiveIWCFlushMode liveIWCFlushMode;

  static void setLiveIWCFlushMode(LiveIWCFlushMode flushMode);

  // -----------------------------------------------------------------
  // Suite and test case setup/ cleanup.
  // -----------------------------------------------------------------

  /**
   * For subclasses to override. Overrides must call {@code super.setUp()}.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void setUp() throws Exception
  virtual void setUp() ;

  /**
   * For subclasses to override. Overrides must call {@code super.tearDown()}.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void tearDown() throws Exception
  virtual void tearDown() ;

  /** Tells {@link IndexWriter} to enforce the specified limit as the maximum
   * number of documents in one index; call
   *  {@link #restoreIndexWriterMaxDocs} once your test is done. */
  virtual void setIndexWriterMaxDocs(int limit);

  /** Returns to the default {@link IndexWriter#MAX_DOCS} limit. */
  virtual void restoreIndexWriterMaxDocs();

  // -----------------------------------------------------------------
  // Test facilities and facades for subclasses.
  // -----------------------------------------------------------------

  /**
   * Access to the current {@link RandomizedContext}'s Random instance. It is
   * safe to use this method from multiple threads, etc., but it should be
   * called while within a runner's scope (so no static initializers). The
   * returned {@link Random} instance will be <b>different</b> when this method
   * is called inside a {@link BeforeClass} hook (static suite scope) and within
   * {@link Before}/ {@link After} hooks or test methods.
   *
   * <p>The returned instance must not be shared with other threads or cross a
   * single scope's boundary. For example, a {@link Random} acquired within a
   * test method shouldn't be reused for another test case.
   *
   * <p>There is an overhead connected with getting the {@link Random} for a
   * particular context and thread. It is better to cache the {@link Random}
   * locally if tight loops with multiple invocations are present or create a
   * derivative local {@link Random} for millions of calls like this: <pre>
   * Random random = new Random(random().nextLong());
   * // tight loop with many invocations.
   * </pre>
   */
  static std::shared_ptr<Random> random();

  /**
   * Registers a {@link Closeable} resource that should be closed after the test
   * completes.
   *
   * @return <code>resource</code> (for call chaining).
   */
  template <typename T>
  T closeAfterTest(T resource);

  /**
   * Registers a {@link Closeable} resource that should be closed after the
   * suite completes.
   *
   * @return <code>resource</code> (for call chaining).
   */
  template <typename T>
  static T closeAfterSuite(T resource);

  /**
   * Return the current class being tested.
   */
  static std::type_info getTestClass();

  /**
   * Return the name of the currently executing test case.
   */
  virtual std::wstring getTestName();

  /**
   * Some tests expect the directory to contain a single segment, and want to
   * do tests on that segment's reader. This is an utility method to help them.
   */
  /*
public static SegmentReader getOnlySegmentReader(DirectoryReader reader) {
  List<LeafReaderContext> subReaders = reader.leaves();
  if (subReaders.size() != 1) {
    throw new IllegalArgumentException(reader + " has " + subReaders.size() + "
segments instead of exactly one");
  }
  final LeafReader r = subReaders.get(0).reader();
  assertTrue("expected a SegmentReader but got " + r, r instanceof
SegmentReader); return (SegmentReader) r;
}
  */

  /**
   * Some tests expect the directory to contain a single segment, and want to
   * do tests on that segment's reader. This is an utility method to help them.
   */
  static std::shared_ptr<LeafReader>
  getOnlyLeafReader(std::shared_ptr<IndexReader> reader);

  /**
   * Returns true if and only if the calling thread is the primary thread
   * executing the test case.
   */
protected:
  virtual bool isTestThread();

  /**
   * Returns a number of at least <code>i</code>
   * <p>
   * The actual number returned will be influenced by whether {@link
   * #TEST_NIGHTLY} is active and {@link #RANDOM_MULTIPLIER}, but also with some
   * random fudge.
   */
public:
  static int atLeast(std::shared_ptr<Random> random, int i);

  static int atLeast(int i);

  /**
   * Returns true if something should happen rarely,
   * <p>
   * The actual number returned will be influenced by whether {@link
   * #TEST_NIGHTLY} is active and {@link #RANDOM_MULTIPLIER}.
   */
  static bool rarely(std::shared_ptr<Random> random);

  static bool rarely();

  static bool usually(std::shared_ptr<Random> random);

  static bool usually();

  static void assumeTrue(const std::wstring &msg, bool condition);

  static void assumeFalse(const std::wstring &msg, bool condition);

  static void assumeNoException(const std::wstring &msg, std::runtime_error e);

  /**
   * Return <code>args</code> as a {@link Set} instance. The order of elements
   * is not preserved in iterators.
   */
  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SafeVarargs @SuppressWarnings("varargs") public static <T>
  // java.util.Set<T> asSet(T... args)
  static std::shared_ptr<Set<T>> asSet(std::deque<T> &args);

  /**
   * Convenience method for logging an iterator.
   *
   * @param label  std::wstring logged before/after the items in the iterator
   * @param iter   Each next() is toString()ed and logged on it's own line. If
   * iter is null this is logged differnetly then an empty iterator.
   * @param stream Stream to log messages to.
   */
  template <typename T1>
  static void dumpIterator(const std::wstring &label,
                           std::shared_ptr<Iterator<T1>> iter,
                           std::shared_ptr<PrintStream> stream);

  /**
   * Convenience method for logging an array.  Wraps the array in an iterator
   * and delegates
   *
   * @see #dumpIterator(std::wstring,Iterator,PrintStream)
   */
  static void dumpArray(const std::wstring &label, std::deque<std::any> &objs,
                        std::shared_ptr<PrintStream> stream);

  /** create a new index writer config with random defaults */
  static std::shared_ptr<IndexWriterConfig> newIndexWriterConfig();

  /** create a new index writer config with random defaults */
  static std::shared_ptr<IndexWriterConfig>
  newIndexWriterConfig(std::shared_ptr<Analyzer> a);

  /** create a new index writer config with random defaults using the specified
   * random */
  static std::shared_ptr<IndexWriterConfig>
  newIndexWriterConfig(std::shared_ptr<Random> r, std::shared_ptr<Analyzer> a);

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
  public:
    ConcurrentMergeSchedulerAnonymousInnerClass();

  protected:
    // C++ WARNING: The following method was originally marked 'synchronized':
    bool maybeStall(std::shared_ptr<IndexWriter> writer) override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

private:
  static void avoidPathologicalMerging(std::shared_ptr<IndexWriterConfig> iwc);

public:
  static std::shared_ptr<MergePolicy> newMergePolicy(std::shared_ptr<Random> r);

  static std::shared_ptr<MergePolicy> newMergePolicy(std::shared_ptr<Random> r,
                                                     bool includeMockMP);

  static std::shared_ptr<MergePolicy> newMergePolicy();

  static std::shared_ptr<LogMergePolicy> newLogMergePolicy();

  static std::shared_ptr<TieredMergePolicy> newTieredMergePolicy();

  static std::shared_ptr<AlcoholicMergePolicy> newAlcoholicMergePolicy();

  static std::shared_ptr<AlcoholicMergePolicy>
  newAlcoholicMergePolicy(std::shared_ptr<Random> r,
                          std::shared_ptr<TimeZone> tz);

  static std::shared_ptr<LogMergePolicy>
  newLogMergePolicy(std::shared_ptr<Random> r);

private:
  static void configureRandom(std::shared_ptr<Random> r,
                              std::shared_ptr<MergePolicy> mergePolicy);

public:
  static std::shared_ptr<TieredMergePolicy>
  newTieredMergePolicy(std::shared_ptr<Random> r);

  static std::shared_ptr<MergePolicy> newLogMergePolicy(bool useCFS);

  static std::shared_ptr<MergePolicy> newLogMergePolicy(bool useCFS,
                                                        int mergeFactor);

  static std::shared_ptr<MergePolicy> newLogMergePolicy(int mergeFactor);

  // if you want it in LiveIndexWriterConfig: it must and will be tested here.
  static void
  maybeChangeLiveIndexWriterConfig(std::shared_ptr<Random> r,
                                   std::shared_ptr<LiveIndexWriterConfig> c);

  /**
   * Returns a new Directory instance. Use this when the test does not
   * care about the specific Directory implementation (most tests).
   * <p>
   * The Directory is wrapped with {@link BaseDirectoryWrapper}.
   * this means usually it will be picky, such as ensuring that you
   * properly close it and all open files in your test. It will emulate
   * some features of Windows, such as not allowing open files to be
   * overwritten.
   */
  static std::shared_ptr<BaseDirectoryWrapper> newDirectory();

  /** Like {@link #newDirectory} except randomly the {@link VirusCheckingFS} may
   * be installed */
  static std::shared_ptr<BaseDirectoryWrapper> newMaybeVirusCheckingDirectory();

  /**
   * Returns a new Directory instance, using the specified random.
   * See {@link #newDirectory()} for more information.
   */
  static std::shared_ptr<BaseDirectoryWrapper>
  newDirectory(std::shared_ptr<Random> r);

  /**
   * Returns a new Directory instance, using the specified random.
   * See {@link #newDirectory()} for more information.
   */
  static std::shared_ptr<BaseDirectoryWrapper>
  newDirectory(std::shared_ptr<Random> r, std::shared_ptr<LockFactory> lf);

  static std::shared_ptr<MockDirectoryWrapper> newMockDirectory();

  static std::shared_ptr<MockDirectoryWrapper>
  newMockDirectory(std::shared_ptr<Random> r);

  static std::shared_ptr<MockDirectoryWrapper>
  newMockDirectory(std::shared_ptr<Random> r, std::shared_ptr<LockFactory> lf);

  static std::shared_ptr<MockDirectoryWrapper>
  newMockFSDirectory(std::shared_ptr<Path> f);

  static std::shared_ptr<MockDirectoryWrapper>
  newMockFSDirectory(std::shared_ptr<Path> f, std::shared_ptr<LockFactory> lf);

  static std::shared_ptr<Path> addVirusChecker(std::shared_ptr<Path> path);

  /**
   * Returns a new Directory instance, with contents copied from the
   * provided directory. See {@link #newDirectory()} for more
   * information.
   */
  static std::shared_ptr<BaseDirectoryWrapper>
  newDirectory(std::shared_ptr<Directory> d) ;

  /** Returns a new FSDirectory instance over the given file, which must be a
   * folder. */
  static std::shared_ptr<BaseDirectoryWrapper>
  newFSDirectory(std::shared_ptr<Path> f);

  /** Like {@link #newFSDirectory(Path)}, but randomly insert {@link
   * VirusCheckingFS} */
  static std::shared_ptr<BaseDirectoryWrapper>
  newMaybeVirusCheckingFSDirectory(std::shared_ptr<Path> f);

  /** Returns a new FSDirectory instance over the given file, which must be a
   * folder. */
  static std::shared_ptr<BaseDirectoryWrapper>
  newFSDirectory(std::shared_ptr<Path> f, std::shared_ptr<LockFactory> lf);

private:
  static std::shared_ptr<BaseDirectoryWrapper>
  newFSDirectory(std::shared_ptr<Path> f, std::shared_ptr<LockFactory> lf,
                 bool bare);

  /**
   * Returns a new Directory instance, using the specified random
   * with contents copied from the provided directory. See
   * {@link #newDirectory()} for more information.
   */
public:
  static std::shared_ptr<BaseDirectoryWrapper>
  newDirectory(std::shared_ptr<Random> r,
               std::shared_ptr<Directory> d) ;

private:
  static std::shared_ptr<BaseDirectoryWrapper>
  wrapDirectory(std::shared_ptr<Random> random,
                std::shared_ptr<Directory> directory, bool bare);

public:
  static std::shared_ptr<Field> newStringField(const std::wstring &name,
                                               const std::wstring &value,
                                               Store stored);

  static std::shared_ptr<Field> newStringField(const std::wstring &name,
                                               std::shared_ptr<BytesRef> value,
                                               Store stored);

  static std::shared_ptr<Field> newTextField(const std::wstring &name,
                                             const std::wstring &value,
                                             Store stored);

  static std::shared_ptr<Field> newStringField(std::shared_ptr<Random> random,
                                               const std::wstring &name,
                                               const std::wstring &value,
                                               Store stored);

  static std::shared_ptr<Field> newStringField(std::shared_ptr<Random> random,
                                               const std::wstring &name,
                                               std::shared_ptr<BytesRef> value,
                                               Store stored);

  static std::shared_ptr<Field> newTextField(std::shared_ptr<Random> random,
                                             const std::wstring &name,
                                             const std::wstring &value,
                                             Store stored);

  static std::shared_ptr<Field> newField(const std::wstring &name,
                                         const std::wstring &value,
                                         std::shared_ptr<FieldType> type);

  /** Returns a FieldType derived from newType but whose
   *  term deque options match the old type */
private:
  static std::shared_ptr<FieldType>
  mergeTermVectorOptions(std::shared_ptr<FieldType> newType,
                         std::shared_ptr<FieldType> oldType);

  // TODO: if we can pull out the "make term deque options
  // consistent across all instances of the same field name"
  // write-once schema sort of helper class then we can
  // remove the sync here.  We can also fold the random
  // "enable norms" (now commented out, below) into that:
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<Field> newField(std::shared_ptr<Random> random,
                                         const std::wstring &name,
                                         std::any value,
                                         std::shared_ptr<FieldType> type);

private:
  static std::shared_ptr<Field>
  createField(const std::wstring &name, std::any value,
              std::shared_ptr<FieldType> fieldType);

  static std::deque<std::wstring> const availableLanguageTags;

  /**
   * Return a random Locale from the available locales on the system.
   * @see <a
   * href="http://issues.apache.org/jira/browse/LUCENE-4020">LUCENE-4020</a>
   */
public:
  static std::shared_ptr<Locale> randomLocale(std::shared_ptr<Random> random);

  /**
   * Return a random TimeZone from the available timezones on the system
   * @see <a
   * href="http://issues.apache.org/jira/browse/LUCENE-4020">LUCENE-4020</a>
   */
  static std::shared_ptr<TimeZone>
  randomTimeZone(std::shared_ptr<Random> random);

  /** return a Locale object equivalent to its programmatic name */
  static std::shared_ptr<Locale>
  localeForLanguageTag(const std::wstring &languageTag);

private:
  static std::shared_ptr<Directory>
  newFSDirectoryImpl(std::type_info clazz, std::shared_ptr<Path> path,
                     std::shared_ptr<LockFactory> lf) ;

public:
  static std::shared_ptr<Directory>
  newDirectoryImpl(std::shared_ptr<Random> random,
                   const std::wstring &clazzName);

  static std::shared_ptr<Directory>
  newDirectoryImpl(std::shared_ptr<Random> random,
                   const std::wstring &clazzName,
                   std::shared_ptr<LockFactory> lf);

  static std::shared_ptr<IndexReader>
  wrapReader(std::shared_ptr<IndexReader> r) ;

  /**
   * Sometimes wrap the IndexReader as slow, parallel or filter reader (or
   * combinations of that)
   */
  static std::shared_ptr<IndexReader>
  maybeWrapReader(std::shared_ptr<IndexReader> r) ;

  /** TODO: javadoc */
  static std::shared_ptr<IOContext>
  newIOContext(std::shared_ptr<Random> random);

  /** TODO: javadoc */
  static std::shared_ptr<IOContext>
  newIOContext(std::shared_ptr<Random> random,
               std::shared_ptr<IOContext> oldContext);

private:
  static const std::shared_ptr<QueryCache> DEFAULT_QUERY_CACHE;
  static const std::shared_ptr<QueryCachingPolicy> DEFAULT_CACHING_POLICY;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void overrideTestDefaultQueryCache()
  virtual void overrideTestDefaultQueryCache();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void overrideDefaultQueryCache()
  static void overrideDefaultQueryCache();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void resetDefaultQueryCache()
  static void resetDefaultQueryCache();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void setupCPUCoreCount()
  static void setupCPUCoreCount();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void restoreCPUCoreCount()
  static void restoreCPUCoreCount();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void setupSpins()
  static void setupSpins();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void restoreSpins()
  static void restoreSpins();

  /**
   * Create a new searcher over the reader. This searcher might randomly use
   * threads.
   */
  static std::shared_ptr<IndexSearcher>
  newSearcher(std::shared_ptr<IndexReader> r);

  /**
   * Create a new searcher over the reader. This searcher might randomly use
   * threads.
   */
  static std::shared_ptr<IndexSearcher>
  newSearcher(std::shared_ptr<IndexReader> r, bool maybeWrap);

  /**
   * Create a new searcher over the reader. This searcher might randomly use
   * threads. if <code>maybeWrap</code> is true, this searcher might wrap the
   * reader with one that returns null for getSequentialSubReaders. If
   * <code>wrapWithAssertions</code> is true, this searcher might be an
   * {@link AssertingIndexSearcher} instance.
   */
  static std::shared_ptr<IndexSearcher>
  newSearcher(std::shared_ptr<IndexReader> r, bool maybeWrap,
              bool wrapWithAssertions);

  /**
   * Gets a resource from the test's classpath as {@link Path}. This method
   * should only be used, if a real file is needed. To get a stream, code should
   * prefer
   * {@link #getDataInputStream(std::wstring)}.
   */
protected:
  virtual std::shared_ptr<Path>
  getDataPath(const std::wstring &name) ;

  /**
   * Gets a resource from the test's classpath as {@link InputStream}.
   */
  virtual std::shared_ptr<InputStream>
  getDataInputStream(const std::wstring &name) ;

public:
  virtual void assertReaderEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  /**
   * checks that reader-level statistics are the same
   */
  virtual void assertReaderStatisticsEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  /**
   * Fields api equivalency
   */
  virtual void assertFieldsEquals(const std::wstring &info,
                                  std::shared_ptr<IndexReader> leftReader,
                                  std::shared_ptr<Fields> leftFields,
                                  std::shared_ptr<Fields> rightFields,
                                  bool deep) ;

  /**
   * checks that top-level statistics on Fields are the same
   */
  virtual void assertFieldStatisticsEquals(
      const std::wstring &info, std::shared_ptr<Fields> leftFields,
      std::shared_ptr<Fields> rightFields) ;

  /**
   * Terms api equivalency
   */
  virtual void assertTermsEquals(const std::wstring &info,
                                 std::shared_ptr<IndexReader> leftReader,
                                 std::shared_ptr<Terms> leftTerms,
                                 std::shared_ptr<Terms> rightTerms,
                                 bool deep) ;

  /**
   * checks collection-level statistics on Terms
   */
  virtual void assertTermsStatisticsEquals(
      const std::wstring &info, std::shared_ptr<Terms> leftTerms,
      std::shared_ptr<Terms> rightTerms) ;

private:
  class RandomBits : public std::enable_shared_from_this<RandomBits>,
                     public Bits
  {
    GET_CLASS_NAME(RandomBits)
  public:
    std::shared_ptr<FixedBitSet> bits;

    RandomBits(int maxDoc, double pctLive, std::shared_ptr<Random> random);

    bool get(int index) override;

    int length() override;
  };

  /**
   * checks the terms enum sequentially
   * if deep is false, it does a 'shallow' test that doesnt go down to the
   * docsenums
   */
public:
  virtual void assertTermsEnumEquals(const std::wstring &info,
                                     std::shared_ptr<IndexReader> leftReader,
                                     std::shared_ptr<TermsEnum> leftTermsEnum,
                                     std::shared_ptr<TermsEnum> rightTermsEnum,
                                     bool deep) ;

  /**
   * checks docs + freqs + positions + payloads, sequentially
   */
  virtual void assertDocsAndPositionsEnumEquals(
      const std::wstring &info, std::shared_ptr<PostingsEnum> leftDocs,
      std::shared_ptr<PostingsEnum> rightDocs) ;

  /**
   * checks docs + freqs, sequentially
   */
  virtual void assertDocsEnumEquals(const std::wstring &info,
                                    std::shared_ptr<PostingsEnum> leftDocs,
                                    std::shared_ptr<PostingsEnum> rightDocs,
                                    bool hasFreqs) ;

  /**
   * checks advancing docs
   */
  virtual void assertDocsSkippingEquals(const std::wstring &info,
                                        std::shared_ptr<IndexReader> leftReader,
                                        int docFreq,
                                        std::shared_ptr<PostingsEnum> leftDocs,
                                        std::shared_ptr<PostingsEnum> rightDocs,
                                        bool hasFreqs) ;

  /**
   * checks advancing docs + positions
   */
  virtual void assertPositionsSkippingEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      int docFreq, std::shared_ptr<PostingsEnum> leftDocs,
      std::shared_ptr<PostingsEnum> rightDocs) ;

private:
  void assertTermsSeekingEquals(
      const std::wstring &info, std::shared_ptr<Terms> leftTerms,
      std::shared_ptr<Terms> rightTerms) ;

  /**
   * checks term-level statistics
   */
public:
  virtual void assertTermStatsEquals(
      const std::wstring &info, std::shared_ptr<TermsEnum> leftTermsEnum,
      std::shared_ptr<TermsEnum> rightTermsEnum) ;

  /**
   * checks that norms are the same across all fields
   */
  virtual void assertNormsEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  /**
   * checks that stored fields of all documents are the same
   */
  virtual void assertStoredFieldsEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<IndexableField>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<LuceneTestCase> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<LuceneTestCase> outerInstance);

    int compare(std::shared_ptr<IndexableField> arg0,
                std::shared_ptr<IndexableField> arg1);
  };

  /**
   * checks that two stored fields are equivalent
   */
public:
  virtual void
  assertStoredFieldEquals(const std::wstring &info,
                          std::shared_ptr<IndexableField> leftField,
                          std::shared_ptr<IndexableField> rightField);

  /**
   * checks that term vectors across all fields are equivalent
   */
  virtual void assertTermVectorsEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

private:
  static std::shared_ptr<Set<std::wstring>>
  getDVFields(std::shared_ptr<IndexReader> reader);

  /**
   * checks that docvalues across all fields are equivalent
   */
public:
  virtual void assertDocValuesEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  virtual void assertDocValuesEquals(
      const std::wstring &info, int num,
      std::shared_ptr<NumericDocValues> leftDocValues,
      std::shared_ptr<NumericDocValues> rightDocValues) ;

  // TODO: this is kinda stupid, we don't delete documents in the test.
  virtual void assertDeletedDocsEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  virtual void assertFieldInfosEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  // naive silly memory heavy uninversion!!  maps docID -> packed values (a Set
  // because a given doc can be multi-valued)
private:
  std::unordered_map<int, Set<std::shared_ptr<BytesRef>>>
  uninvert(const std::wstring &fieldName,
           std::shared_ptr<IndexReader> reader) ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<LuceneTestCase> outerInstance;

    std::unordered_map<int, Set<std::shared_ptr<BytesRef>>> docValues;
    std::shared_ptr<LeafReaderContext> ctx;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<LuceneTestCase> outerInstance,
        std::unordered_map<int, Set<std::shared_ptr<BytesRef>>> &docValues,
        std::shared_ptr<LeafReaderContext> ctx);

    void visit(int docID) override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

public:
  virtual void assertPointsEquals(
      const std::wstring &info, std::shared_ptr<IndexReader> leftReader,
      std::shared_ptr<IndexReader> rightReader) ;

  /** A runnable that can throw any checked exception. */
  using ThrowingRunnable = std::function<void()>;

  /** Checks a specific exception class is thrown by the given runnable, and
   * returns it. */
public:
  template <typename T>
  static T expectThrows(std::type_info<T> &expectedType,
                        ThrowingRunnable runnable);

  /**
   * Checks that specific wrapped and outer exception classes are thrown
   * by the given runnable, and returns the wrapped exception.
   */
  template <typename TO, typename TW>
  static TW expectThrows(std::type_info<TO> &expectedOuterType,
                         std::type_info<TW> &expectedWrappedType,
                         ThrowingRunnable runnable);

  /** Returns true if the file exists (can be opened), false
   *  if it cannot be opened, and (unlike Java's
   *  File.exists) throws IOException if there's some
   *  unexpected error. */
  static bool slowFileExists(std::shared_ptr<Directory> dir,
                             const std::wstring &fileName) ;

  /**
   * This method is deprecated for a reason. Do not use it. Call {@link
   * #createTempDir()} or {@link #createTempDir(std::wstring)} or {@link
   * #createTempFile(std::wstring, std::wstring)}.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static java.nio.file.Path
  // getBaseTempDirForTestClass()
  static std::shared_ptr<Path> getBaseTempDirForTestClass();

  /**
   * Creates an empty, temporary folder (when the name of the folder is of no
   * importance).
   *
   * @see #createTempDir(std::wstring)
   */
  static std::shared_ptr<Path> createTempDir();

  /**
   * Creates an empty, temporary folder with the given name prefix under the
   * test class's {@link #getBaseTempDirForTestClass()}.
   *
   * <p>The folder will be automatically removed after the
   * test class completes successfully. The test should close any file handles
   * that would prevent the folder from being removed.
   */
  static std::shared_ptr<Path> createTempDir(const std::wstring &prefix);

  /**
   * Creates an empty file with the given prefix and suffix under the
   * test class's {@link #getBaseTempDirForTestClass()}.
   *
   * <p>The file will be automatically removed after the
   * test class completes successfully. The test should close any file handles
   * that would prevent the folder from being removed.
   */
  static std::shared_ptr<Path>
  createTempFile(const std::wstring &prefix,
                 const std::wstring &suffix) ;

  /**
   * Creates an empty temporary file.
   *
   * @see #createTempFile(std::wstring, std::wstring)
   */
  static std::shared_ptr<Path> createTempFile() ;

  /**
   * Runs a code part with restricted permissions (be sure to add all required
   * permissions, because it would start with empty permissions). You cannot
   * grant more permissions than our policy file allows, but you may restrict
   * writing to several dirs... <p><em>Note:</em> This assumes a {@link
   * SecurityManager} enabled, otherwise it stops test execution. If enabled, it
   * needs the following {@link SecurityPermission}:
   * {@code "createAccessControlContext"}
   */
  template <typename T>
  static T runWithRestrictedPermissions(
      std::shared_ptr<PrivilegedExceptionAction<T>> action,
      std::deque<Permission> &permissions) ;

  /** True if assertions (-ea) are enabled (at least for this class). */
  static const bool assertsAreEnabled = false;

  /**
   * Compares two strings with a collator, also looking to see if the the
   * strings are impacted by jdk bugs. may not avoid all jdk bugs in tests. see
   * https://bugs.openjdk.java.net/browse/JDK-8071862
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "dodges JDK-8071862") public
  // static int collate(java.text.Collator collator, std::wstring s1, std::wstring s2)
  static int collate(std::shared_ptr<Collator> collator, const std::wstring &s1,
                     const std::wstring &s2);

protected:
  std::shared_ptr<LuceneTestCase> shared_from_this()
  {
    return std::static_pointer_cast<LuceneTestCase>(
        org.junit.Assert::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
