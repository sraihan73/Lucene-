using namespace std;

#include "HMMChineseTokenizerFactory.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "HMMChineseTokenizer.h"

namespace org::apache::lucene::analysis::cn::smart
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

HMMChineseTokenizerFactory::HMMChineseTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<Tokenizer>
HMMChineseTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<HMMChineseTokenizer>(factory);
}
} // namespace org::apache::lucene::analysis::cn::smart