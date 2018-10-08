using namespace std;

#include "OpenNLPTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "OpenNLPTokenizer.h"
#include "tools/NLPSentenceDetectorOp.h"
#include "tools/NLPTokenizerOp.h"
#include "tools/OpenNLPOpsFactory.h"

namespace org::apache::lucene::analysis::opennlp
{
using NLPSentenceDetectorOp =
    org::apache::lucene::analysis::opennlp::tools::NLPSentenceDetectorOp;
using NLPTokenizerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPTokenizerOp;
using OpenNLPOpsFactory =
    org::apache::lucene::analysis::opennlp::tools::OpenNLPOpsFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
const wstring OpenNLPTokenizerFactory::SENTENCE_MODEL = L"sentenceModel";
const wstring OpenNLPTokenizerFactory::TOKENIZER_MODEL = L"tokenizerModel";

OpenNLPTokenizerFactory::OpenNLPTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      sentenceModelFile(require(args, SENTENCE_MODEL)),
      tokenizerModelFile(require(args, TOKENIZER_MODEL))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<OpenNLPTokenizer>
OpenNLPTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  try {
    shared_ptr<NLPSentenceDetectorOp> sentenceOp =
        OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
    shared_ptr<NLPTokenizerOp> tokenizerOp =
        OpenNLPOpsFactory::getTokenizer(tokenizerModelFile);
    return make_shared<OpenNLPTokenizer>(factory, sentenceOp, tokenizerOp);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

void OpenNLPTokenizerFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  // register models in cache with file/resource names
  if (sentenceModelFile != L"") {
    OpenNLPOpsFactory::getSentenceModel(sentenceModelFile, loader);
  }
  if (tokenizerModelFile != L"") {
    OpenNLPOpsFactory::getTokenizerModel(tokenizerModelFile, loader);
  }
}
} // namespace org::apache::lucene::analysis::opennlp