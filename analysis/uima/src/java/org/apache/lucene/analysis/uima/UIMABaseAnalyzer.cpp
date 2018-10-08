using namespace std;

#include "UIMABaseAnalyzer.h"
#include "UIMAAnnotationsTokenizer.h"

namespace org::apache::lucene::analysis::uima
{
using Analyzer = org::apache::lucene::analysis::Analyzer;

UIMABaseAnalyzer::UIMABaseAnalyzer(
    const wstring &descriptorPath, const wstring &tokenType,
    unordered_map<wstring, any> &configurationParameters)
    : descriptorPath(descriptorPath), tokenType(tokenType),
      configurationParameters(configurationParameters)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
UIMABaseAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<UIMAAnnotationsTokenizer>(descriptorPath, tokenType,
                                            configurationParameters));
}
} // namespace org::apache::lucene::analysis::uima