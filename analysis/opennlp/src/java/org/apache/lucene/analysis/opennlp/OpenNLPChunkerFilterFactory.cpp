using namespace std;

#include "OpenNLPChunkerFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "OpenNLPChunkerFilter.h"
#include "tools/NLPChunkerOp.h"
#include "tools/OpenNLPOpsFactory.h"

namespace org::apache::lucene::analysis::opennlp
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NLPChunkerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPChunkerOp;
using OpenNLPOpsFactory =
    org::apache::lucene::analysis::opennlp::tools::OpenNLPOpsFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring OpenNLPChunkerFilterFactory::CHUNKER_MODEL = L"chunkerModel";

OpenNLPChunkerFilterFactory::OpenNLPChunkerFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      chunkerModelFile(get(args, CHUNKER_MODEL))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<OpenNLPChunkerFilter>
OpenNLPChunkerFilterFactory::create(shared_ptr<TokenStream> in_)
{
  try {
    shared_ptr<NLPChunkerOp> chunkerOp = nullptr;

    if (chunkerModelFile != L"") {
      chunkerOp = OpenNLPOpsFactory::getChunker(chunkerModelFile);
    }
    return make_shared<OpenNLPChunkerFilter>(in_, chunkerOp);
  } catch (const IOException &e) {
    throw invalid_argument(e);
  }
}

void OpenNLPChunkerFilterFactory::inform(shared_ptr<ResourceLoader> loader)
{
  try {
    // load and register read-only models in cache with file/resource names
    if (chunkerModelFile != L"") {
      OpenNLPOpsFactory::getChunkerModel(chunkerModelFile, loader);
    }
  } catch (const IOException &e) {
    throw invalid_argument(e);
  }
}
} // namespace org::apache::lucene::analysis::opennlp