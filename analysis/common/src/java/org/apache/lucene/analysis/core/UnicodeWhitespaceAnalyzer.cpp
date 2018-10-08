using namespace std;

#include "UnicodeWhitespaceAnalyzer.h"
#include "UnicodeWhitespaceTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;

UnicodeWhitespaceAnalyzer::UnicodeWhitespaceAnalyzer() {}

shared_ptr<Analyzer::TokenStreamComponents>
UnicodeWhitespaceAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<UnicodeWhitespaceTokenizer>());
}
} // namespace org::apache::lucene::analysis::core