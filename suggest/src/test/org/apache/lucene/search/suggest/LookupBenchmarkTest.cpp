using namespace std;

#include "LookupBenchmarkTest.h"

namespace org::apache::lucene::search::suggest
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using AnalyzingInfixSuggester =
    org::apache::lucene::search::suggest::analyzing::AnalyzingInfixSuggester;
using AnalyzingSuggester =
    org::apache::lucene::search::suggest::analyzing::AnalyzingSuggester;
using BlendedInfixSuggester =
    org::apache::lucene::search::suggest::analyzing::BlendedInfixSuggester;
using FreeTextSuggester =
    org::apache::lucene::search::suggest::analyzing::FreeTextSuggester;
using FuzzySuggester =
    org::apache::lucene::search::suggest::analyzing::FuzzySuggester;
using FSTCompletionLookup =
    org::apache::lucene::search::suggest::fst::FSTCompletionLookup;
using WFSTCompletionLookup =
    org::apache::lucene::search::suggest::fst::WFSTCompletionLookup;
using JaspellLookup =
    org::apache::lucene::search::suggest::jaspell::JaspellLookup;
using TSTLookup = org::apache::lucene::search::suggest::tst::TSTLookup;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using namespace org::apache::lucene::util;
using org::junit::BeforeClass;
using org::junit::Ignore;
const shared_ptr<java::util::Random> LookupBenchmarkTest::random =
    make_shared<java::util::Random>(0xdeadbeef);
std::deque<std::shared_ptr<Input>> LookupBenchmarkTest::dictionaryInput;
deque<std::shared_ptr<Input>> LookupBenchmarkTest::benchmarkInput;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setup() throws Exception
void LookupBenchmarkTest::setup() 
{
  assert((false, L"disable assertions before running benchmarks!"));
  deque<std::shared_ptr<Input>> input = readTop50KWiki();
  Collections::shuffle(input, random);
  LookupBenchmarkTest::dictionaryInput =
      input.toArray(std::deque<std::shared_ptr<Input>>(input.size()));
  Collections::shuffle(input, random);
  LookupBenchmarkTest::benchmarkInput = input;
}

const shared_ptr<java::nio::charset::Charset> LookupBenchmarkTest::UTF_8 =
    java::nio::charset::StandardCharsets::UTF_8;

deque<std::shared_ptr<Input>>
LookupBenchmarkTest::readTop50KWiki() 
{
  deque<std::shared_ptr<Input>> input = deque<std::shared_ptr<Input>>();
  shared_ptr<URL> resource =
      LookupBenchmarkTest::typeid->getResource(L"Top50KWiki.utf8");
  assert((resource != nullptr, L"Resource missing: Top50KWiki.utf8"));

  wstring line = L"";
  shared_ptr<BufferedReader> br = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(resource->openStream(), UTF_8));
  while ((line = br->readLine()) != L"") {
    int tab = (int)line.find(L'|');
    assertTrue(L"No | separator?: " + line, tab >= 0);
    int weight = stoi(line.substr(tab + 1));
    wstring key = line.substr(0, tab);
    input.push_back(make_shared<Input>(key, weight));
  }
  br->close();
  return input;
}

void LookupBenchmarkTest::testConstructionTime() 
{
  System::err::println(L"-- construction time");
  for (auto cls : benchmarkClasses) {
    shared_ptr<BenchmarkResult> result = measure(
        make_shared<CallableAnonymousInnerClass>(shared_from_this(), cls));

    // C++ TODO: There is no native C++ equivalent to 'toString':
    System::err::println(wstring::format(
        Locale::ROOT, L"%-15s input: %d, time[ms]: %s", cls.getSimpleName(),
        dictionaryInput.size(), result->average->toString()));
  }
}

LookupBenchmarkTest::CallableAnonymousInnerClass::CallableAnonymousInnerClass(
    shared_ptr<LookupBenchmarkTest> outerInstance, type_info cls)
{
  this->outerInstance = outerInstance;
  this->cls = cls;
}

optional<int>
LookupBenchmarkTest::CallableAnonymousInnerClass::call() 
{
  shared_ptr<Lookup> *const lookup =
      outerInstance->buildLookup(cls, dictionaryInput);
  return lookup->hashCode();
}

void LookupBenchmarkTest::testStorageNeeds() 
{
  System::err::println(L"-- RAM consumption");
  for (auto cls : benchmarkClasses) {
    shared_ptr<Lookup> lookup = buildLookup(cls, dictionaryInput);
    int64_t sizeInBytes = lookup->ramBytesUsed();
    System::err::println(wstring::format(Locale::ROOT, L"%-15s size[B]:%,13d",
                                         lookup->getClass().getSimpleName(),
                                         sizeInBytes));
  }
}

