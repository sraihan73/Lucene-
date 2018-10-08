using namespace std;

#include "KeywordTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "KeywordTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

KeywordTokenizerFactory::KeywordTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxTokenLen(
          getInt(args, L"maxTokenLen", KeywordTokenizer::DEFAULT_BUFFER_SIZE))
{
  if (maxTokenLen > MAX_TOKEN_LENGTH_LIMIT || maxTokenLen <= 0) {
    throw invalid_argument(
        L"maxTokenLen must be greater than 0 and less than " +
        MAX_TOKEN_LENGTH_LIMIT + L" passed: " + to_wstring(maxTokenLen));
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<KeywordTokenizer>
KeywordTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<KeywordTokenizer>(factory, maxTokenLen);
}
} // namespace org::apache::lucene::analysis::core