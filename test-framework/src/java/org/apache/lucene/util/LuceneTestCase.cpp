using namespace std;

#include "LuceneTestCase.h"

namespace org::apache::lucene::util
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using namespace org::apache::lucene::index;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using FilterPath = org::apache::lucene::mockfile::FilterPath;
using VirusCheckingFS = org::apache::lucene::mockfile::VirusCheckingFS;
using AssertingIndexSearcher =
    org::apache::lucene::search::AssertingIndexSearcher;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LRUQueryCache = org::apache::lucene::search::LRUQueryCache;
using Query = org::apache::lucene::search::Query;
using QueryCache = org::apache::lucene::search::QueryCache;
using QueryCachingPolicy = org::apache::lucene::search::QueryCachingPolicy;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FSLockFactory = org::apache::lucene::store::FSLockFactory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using LockFactory = org::apache::lucene::store::LockFactory;
using MMapDirectory = org::apache::lucene::store::MMapDirectory;
using MergeInfo = org::apache::lucene::store::MergeInfo;
using Throttling = org::apache::lucene::store::MockDirectoryWrapper::Throttling;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using NRTCachingDirectory = org::apache::lucene::store::NRTCachingDirectory;
using RawDirectoryWrapper = org::apache::lucene::store::RawDirectoryWrapper;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using com::carrotsearch::randomizedtesting::JUnit4MethodProvider;
using com::carrotsearch::randomizedtesting::LifecycleScope;
using com::carrotsearch::randomizedtesting::MixWithSuiteName;
using com::carrotsearch::randomizedtesting::RandomizedContext;
using com::carrotsearch::randomizedtesting::RandomizedRunner;
using com::carrotsearch::randomizedtesting::RandomizedTest;
using com::carrotsearch::randomizedtesting::annotations::Listeners;
using com::carrotsearch::randomizedtesting::annotations::SeedDecorators;
using com::carrotsearch::randomizedtesting::annotations::TestGroup;
using com::carrotsearch::randomizedtesting::annotations::TestMethodProviders;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakAction;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakFilters;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakGroup;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakLingering;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakScope;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakZombies;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakAction::
    Action;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakGroup::Group;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakScope::Scope;
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakZombies::
    Consequence;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using com::carrotsearch::randomizedtesting::rules::NoClassHooksShadowingRule;
using com::carrotsearch::randomizedtesting::rules::NoInstanceHooksOverridesRule;
using com::carrotsearch::randomizedtesting::rules::StaticFieldsInvariantRule;
using junit::framework::AssertionFailedError;
using org::junit::After;
using org::junit::AfterClass;
using org::junit::Assert;
using org::junit::Before;
using org::junit::BeforeClass;
using org::junit::ClassRule;
using org::junit::Rule;
using org::junit::Test;
using org::junit::rules::RuleChain;
using org::junit::rules::TestRule;
using org::junit::runner::RunWith;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.systemPropertyAsBoolean;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.systemPropertyAsInt;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
const wstring LuceneTestCase::SYSPROP_NIGHTLY = L"tests.nightly";
const wstring LuceneTestCase::SYSPROP_WEEKLY = L"tests.weekly";
const wstring LuceneTestCase::SYSPROP_MONSTER = L"tests.monster";
const wstring LuceneTestCase::SYSPROP_AWAITSFIX = L"tests.awaitsfix";
const wstring LuceneTestCase::SYSPROP_SLOW = L"tests.slow";
const wstring LuceneTestCase::SYSPROP_BADAPPLES = L"tests.badapples";
const wstring LuceneTestCase::SYSPROP_MAXFAILURES = L"tests.maxfailures";
const wstring LuceneTestCase::SYSPROP_FAILFAST = L"tests.failfast";
const wstring LuceneTestCase::DEFAULT_LINE_DOCS_FILE = L"europarl.lines.txt.gz";
const wstring LuceneTestCase::JENKINS_LARGE_LINE_DOCS_FILE =
    L"enwiki.random.lines.txt";
const wstring LuceneTestCase::TEST_CODEC =
    System::getProperty(L"tests.codec", L"random");
const wstring LuceneTestCase::TEST_POSTINGSFORMAT =
    System::getProperty(L"tests.postingsformat", L"random");
const wstring LuceneTestCase::TEST_DOCVALUESFORMAT =
    System::getProperty(L"tests.docvaluesformat", L"random");
const wstring LuceneTestCase::TEST_DIRECTORY =
    System::getProperty(L"tests.directory", L"random");
const wstring LuceneTestCase::TEST_LINE_DOCS_FILE =
    System::getProperty(L"tests.linedocsfile", DEFAULT_LINE_DOCS_FILE);
const org::apache::lucene::store::MockDirectoryWrapper::Throttling
    LuceneTestCase::TEST_THROTTLING =
        TEST_NIGHTLY ? org::apache::lucene::store::MockDirectoryWrapper::
                           Throttling::SOMETIMES
                     : org::apache::lucene::store::MockDirectoryWrapper::
                           Throttling::NEVER;

LuceneTestCase::StaticConstructor::StaticConstructor()
{
  bool defaultValue = false;
  for (wstring property_ :
       Arrays::asList(L"tests.leaveTemporary", L"tests.leavetemporary",
                      L"tests.leavetmpdir", L"solr.test.leavetmpdir")) {
    defaultValue |= systemPropertyAsBoolean(property_, false);
  }
  LEAVE_TEMPORARY = defaultValue;
  CORE_DIRECTORIES = deque<>(FS_DIRECTORIES);
  CORE_DIRECTORIES.push_back(L"RAMDirectory");
  int maxFailures =
      systemPropertyAsInt(SYSPROP_MAXFAILURES, numeric_limits<int>::max());
  bool failFast = systemPropertyAsBoolean(SYSPROP_FAILFAST, false);

  if (failFast) {
    if (maxFailures == numeric_limits<int>::max()) {
      maxFailures = 1;
    } else {
      Logger::getLogger(LuceneTestCase::typeid->getSimpleName())
          .warning(L"Property '" + SYSPROP_MAXFAILURES + L"'=" +
                   to_wstring(maxFailures) + L", 'failfast' is" + L" ignored.");
    }
  }

  ignoreAfterMaxFailuresDelegate =
      make_shared<AtomicReference<TestRuleIgnoreAfterMaxFailures>>(
          make_shared<TestRuleIgnoreAfterMaxFailures>(maxFailures));
  ignoreAfterMaxFailures = TestRuleDelegate::of(ignoreAfterMaxFailuresDelegate);
  TestRuleLimitSysouts::checkCaptureStreams();
  Logger::getGlobal().getHandlers();
  shared_ptr<RuleChain> r =
      RuleChain::outerRule(make_shared<TestRuleIgnoreTestSuites>())
          .around(ignoreAfterMaxFailures)
          .around(suiteFailureMarker = make_shared<TestRuleMarkFailure>())
          .around(make_shared<TestRuleAssertionsRequired>())
          .around(make_shared<TestRuleLimitSysouts>(suiteFailureMarker))
          .around(tempFilesCleanupRule =
                      make_shared<TestRuleTemporaryFilesCleanup>(
                          suiteFailureMarker));
  // TODO LUCENE-7595: Java 9 does not allow to look into runtime classes, so we
  // have to fix the RAM usage checker!
  if (!Constants::JRE_IS_MINIMUM_JAVA9) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    r = r->around(make_shared<StaticFieldsInvariantRuleAnonymousInnerClass>(
        shared_from_this()));
  }
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  classRules =
      r->around(make_shared<NoClassHooksShadowingRule>())
          .around(make_shared<NoInstanceHooksOverridesRuleAnonymousInnerClass>(
              shared_from_this()))
          .around(classNameRule = make_shared<TestRuleStoreClassName>())
          .around(make_shared<TestRuleRestoreSystemProperties>(
              L"user.language", L"user.timezone", L"solr.directoryFactory",
              L"solr.solr.home", L"solr.data.dir"))
          .around(classEnvRule =
                      make_shared<TestRuleSetupAndRestoreClassEnv>());
  bool enabled = false;
  assert(enabled = true); // Intentional side-effect!!!
  assertsAreEnabled = enabled;
}

LuceneTestCase::StaticConstructor::
    StaticFieldsInvariantRuleAnonymousInnerClass::
        StaticFieldsInvariantRuleAnonymousInnerClass(
            StaticConstructor outerInstance)
    : com::carrotsearch::randomizedtesting::rules::StaticFieldsInvariantRule(
          STATIC_LEAK_THRESHOLD, true)
{
  this->outerInstance = outerInstance;
}

bool LuceneTestCase::StaticConstructor::
    StaticFieldsInvariantRuleAnonymousInnerClass::accept(
        shared_ptr<java::lang::reflect::Field> field)
{
  // Don't count known classes that consume memory once.
  if (STATIC_LEAK_IGNORED_TYPES->contains(field->getType().getName())) {
    return false;
  }
  // Don't count references from ourselves, we're top-level.
  if (field->getDeclaringClass() == LuceneTestCase::typeid) {
    return false;
  }
  return outerInstance->outerInstance.super.accept(field);
}

LuceneTestCase::StaticConstructor::
    NoInstanceHooksOverridesRuleAnonymousInnerClass::
        NoInstanceHooksOverridesRuleAnonymousInnerClass(
            StaticConstructor outerInstance)
{
  this->outerInstance = outerInstance;
}

bool LuceneTestCase::StaticConstructor::
    NoInstanceHooksOverridesRuleAnonymousInnerClass::verify(
        shared_ptr<Method> key)
{
  wstring name = key->getName();
  return !(name == L"setUp" || name == L"tearDown");
}

LuceneTestCase::StaticConstructor LuceneTestCase::staticConstructor;

bool LuceneTestCase::hasWorkingMMapOnWindows()
{
  return !Constants::WINDOWS || MMapDirectory::UNMAP_SUPPORTED;
}

void LuceneTestCase::assumeWorkingMMapOnWindows()
{
  assumeTrue(MMapDirectory::UNMAP_NOT_SUPPORTED_REASON,
             hasWorkingMMapOnWindows());
}

const deque<wstring> LuceneTestCase::FS_DIRECTORIES =
    java::util::Arrays::asList(
        L"SimpleFSDirectory", L"NIOFSDirectory",
        hasWorkingMMapOnWindows() ? L"MMapDirectory" : L"SimpleFSDirectory");
const deque<wstring> LuceneTestCase::CORE_DIRECTORIES;
const shared_ptr<org::apache::lucene::search::QueryCachingPolicy>
    LuceneTestCase::MAYBE_CACHE_POLICY =
        make_shared<QueryCachingPolicyAnonymousInnerClass>();

LuceneTestCase::QueryCachingPolicyAnonymousInnerClass::
    QueryCachingPolicyAnonymousInnerClass()
{
}

void LuceneTestCase::QueryCachingPolicyAnonymousInnerClass::onUse(
    shared_ptr<Query> query)
{
}

bool LuceneTestCase::QueryCachingPolicyAnonymousInnerClass::shouldCache(
    shared_ptr<Query> query) 
{
  return random()->nextBoolean();
}

const shared_ptr<TestRuleStoreClassName> LuceneTestCase::classNameRule;
const shared_ptr<TestRuleSetupAndRestoreClassEnv> LuceneTestCase::classEnvRule;
shared_ptr<TestRuleMarkFailure> LuceneTestCase::suiteFailureMarker;
shared_ptr<TestRuleTemporaryFilesCleanup> LuceneTestCase::tempFilesCleanupRule;
const shared_ptr<java::util::concurrent::atomic::AtomicReference<
    std::shared_ptr<TestRuleIgnoreAfterMaxFailures>>>
    LuceneTestCase::ignoreAfterMaxFailuresDelegate;
const shared_ptr<org::junit::rules::TestRule>
    LuceneTestCase::ignoreAfterMaxFailures;

shared_ptr<TestRuleIgnoreAfterMaxFailures>
LuceneTestCase::replaceMaxFailureRule(
    shared_ptr<TestRuleIgnoreAfterMaxFailures> newValue)
{
  return ignoreAfterMaxFailuresDelegate->getAndSet(newValue);
}

const shared_ptr<java::util::Set<wstring>>
    LuceneTestCase::STATIC_LEAK_IGNORED_TYPES =
        java::util::Collections::unmodifiableSet(
            unordered_set<wstring>(java::util::Arrays::asList(
                L"org.slf4j.Logger", L"org.apache.solr.SolrLogFormatter",
                L"java.io.File", java::nio::file::Path::typeid->getName(),
                type_info::typeid->getName(),
                java::util::EnumSet::typeid->getName())));
