using namespace std;

#include "WhitespaceAnalyzer.h"
#include "WhitespaceTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;

WhitespaceAnalyzer::WhitespaceAnalyzer() {}

shared_ptr<Analyzer::TokenStreamComponents>
WhitespaceAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<WhitespaceTokenizer>());
}
} // namespace org::apache::lucene::analysis::core