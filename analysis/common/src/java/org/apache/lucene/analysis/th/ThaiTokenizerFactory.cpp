using namespace std;

#include "ThaiTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "ThaiTokenizer.h"

namespace org::apache::lucene::analysis::th
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

ThaiTokenizerFactory::ThaiTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<Tokenizer>
ThaiTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<ThaiTokenizer>(factory);
}
} // namespace org::apache::lucene::analysis::th