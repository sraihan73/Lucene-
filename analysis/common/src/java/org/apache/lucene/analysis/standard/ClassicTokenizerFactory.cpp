using namespace std;

#include "ClassicTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "ClassicTokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

ClassicTokenizerFactory::ClassicTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxTokenLength(getInt(args, L"maxTokenLength",
                            StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ClassicTokenizer>
ClassicTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  shared_ptr<ClassicTokenizer> tokenizer =
      make_shared<ClassicTokenizer>(factory);
  tokenizer->setMaxTokenLength(maxTokenLength);
  return tokenizer;
}
} // namespace org::apache::lucene::analysis::standard