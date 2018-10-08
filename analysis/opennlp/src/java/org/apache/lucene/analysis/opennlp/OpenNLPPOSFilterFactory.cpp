using namespace std;

#include "OpenNLPPOSFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "OpenNLPPOSFilter.h"
#include "tools/OpenNLPOpsFactory.h"

namespace org::apache::lucene::analysis::opennlp
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OpenNLPOpsFactory =
    org::apache::lucene::analysis::opennlp::tools::OpenNLPOpsFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring OpenNLPPOSFilterFactory::POS_TAGGER_MODEL = L"posTaggerModel";

OpenNLPPOSFilterFactory::OpenNLPPOSFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      posTaggerModelFile(require(args, POS_TAGGER_MODEL))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<OpenNLPPOSFilter>
OpenNLPPOSFilterFactory::create(shared_ptr<TokenStream> in_)
{
  try {
    return make_shared<OpenNLPPOSFilter>(
        in_, OpenNLPOpsFactory::getPOSTagger(posTaggerModelFile));
  } catch (const IOException &e) {
    throw invalid_argument(e);
  }
}

void OpenNLPPOSFilterFactory::inform(shared_ptr<ResourceLoader> loader)
{
  try { // load and register the read-only model in cache with file/resource
        // name
    OpenNLPOpsFactory::getPOSTaggerModel(posTaggerModelFile, loader);
  } catch (const IOException &e) {
    throw invalid_argument(e);
  }
}
} // namespace org::apache::lucene::analysis::opennlp