shared_ptr<org::junit::rules::TestRule> LuceneTestCase::classRules;
const unordered_map<wstring,
                    std::shared_ptr<org::apache::lucene::document::FieldType>>
    LuceneTestCase::fieldToType = unordered_map<
        wstring, std::shared_ptr<org::apache::lucene::document::FieldType>>();
LiveIWCFlushMode LuceneTestCase::liveIWCFlushMode = (LiveIWCFlushMode)0;

void LuceneTestCase::setLiveIWCFlushMode(LiveIWCFlushMode flushMode)
{
  liveIWCFlushMode = flushMode;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void setUp() throws Exception
void LuceneTestCase::setUp() 
{
  parentChainCallRule->setupCalled = true;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void tearDown() throws Exception
void LuceneTestCase::tearDown() 
{
  parentChainCallRule->teardownCalled = true;
  fieldToType.clear();

  // Test is supposed to call this itself, but we do this defensively in case it
  // forgot:
  restoreIndexWriterMaxDocs();
}

void LuceneTestCase::setIndexWriterMaxDocs(int limit)
{
  IndexWriterMaxDocsChanger::setMaxDocs(limit);
}

void LuceneTestCase::restoreIndexWriterMaxDocs()
{
  IndexWriterMaxDocsChanger::restoreMaxDocs();
}

shared_ptr<Random> LuceneTestCase::random()
{
  return RandomizedContext::current().getRandom();
}

template <typename T>
T LuceneTestCase::closeAfterTest(T resource)
{
  static_assert(is_base_of<java.io.Closeable, T>::value,
                L"T must inherit from java.io.Closeable");

  return RandomizedContext::current().closeAtEnd(resource,
                                                 LifecycleScope::TEST);
}

template <typename T>
T LuceneTestCase::closeAfterSuite(T resource)
{
  static_assert(is_base_of<java.io.Closeable, T>::value,
                L"T must inherit from java.io.Closeable");

  return RandomizedContext::current().closeAtEnd(resource,
                                                 LifecycleScope::SUITE);
}

type_info LuceneTestCase::getTestClass()
{
  return classNameRule->getTestClass();
}

wstring LuceneTestCase::getTestName()
{
  return threadAndTestNameRule->testMethodName;
}

shared_ptr<LeafReader>
LuceneTestCase::getOnlyLeafReader(shared_ptr<IndexReader> reader)
{
  deque<std::shared_ptr<LeafReaderContext>> subReaders = reader->leaves();
  if (subReaders.size() != 1) {
    throw invalid_argument(reader + L" has " + subReaders.size() +
                           L" segments instead of exactly one");
  }
  return subReaders[0]->reader();
}

bool LuceneTestCase::isTestThread()
{
  assertNotNull(L"Test case thread not set?",
                threadAndTestNameRule->testCaseThread);
  return Thread::currentThread() == threadAndTestNameRule->testCaseThread;
}

int LuceneTestCase::atLeast(shared_ptr<Random> random, int i)
{
  int min = (TEST_NIGHTLY ? 2 * i : i) * RANDOM_MULTIPLIER;
  int max = min + (min / 2);
  return TestUtil::nextInt(random, min, max);
}

int LuceneTestCase::atLeast(int i) { return atLeast(random(), i); }

bool LuceneTestCase::rarely(shared_ptr<Random> random)
{
  int p = TEST_NIGHTLY ? 10 : 1;
  p += (p * log(RANDOM_MULTIPLIER));
  int min = 100 - min(p, 50); // never more than 50
  return random->nextInt(100) >= min;
}

bool LuceneTestCase::rarely() { return rarely(random()); }

bool LuceneTestCase::usually(shared_ptr<Random> random)
{
  return !rarely(random);
}

bool LuceneTestCase::usually() { return usually(random()); }

void LuceneTestCase::assumeTrue(const wstring &msg, bool condition)
{
  RandomizedTest::assumeTrue(msg, condition);
}

void LuceneTestCase::assumeFalse(const wstring &msg, bool condition)
{
  RandomizedTest::assumeFalse(msg, condition);
}

void LuceneTestCase::assumeNoException(const wstring &msg, runtime_error e)
{
  RandomizedTest::assumeNoException(msg, e);
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SafeVarargs @SuppressWarnings("varargs") public static <T>
// java.util.Set<T> asSet(T... args)
shared_ptr<Set<T>> LuceneTestCase::asSet(deque<T> &args)
{
  return unordered_set<>(Arrays::asList(args));
}

template <typename T1>
void LuceneTestCase::dumpIterator(const wstring &label,
                                  shared_ptr<Iterator<T1>> iter,
                                  shared_ptr<PrintStream> stream)
{
  stream->println(L"*** BEGIN " + label + L" ***");
  if (nullptr == iter) {
    stream->println(L" ... NULL ...");
  } else {
    while (iter->hasNext()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      stream->println(iter->next()->toString());
      iter++;
    }
  }
  stream->println(L"*** END " + label + L" ***");
}

void LuceneTestCase::dumpArray(const wstring &label, std::deque<any> &objs,
                               shared_ptr<PrintStream> stream)
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.Iterator<?> iter = (null == objs) ? null :
  // java.util.Arrays.asList(objs).iterator();
  Iterator < ? > iter =
                   (nullptr == objs) ? nullptr : Arrays::asList(objs).begin();
  dumpIterator(label, iter, stream);
}

shared_ptr<IndexWriterConfig> LuceneTestCase::newIndexWriterConfig()
{
  return newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
}

shared_ptr<IndexWriterConfig>
LuceneTestCase::newIndexWriterConfig(shared_ptr<Analyzer> a)
{
  return newIndexWriterConfig(random(), a);
}

shared_ptr<IndexWriterConfig>
LuceneTestCase::newIndexWriterConfig(shared_ptr<Random> r,
                                     shared_ptr<Analyzer> a)
{
  shared_ptr<IndexWriterConfig> c = make_shared<IndexWriterConfig>(a);
  c->setSimilarity(classEnvRule->similarity);
  if (VERBOSE) {
    // Even though TestRuleSetupAndRestoreClassEnv calls
    // InfoStream.setDefault, we do it again here so that
    // the PrintStreamInfoStream.messageID increments so
    // that when there are separate instances of
    // IndexWriter created we see "IW 0", "IW 1", "IW 2",
    // ... instead of just always "IW 0":
    c->setInfoStream(
        make_shared<TestRuleSetupAndRestoreClassEnv::
                        ThreadNameFixingPrintStreamInfoStream>(System::out));
  }

  if (r->nextBoolean()) {
    c->setMergeScheduler(make_shared<SerialMergeScheduler>());
  } else if (rarely(r)) {
    shared_ptr<ConcurrentMergeScheduler> cms;
    if (r->nextBoolean()) {
      cms = make_shared<ConcurrentMergeScheduler>();
    } else {
      cms = make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>();
    }
    int maxThreadCount = TestUtil::nextInt(r, 1, 4);
    int maxMergeCount =
        TestUtil::nextInt(r, maxThreadCount, maxThreadCount + 4);
    cms->setMaxMergesAndThreads(maxMergeCount, maxThreadCount);
    if (random()->nextBoolean()) {
      cms->disableAutoIOThrottle();
      assertFalse(cms->getAutoIOThrottle());
    }
    cms->setForceMergeMBPerSec(10 + 10 * random()->nextDouble());
    c->setMergeScheduler(cms);
  } else {
    // Always use consistent settings, else CMS's dynamic (SSD or not)
    // defaults can change, hurting reproducibility:
    shared_ptr<ConcurrentMergeScheduler> cms =
        make_shared<ConcurrentMergeScheduler>();

    // Only 1 thread can run at once (should maybe help reproducibility),
    // with up to 3 pending merges before segment-producing threads are
    // stalled:
    cms->setMaxMergesAndThreads(3, 1);
    c->setMergeScheduler(cms);
  }

  if (r->nextBoolean()) {
    if (rarely(r)) {
      // crazy value
      c->setMaxBufferedDocs(TestUtil::nextInt(r, 2, 15));
    } else {
      // reasonable value
      c->setMaxBufferedDocs(TestUtil::nextInt(r, 16, 1000));
    }
  }

  c->setMergePolicy(newMergePolicy(r));

  avoidPathologicalMerging(c);

  if (rarely(r)) {
    c->setMergedSegmentWarmer(
        make_shared<SimpleMergedSegmentWarmer>(c->getInfoStream()));
  }
  c->setUseCompoundFile(r->nextBoolean());
  c->setReaderPooling(r->nextBoolean());
  if (rarely(r)) {
    c->setCheckPendingFlushUpdate(false);
  }
  return c;
}

LuceneTestCase::ConcurrentMergeSchedulerAnonymousInnerClass::
    ConcurrentMergeSchedulerAnonymousInnerClass()
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool LuceneTestCase::ConcurrentMergeSchedulerAnonymousInnerClass::maybeStall(
    shared_ptr<IndexWriter> writer)
{
  return true;
}

void LuceneTestCase::avoidPathologicalMerging(shared_ptr<IndexWriterConfig> iwc)
{
  // Don't allow "tiny" flushed segments with "big" merge
  // floor: this leads to pathological O(N^2) merge costs:
  int64_t estFlushSizeBytes = numeric_limits<int64_t>::max();
  if (iwc->getMaxBufferedDocs() != IndexWriterConfig::DISABLE_AUTO_FLUSH) {
    // Gross estimation of 1 KB segment bytes for each doc indexed:
    estFlushSizeBytes =
        min(estFlushSizeBytes, iwc->getMaxBufferedDocs() * 1024);
  }
  if (iwc->getRAMBufferSizeMB() != IndexWriterConfig::DISABLE_AUTO_FLUSH) {
    estFlushSizeBytes =
        min(estFlushSizeBytes,
            static_cast<int64_t>(iwc->getRAMBufferSizeMB() * 1024 * 1024));
  }
  assert(estFlushSizeBytes > 0);

  shared_ptr<MergePolicy> mp = iwc->getMergePolicy();
  if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
    shared_ptr<TieredMergePolicy> tmp =
        std::static_pointer_cast<TieredMergePolicy>(mp);
    int64_t floorSegBytes =
        static_cast<int64_t>(tmp->getFloorSegmentMB() * 1024 * 1024);
    if (floorSegBytes / estFlushSizeBytes > 10) {
      double newValue = estFlushSizeBytes * 10.0 / 1024 / 1024;
      if (VERBOSE) {
        wcout << L"NOTE: LuceneTestCase: changing "
                 L"TieredMergePolicy.floorSegmentMB from "
              << tmp->getFloorSegmentMB() << L" to " << newValue
              << L" to avoid pathological merging" << endl;
      }
      tmp->setFloorSegmentMB(newValue);
    }
  } else if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
    shared_ptr<LogByteSizeMergePolicy> lmp =
        std::static_pointer_cast<LogByteSizeMergePolicy>(mp);
    if ((lmp->getMinMergeMB() * 1024 * 1024) / estFlushSizeBytes > 10) {
      double newValue = estFlushSizeBytes * 10.0 / 1024 / 1024;
      if (VERBOSE) {
        wcout << L"NOTE: LuceneTestCase: changing "
                 L"LogByteSizeMergePolicy.minMergeMB from "
              << lmp->getMinMergeMB() << L" to " << newValue
              << L" to avoid pathological merging" << endl;
      }
      lmp->setMinMergeMB(newValue);
    }
  } else if (std::dynamic_pointer_cast<LogDocMergePolicy>(mp) != nullptr) {
    shared_ptr<LogDocMergePolicy> lmp =
        std::static_pointer_cast<LogDocMergePolicy>(mp);
    assert((estFlushSizeBytes / 1024 < std, : numeric_limits<int>::max() / 10));
    int estFlushDocs = max(1, static_cast<int>(estFlushSizeBytes / 1024));
    if (lmp->getMinMergeDocs() / estFlushDocs > 10) {
      int newValue = estFlushDocs * 10;
      if (VERBOSE) {
        wcout << L"NOTE: LuceneTestCase: changing "
                 L"LogDocMergePolicy.minMergeDocs from "
              << lmp->getMinMergeDocs() << L" to " << newValue
              << L" to avoid pathological merging" << endl;
      }
      lmp->setMinMergeDocs(newValue);
    }
  }
}

