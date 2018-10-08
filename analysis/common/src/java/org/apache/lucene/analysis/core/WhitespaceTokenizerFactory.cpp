using namespace std;

#include "WhitespaceTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "UnicodeWhitespaceTokenizer.h"
#include "WhitespaceTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;
const wstring WhitespaceTokenizerFactory::RULE_JAVA = L"java";
const wstring WhitespaceTokenizerFactory::RULE_UNICODE = L"unicode";
const shared_ptr<java::util::deque<wstring>>
    WhitespaceTokenizerFactory::RULE_NAMES =
        java::util::Arrays::asList(RULE_JAVA, RULE_UNICODE);

WhitespaceTokenizerFactory::WhitespaceTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      rule(get(args, L"rule", RULE_NAMES, RULE_JAVA)),
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

shared_ptr<Tokenizer>
WhitespaceTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  switch (rule) {
  case RULE_JAVA:
    return make_shared<WhitespaceTokenizer>(factory, maxTokenLen);
  case RULE_UNICODE:
    return make_shared<UnicodeWhitespaceTokenizer>(factory, maxTokenLen);
  default:
    throw make_shared<AssertionError>();
  }
}
} // namespace org::apache::lucene::analysis::core