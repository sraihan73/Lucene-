using namespace std;

#include "LetterTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "LetterTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

LetterTokenizerFactory::LetterTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      maxTokenLen(
          getInt(args, L"maxTokenLen", CharTokenizer::DEFAULT_MAX_WORD_LEN))
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

shared_ptr<LetterTokenizer>
LetterTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<LetterTokenizer>(factory, maxTokenLen);
}
} // namespace org::apache::lucene::analysis::core