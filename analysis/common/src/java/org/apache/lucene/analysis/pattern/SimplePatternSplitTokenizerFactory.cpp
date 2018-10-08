using namespace std;

#include "SimplePatternSplitTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "SimplePatternSplitTokenizer.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;
const wstring SimplePatternSplitTokenizerFactory::PATTERN = L"pattern";

SimplePatternSplitTokenizerFactory::SimplePatternSplitTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      dfa(Operations::determinize(
          (make_shared<RegExp>(require(args, PATTERN)))->toAutomaton(),
          maxDeterminizedStates)),
      maxDeterminizedStates(getInt(args, L"maxDeterminizedStates",
                                   Operations::DEFAULT_MAX_DETERMINIZED_STATES))
{
  if (args.empty() == false) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<SimplePatternSplitTokenizer>
SimplePatternSplitTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<SimplePatternSplitTokenizer>(factory, dfa);
}
} // namespace org::apache::lucene::analysis::pattern