shared_ptr<Lookup> LookupBenchmarkTest::buildLookup(
    type_info cls,
    std::deque<std::shared_ptr<Input>> &input) 
{
  shared_ptr<Lookup> lookup = nullptr;
  try {
    lookup = cls.newInstance();
  } catch (const InstantiationException &e) {
    shared_ptr<Analyzer> a =
        make_shared<MockAnalyzer>(random, MockTokenizer::KEYWORD, false);
    if (cls == AnalyzingInfixSuggester::typeid ||
        cls == BlendedInfixSuggester::typeid) {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: Constructor<? extends Lookup> ctor =
      // cls.getConstructor(org.apache.lucene.store.Directory.class,
      // org.apache.lucene.analysis.Analyzer.class);
      shared_ptr < Constructor < ? extends Lookup >> ctor = cls.getConstructor(
                                       Directory::typeid, Analyzer::typeid);
      lookup = ctor->newInstance(
          FSDirectory::open(createTempDir(L"LookupBenchmarkTest")), a);
    } else {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: Constructor<? extends Lookup> ctor =
      // cls.getConstructor(org.apache.lucene.analysis.Analyzer.class);
      shared_ptr < Constructor < ? extends Lookup >> ctor =
                                       cls.getConstructor(Analyzer::typeid);
      lookup = ctor->newInstance(a);
    }
  }
  lookup->build(make_shared<InputArrayIterator>(input));
  return lookup;
}

void LookupBenchmarkTest::testPerformanceOnFullHits() 
{
  constexpr int minPrefixLen = 100;
  constexpr int maxPrefixLen = 200;
  runPerformanceTest(minPrefixLen, maxPrefixLen, num, onlyMorePopular);
}

void LookupBenchmarkTest::testPerformanceOnPrefixes6_9() 
{
  constexpr int minPrefixLen = 6;
  constexpr int maxPrefixLen = 9;
  runPerformanceTest(minPrefixLen, maxPrefixLen, num, onlyMorePopular);
}

void LookupBenchmarkTest::testPerformanceOnPrefixes2_4() 
{
  constexpr int minPrefixLen = 2;
  constexpr int maxPrefixLen = 4;
  runPerformanceTest(minPrefixLen, maxPrefixLen, num, onlyMorePopular);
}

void LookupBenchmarkTest::runPerformanceTest(
    int const minPrefixLen, int const maxPrefixLen, int const num,
    bool const onlyMorePopular) 
{
  System::err::println(wstring::format(
      Locale::ROOT, L"-- prefixes: %d-%d, num: %d, onlyMorePopular: %s",
      minPrefixLen, maxPrefixLen, num, onlyMorePopular));

  for (auto cls : benchmarkClasses) {
    shared_ptr<Lookup> *const lookup = buildLookup(cls, dictionaryInput);

    const deque<wstring> input = deque<wstring>(benchmarkInput.size());
    for (auto tf : benchmarkInput) {
      wstring s = tf->term->utf8ToString();
      wstring sub = s.substr(
          0, min(s.length(), minPrefixLen + random->nextInt(maxPrefixLen -
                                                            minPrefixLen + 1)));
      input.push_back(sub);
    }

    shared_ptr<BenchmarkResult> result =
        measure(make_shared<CallableAnonymousInnerClass2>(
            shared_from_this(), num, onlyMorePopular, lookup, input));

    // C++ TODO: There is no native C++ equivalent to 'toString':
    System::err::println(wstring::format(
        Locale::ROOT, L"%-15s queries: %d, time[ms]: %s, ~kQPS: %.0f",
        lookup->getClass().getSimpleName(), input.size(),
        result->average->toString(), input.size() / result->average->avg));
  }
}

LookupBenchmarkTest::CallableAnonymousInnerClass2::CallableAnonymousInnerClass2(
    shared_ptr<LookupBenchmarkTest> outerInstance, int num,
    bool onlyMorePopular,
    shared_ptr<org::apache::lucene::search::suggest::Lookup> lookup,
    deque<wstring> &input)
{
  this->outerInstance = outerInstance;
  this->num = num;
  this->onlyMorePopular = onlyMorePopular;
  this->lookup = lookup;
  this->input = input;
}

optional<int>
LookupBenchmarkTest::CallableAnonymousInnerClass2::call() 
{
  int v = 0;
  for (auto term : input) {
    v += lookup->lookup(term, onlyMorePopular, num).size();
  }
  return v;
}

shared_ptr<BenchmarkResult>
LookupBenchmarkTest::measure(shared_ptr<Callable<int>> callable)
{
  constexpr double NANOS_PER_MS = 1000000;

  try {
    deque<double> times = deque<double>();
    for (int i = 0; i < warmup + rounds; i++) {
      constexpr int64_t start = System::nanoTime();
      guard = callable->call().intValue();
      times.push_back((System::nanoTime() - start) / NANOS_PER_MS);
    }
    return make_shared<BenchmarkResult>(times, warmup, rounds);
  } catch (const runtime_error &e) {
    e.printStackTrace();
    throw runtime_error(e);
  }
}

int LookupBenchmarkTest::guard = 0;

LookupBenchmarkTest::BenchmarkResult::BenchmarkResult(deque<double> &times,
                                                      int warmup, int rounds)
    : average(Average::from(times.subList(warmup, times.size())))
{
}
} // namespace org::apache::lucene::search::suggest