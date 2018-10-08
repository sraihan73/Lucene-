using namespace std;

#include "Benchmark.h"
#include "PerfRunData.h"
#include "utils/Algorithm.h"
#include "utils/Config.h"

namespace org::apache::lucene::benchmark::byTask
{
using Algorithm = org::apache::lucene::benchmark::byTask::utils::Algorithm;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

Benchmark::Benchmark(shared_ptr<Reader> algReader) 
{
  // prepare run data
  try {
    runData = make_shared<PerfRunData>(make_shared<Config>(algReader));
  } catch (const runtime_error &e) {
    e.printStackTrace();
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new Exception("Error: cannot init PerfRunData!",e);
    throw runtime_error(L"Error: cannot init PerfRunData!");
  }

  // parse algorithm
  try {
    algorithm = make_shared<Algorithm>(runData);
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new Exception("Error: cannot understand
    // algorithm!",e);
    throw runtime_error(L"Error: cannot understand algorithm!");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void Benchmark::execute() 
{
  if (executed) {
    throw make_shared<IllegalStateException>(L"Benchmark was already executed");
  }
  executed = true;
  runData->setStartTimeMillis();
  algorithm->execute();
}

void Benchmark::main(std::deque<wstring> &args) { exec(args); }

void Benchmark::exec(std::deque<wstring> &args)
{
  // verify command line args
  if (args.size() < 1) {
    System::err::println(L"Usage: java Benchmark <algorithm file>");
    exit(1);
  }

  // verify input files
  shared_ptr<Path> algFile = Paths->get(args[0]);
  if (!Files::isReadable(algFile)) {
    System::err::println(L"cannot find/read algorithm file: " +
                         algFile->toAbsolutePath());
    exit(1);
  }

  wcout << L"Running algorithm from: " << algFile->toAbsolutePath() << endl;

  shared_ptr<Benchmark> benchmark = nullptr;
  try {
    benchmark = make_shared<Benchmark>(
        Files::newBufferedReader(algFile, StandardCharsets::UTF_8));
  } catch (const runtime_error &e) {
    e.printStackTrace();
    exit(1);
  }

  wcout << L"------------> algorithm:" << endl;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wcout << benchmark->getAlgorithm()->toString() << endl;

  // execute
  try {
    benchmark->execute();
  } catch (const runtime_error &e) {
    System::err::println(L"Error: cannot execute the algorithm! " + e.what());
    e.printStackTrace();
  }

  wcout << L"####################" << endl;
  wcout << L"###  D O N E !!! ###" << endl;
  wcout << L"####################" << endl;
}

shared_ptr<Algorithm> Benchmark::getAlgorithm() { return algorithm; }

shared_ptr<PerfRunData> Benchmark::getRunData() { return runData; }
} // namespace org::apache::lucene::benchmark::byTask