shared_ptr<MergePolicy> LuceneTestCase::newMergePolicy(shared_ptr<Random> r)
{
  return newMergePolicy(r, true);
}

shared_ptr<MergePolicy> LuceneTestCase::newMergePolicy(shared_ptr<Random> r,
                                                       bool includeMockMP)
{
  if (includeMockMP && rarely(r)) {
    return make_shared<MockRandomMergePolicy>(r);
  } else if (r->nextBoolean()) {
    return newTieredMergePolicy(r);
  } else if (r->nextInt(5) == 0) {
    return newAlcoholicMergePolicy(r, classEnvRule->timeZone);
  }
  return newLogMergePolicy(r);
}

shared_ptr<MergePolicy> LuceneTestCase::newMergePolicy()
{
  return newMergePolicy(random());
}

shared_ptr<LogMergePolicy> LuceneTestCase::newLogMergePolicy()
{
  return newLogMergePolicy(random());
}

shared_ptr<TieredMergePolicy> LuceneTestCase::newTieredMergePolicy()
{
  return newTieredMergePolicy(random());
}

shared_ptr<AlcoholicMergePolicy> LuceneTestCase::newAlcoholicMergePolicy()
{
  return newAlcoholicMergePolicy(random(), classEnvRule->timeZone);
}

shared_ptr<AlcoholicMergePolicy>
LuceneTestCase::newAlcoholicMergePolicy(shared_ptr<Random> r,
                                        shared_ptr<TimeZone> tz)
{
  return make_shared<AlcoholicMergePolicy>(tz,
                                           make_shared<Random>(r->nextLong()));
}

shared_ptr<LogMergePolicy>
LuceneTestCase::newLogMergePolicy(shared_ptr<Random> r)
{
  shared_ptr<LogMergePolicy> logmp =
      r->nextBoolean() ? make_shared<LogDocMergePolicy>()
                       : make_shared<LogByteSizeMergePolicy>();
  logmp->setCalibrateSizeByDeletes(r->nextBoolean());
  if (rarely(r)) {
    logmp->setMergeFactor(TestUtil::nextInt(r, 2, 9));
  } else {
    logmp->setMergeFactor(TestUtil::nextInt(r, 10, 50));
  }
  configureRandom(r, logmp);
  return logmp;
}

void LuceneTestCase::configureRandom(shared_ptr<Random> r,
                                     shared_ptr<MergePolicy> mergePolicy)
{
  if (r->nextBoolean()) {
    mergePolicy->setNoCFSRatio(0.1 + r->nextDouble() * 0.8);
  } else {
    mergePolicy->setNoCFSRatio(r->nextBoolean() ? 1.0 : 0.0);
  }

  if (rarely(r)) {
    mergePolicy->setMaxCFSSegmentSizeMB(0.2 + r->nextDouble() * 2.0);
  } else {
    mergePolicy->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  }
}

shared_ptr<TieredMergePolicy>
LuceneTestCase::newTieredMergePolicy(shared_ptr<Random> r)
{
  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  if (rarely(r)) {
    tmp->setMaxMergeAtOnce(TestUtil::nextInt(r, 2, 9));
    tmp->setMaxMergeAtOnceExplicit(TestUtil::nextInt(r, 2, 9));
  } else {
    tmp->setMaxMergeAtOnce(TestUtil::nextInt(r, 10, 50));
    tmp->setMaxMergeAtOnceExplicit(TestUtil::nextInt(r, 10, 50));
  }
  if (rarely(r)) {
    tmp->setMaxMergedSegmentMB(0.2 + r->nextDouble() * 2.0);
  } else {
    tmp->setMaxMergedSegmentMB(r->nextDouble() * 100);
  }
  tmp->setFloorSegmentMB(0.2 + r->nextDouble() * 2.0);
  tmp->setForceMergeDeletesPctAllowed(0.0 + r->nextDouble() * 30.0);
  if (rarely(r)) {
    tmp->setSegmentsPerTier(TestUtil::nextInt(r, 2, 20));
  } else {
    tmp->setSegmentsPerTier(TestUtil::nextInt(r, 10, 50));
  }
  configureRandom(r, tmp);
  tmp->setReclaimDeletesWeight(r->nextDouble() * 4);
  return tmp;
}

shared_ptr<MergePolicy> LuceneTestCase::newLogMergePolicy(bool useCFS)
{
  shared_ptr<MergePolicy> logmp = newLogMergePolicy();
  logmp->setNoCFSRatio(useCFS ? 1.0 : 0.0);
  return logmp;
}

shared_ptr<MergePolicy> LuceneTestCase::newLogMergePolicy(bool useCFS,
                                                          int mergeFactor)
{
  shared_ptr<LogMergePolicy> logmp = newLogMergePolicy();
  logmp->setNoCFSRatio(useCFS ? 1.0 : 0.0);
  logmp->setMergeFactor(mergeFactor);
  return logmp;
}

shared_ptr<MergePolicy> LuceneTestCase::newLogMergePolicy(int mergeFactor)
{
  shared_ptr<LogMergePolicy> logmp = newLogMergePolicy();
  logmp->setMergeFactor(mergeFactor);
  return logmp;
}

void LuceneTestCase::maybeChangeLiveIndexWriterConfig(
    shared_ptr<Random> r, shared_ptr<LiveIndexWriterConfig> c)
{
  bool didChange = false;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring previous = c->toString();

  if (rarely(r)) {
    // change flush parameters:
    // this is complicated because the api requires you "invoke setters in a
    // magical order!" LUCENE-5661: workaround for race conditions in the API
    {
      lock_guard<mutex> lock(c);
      bool flushByRAM;
      switch (liveIWCFlushMode) {
      case org::apache::lucene::util::LuceneTestCase::LiveIWCFlushMode::BY_RAM:
        flushByRAM = true;
        break;
      case org::apache::lucene::util::LuceneTestCase::LiveIWCFlushMode::BY_DOCS:
        flushByRAM = false;
        break;
      case org::apache::lucene::util::LuceneTestCase::LiveIWCFlushMode::EITHER:
        flushByRAM = r->nextBoolean();
        break;
      default:
        throw make_shared<AssertionError>();
      }
      if (flushByRAM) {
        c->setRAMBufferSizeMB(TestUtil::nextInt(r, 1, 10));
        c->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
      } else {
        if (rarely(r)) {
          // crazy value
          c->setMaxBufferedDocs(TestUtil::nextInt(r, 2, 15));
        } else {
          // reasonable value
          c->setMaxBufferedDocs(TestUtil::nextInt(r, 16, 1000));
        }
        c->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
      }
    }
    didChange = true;
  }

  if (rarely(r)) {
    IndexWriter::IndexReaderWarmer curWarmer = c->getMergedSegmentWarmer();
    if (curWarmer == nullptr ||
        std::dynamic_pointer_cast<SimpleMergedSegmentWarmer>(curWarmer) !=
            nullptr) {
      // change warmer parameters
      if (r->nextBoolean()) {
        c->setMergedSegmentWarmer(
            make_shared<SimpleMergedSegmentWarmer>(c->getInfoStream()));
      } else {
        c->setMergedSegmentWarmer(nullptr);
      }
    }
    didChange = true;
  }

  if (rarely(r)) {
    // change CFS flush parameters
    c->setUseCompoundFile(r->nextBoolean());
    didChange = true;
  }

  if (rarely(r)) {
    // change CMS merge parameters
    shared_ptr<MergeScheduler> ms = c->getMergeScheduler();
    if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
      shared_ptr<ConcurrentMergeScheduler> cms =
          std::static_pointer_cast<ConcurrentMergeScheduler>(ms);
      int maxThreadCount = TestUtil::nextInt(r, 1, 4);
      int maxMergeCount =
          TestUtil::nextInt(r, maxThreadCount, maxThreadCount + 4);
      bool enableAutoIOThrottle = random()->nextBoolean();
      if (enableAutoIOThrottle) {
        cms->enableAutoIOThrottle();
      } else {
        cms->disableAutoIOThrottle();
      }
      cms->setMaxMergesAndThreads(maxMergeCount, maxThreadCount);
      didChange = true;
    }
  }

  if (rarely(r)) {
    shared_ptr<MergePolicy> mp = c->getMergePolicy();
    configureRandom(r, mp);
    if (std::dynamic_pointer_cast<LogMergePolicy>(mp) != nullptr) {
      shared_ptr<LogMergePolicy> logmp =
          std::static_pointer_cast<LogMergePolicy>(mp);
      logmp->setCalibrateSizeByDeletes(r->nextBoolean());
      if (rarely(r)) {
        logmp->setMergeFactor(TestUtil::nextInt(r, 2, 9));
      } else {
        logmp->setMergeFactor(TestUtil::nextInt(r, 10, 50));
      }
    } else if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
      shared_ptr<TieredMergePolicy> tmp =
          std::static_pointer_cast<TieredMergePolicy>(mp);
      if (rarely(r)) {
        tmp->setMaxMergeAtOnce(TestUtil::nextInt(r, 2, 9));
        tmp->setMaxMergeAtOnceExplicit(TestUtil::nextInt(r, 2, 9));
      } else {
        tmp->setMaxMergeAtOnce(TestUtil::nextInt(r, 10, 50));
        tmp->setMaxMergeAtOnceExplicit(TestUtil::nextInt(r, 10, 50));
      }
      if (rarely(r)) {
        tmp->setMaxMergedSegmentMB(0.2 + r->nextDouble() * 2.0);
      } else {
        tmp->setMaxMergedSegmentMB(r->nextDouble() * 100);
      }
      tmp->setFloorSegmentMB(0.2 + r->nextDouble() * 2.0);
      tmp->setForceMergeDeletesPctAllowed(0.0 + r->nextDouble() * 30.0);
      if (rarely(r)) {
        tmp->setSegmentsPerTier(TestUtil::nextInt(r, 2, 20));
      } else {
        tmp->setSegmentsPerTier(TestUtil::nextInt(r, 10, 50));
      }
      configureRandom(r, tmp);
      tmp->setReclaimDeletesWeight(r->nextDouble() * 4);
    }
    didChange = true;
  }
  if (VERBOSE && didChange) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring current = c->toString();
    std::deque<wstring> previousLines = previous.split(L"\n");
    std::deque<wstring> currentLines = current.split(L"\n");
    shared_ptr<StringBuilder> diff = make_shared<StringBuilder>();

    // this should always be the case, diff each line
    if (previousLines.size() == currentLines.size()) {
      for (int i = 0; i < previousLines.size(); i++) {
        if (previousLines[i] != currentLines[i]) {
          diff->append(L"- " + previousLines[i] + L"\n");
          diff->append(L"+ " + currentLines[i] + L"\n");
        }
      }
    } else {
      // but just in case of something ridiculous...
      // C++ TODO: There is no native C++ equivalent to 'toString':
      diff->append(current.toString());
    }

    // its possible to be empty, if we "change" a value to what it had before.
    if (diff->length() > 0) {
      wcout << L"NOTE: LuceneTestCase: randomly changed IWC's live settings:"
            << endl;
      wcout << diff << endl;
    }
  }
}

