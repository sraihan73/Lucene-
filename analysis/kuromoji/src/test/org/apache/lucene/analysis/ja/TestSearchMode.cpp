using namespace std;

#include "TestSearchMode.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
const wstring TestSearchMode::SEGMENTATION_FILENAME =
    L"search-segmentation-tests.txt";

void TestSearchMode::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestSearchMode::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSearchMode> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSearchMode::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestSearchMode::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestSearchMode::testSearchSegmentation() 
{
  shared_ptr<InputStream> is =
      TestSearchMode::typeid->getResourceAsStream(SEGMENTATION_FILENAME);
  if (is == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(
        L"Cannot find " + SEGMENTATION_FILENAME + L" in test classpath");
  }
  try {
    shared_ptr<LineNumberReader> reader = make_shared<LineNumberReader>(
        make_shared<InputStreamReader>(is, StandardCharsets::UTF_8));
    wstring line = L"";
    while ((line = reader->readLine()) != L"") {
      // Remove comments
      line = line.replaceAll(L"#.*$", L"");
      // Skip empty lines or comment lines
      if (StringHelper::trim(line)->isEmpty()) {
        continue;
      }
      if (VERBOSE) {
        wcout << L"Line no. " << reader->getLineNumber() << L": " << line
              << endl;
      }
      std::deque<wstring> fields = line.split(L"\t", 2);
      wstring sourceText = fields[0];
      std::deque<wstring> expectedTokens = fields[1].split(L"\\s+");
      std::deque<int> expectedPosIncrs(expectedTokens.size());
      std::deque<int> expectedPosLengths(expectedTokens.size());
      for (int tokIDX = 0; tokIDX < expectedTokens.size(); tokIDX++) {
        if (StringHelper::endsWith(expectedTokens[tokIDX], L"/0")) {
          expectedTokens[tokIDX] =
              StringHelper::replace(expectedTokens[tokIDX], L"/0", L"");
          expectedPosLengths[tokIDX] = expectedTokens.size() - 1;
        } else {
          expectedPosIncrs[tokIDX] = 1;
          expectedPosLengths[tokIDX] = 1;
        }
      }
      assertAnalyzesTo(analyzer, sourceText, expectedTokens, expectedPosIncrs);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    is->close();
  }
}
} // namespace org::apache::lucene::analysis::ja