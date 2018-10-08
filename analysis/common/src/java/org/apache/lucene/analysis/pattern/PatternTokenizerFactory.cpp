using namespace std;

#include "PatternTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "PatternTokenizer.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
const wstring PatternTokenizerFactory::PATTERN = L"pattern";
const wstring PatternTokenizerFactory::GROUP = L"group";

PatternTokenizerFactory::PatternTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      pattern(getPattern(args, PATTERN)), group(getInt(args, GROUP, -1))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<PatternTokenizer>
PatternTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<PatternTokenizer>(factory, pattern, group);
}
} // namespace org::apache::lucene::analysis::pattern