shared_ptr<BaseDirectoryWrapper> LuceneTestCase::newDirectory()
{
  return newDirectory(random());
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newMaybeVirusCheckingDirectory()
{
  if (random()->nextInt(5) == 4) {
    shared_ptr<Path> path = addVirusChecker(createTempDir());
    return newFSDirectory(path);
  } else {
    return newDirectory(random());
  }
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newDirectory(shared_ptr<Random> r)
{
  return wrapDirectory(r, newDirectoryImpl(r, TEST_DIRECTORY), rarely(r));
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newDirectory(shared_ptr<Random> r, shared_ptr<LockFactory> lf)
{
  return wrapDirectory(r, newDirectoryImpl(r, TEST_DIRECTORY, lf), rarely(r));
}

shared_ptr<MockDirectoryWrapper> LuceneTestCase::newMockDirectory()
{
  return newMockDirectory(random());
}

shared_ptr<MockDirectoryWrapper>
LuceneTestCase::newMockDirectory(shared_ptr<Random> r)
{
  return std::static_pointer_cast<MockDirectoryWrapper>(
      wrapDirectory(r, newDirectoryImpl(r, TEST_DIRECTORY), false));
}

shared_ptr<MockDirectoryWrapper>
LuceneTestCase::newMockDirectory(shared_ptr<Random> r,
                                 shared_ptr<LockFactory> lf)
{
  return std::static_pointer_cast<MockDirectoryWrapper>(
      wrapDirectory(r, newDirectoryImpl(r, TEST_DIRECTORY, lf), false));
}

shared_ptr<MockDirectoryWrapper>
LuceneTestCase::newMockFSDirectory(shared_ptr<Path> f)
{
  return std::static_pointer_cast<MockDirectoryWrapper>(
      newFSDirectory(f, FSLockFactory::getDefault(), false));
}

shared_ptr<MockDirectoryWrapper>
LuceneTestCase::newMockFSDirectory(shared_ptr<Path> f,
                                   shared_ptr<LockFactory> lf)
{
  return std::static_pointer_cast<MockDirectoryWrapper>(
      newFSDirectory(f, lf, false));
}

shared_ptr<Path> LuceneTestCase::addVirusChecker(shared_ptr<Path> path)
{
  if (TestUtil::hasVirusChecker(path) == false) {
    shared_ptr<VirusCheckingFS> fs = make_shared<VirusCheckingFS>(
        path->getFileSystem(), random()->nextLong());
    shared_ptr<FileSystem> filesystem =
        fs->getFileSystem(URI::create(L"file:///"));
    path = make_shared<FilterPath>(path, filesystem);
  }
  return path;
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newDirectory(shared_ptr<Directory> d) 
{
  return newDirectory(random(), d);
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newFSDirectory(shared_ptr<Path> f)
{
  return newFSDirectory(f, FSLockFactory::getDefault());
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newMaybeVirusCheckingFSDirectory(shared_ptr<Path> f)
{
  if (random()->nextInt(5) == 4) {
    f = addVirusChecker(f);
  }
  return newFSDirectory(f, FSLockFactory::getDefault());
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newFSDirectory(shared_ptr<Path> f, shared_ptr<LockFactory> lf)
{
  return newFSDirectory(f, lf, rarely());
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newFSDirectory(shared_ptr<Path> f, shared_ptr<LockFactory> lf,
                               bool bare)
{
  wstring fsdirClass = TEST_DIRECTORY;
  if (fsdirClass == L"random") {
    fsdirClass = RandomPicks::randomFrom(random(), FS_DIRECTORIES);
  }

  type_info clazz;
  try {
    try {
      clazz = CommandLineUtil::loadFSDirectoryClass(fsdirClass);
    } catch (const ClassCastException &e) {
      // TEST_DIRECTORY is not a sub-class of FSDirectory, so draw one at random
      fsdirClass = RandomPicks::randomFrom(random(), FS_DIRECTORIES);
      clazz = CommandLineUtil::loadFSDirectoryClass(fsdirClass);
    }

    shared_ptr<Directory> fsdir = newFSDirectoryImpl(clazz, f, lf);
    shared_ptr<BaseDirectoryWrapper> wrapped =
        wrapDirectory(random(), fsdir, bare);
    return wrapped;
  } catch (const runtime_error &e) {
    Rethrow::rethrow(e);
    throw nullptr; // dummy to prevent compiler failure
  }
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::newDirectory(shared_ptr<Random> r,
                             shared_ptr<Directory> d) 
{
  shared_ptr<Directory> impl = newDirectoryImpl(r, TEST_DIRECTORY);
  for (auto file : d->listAll()) {
    if (file.startsWith(IndexFileNames::SEGMENTS) ||
        IndexFileNames::CODEC_FILE_PATTERN->matcher(file).matches()) {
      impl->copyFrom(d, file, file, newIOContext(r));
    }
  }
  return wrapDirectory(r, impl, rarely(r));
}

shared_ptr<BaseDirectoryWrapper>
LuceneTestCase::wrapDirectory(shared_ptr<Random> random,
                              shared_ptr<Directory> directory, bool bare)
{
  if (rarely(random) && !bare) {
    directory = make_shared<NRTCachingDirectory>(
        directory, random->nextDouble(), random->nextDouble());
  }

  if (bare) {
    shared_ptr<BaseDirectoryWrapper> base =
        make_shared<RawDirectoryWrapper>(directory);
    closeAfterSuite(make_shared<CloseableDirectory>(base, suiteFailureMarker));
    return base;
  } else {
    shared_ptr<MockDirectoryWrapper> mock =
        make_shared<MockDirectoryWrapper>(random, directory);

    mock->setThrottling(TEST_THROTTLING);
    closeAfterSuite(make_shared<CloseableDirectory>(mock, suiteFailureMarker));
    return mock;
  }
}

shared_ptr<Field> LuceneTestCase::newStringField(const wstring &name,
                                                 const wstring &value,
                                                 Store stored)
{
  return newField(random(), name, value,
                  stored == Store::YES ? StringField::TYPE_STORED
                                       : StringField::TYPE_NOT_STORED);
}

shared_ptr<Field> LuceneTestCase::newStringField(const wstring &name,
                                                 shared_ptr<BytesRef> value,
                                                 Store stored)
{
  return newField(random(), name, value,
                  stored == Store::YES ? StringField::TYPE_STORED
                                       : StringField::TYPE_NOT_STORED);
}

shared_ptr<Field> LuceneTestCase::newTextField(const wstring &name,
                                               const wstring &value,
                                               Store stored)
{
  return newField(random(), name, value,
                  stored == Store::YES ? TextField::TYPE_STORED
                                       : TextField::TYPE_NOT_STORED);
}

shared_ptr<Field> LuceneTestCase::newStringField(shared_ptr<Random> random,
                                                 const wstring &name,
                                                 const wstring &value,
                                                 Store stored)
{
  return newField(random, name, value,
                  stored == Store::YES ? StringField::TYPE_STORED
                                       : StringField::TYPE_NOT_STORED);
}

shared_ptr<Field> LuceneTestCase::newStringField(shared_ptr<Random> random,
                                                 const wstring &name,
                                                 shared_ptr<BytesRef> value,
                                                 Store stored)
{
  return newField(random, name, value,
                  stored == Store::YES ? StringField::TYPE_STORED
                                       : StringField::TYPE_NOT_STORED);
}

shared_ptr<Field> LuceneTestCase::newTextField(shared_ptr<Random> random,
                                               const wstring &name,
                                               const wstring &value,
                                               Store stored)
{
  return newField(random, name, value,
                  stored == Store::YES ? TextField::TYPE_STORED
                                       : TextField::TYPE_NOT_STORED);
}

shared_ptr<Field> LuceneTestCase::newField(const wstring &name,
                                           const wstring &value,
                                           shared_ptr<FieldType> type)
{
  return newField(random(), name, value, type);
}

shared_ptr<FieldType>
LuceneTestCase::mergeTermVectorOptions(shared_ptr<FieldType> newType,
                                       shared_ptr<FieldType> oldType)
{
  if (newType->indexOptions() != IndexOptions::NONE &&
      oldType->storeTermVectors() == true &&
      newType->storeTermVectors() == false) {
    newType = make_shared<FieldType>(newType);
    newType->setStoreTermVectors(oldType->storeTermVectors());
    newType->setStoreTermVectorPositions(oldType->storeTermVectorPositions());
    newType->setStoreTermVectorOffsets(oldType->storeTermVectorOffsets());
    newType->setStoreTermVectorPayloads(oldType->storeTermVectorPayloads());
    newType->freeze();
  }

  return newType;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Field> LuceneTestCase::newField(shared_ptr<Random> random,
                                           const wstring &name, any value,
                                           shared_ptr<FieldType> type)
{

  // Defeat any consumers that illegally rely on intern'd
  // strings (we removed this from Lucene a while back):
  name = wstring(name);

  shared_ptr<FieldType> prevType = fieldToType[name];

  if (usually(random) || type->indexOptions() == IndexOptions::NONE ||
      prevType != nullptr) {
    // most of the time, don't modify the params
    if (prevType == nullptr) {
      fieldToType.emplace(name, make_shared<FieldType>(type));
    } else {
      type = mergeTermVectorOptions(type, prevType);
    }

    return createField(name, value, type);
  }

  // TODO: once all core & test codecs can index
  // offsets, sometimes randomly turn on offsets if we are
  // already indexing positions...

  shared_ptr<FieldType> newType = make_shared<FieldType>(type);
  if (!newType->stored() && random->nextBoolean()) {
    newType->setStored(true); // randomly store it
  }

  // Randomly turn on term deque options, but always do
  // so consistently for the same field name:
  if (!newType->storeTermVectors() && random->nextBoolean()) {
    newType->setStoreTermVectors(true);
    if (!newType->storeTermVectorPositions()) {
      newType->setStoreTermVectorPositions(random->nextBoolean());

      if (newType->storeTermVectorPositions()) {
        if (!newType->storeTermVectorPayloads()) {
          newType->setStoreTermVectorPayloads(random->nextBoolean());
        }
      }
    }

    if (!newType->storeTermVectorOffsets()) {
      newType->setStoreTermVectorOffsets(random->nextBoolean());
    }

    if (VERBOSE) {
      wcout << L"NOTE: LuceneTestCase: upgrade name=" << name << L" type="
            << newType << endl;
    }
  }
  newType->freeze();
  fieldToType.emplace(name, newType);

  // TODO: we need to do this, but smarter, ie, most of
  // the time we set the same value for a given field but
  // sometimes (rarely) we change it up:
  /*
  if (newType.omitNorms()) {
    newType.setOmitNorms(random.nextBoolean());
  }
  */

  return createField(name, value, newType);
}

shared_ptr<Field> LuceneTestCase::createField(const wstring &name, any value,
                                              shared_ptr<FieldType> fieldType)
{
  if (dynamic_cast<wstring>(value) != nullptr) {
    return make_shared<Field>(name, any_cast<wstring>(value), fieldType);
  } else if (std::dynamic_pointer_cast<BytesRef>(value) != nullptr) {
    return make_shared<Field>(name, any_cast<std::shared_ptr<BytesRef>>(value),
                              fieldType);
  } else {
    throw invalid_argument(L"value must be std::wstring or BytesRef");
  }
}

std::deque<wstring> const LuceneTestCase::availableLanguageTags =
    java::util::Arrays::stream(java::util::Locale::getAvailableLocales())
        .map_obj(java::util::Locale::toLanguageTag)
        .sorted()
        .distinct()
        .toArray(wstring[] ::new);

shared_ptr<Locale> LuceneTestCase::randomLocale(shared_ptr<Random> random)
{
  return localeForLanguageTag(
      availableLanguageTags[random->nextInt(availableLanguageTags.size())]);
}

shared_ptr<TimeZone> LuceneTestCase::randomTimeZone(shared_ptr<Random> random)
{
  std::deque<wstring> tzIds = TimeZone::getAvailableIDs();
  return TimeZone::getTimeZone(tzIds[random->nextInt(tzIds.size())]);
}

shared_ptr<Locale>
LuceneTestCase::localeForLanguageTag(const wstring &languageTag)
{
  return (make_shared<Locale::Builder>())->setLanguageTag(languageTag).build();
}

shared_ptr<Directory> LuceneTestCase::newFSDirectoryImpl(
    type_info clazz, shared_ptr<Path> path,
    shared_ptr<LockFactory> lf) 
{
  shared_ptr<FSDirectory> d = nullptr;
  try {
    d = CommandLineUtil::newFSDirectory(clazz, path, lf);
  } catch (const ReflectiveOperationException &e) {
    Rethrow::rethrow(e);
  }
  return d;
}

shared_ptr<Directory>
LuceneTestCase::newDirectoryImpl(shared_ptr<Random> random,
                                 const wstring &clazzName)
{
  return newDirectoryImpl(random, clazzName, FSLockFactory::getDefault());
}

shared_ptr<Directory>
LuceneTestCase::newDirectoryImpl(shared_ptr<Random> random,
                                 const wstring &clazzName,
                                 shared_ptr<LockFactory> lf)
{
  if (clazzName == L"random") {
    if (rarely(random)) {
      clazzName = RandomPicks::randomFrom(random, CORE_DIRECTORIES);
    } else {
      clazzName = L"RAMDirectory";
    }
  }

  try {
    constexpr type_info clazz = CommandLineUtil::loadDirectoryClass(clazzName);
    // If it is a FSDirectory type, try its ctor(Path)
    if (FSDirectory::typeid->isAssignableFrom(clazz)) {
      shared_ptr<Path> *const dir = createTempDir(L"index-" + clazzName);
      return newFSDirectoryImpl(clazz.asSubclass(FSDirectory::typeid), dir, lf);
    }

    // See if it has a Path/LockFactory ctor even though it's not an
    // FSDir subclass:
    try {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: Constructor<? extends org.apache.lucene.store.Directory>
      // pathCtor = clazz.getConstructor(java.nio.file.Path.class,
      // org.apache.lucene.store.LockFactory.class);
      shared_ptr < Constructor <
          ? extends Directory >> pathCtor =
                clazz.getConstructor(Path::typeid, LockFactory::typeid);
      shared_ptr<Path> *const dir = createTempDir(L"index");
      return pathCtor->newInstance(dir, lf);
    } catch (const NoSuchMethodException &nsme) {
      // Ignore
    }

    // the remaining dirs are no longer filesystem based, so we must check that
    // the passedLockFactory is not file based:
    if (!(std::dynamic_pointer_cast<FSLockFactory>(lf) != nullptr)) {
      // try ctor with only LockFactory (e.g. RAMDirectory)
      try {
        return clazz.getConstructor(LockFactory::typeid).newInstance(lf);
      } catch (const NoSuchMethodException &nsme) {
        // Ignore
      }
    }

    // try empty ctor
    return clazz.newInstance();
  } catch (const runtime_error &e) {
    Rethrow::rethrow(e);
    throw nullptr; // dummy to prevent compiler failure
  }
}

shared_ptr<IndexReader>
LuceneTestCase::wrapReader(shared_ptr<IndexReader> r) 
{
  shared_ptr<Random> random = LuceneTestCase::random();

  for (int i = 0, c = random->nextInt(6) + 1; i < c; i++) {
    switch (random->nextInt(4)) {
    case 0:
      // will create no FC insanity in atomic case, as ParallelLeafReader has
      // own cache key:
      if (VERBOSE) {
        wcout << L"NOTE: LuceneTestCase.wrapReader: wrapping previous reader="
              << r << L" with ParallelLeaf/CompositeReader" << endl;
      }
      r = (std::dynamic_pointer_cast<LeafReader>(r) != nullptr)
              ? make_shared<ParallelLeafReader>(
                    std::static_pointer_cast<LeafReader>(r))
              : make_shared<ParallelCompositeReader>(
                    std::static_pointer_cast<CompositeReader>(r));
      break;
    case 1:
      if (std::dynamic_pointer_cast<LeafReader>(r) != nullptr) {
        shared_ptr<LeafReader> *const ar =
            std::static_pointer_cast<LeafReader>(r);
        const deque<wstring> allFields = deque<wstring>();
        for (auto fi : ar->getFieldInfos()) {
          allFields.push_back(fi->name);
        }
        Collections::shuffle(allFields, random);
        constexpr int end =
            allFields.empty() ? 0 : random->nextInt(allFields.size());
        shared_ptr<Set<wstring>> *const fields =
            unordered_set<wstring>(allFields.subList(0, end));
        // will create no FC insanity as ParallelLeafReader has own cache key:
        if (VERBOSE) {
          wcout << L"NOTE: LuceneTestCase.wrapReader: wrapping previous reader="
                << r << L" with ParallelLeafReader" << endl;
        }
        r = make_shared<ParallelLeafReader>(
            make_shared<FieldFilterLeafReader>(ar, fields, false),
            make_shared<FieldFilterLeafReader>(ar, fields, true));
      }
      break;
    case 2:
      // Häckidy-Hick-Hack: a standard Reader will cause FC insanity, so we use
      // QueryUtils' reader with a fake cache key, so insanity checker cannot
      // walk along our reader:
      if (VERBOSE) {
        wcout << L"NOTE: LuceneTestCase.wrapReader: wrapping previous reader="
              << r << L" with AssertingLeaf/DirectoryReader" << endl;
      }
      if (std::dynamic_pointer_cast<LeafReader>(r) != nullptr) {
        r = make_shared<AssertingLeafReader>(
            std::static_pointer_cast<LeafReader>(r));
      } else if (std::dynamic_pointer_cast<DirectoryReader>(r) != nullptr) {
        r = make_shared<AssertingDirectoryReader>(
            std::static_pointer_cast<DirectoryReader>(r));
      }
      break;
    case 3:
      if (VERBOSE) {
        wcout << L"NOTE: LuceneTestCase.wrapReader: wrapping previous reader="
              << r << L" with MismatchedLeaf/DirectoryReader" << endl;
      }
      if (std::dynamic_pointer_cast<LeafReader>(r) != nullptr) {
        r = make_shared<MismatchedLeafReader>(
            std::static_pointer_cast<LeafReader>(r), random);
      } else if (std::dynamic_pointer_cast<DirectoryReader>(r) != nullptr) {
        r = make_shared<MismatchedDirectoryReader>(
            std::static_pointer_cast<DirectoryReader>(r), random);
      }
      break;
    default:
      fail(L"should not get here");
    }
  }

  if (VERBOSE) {
    wcout << L"wrapReader wrapped: " << r << endl;
  }

  return r;
}

shared_ptr<IndexReader>
LuceneTestCase::maybeWrapReader(shared_ptr<IndexReader> r) 
{
  if (rarely()) {
    r = wrapReader(r);
  }
  return r;
}

shared_ptr<IOContext> LuceneTestCase::newIOContext(shared_ptr<Random> random)
{
  return newIOContext(random, IOContext::DEFAULT);
}

shared_ptr<IOContext>
LuceneTestCase::newIOContext(shared_ptr<Random> random,
                             shared_ptr<IOContext> oldContext)
{
  constexpr int randomNumDocs = random->nextInt(4192);
  constexpr int size = random->nextInt(512) * randomNumDocs;
  if (oldContext->flushInfo != nullptr) {
    // Always return at least the estimatedSegmentSize of
    // the incoming IOContext:
    return make_shared<IOContext>(make_shared<FlushInfo>(
        randomNumDocs, max(oldContext->flushInfo->estimatedSegmentSize, size)));
  } else if (oldContext->mergeInfo != nullptr) {
    // Always return at least the estimatedMergeBytes of
    // the incoming IOContext:
    return make_shared<IOContext>(make_shared<MergeInfo>(
        randomNumDocs, max(oldContext->mergeInfo->estimatedMergeBytes, size),
        random->nextBoolean(), TestUtil::nextInt(random, 1, 100)));
  } else {
    // Make a totally random IOContext:
    shared_ptr<IOContext> *const context;
    switch (random->nextInt(5)) {
    case 0:
      context = IOContext::DEFAULT;
      break;
    case 1:
      context = IOContext::READ;
      break;
    case 2:
      context = IOContext::READONCE;
      break;
    case 3:
      context = make_shared<IOContext>(
          make_shared<MergeInfo>(randomNumDocs, size, true, -1));
      break;
    case 4:
      context =
          make_shared<IOContext>(make_shared<FlushInfo>(randomNumDocs, size));
      break;
    default:
      context = IOContext::DEFAULT;
    }
    return context;
  }
}

const shared_ptr<org::apache::lucene::search::QueryCache>
    LuceneTestCase::DEFAULT_QUERY_CACHE =
        org::apache::lucene::search::IndexSearcher::getDefaultQueryCache();
const shared_ptr<org::apache::lucene::search::QueryCachingPolicy>
    LuceneTestCase::DEFAULT_CACHING_POLICY = org::apache::lucene::search::
        IndexSearcher::getDefaultQueryCachingPolicy();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void overrideTestDefaultQueryCache()
void LuceneTestCase::overrideTestDefaultQueryCache()
{
  // Make sure each test method has its own cache
  overrideDefaultQueryCache();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void overrideDefaultQueryCache()
void LuceneTestCase::overrideDefaultQueryCache()
{
  // we need to reset the query cache in an @BeforeClass so that tests that
  // instantiate an IndexSearcher in an @BeforeClass method use a fresh new
  // cache
  IndexSearcher::setDefaultQueryCache(
      make_shared<LRUQueryCache>(10000, 1 << 25, [&](any context) { true; }));
  IndexSearcher::setDefaultQueryCachingPolicy(MAYBE_CACHE_POLICY);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void resetDefaultQueryCache()
void LuceneTestCase::resetDefaultQueryCache()
{
  IndexSearcher::setDefaultQueryCache(DEFAULT_QUERY_CACHE);
  IndexSearcher::setDefaultQueryCachingPolicy(DEFAULT_CACHING_POLICY);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setupCPUCoreCount()
void LuceneTestCase::setupCPUCoreCount()
{
  // Randomize core count so CMS varies its dynamic defaults, and this also
  // "fixes" core count from the master seed so it will always be the same on
  // reproduce:
  int numCores = TestUtil::nextInt(random(), 1, 4);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  System::setProperty(ConcurrentMergeScheduler::DEFAULT_CPU_CORE_COUNT_PROPERTY,
                      Integer::toString(numCores));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void restoreCPUCoreCount()
void LuceneTestCase::restoreCPUCoreCount()
{
  System::clearProperty(
      ConcurrentMergeScheduler::DEFAULT_CPU_CORE_COUNT_PROPERTY);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setupSpins()
void LuceneTestCase::setupSpins()
{
  // Randomize IOUtils.spins() count so CMS varies its dynamic defaults, and
  // this also "fixes" core count from the master seed so it will always be the
  // same on reproduce:
  bool spins = random()->nextBoolean();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  System::setProperty(ConcurrentMergeScheduler::DEFAULT_SPINS_PROPERTY,
                      Boolean::toString(spins));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void restoreSpins()
void LuceneTestCase::restoreSpins()
{
  System::clearProperty(ConcurrentMergeScheduler::DEFAULT_SPINS_PROPERTY);
}

shared_ptr<IndexSearcher> LuceneTestCase::newSearcher(shared_ptr<IndexReader> r)
{
  return newSearcher(r, true);
}

shared_ptr<IndexSearcher> LuceneTestCase::newSearcher(shared_ptr<IndexReader> r,
                                                      bool maybeWrap)
{
  return newSearcher(r, maybeWrap, true);
}

shared_ptr<IndexSearcher> LuceneTestCase::newSearcher(shared_ptr<IndexReader> r,
                                                      bool maybeWrap,
                                                      bool wrapWithAssertions)
{
  shared_ptr<Random> random = LuceneTestCase::random();
  if (usually()) {
    if (maybeWrap) {
      try {
        r = maybeWrapReader(r);
      } catch (const IOException &e) {
        Rethrow::rethrow(e);
      }
    }
    // TODO: this whole check is a coverage hack, we should move it to tests for
    // various filterreaders. ultimately whatever you do will be checkIndex'd at
    // the end anyway.
    if (random->nextInt(500) == 0 &&
        std::dynamic_pointer_cast<LeafReader>(r) != nullptr) {
      // TODO: not useful to check DirectoryReader (redundant with checkindex)
      // but maybe sometimes run this on the other crazy readers maybeWrapReader
      // creates?
      try {
        TestUtil::checkReader(r);
      } catch (const IOException &e) {
        Rethrow::rethrow(e);
      }
    }
    shared_ptr<IndexSearcher> *const ret;
    if (wrapWithAssertions) {
      ret = random->nextBoolean()
                ? make_shared<AssertingIndexSearcher>(random, r)
                : make_shared<AssertingIndexSearcher>(random, r->getContext());
    } else {
      ret = random->nextBoolean() ? make_shared<IndexSearcher>(r)
                                  : make_shared<IndexSearcher>(r->getContext());
    }
    ret->setSimilarity(classEnvRule->similarity);
    return ret;
  } else {
    int threads = 0;
    shared_ptr<ThreadPoolExecutor> *const ex;
    if (r->getReaderCacheHelper() == nullptr || random->nextBoolean()) {
      ex.reset();
    } else {
      threads = TestUtil::nextInt(random, 1, 8);
      ex = make_shared<ThreadPoolExecutor>(
          threads, threads, 0LL, TimeUnit::MILLISECONDS,
          make_shared<LinkedBlockingQueue<Runnable>>(),
          make_shared<NamedThreadFactory>(L"LuceneTestCase"));
      // uncomment to intensify LUCENE-3840
      // ex.prestartAllCoreThreads();
    }
    if (ex != nullptr) {
      if (VERBOSE) {
        wcout << L"NOTE: newSearcher using ExecutorService with " << threads
              << L" threads" << endl;
      }
      r->getReaderCacheHelper()->addClosedListener(
          [&](any cacheKey) { TestUtil::shutdownExecutorService(ex); });
    }
    shared_ptr<IndexSearcher> ret;
    if (wrapWithAssertions) {
      ret = random->nextBoolean()
                ? make_shared<AssertingIndexSearcher>(random, r, ex)
                : make_shared<AssertingIndexSearcher>(random, r->getContext(),
                                                      ex);
    } else {
      ret = random->nextBoolean()
                ? make_shared<IndexSearcher>(r, ex)
                : make_shared<IndexSearcher>(r->getContext(), ex);
    }
    ret->setSimilarity(classEnvRule->similarity);
    ret->setQueryCachingPolicy(MAYBE_CACHE_POLICY);
    return ret;
  }
}

shared_ptr<Path>
LuceneTestCase::getDataPath(const wstring &name) 
{
  try {
    return Paths->get(this->getClass().getResource(name).toURI());
  } catch (const runtime_error &e) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Cannot find resource: " + name);
  }
}

shared_ptr<InputStream>
LuceneTestCase::getDataInputStream(const wstring &name) 
{
  shared_ptr<InputStream> in_ = this->getClass().getResourceAsStream(name);
  if (in_ == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Cannot find resource: " + name);
  }
  return in_;
}

void LuceneTestCase::assertReaderEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  assertReaderStatisticsEquals(info, leftReader, rightReader);
  assertFieldsEquals(info, leftReader, MultiFields::getFields(leftReader),
                     MultiFields::getFields(rightReader), true);
  assertNormsEquals(info, leftReader, rightReader);
  assertStoredFieldsEquals(info, leftReader, rightReader);
  assertTermVectorsEquals(info, leftReader, rightReader);
  assertDocValuesEquals(info, leftReader, rightReader);
  assertDeletedDocsEquals(info, leftReader, rightReader);
  assertFieldInfosEquals(info, leftReader, rightReader);
  assertPointsEquals(info, leftReader, rightReader);
}

void LuceneTestCase::assertReaderStatisticsEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  // Somewhat redundant: we never delete docs
  assertEquals(info, leftReader->maxDoc(), rightReader->maxDoc());
  assertEquals(info, leftReader->numDocs(), rightReader->numDocs());
  assertEquals(info, leftReader->numDeletedDocs(),
               rightReader->numDeletedDocs());
  assertEquals(info, leftReader->hasDeletions(), rightReader->hasDeletions());
}

void LuceneTestCase::assertFieldsEquals(const wstring &info,
                                        shared_ptr<IndexReader> leftReader,
                                        shared_ptr<Fields> leftFields,
                                        shared_ptr<Fields> rightFields,
                                        bool deep) 
{
  // Fields could be null if there are no postings,
  // but then it must be null for both
  if (leftFields->empty() || rightFields->empty()) {
    assertNull(info, leftFields);
    assertNull(info, rightFields);
    return;
  }
  assertFieldStatisticsEquals(info, leftFields, rightFields);

  org::apache::lucene::index::Fields::const_iterator leftEnum =
      leftFields->begin();
  org::apache::lucene::index::Fields::const_iterator rightEnum =
      rightFields->begin();

  while (leftEnum != leftFields->end()) {
    wstring field = *leftEnum;
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    assertEquals(info, field, rightEnum->next());
    assertTermsEquals(info, leftReader, leftFields->terms(field),
                      rightFields->terms(field), deep);
    leftEnum++;
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(rightEnum->hasNext());
}

void LuceneTestCase::assertFieldStatisticsEquals(
    const wstring &info, shared_ptr<Fields> leftFields,
    shared_ptr<Fields> rightFields) 
{
  if (leftFields->size() != -1 && rightFields->size() != -1) {
    assertEquals(info, leftFields->size(), rightFields->size());
  }
}

void LuceneTestCase::assertTermsEquals(const wstring &info,
                                       shared_ptr<IndexReader> leftReader,
                                       shared_ptr<Terms> leftTerms,
                                       shared_ptr<Terms> rightTerms,
                                       bool deep) 
{
  if (leftTerms == nullptr || rightTerms == nullptr) {
    assertNull(info, leftTerms);
    assertNull(info, rightTerms);
    return;
  }
  assertTermsStatisticsEquals(info, leftTerms, rightTerms);
  assertEquals(L"hasOffsets", leftTerms->hasOffsets(),
               rightTerms->hasOffsets());
  assertEquals(L"hasPositions", leftTerms->hasPositions(),
               rightTerms->hasPositions());
  assertEquals(L"hasPayloads", leftTerms->hasPayloads(),
               rightTerms->hasPayloads());

  shared_ptr<TermsEnum> leftTermsEnum = leftTerms->begin();
  shared_ptr<TermsEnum> rightTermsEnum = rightTerms->begin();
  assertTermsEnumEquals(info, leftReader, leftTermsEnum, rightTermsEnum, true);

  assertTermsSeekingEquals(info, leftTerms, rightTerms);

  if (deep) {
    int numIntersections = atLeast(3);
    for (int i = 0; i < numIntersections; i++) {
      wstring re = AutomatonTestUtil::randomRegexp(random());
      shared_ptr<CompiledAutomaton> automaton = make_shared<CompiledAutomaton>(
          (make_shared<RegExp>(re, RegExp::NONE))->toAutomaton());
      if (automaton->type == CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
        // TODO: test start term too
        shared_ptr<TermsEnum> leftIntersection =
            leftTerms->intersect(automaton, nullptr);
        shared_ptr<TermsEnum> rightIntersection =
            rightTerms->intersect(automaton, nullptr);
        assertTermsEnumEquals(info, leftReader, leftIntersection,
                              rightIntersection, rarely());
      }
    }
  }
}

void LuceneTestCase::assertTermsStatisticsEquals(
    const wstring &info, shared_ptr<Terms> leftTerms,
    shared_ptr<Terms> rightTerms) 
{
  if (leftTerms->getDocCount() != -1 && rightTerms->getDocCount() != -1) {
    assertEquals(info, leftTerms->getDocCount(), rightTerms->getDocCount());
  }
  if (leftTerms->getSumDocFreq() != -1 && rightTerms->getSumDocFreq() != -1) {
    assertEquals(info, leftTerms->getSumDocFreq(), rightTerms->getSumDocFreq());
  }
  if (leftTerms->getSumTotalTermFreq() != -1 &&
      rightTerms->getSumTotalTermFreq() != -1) {
    assertEquals(info, leftTerms->getSumTotalTermFreq(),
                 rightTerms->getSumTotalTermFreq());
  }
  if (leftTerms->size() != -1 && rightTerms->size() != -1) {
    assertEquals(info, leftTerms->size(), rightTerms->size());
  }
}

LuceneTestCase::RandomBits::RandomBits(int maxDoc, double pctLive,
                                       shared_ptr<Random> random)
{
  bits = make_shared<FixedBitSet>(maxDoc);
  for (int i = 0; i < maxDoc; i++) {
    if (random->nextDouble() <= pctLive) {
      bits->set(i);
    }
  }
}

bool LuceneTestCase::RandomBits::get(int index) { return bits->get(index); }

int LuceneTestCase::RandomBits::length() { return bits->length(); }

void LuceneTestCase::assertTermsEnumEquals(const wstring &info,
                                           shared_ptr<IndexReader> leftReader,
                                           shared_ptr<TermsEnum> leftTermsEnum,
                                           shared_ptr<TermsEnum> rightTermsEnum,
                                           bool deep) 
{
  shared_ptr<BytesRef> term;
  shared_ptr<PostingsEnum> leftPositions = nullptr;
  shared_ptr<PostingsEnum> rightPositions = nullptr;
  shared_ptr<PostingsEnum> leftDocs = nullptr;
  shared_ptr<PostingsEnum> rightDocs = nullptr;

  while ((term = leftTermsEnum->next()) != nullptr) {
    assertEquals(info, term, rightTermsEnum->next());
    assertTermStatsEquals(info, leftTermsEnum, rightTermsEnum);
    if (deep) {
      assertDocsAndPositionsEnumEquals(
          info,
          leftPositions =
              leftTermsEnum->postings(leftPositions, PostingsEnum::ALL),
          rightPositions =
              rightTermsEnum->postings(rightPositions, PostingsEnum::ALL));

      assertPositionsSkippingEquals(info, leftReader, leftTermsEnum->docFreq(),
                                    leftPositions = leftTermsEnum->postings(
                                        leftPositions, PostingsEnum::ALL),
                                    rightPositions = rightTermsEnum->postings(
                                        rightPositions, PostingsEnum::ALL));

      // with freqs:
      assertDocsEnumEquals(info, leftDocs = leftTermsEnum->postings(leftDocs),
                           rightDocs = rightTermsEnum->postings(rightDocs),
                           true);

      // w/o freqs:
      assertDocsEnumEquals(
          info,
          leftDocs = leftTermsEnum->postings(leftDocs, PostingsEnum::NONE),
          rightDocs = rightTermsEnum->postings(rightDocs, PostingsEnum::NONE),
          false);

      // with freqs:
      assertDocsSkippingEquals(info, leftReader, leftTermsEnum->docFreq(),
                               leftDocs = leftTermsEnum->postings(leftDocs),
                               rightDocs = rightTermsEnum->postings(rightDocs),
                               true);

      // w/o freqs:
      assertDocsSkippingEquals(
          info, leftReader, leftTermsEnum->docFreq(),
          leftDocs = leftTermsEnum->postings(leftDocs, PostingsEnum::NONE),
          rightDocs = rightTermsEnum->postings(rightDocs, PostingsEnum::NONE),
          false);
    }
  }
  assertNull(info, rightTermsEnum->next());
}

void LuceneTestCase::assertDocsAndPositionsEnumEquals(
    const wstring &info, shared_ptr<PostingsEnum> leftDocs,
    shared_ptr<PostingsEnum> rightDocs) 
{
  assertNotNull(leftDocs);
  assertNotNull(rightDocs);
  assertEquals(info, -1, leftDocs->docID());
  assertEquals(info, -1, rightDocs->docID());
  int docid;
  while ((docid = leftDocs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    assertEquals(info, docid, rightDocs->nextDoc());
    int freq = leftDocs->freq();
    assertEquals(info, freq, rightDocs->freq());
    for (int i = 0; i < freq; i++) {
      assertEquals(info, leftDocs->nextPosition(), rightDocs->nextPosition());
      assertEquals(info, leftDocs->getPayload(), rightDocs->getPayload());
      assertEquals(info, leftDocs->startOffset(), rightDocs->startOffset());
      assertEquals(info, leftDocs->endOffset(), rightDocs->endOffset());
    }
  }
  assertEquals(info, DocIdSetIterator::NO_MORE_DOCS, rightDocs->nextDoc());
}

void LuceneTestCase::assertDocsEnumEquals(const wstring &info,
                                          shared_ptr<PostingsEnum> leftDocs,
                                          shared_ptr<PostingsEnum> rightDocs,
                                          bool hasFreqs) 
{
  if (leftDocs == nullptr) {
    assertNull(rightDocs);
    return;
  }
  assertEquals(info, -1, leftDocs->docID());
  assertEquals(info, -1, rightDocs->docID());
  int docid;
  while ((docid = leftDocs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    assertEquals(info, docid, rightDocs->nextDoc());
    if (hasFreqs) {
      assertEquals(info, leftDocs->freq(), rightDocs->freq());
    }
  }
  assertEquals(info, DocIdSetIterator::NO_MORE_DOCS, rightDocs->nextDoc());
}

void LuceneTestCase::assertDocsSkippingEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader, int docFreq,
    shared_ptr<PostingsEnum> leftDocs, shared_ptr<PostingsEnum> rightDocs,
    bool hasFreqs) 
{
  if (leftDocs == nullptr) {
    assertNull(rightDocs);
    return;
  }
  int docid = -1;
  int averageGap = leftReader->maxDoc() / (1 + docFreq);
  int skipInterval = 16;

  while (true) {
    if (random()->nextBoolean()) {
      // nextDoc()
      docid = leftDocs->nextDoc();
      assertEquals(info, docid, rightDocs->nextDoc());
    } else {
      // advance()
      int skip =
          docid + static_cast<int>(ceil(abs(
                      skipInterval + random()->nextGaussian() * averageGap)));
      docid = leftDocs->advance(skip);
      assertEquals(info, docid, rightDocs->advance(skip));
    }

    if (docid == DocIdSetIterator::NO_MORE_DOCS) {
      return;
    }
    if (hasFreqs) {
      assertEquals(info, leftDocs->freq(), rightDocs->freq());
    }
  }
}

void LuceneTestCase::assertPositionsSkippingEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader, int docFreq,
    shared_ptr<PostingsEnum> leftDocs,
    shared_ptr<PostingsEnum> rightDocs) 
{
  if (leftDocs == nullptr || rightDocs == nullptr) {
    assertNull(leftDocs);
    assertNull(rightDocs);
    return;
  }

  int docid = -1;
  int averageGap = leftReader->maxDoc() / (1 + docFreq);
  int skipInterval = 16;

  while (true) {
    if (random()->nextBoolean()) {
      // nextDoc()
      docid = leftDocs->nextDoc();
      assertEquals(info, docid, rightDocs->nextDoc());
    } else {
      // advance()
      int skip =
          docid + static_cast<int>(ceil(abs(
                      skipInterval + random()->nextGaussian() * averageGap)));
      docid = leftDocs->advance(skip);
      assertEquals(info, docid, rightDocs->advance(skip));
    }

    if (docid == DocIdSetIterator::NO_MORE_DOCS) {
      return;
    }
    int freq = leftDocs->freq();
    assertEquals(info, freq, rightDocs->freq());
    for (int i = 0; i < freq; i++) {
      assertEquals(info, leftDocs->nextPosition(), rightDocs->nextPosition());
      assertEquals(info, leftDocs->getPayload(), rightDocs->getPayload());
    }
  }
}

void LuceneTestCase::assertTermsSeekingEquals(
    const wstring &info, shared_ptr<Terms> leftTerms,
    shared_ptr<Terms> rightTerms) 
{

  // just an upper bound
  int numTests = atLeast(20);
  shared_ptr<Random> random = LuceneTestCase::random();

  shared_ptr<TermsEnum> leftEnum = nullptr;

  // collect this number of terms from the left side
  unordered_set<std::shared_ptr<BytesRef>> tests =
      unordered_set<std::shared_ptr<BytesRef>>();
  int numPasses = 0;
  while (numPasses < 10 && tests.size() < numTests) {
    leftEnum = leftTerms->begin();
    shared_ptr<BytesRef> term = nullptr;
    while ((term = leftEnum->next()) != nullptr) {
      int code = random->nextInt(10);
      if (code == 0) {
        // the term
        tests.insert(BytesRef::deepCopyOf(term));
      } else if (code == 1) {
        // truncated subsequence of term
        term = BytesRef::deepCopyOf(term);
        if (term->length > 0) {
          // truncate it
          term->length = random->nextInt(term->length);
        }
      } else if (code == 2) {
        // term, but ensure a non-zero offset
        std::deque<char> newbytes(term->length + 5);
        System::arraycopy(term->bytes, term->offset, newbytes, 5, term->length);
        tests.insert(make_shared<BytesRef>(newbytes, 5, term->length));
      } else if (code == 3) {
        switch (LuceneTestCase::random()->nextInt(3)) {
        case 0:
          tests.insert(make_shared<BytesRef>()); // before the first term
          break;
        case 1:
          tests.insert(make_shared<BytesRef>(std::deque<char>{
              static_cast<char>(0xFF),
              static_cast<char>(0xFF)})); // past the last term
          break;
        case 2:
          tests.insert(make_shared<BytesRef>(TestUtil::randomSimpleString(
              LuceneTestCase::random()))); // random term
          break;
        default:
          throw make_shared<AssertionError>();
        }
      }
    }
    numPasses++;
  }

  shared_ptr<TermsEnum> rightEnum = rightTerms->begin();

  deque<std::shared_ptr<BytesRef>> shuffledTests =
      deque<std::shared_ptr<BytesRef>>(tests);
  Collections::shuffle(shuffledTests, random);

  for (auto b : shuffledTests) {
    if (rarely()) {
      // make new enums
      leftEnum = leftTerms->begin();
      rightEnum = rightTerms->begin();
    }

    constexpr bool seekExact = LuceneTestCase::random()->nextBoolean();

    if (seekExact) {
      assertEquals(info, leftEnum->seekExact(b), rightEnum->seekExact(b));
    } else {
      SeekStatus leftStatus = leftEnum->seekCeil(b);
      SeekStatus rightStatus = rightEnum->seekCeil(b);
      assertEquals(info, leftStatus, rightStatus);
      if (leftStatus != SeekStatus::END) {
        assertEquals(info, leftEnum->term(), rightEnum->term());
        assertTermStatsEquals(info, leftEnum, rightEnum);
      }
    }
  }
}

void LuceneTestCase::assertTermStatsEquals(
    const wstring &info, shared_ptr<TermsEnum> leftTermsEnum,
    shared_ptr<TermsEnum> rightTermsEnum) 
{
  assertEquals(info, leftTermsEnum->docFreq(), rightTermsEnum->docFreq());
  if (leftTermsEnum->totalTermFreq() != -1 &&
      rightTermsEnum->totalTermFreq() != -1) {
    assertEquals(info, leftTermsEnum->totalTermFreq(),
                 rightTermsEnum->totalTermFreq());
  }
}

void LuceneTestCase::assertNormsEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  shared_ptr<Fields> leftFields = MultiFields::getFields(leftReader);
  shared_ptr<Fields> rightFields = MultiFields::getFields(rightReader);
  // Fields could be null if there are no postings,
  // but then it must be null for both
  if (leftFields->empty() || rightFields->empty()) {
    assertNull(info, leftFields);
    assertNull(info, rightFields);
    return;
  }

  for (auto field : leftFields) {
    shared_ptr<NumericDocValues> leftNorms =
        MultiDocValues::getNormValues(leftReader, field);
    shared_ptr<NumericDocValues> rightNorms =
        MultiDocValues::getNormValues(rightReader, field);
    if (leftNorms != nullptr && rightNorms != nullptr) {
      assertDocValuesEquals(info, leftReader->maxDoc(), leftNorms, rightNorms);
    } else {
      assertNull(info, leftNorms);
      assertNull(info, rightNorms);
    }
  }
}

void LuceneTestCase::assertStoredFieldsEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  assert(leftReader->maxDoc() == rightReader->maxDoc());
  for (int i = 0; i < leftReader->maxDoc(); i++) {
    shared_ptr<Document> leftDoc = leftReader->document(i);
    shared_ptr<Document> rightDoc = rightReader->document(i);

    // TODO: I think this is bogus because we don't document what the order
    // should be from these iterators, etc. I think the codec/IndexReader should
    // be free to order this stuff in whatever way it wants (e.g. maybe it packs
    // related fields together or something) To fix this, we sort the fields in
    // both documents by name, but we still assume that all instances with same
    // name are in order:
    shared_ptr<Comparator<std::shared_ptr<IndexableField>>> comp =
        make_shared<ComparatorAnonymousInnerClass>(shared_from_this());
    deque<std::shared_ptr<IndexableField>> leftFields =
        deque<std::shared_ptr<IndexableField>>(leftDoc->getFields());
    deque<std::shared_ptr<IndexableField>> rightFields =
        deque<std::shared_ptr<IndexableField>>(rightDoc->getFields());
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(leftFields, comp);
    sort(leftFields.begin(), leftFields.end(), comp);
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(rightFields, comp);
    sort(rightFields.begin(), rightFields.end(), comp);

    deque<std::shared_ptr<IndexableField>>::const_iterator leftIterator =
        leftFields.begin();
    deque<std::shared_ptr<IndexableField>>::const_iterator rightIterator =
        rightFields.begin();
    while (leftIterator != leftFields.end()) {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      assertTrue(info, rightIterator.hasNext());
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      assertStoredFieldEquals(info, *leftIterator, rightIterator.next());
      leftIterator++;
    }
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    assertFalse(info, rightIterator.hasNext());
  }
}

LuceneTestCase::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<LuceneTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

int LuceneTestCase::ComparatorAnonymousInnerClass::compare(
    shared_ptr<IndexableField> arg0, shared_ptr<IndexableField> arg1)
{
  return arg0->name().compare(arg1->name());
}

void LuceneTestCase::assertStoredFieldEquals(
    const wstring &info, shared_ptr<IndexableField> leftField,
    shared_ptr<IndexableField> rightField)
{
  assertEquals(info, leftField->name(), rightField->name());
  assertEquals(info, leftField->binaryValue(), rightField->binaryValue());
  assertEquals(info, leftField->stringValue(), rightField->stringValue());
  assertEquals(info, leftField->numericValue(), rightField->numericValue());
  // TODO: should we check the FT at all?
}

void LuceneTestCase::assertTermVectorsEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  assert(leftReader->maxDoc() == rightReader->maxDoc());
  for (int i = 0; i < leftReader->maxDoc(); i++) {
    shared_ptr<Fields> leftFields = leftReader->getTermVectors(i);
    shared_ptr<Fields> rightFields = rightReader->getTermVectors(i);
    assertFieldsEquals(info, leftReader, leftFields, rightFields, rarely());
  }
}

shared_ptr<Set<wstring>>
LuceneTestCase::getDVFields(shared_ptr<IndexReader> reader)
{
  shared_ptr<Set<wstring>> fields = unordered_set<wstring>();
  for (auto fi : MultiFields::getMergedFieldInfos(reader)) {
    if (fi->getDocValuesType() != DocValuesType::NONE) {
      fields->add(fi->name);
    }
  }

  return fields;
}

void LuceneTestCase::assertDocValuesEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  shared_ptr<Set<wstring>> leftFields = getDVFields(leftReader);
  shared_ptr<Set<wstring>> rightFields = getDVFields(rightReader);
  assertEquals(info, leftFields, rightFields);

  for (auto field : leftFields) {
    {
      // TODO: clean this up... very messy
      shared_ptr<NumericDocValues> leftValues =
          MultiDocValues::getNumericValues(leftReader, field);
      shared_ptr<NumericDocValues> rightValues =
          MultiDocValues::getNumericValues(rightReader, field);
      if (leftValues != nullptr && rightValues != nullptr) {
        assertDocValuesEquals(info, leftReader->maxDoc(), leftValues,
                              rightValues);
      } else {
        assertTrue(info + L": left numeric doc values for field=\"" + field +
                       L"\" are not null",
                   leftValues == nullptr ||
                       leftValues->nextDoc() == DocIdSetIterator::NO_MORE_DOCS);
        assertTrue(info + L": right numeric doc values for field=\"" + field +
                       L"\" are not null",
                   rightValues == nullptr ||
                       rightValues->nextDoc() ==
                           DocIdSetIterator::NO_MORE_DOCS);
      }
    }

    {
      shared_ptr<BinaryDocValues> leftValues =
          MultiDocValues::getBinaryValues(leftReader, field);
      shared_ptr<BinaryDocValues> rightValues =
          MultiDocValues::getBinaryValues(rightReader, field);
      if (leftValues != nullptr && rightValues != nullptr) {
        while (true) {
          int docID = leftValues->nextDoc();
          assertEquals(docID, rightValues->nextDoc());
          if (docID == DocIdSetIterator::NO_MORE_DOCS) {
            break;
          }
          assertEquals(leftValues->binaryValue(), rightValues->binaryValue());
        }
      } else {
        assertTrue(info,
                   leftValues == nullptr ||
                       leftValues->nextDoc() == DocIdSetIterator::NO_MORE_DOCS);
        assertTrue(info, rightValues == nullptr ||
                             rightValues->nextDoc() ==
                                 DocIdSetIterator::NO_MORE_DOCS);
      }
    }

    {
      shared_ptr<SortedDocValues> leftValues =
          MultiDocValues::getSortedValues(leftReader, field);
      shared_ptr<SortedDocValues> rightValues =
          MultiDocValues::getSortedValues(rightReader, field);
      if (leftValues != nullptr && rightValues != nullptr) {
        // numOrds
        assertEquals(info, leftValues->getValueCount(),
                     rightValues->getValueCount());
        // ords
        for (int i = 0; i < leftValues->getValueCount(); i++) {
          shared_ptr<BytesRef> *const left =
              BytesRef::deepCopyOf(leftValues->lookupOrd(i));
          shared_ptr<BytesRef> *const right = rightValues->lookupOrd(i);
          assertEquals(info, left, right);
        }
        // bytes
        for (int docID = 0; docID < leftReader->maxDoc(); docID++) {
          assertEquals(docID, leftValues->nextDoc());
          assertEquals(docID, rightValues->nextDoc());
          shared_ptr<BytesRef> *const left =
              BytesRef::deepCopyOf(leftValues->binaryValue());
          shared_ptr<BytesRef> *const right = rightValues->binaryValue();
          assertEquals(info, left, right);
        }
      } else {
        assertNull(info, leftValues);
        assertNull(info, rightValues);
      }
    }

    {
      shared_ptr<SortedSetDocValues> leftValues =
          MultiDocValues::getSortedSetValues(leftReader, field);
      shared_ptr<SortedSetDocValues> rightValues =
          MultiDocValues::getSortedSetValues(rightReader, field);
      if (leftValues != nullptr && rightValues != nullptr) {
        // numOrds
        assertEquals(info, leftValues->getValueCount(),
                     rightValues->getValueCount());
        // ords
        for (int i = 0; i < leftValues->getValueCount(); i++) {
          shared_ptr<BytesRef> *const left =
              BytesRef::deepCopyOf(leftValues->lookupOrd(i));
          shared_ptr<BytesRef> *const right = rightValues->lookupOrd(i);
          assertEquals(info, left, right);
        }
        // ord lists
        while (true) {
          int docID = leftValues->nextDoc();
          assertEquals(docID, rightValues->nextDoc());
          if (docID == DocIdSetIterator::NO_MORE_DOCS) {
            break;
          }
          int64_t ord;
          while ((ord = leftValues->nextOrd()) !=
                 SortedSetDocValues::NO_MORE_ORDS) {
            assertEquals(info, ord, rightValues->nextOrd());
          }
          assertEquals(info, SortedSetDocValues::NO_MORE_ORDS,
                       rightValues->nextOrd());
        }
      } else {
        assertNull(info, leftValues);
        assertNull(info, rightValues);
      }
    }

    {
      shared_ptr<SortedNumericDocValues> leftValues =
          MultiDocValues::getSortedNumericValues(leftReader, field);
      shared_ptr<SortedNumericDocValues> rightValues =
          MultiDocValues::getSortedNumericValues(rightReader, field);
      if (leftValues != nullptr && rightValues != nullptr) {
        while (true) {
          int docID = leftValues->nextDoc();
          assertEquals(docID, rightValues->nextDoc());
          if (docID == DocIdSetIterator::NO_MORE_DOCS) {
            break;
          }
          assertEquals(info, leftValues->docValueCount(),
                       rightValues->docValueCount());
          for (int j = 0; j < leftValues->docValueCount(); j++) {
            assertEquals(info, leftValues->nextValue(),
                         rightValues->nextValue());
          }
        }
      } else {
        assertNull(info, leftValues);
        assertNull(info, rightValues);
      }
    }
  }
}

void LuceneTestCase::assertDocValuesEquals(
    const wstring &info, int num, shared_ptr<NumericDocValues> leftDocValues,
    shared_ptr<NumericDocValues> rightDocValues) 
{
  assertNotNull(info, leftDocValues);
  assertNotNull(info, rightDocValues);
  while (true) {
    int leftDocID = leftDocValues->nextDoc();
    int rightDocID = rightDocValues->nextDoc();
    assertEquals(leftDocID, rightDocID);
    if (leftDocID == DocIdSetIterator::NO_MORE_DOCS) {
      return;
    }
    assertEquals(leftDocValues->longValue(), rightDocValues->longValue());
  }
}

void LuceneTestCase::assertDeletedDocsEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  assert(leftReader->numDeletedDocs() == rightReader->numDeletedDocs());
  shared_ptr<Bits> leftBits = MultiFields::getLiveDocs(leftReader);
  shared_ptr<Bits> rightBits = MultiFields::getLiveDocs(rightReader);

  if (leftBits == nullptr || rightBits == nullptr) {
    assertNull(info, leftBits);
    assertNull(info, rightBits);
    return;
  }

  assert(leftReader->maxDoc() == rightReader->maxDoc());
  assertEquals(info, leftBits->length(), rightBits->length());
  for (int i = 0; i < leftReader->maxDoc(); i++) {
    assertEquals(info, leftBits->get(i), rightBits->get(i));
  }
}

void LuceneTestCase::assertFieldInfosEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  shared_ptr<FieldInfos> leftInfos =
      MultiFields::getMergedFieldInfos(leftReader);
  shared_ptr<FieldInfos> rightInfos =
      MultiFields::getMergedFieldInfos(rightReader);

  // TODO: would be great to verify more than just the names of the fields!
  set<wstring> left = set<wstring>();
  set<wstring> right = set<wstring>();

  for (auto fi : leftInfos) {
    left.insert(fi->name);
  }

  for (auto fi : rightInfos) {
    right.insert(fi->name);
  }

  assertEquals(info, left, right);
}

unordered_map<int, Set<std::shared_ptr<BytesRef>>>
LuceneTestCase::uninvert(const wstring &fieldName,
                         shared_ptr<IndexReader> reader) 
{
  const unordered_map<int, Set<std::shared_ptr<BytesRef>>> docValues =
      unordered_map<int, Set<std::shared_ptr<BytesRef>>>();
  for (auto ctx : reader->leaves()) {

    shared_ptr<PointValues> points = ctx->reader()->getPointValues(fieldName);
    if (points == nullptr) {
      continue;
    }

    points->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
        shared_from_this(), docValues, ctx));
  }

  return docValues;
}

LuceneTestCase::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<LuceneTestCase> outerInstance,
        unordered_map<int, Set<std::shared_ptr<BytesRef>>> &docValues,
        shared_ptr<LeafReaderContext> ctx)
{
  this->outerInstance = outerInstance;
  this->docValues = docValues;
  this->ctx = ctx;
}

void LuceneTestCase::IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  throw make_shared<UnsupportedOperationException>();
}

void LuceneTestCase::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  int topDocID = ctx->docBase + docID;
  if (docValues.find(topDocID) != docValues.end() == false) {
    docValues.emplace(topDocID, unordered_set<std::shared_ptr<BytesRef>>());
  }
  docValues[topDocID]->add(make_shared<BytesRef>(packedValue.clone()));
}

PointValues::Relation
LuceneTestCase::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  // We pretend our query shape is so hairy that it crosses every single cell:
  return PointValues::Relation::CELL_CROSSES_QUERY;
}

void LuceneTestCase::assertPointsEquals(
    const wstring &info, shared_ptr<IndexReader> leftReader,
    shared_ptr<IndexReader> rightReader) 
{
  shared_ptr<FieldInfos> fieldInfos1 =
      MultiFields::getMergedFieldInfos(leftReader);
  shared_ptr<FieldInfos> fieldInfos2 =
      MultiFields::getMergedFieldInfos(rightReader);
  for (auto fieldInfo1 : fieldInfos1) {
    if (fieldInfo1->getPointDimensionCount() != 0) {
      shared_ptr<FieldInfo> fieldInfo2 =
          fieldInfos2->fieldInfo(fieldInfo1->name);
      // same dimension count?
      assertEquals(info, fieldInfo2->getPointDimensionCount(),
                   fieldInfo2->getPointDimensionCount());
      // same bytes per dimension?
      assertEquals(info, fieldInfo2->getPointNumBytes(),
                   fieldInfo2->getPointNumBytes());

      assertEquals(info + L" field=" + fieldInfo1->name,
                   uninvert(fieldInfo1->name, leftReader),
                   uninvert(fieldInfo1->name, rightReader));
    }
  }

  // make sure FieldInfos2 doesn't have any point fields that FieldInfo1 didn't
  // have
  for (auto fieldInfo2 : fieldInfos2) {
    if (fieldInfo2->getPointDimensionCount() != 0) {
      shared_ptr<FieldInfo> fieldInfo1 =
          fieldInfos1->fieldInfo(fieldInfo2->name);
      // same dimension count?
      assertEquals(info, fieldInfo2->getPointDimensionCount(),
                   fieldInfo1->getPointDimensionCount());
      // same bytes per dimension?
      assertEquals(info, fieldInfo2->getPointNumBytes(),
                   fieldInfo1->getPointNumBytes());

      // we don't need to uninvert and compare here ... we did that in the first
      // loop above
    }
  }
}

template <typename T>
T LuceneTestCase::expectThrows(type_info<T> &expectedType,
                               ThrowingRunnable runnable)
{
  static_assert(is_base_of<runtime_error, T>::value,
                L"T must inherit from std::runtime_error");

  try {
    runnable();
  } catch (const runtime_error &e) {
    if (expectedType.isInstance(e)) {
      return expectedType.cast(e);
    }
    shared_ptr<AssertionFailedError> assertion =
        make_shared<AssertionFailedError>(
            L"Unexpected exception type, expected " + expectedType.name() +
            L" but got " + e);
    assertion->initCause(e);
    throw assertion;
  }
  // C++ TODO: The following line could not be converted:
  throw junit.framework.AssertionFailedError(L"Expected exception " +
                                             expectedType.getSimpleName() +
                                             L" but no exception was thrown");
}

template <typename TO, typename TW>
TW LuceneTestCase::expectThrows(type_info<TO> &expectedOuterType,
                                type_info<TW> &expectedWrappedType,
                                ThrowingRunnable runnable)
{
  static_assert(is_base_of<runtime_error, TW>::value,
                L"TW must inherit from std::runtime_error");
  static_assert(is_base_of<runtime_error, TO>::value,
                L"TO must inherit from std::runtime_error");

  try {
    runnable();
  } catch (const runtime_error &e) {
    if (expectedOuterType.isInstance(e)) {
      runtime_error cause = e.getCause();
      if (expectedWrappedType.isInstance(cause)) {
        return expectedWrappedType.cast(cause);
      } else {
        shared_ptr<AssertionFailedError> assertion =
            make_shared<AssertionFailedError>(
                L"Unexpected wrapped exception type, expected " +
                expectedWrappedType.name());
        assertion->initCause(e);
        throw assertion;
      }
    }
    shared_ptr<AssertionFailedError> assertion =
        make_shared<AssertionFailedError>(
            L"Unexpected outer exception type, expected " +
            expectedOuterType.name());
    assertion->initCause(e);
    throw assertion;
  }
  // C++ TODO: The following line could not be converted:
  throw junit.framework.AssertionFailedError(L"Expected outer exception " +
                                             expectedOuterType.getSimpleName());
}

bool LuceneTestCase::slowFileExists(shared_ptr<Directory> dir,
                                    const wstring &fileName) 
{
  try {
    delete dir->openInput(fileName, IOContext::DEFAULT);
    return true;
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | FileNotFoundException e) {
    return false;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static java.nio.file.Path
// getBaseTempDirForTestClass()
shared_ptr<Path> LuceneTestCase::getBaseTempDirForTestClass()
{
  return tempFilesCleanupRule->getPerTestClassTempDir();
}

shared_ptr<Path> LuceneTestCase::createTempDir()
{
  return createTempDir(L"tempDir");
}

shared_ptr<Path> LuceneTestCase::createTempDir(const wstring &prefix)
{
  return tempFilesCleanupRule->createTempDir(prefix);
}

shared_ptr<Path>
LuceneTestCase::createTempFile(const wstring &prefix,
                               const wstring &suffix) 
{
  return tempFilesCleanupRule->createTempFile(prefix, suffix);
}

shared_ptr<Path> LuceneTestCase::createTempFile() 
{
  return createTempFile(L"tempFile", L".tmp");
}

template <typename T>
T LuceneTestCase::runWithRestrictedPermissions(
    shared_ptr<PrivilegedExceptionAction<T>> action,
    deque<Permission> &permissions) 
{
  assumeTrue(L"runWithRestrictedPermissions requires a SecurityManager enabled",
             System::getSecurityManager() != nullptr);
  // be sure to have required permission, otherwise doPrivileged runs with *no*
  // permissions:
  AccessController::checkPermission(
      make_shared<SecurityPermission>(L"createAccessControlContext"));
  shared_ptr<PermissionCollection> *const perms = make_shared<Permissions>();
  Arrays::stream(permissions).forEach(perms::add);
  shared_ptr<AccessControlContext> *const ctx =
      make_shared<AccessControlContext>(
          std::deque<std::shared_ptr<ProtectionDomain>>{
              make_shared<ProtectionDomain>(nullptr, perms)});
  try {
    return AccessController::doPrivileged(action, ctx);
  } catch (const PrivilegedActionException &e) {
    throw e->getException();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "dodges JDK-8071862") public
// static int collate(java.text.Collator collator, std::wstring s1, std::wstring s2)
int LuceneTestCase::collate(shared_ptr<Collator> collator, const wstring &s1,
                            const wstring &s2)
{
  int v1 = collator->compare(s1, s2);
  int v2 =
      collator->getCollationKey(s1)->compareTo(collator->getCollationKey(s2));
  // if collation keys don't really respect collation order, things are screwed.
  assumeTrue(L"hit JDK collator bug",
             Integer::signum(v1) == Integer::signum(v2));
  return v1;
}
} // namespace org::apache::lucene::util