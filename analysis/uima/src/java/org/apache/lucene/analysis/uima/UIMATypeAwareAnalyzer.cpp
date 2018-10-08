using namespace std;

#include "UIMATypeAwareAnalyzer.h"
#include "UIMATypeAwareAnnotationsTokenizer.h"

namespace org::apache::lucene::analysis::uima
{
using Analyzer = org::apache::lucene::analysis::Analyzer;

UIMATypeAwareAnalyzer::UIMATypeAwareAnalyzer(
    const wstring &descriptorPath, const wstring &tokenType,
    const wstring &featurePath,
    unordered_map<wstring, any> &configurationParameters)
    : descriptorPath(descriptorPath), tokenType(tokenType),
      featurePath(featurePath), configurationParameters(configurationParameters)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
UIMATypeAwareAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<UIMATypeAwareAnnotationsTokenizer>(
          descriptorPath, tokenType, featurePath, configurationParameters));
}
} // namespace org::apache::lucene::analysis::uima