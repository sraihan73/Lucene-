using namespace std;

#include "UIMAAnnotationsTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "UIMAAnnotationsTokenizer.h"

namespace org::apache::lucene::analysis::uima
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

UIMAAnnotationsTokenizerFactory::UIMAAnnotationsTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args)
{
  tokenType = require(args, L"tokenType");
  descriptorPath = require(args, L"descriptorPath");
  configurationParameters.putAll(args);
}

shared_ptr<UIMAAnnotationsTokenizer>
UIMAAnnotationsTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<UIMAAnnotationsTokenizer>(
      descriptorPath, tokenType, configurationParameters, factory);
}
} // namespace org::apache::lucene::analysis::uima