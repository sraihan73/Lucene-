using namespace std;

#include "BenchmarkTestCase.h"
#include "../../../../../java/org/apache/lucene/benchmark/byTask/Benchmark.h"

namespace org::apache::lucene::benchmark
{
using Benchmark = org::apache::lucene::benchmark::byTask::Benchmark;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<java::nio::file::Path> BenchmarkTestCase::WORKDIR;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClassBenchmarkTestCase()
void BenchmarkTestCase::beforeClassBenchmarkTestCase()
{
  WORKDIR = createTempDir(L"benchmark");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassBenchmarkTestCase()
void BenchmarkTestCase::afterClassBenchmarkTestCase() { WORKDIR.reset(); }

shared_ptr<Path> BenchmarkTestCase::getWorkDir() { return WORKDIR; }

void BenchmarkTestCase::copyToWorkDir(const wstring &resourceName) throw(
    IOException)
{
  shared_ptr<Path> target = getWorkDir()->resolve(resourceName);
  Files::deleteIfExists(target);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
  // getClass().getResourceAsStream(resourceName))
  {
    java::io::InputStream resource =
        getClass().getResourceAsStream(resourceName);
    Files::copy(resource, target);
  }
}

wstring BenchmarkTestCase::getWorkDirResourcePath(const wstring &resourceName)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getWorkDir()
      ->resolve(resourceName)
      .toAbsolutePath()
      ->toString()
      ->replace(L"\\", L"/");
}

wstring BenchmarkTestCase::getWorkDirPath()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getWorkDir()->toAbsolutePath()->toString()->replace(L"\\", L"/");
}

shared_ptr<Benchmark> BenchmarkTestCase::execBenchmark(
    std::deque<wstring> &algLines) 
{
  wstring algText = algLinesToText(algLines);
  logTstLogic(algText);
  shared_ptr<Benchmark> benchmark =
      make_shared<Benchmark>(make_shared<StringReader>(algText));
  benchmark->execute();
  return benchmark;
}

const wstring BenchmarkTestCase::NEW_LINE =
    System::getProperty(L"line.separator");

wstring BenchmarkTestCase::algLinesToText(std::deque<wstring> &algLines)
{
  wstring indent = L"  ";
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < propLines.size(); i++) {
    sb->append(indent)->append(propLines[i])->append(NEW_LINE);
  }
  for (int i = 0; i < algLines.size(); i++) {
    sb->append(indent)->append(algLines[i])->append(NEW_LINE);
  }
  return sb->toString();
}

void BenchmarkTestCase::logTstLogic(const wstring &txt)
{
  if (!VERBOSE) {
    return;
  }
  wcout << L"Test logic of:" << endl;
  wcout << txt << endl;
}
} // namespace org::apache::lucene::benchmark