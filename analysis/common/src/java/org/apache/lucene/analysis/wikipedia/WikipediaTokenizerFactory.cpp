using namespace std;

#include "WikipediaTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "WikipediaTokenizer.h"

namespace org::apache::lucene::analysis::wikipedia
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
const wstring WikipediaTokenizerFactory::TOKEN_OUTPUT = L"tokenOutput";
const wstring WikipediaTokenizerFactory::UNTOKENIZED_TYPES =
    L"untokenizedTypes";

WikipediaTokenizerFactory::WikipediaTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      tokenOutput(getInt(args, TOKEN_OUTPUT, WikipediaTokenizer::TOKENS_ONLY))
{
  untokenizedTypes = getSet(args, UNTOKENIZED_TYPES);

  if (untokenizedTypes == nullptr) {
    untokenizedTypes = Collections::emptySet();
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<WikipediaTokenizer>
WikipediaTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<WikipediaTokenizer>(factory, tokenOutput,
                                         untokenizedTypes);
}
} // namespace org::apache::lucene::analysis::wikipedia