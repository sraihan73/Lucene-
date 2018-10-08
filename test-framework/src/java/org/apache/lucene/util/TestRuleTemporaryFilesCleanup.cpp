using namespace std;

#include "TestRuleTemporaryFilesCleanup.h"

namespace org::apache::lucene::util
{
using DisableFsyncFS = org::apache::lucene::mockfile::DisableFsyncFS;
using ExtrasFS = org::apache::lucene::mockfile::ExtrasFS;
using HandleLimitFS = org::apache::lucene::mockfile::HandleLimitFS;
using LeakFS = org::apache::lucene::mockfile::LeakFS;
using ShuffleFS = org::apache::lucene::mockfile::ShuffleFS;
using VerboseFS = org::apache::lucene::mockfile::VerboseFS;
using WindowsFS = org::apache::lucene::mockfile::WindowsFS;
using com::carrotsearch::randomizedtesting::RandomizedContext;
using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;
using org::apache::lucene::util::LuceneTestCase::SuppressFileSystems;
using org::apache::lucene::util::LuceneTestCase::SuppressFsync;
using org::apache::lucene::util::LuceneTestCase::SuppressTempFileChecks;
const deque<std::shared_ptr<java::nio::file::Path>>
    TestRuleTemporaryFilesCleanup::cleanupQueue =
        deque<std::shared_ptr<java::nio::file::Path>>();

TestRuleTemporaryFilesCleanup::TestRuleTemporaryFilesCleanup(
    shared_ptr<TestRuleMarkFailure> failureMarker)
    : failureMarker(failureMarker)
{
}

void TestRuleTemporaryFilesCleanup::registerToRemoveAfterSuite(
    shared_ptr<Path> f)
{
  assert(f != nullptr);

  if (LuceneTestCase::LEAVE_TEMPORARY) {
    System::err::println(L"INFO: Will leave temporary file: " +
                         f->toAbsolutePath());
    return;
  }

  {
    lock_guard<mutex> lock(cleanupQueue);
    cleanupQueue.push_back(f);
  }
}

void TestRuleTemporaryFilesCleanup::before() 
{
  TestRuleAdapter::before();

  assert(tempDirBase == nullptr);
  fileSystem = initializeFileSystem();
  javaTempDir = initializeJavaTempDir();
}

bool TestRuleTemporaryFilesCleanup::allowed(shared_ptr<Set<wstring>> avoid,
                                            type_info clazz)
{
  if (avoid->contains(L"*") || avoid->contains(clazz.name())) {
    return false;
  } else {
    return true;
  }
}

shared_ptr<FileSystem> TestRuleTemporaryFilesCleanup::initializeFileSystem()
{
  type_info targetClass = RandomizedContext::current().getTargetClass();
  shared_ptr<Set<wstring>> avoid = unordered_set<wstring>();
  if (targetClass.isAnnotationPresent(SuppressFileSystems::typeid)) {
    shared_ptr<SuppressFileSystems> a =
        targetClass.getAnnotation(SuppressFileSystems::typeid);
    avoid->addAll(Arrays::asList(a->value()));
  }
  shared_ptr<FileSystem> fs = FileSystems::getDefault();
  if (LuceneTestCase::VERBOSE && allowed(avoid, VerboseFS::typeid)) {
    fs = (make_shared<VerboseFS>(
              fs, make_shared<TestRuleSetupAndRestoreClassEnv::
                                  ThreadNameFixingPrintStreamInfoStream>(
                      System::out)))
             ->getFileSystem(nullptr);
  }

  shared_ptr<Random> random = RandomizedContext::current().getRandom();

  // speed up tests by omitting actual fsync calls to the hardware most of the
  // time.
  if (targetClass.isAnnotationPresent(SuppressFsync::typeid) ||
      random->nextInt(100) > 0) {
    if (allowed(avoid, DisableFsyncFS::typeid)) {
      fs = (make_shared<DisableFsyncFS>(fs))->getFileSystem(nullptr);
    }
  }

  // impacts test reproducibility across platforms.
  if (random->nextInt(100) > 0) {
    if (allowed(avoid, ShuffleFS::typeid)) {
      fs = (make_shared<ShuffleFS>(fs, random->nextLong()))
               ->getFileSystem(nullptr);
    }
  }

  // otherwise, wrap with mockfilesystems for additional checks. some
  // of these have side effects (e.g. concurrency) so it doesn't always happen.
  if (random->nextInt(10) > 0) {
    if (allowed(avoid, LeakFS::typeid)) {
      fs = (make_shared<LeakFS>(fs))->getFileSystem(nullptr);
    }
    if (allowed(avoid, HandleLimitFS::typeid)) {
      fs = (make_shared<HandleLimitFS>(fs, MAX_OPEN_FILES))
               ->getFileSystem(nullptr);
    }
    // windows is currently slow
    if (random->nextInt(10) == 0) {
      // don't try to emulate windows on windows: they don't get along
      if (!Constants::WINDOWS && allowed(avoid, WindowsFS::typeid)) {
        fs = (make_shared<WindowsFS>(fs))->getFileSystem(nullptr);
      }
    }
    if (allowed(avoid, ExtrasFS::typeid)) {
      fs = (make_shared<ExtrasFS>(fs, random->nextInt(4) == 0,
                                  random->nextBoolean()))
               ->getFileSystem(nullptr);
    }
  }
  if (LuceneTestCase::VERBOSE) {
    wcout << L"filesystem: " << fs->provider() << endl;
  }

  return fs->provider().getFileSystem(URI::create(L"file:///"));
}

shared_ptr<Path>
TestRuleTemporaryFilesCleanup::initializeJavaTempDir() 
{
  shared_ptr<Path> javaTempDir = fileSystem->getPath(
      System::getProperty(L"tempDir", System::getProperty(L"java.io.tmpdir")));

  Files::createDirectories(javaTempDir);

  assert(Files::isDirectory(javaTempDir) && Files::isWritable(javaTempDir));

  return javaTempDir->toRealPath();
}

void TestRuleTemporaryFilesCleanup::afterAlways(
    deque<runtime_error> &errors) 
{
  // Drain cleanup queue and clear it.
  std::deque<std::shared_ptr<Path>> everything;
  const wstring tempDirBasePath;
  {
    lock_guard<mutex> lock(cleanupQueue);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tempDirBasePath =
        (tempDirBase != nullptr ? tempDirBase->toAbsolutePath()->toString()
                                : nullptr);
    tempDirBase.reset();

    reverse(cleanupQueue.begin(), cleanupQueue.end());
    everything = std::deque<std::shared_ptr<Path>>(cleanupQueue.size());
    cleanupQueue.toArray(everything);
    cleanupQueue.clear();
  }

  // Only check and throw an IOException on un-removable files if the test
  // was successful. Otherwise just report the path of temporary files
  // and leave them there.
  if (failureMarker->wasSuccessful()) {

    try {
      IOUtils::rm(everything);
    } catch (const IOException &e) {
      type_info suiteClass = RandomizedContext::current().getTargetClass();
      if (suiteClass.isAnnotationPresent(SuppressTempFileChecks::typeid)) {
        System::err::println(
            L"WARNING: Leftover undeleted temporary files (bugUrl: " +
            suiteClass.getAnnotation(SuppressTempFileChecks::typeid).bugUrl() +
            L"): " + e->getMessage());
        return;
      }
      throw e;
    }
    if (fileSystem != FileSystems::getDefault()) {
      fileSystem->close();
    }
  } else {
    if (tempDirBasePath != L"") {
      System::err::println(L"NOTE: leaving temporary files on disk at: " +
                           tempDirBasePath);
    }
  }
}

shared_ptr<Path> TestRuleTemporaryFilesCleanup::getPerTestClassTempDir()
{
  if (tempDirBase == nullptr) {
    shared_ptr<RandomizedContext> ctx = RandomizedContext::current();
    type_info clazz = ctx->getTargetClass();
    wstring prefix = clazz.getName();
    prefix = prefix.replaceFirst(L"^org.apache.lucene.", L"lucene.");
    prefix = prefix.replaceFirst(L"^org.apache.solr.", L"solr.");

    int attempt = 0;
    shared_ptr<Path> f;
    bool success = false;
    do {
      if (attempt++ >= TEMP_NAME_RETRY_THRESHOLD) {
        throw runtime_error(
            L"Failed to get a temporary name too many times, check your temp "
            L"directory and consider manually cleaning it: " +
            javaTempDir->toAbsolutePath());
      }
      f = javaTempDir->resolve(
          prefix + L"_" + ctx->getRunnerSeedAsString() + L"-" +
          wstring::format(Locale::ENGLISH, L"%03d", attempt));
      try {
        Files::createDirectory(f);
        success = true;
      } catch (const IOException &ignore) {
      }
    } while (!success);

    tempDirBase = f;
    registerToRemoveAfterSuite(tempDirBase);
  }
  return tempDirBase;
}

shared_ptr<Path>
TestRuleTemporaryFilesCleanup::createTempDir(const wstring &prefix)
{
  shared_ptr<Path> base = getPerTestClassTempDir();

  int attempt = 0;
  shared_ptr<Path> f;
  bool success = false;
  do {
    if (attempt++ >= TEMP_NAME_RETRY_THRESHOLD) {
      throw runtime_error(
          L"Failed to get a temporary name too many times, check your temp "
          L"directory and consider manually cleaning it: " +
          base->toAbsolutePath());
    }
    f = base->resolve(prefix + L"-" +
                      wstring::format(Locale::ENGLISH, L"%03d", attempt));
    try {
      Files::createDirectory(f);
      success = true;
    } catch (const IOException &ignore) {
    }
  } while (!success);

  registerToRemoveAfterSuite(f);
  return f;
}

shared_ptr<Path> TestRuleTemporaryFilesCleanup::createTempFile(
    const wstring &prefix, const wstring &suffix) 
{
  shared_ptr<Path> base = getPerTestClassTempDir();

  int attempt = 0;
  shared_ptr<Path> f;
  bool success = false;
  do {
    if (attempt++ >= TEMP_NAME_RETRY_THRESHOLD) {
      throw runtime_error(
          L"Failed to get a temporary name too many times, check your temp "
          L"directory and consider manually cleaning it: " +
          base->toAbsolutePath());
    }
    f = base->resolve(prefix + L"-" +
                      wstring::format(Locale::ENGLISH, L"%03d", attempt) +
                      suffix);
    try {
      Files::createFile(f);
      success = true;
    } catch (const IOException &ignore) {
    }
  } while (!success);

  registerToRemoveAfterSuite(f);
  return f;
}
} // namespace org::apache::lucene::util