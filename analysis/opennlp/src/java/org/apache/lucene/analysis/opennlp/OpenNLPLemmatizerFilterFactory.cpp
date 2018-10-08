using namespace std;

#include "OpenNLPLemmatizerFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "OpenNLPLemmatizerFilter.h"
#include "tools/NLPLemmatizerOp.h"
#include "tools/OpenNLPOpsFactory.h"

namespace org::apache::lucene::analysis::opennlp
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NLPLemmatizerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPLemmatizerOp;
using OpenNLPOpsFactory =
    org::apache::lucene::analysis::opennlp::tools::OpenNLPOpsFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring OpenNLPLemmatizerFilterFactory::DICTIONARY = L"dictionary";
const wstring OpenNLPLemmatizerFilterFactory::LEMMATIZER_MODEL =
    L"lemmatizerModel";

OpenNLPLemmatizerFilterFactory::OpenNLPLemmatizerFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      dictionaryFile(get(args, DICTIONARY)),
      lemmatizerModelFile(get(args, LEMMATIZER_MODEL))
{

  if (dictionaryFile == L"" && lemmatizerModelFile == L"") {
    throw invalid_argument(
        L"Configuration Error: missing parameter: at least one of '" +
        DICTIONARY + L"' and '" + LEMMATIZER_MODEL + L"' must be provided.");
  }

  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<OpenNLPLemmatizerFilter>
OpenNLPLemmatizerFilterFactory::create(shared_ptr<TokenStream> in_)
{
  try {
    shared_ptr<NLPLemmatizerOp> lemmatizerOp =
        OpenNLPOpsFactory::getLemmatizer(dictionaryFile, lemmatizerModelFile);
    return make_shared<OpenNLPLemmatizerFilter>(in_, lemmatizerOp);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

void OpenNLPLemmatizerFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  // register models in cache with file/resource names
  if (dictionaryFile != L"") {
    OpenNLPOpsFactory::getLemmatizerDictionary(dictionaryFile, loader);
  }
  if (lemmatizerModelFile != L"") {
    OpenNLPOpsFactory::getLemmatizerModel(lemmatizerModelFile, loader);
  }
}
} // namespace org::apache::lucene::analysis::opennlp