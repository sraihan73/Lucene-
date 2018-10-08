using namespace std;

#include "KeywordAnalyzer.h"
#include "KeywordTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;

KeywordAnalyzer::KeywordAnalyzer() {}

shared_ptr<Analyzer::TokenStreamComponents>
KeywordAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<KeywordTokenizer>());
}
} // namespace org::apache::lucene::analysis::core