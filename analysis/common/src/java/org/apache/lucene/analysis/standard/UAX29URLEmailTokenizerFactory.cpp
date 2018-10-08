using namespace std;

#include "UAX29URLEmailTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "UAX29URLEmailTokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

UAX29URLEmailTokenizerFactory::UAX29URLEmailTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxTokenLength(getInt(args, L"maxTokenLength",
                            StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<UAX29URLEmailTokenizer>
UAX29URLEmailTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  shared_ptr<UAX29URLEmailTokenizer> tokenizer =
      make_shared<UAX29URLEmailTokenizer>(factory);
  tokenizer->setMaxTokenLength(maxTokenLength);
  return tokenizer;
}
} // namespace org::apache::lucene::analysis::standard