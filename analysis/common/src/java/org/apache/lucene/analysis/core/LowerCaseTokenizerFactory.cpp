using namespace std;

#include "LowerCaseTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../util/AbstractAnalysisFactory.h"
#include "LowerCaseFilterFactory.h"
#include "LowerCaseTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

LowerCaseTokenizerFactory::LowerCaseTokenizerFactory(
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

shared_ptr<LowerCaseTokenizer>
LowerCaseTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<LowerCaseTokenizer>(factory, maxTokenLen);
}

shared_ptr<AbstractAnalysisFactory>
LowerCaseTokenizerFactory::getMultiTermComponent()
{
  unordered_map<wstring, wstring> map_obj =
      unordered_map<wstring, wstring>(getOriginalArgs());
  map_obj.erase(L"maxTokenLen"); // removing "maxTokenLen" argument for
                             // LowerCaseFilterFactory init
  return make_shared<LowerCaseFilterFactory>(map_obj);
}
} // namespace org::apache::lucene::analysis::core