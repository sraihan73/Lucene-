using namespace std;

#include "UIMATypeAwareAnnotationsTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "UIMATypeAwareAnnotationsTokenizer.h"

namespace org::apache::lucene::analysis::uima
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

UIMATypeAwareAnnotationsTokenizerFactory::
    UIMATypeAwareAnnotationsTokenizerFactory(
        unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args)
{
  featurePath = require(args, L"featurePath");
  tokenType = require(args, L"tokenType");
  descriptorPath = require(args, L"descriptorPath");
  configurationParameters.putAll(args);
}

shared_ptr<UIMATypeAwareAnnotationsTokenizer>
UIMATypeAwareAnnotationsTokenizerFactory::create(
    shared_ptr<AttributeFactory> factory)
{
  return make_shared<UIMATypeAwareAnnotationsTokenizer>(
      descriptorPath, tokenType, featurePath, configurationParameters, factory);
}
} // namespace org::apache::lucene::analysis::uima