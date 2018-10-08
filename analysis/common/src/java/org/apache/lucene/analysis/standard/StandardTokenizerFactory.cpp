using namespace std;

#include "StandardTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::standard
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

StandardTokenizerFactory::StandardTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxTokenLength(getInt(args, L"maxTokenLength",
                            StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<StandardTokenizer>
StandardTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  shared_ptr<StandardTokenizer> tokenizer =
      make_shared<StandardTokenizer>(factory);
  tokenizer->setMaxTokenLength(maxTokenLength);
  return tokenizer;
}
} // namespace org::apache::lucene::analysis::standard