using namespace std;

#include "PathHierarchyTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "PathHierarchyTokenizer.h"
#include "ReversePathHierarchyTokenizer.h"

namespace org::apache::lucene::analysis::path
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

PathHierarchyTokenizerFactory::PathHierarchyTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      delimiter(getChar(args, L"delimiter",
                        PathHierarchyTokenizer::DEFAULT_DELIMITER)),
      replacement(getChar(args, L"replace", delimiter)),
      reverse(getBoolean(args, L"reverse", false)),
      skip(getInt(args, L"skip", PathHierarchyTokenizer::DEFAULT_SKIP))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<Tokenizer>
PathHierarchyTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  if (reverse) {
    return make_shared<ReversePathHierarchyTokenizer>(factory, delimiter,
                                                      replacement, skip);
  }
  return make_shared<PathHierarchyTokenizer>(factory, delimiter, replacement,
                                             skip);
}
} // namespace org::apache::lucene::analysis::path