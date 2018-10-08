using namespace std;

#include "SimpleAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "LowerCaseTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;

SimpleAnalyzer::SimpleAnalyzer() {}

shared_ptr<Analyzer::TokenStreamComponents>
SimpleAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<LowerCaseTokenizer>());
}

shared_ptr<TokenStream> SimpleAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  return make_shared<LowerCaseFilter>(in_);
}
} // namespace org::apache::lucene::analysis::core