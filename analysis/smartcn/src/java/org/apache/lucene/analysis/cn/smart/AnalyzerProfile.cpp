using namespace std;

#include "AnalyzerProfile.h"

namespace org::apache::lucene::analysis::cn::smart
{

wstring AnalyzerProfile::ANALYSIS_DATA_DIR = L"";

AnalyzerProfile::StaticConstructor::StaticConstructor() { init(); }

AnalyzerProfile::StaticConstructor AnalyzerProfile::staticConstructor;

void AnalyzerProfile::init()
{
  wstring dirName = L"analysis-data";
  wstring propName = L"analysis.properties";

  // Try the system property：-Danalysis.data.dir=/path/to/analysis-data
  ANALYSIS_DATA_DIR = System::getProperty(L"analysis.data.dir", L"");
  if (ANALYSIS_DATA_DIR.length() != 0) {
    return;
  }

  std::deque<std::shared_ptr<Path>> candidateFiles = {
      Paths->get(dirName), Paths->get(L"lib").resolve(dirName),
      Paths->get(propName), Paths->get(L"lib").resolve(propName)};
  for (auto file : candidateFiles) {
    if (Files::exists(file)) {
      if (Files::isDirectory(file)) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        ANALYSIS_DATA_DIR = file->toAbsolutePath()->toString();
      } else if (Files::isRegularFile(file) &&
                 getAnalysisDataDir(file).length() != 0) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        ANALYSIS_DATA_DIR = getAnalysisDataDir(file).toString();
      }
      break;
    }
  }

  if (ANALYSIS_DATA_DIR.length() == 0) {
    // Dictionary directory cannot be found.
    throw runtime_error(
        wstring(L"WARNING: Can not find lexical dictionary directory!") +
        L" This will cause unpredictable exceptions in your application!" +
        L" Please refer to the manual to download the dictionaries.");
  }
}

wstring AnalyzerProfile::getAnalysisDataDir(shared_ptr<Path> propFile)
{
  shared_ptr<Properties> prop = make_shared<Properties>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader reader =
  // java.nio.file.Files.newBufferedReader(propFile,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader reader = java::nio::file::Files::newBufferedReader(
        propFile, java::nio::charset::StandardCharsets::UTF_8);
    try {
      prop->load(reader);
      return prop->getProperty(L"analysis.data.dir", L"");
    } catch (const IOException &e) {
      return L"";
    }
  }
}
} // namespace org::apache::lucene::analysis::cn::smart