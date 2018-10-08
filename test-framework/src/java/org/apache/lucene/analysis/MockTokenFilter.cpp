using namespace std;

#include "MockTokenFilter.h"

namespace org::apache::lucene::analysis
{
//    import static org.apache.lucene.util.automaton.Automata.makeEmpty;
//    import static org.apache.lucene.util.automaton.Automata.makeString;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Operations = org::apache::lucene::util::automaton::Operations;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
const shared_ptr<org::apache::lucene::util::automaton::CharacterRunAutomaton>
    MockTokenFilter::EMPTY_STOPSET = make_shared<
        org::apache::lucene::util::automaton::CharacterRunAutomaton>(
        makeEmpty());
const shared_ptr<org::apache::lucene::util::automaton::CharacterRunAutomaton>
    MockTokenFilter::ENGLISH_STOPSET = make_shared<
        org::apache::lucene::util::automaton::CharacterRunAutomaton>(
        org::apache::lucene::util::automaton::Operations::union_(
            java::util::Arrays::asList(
                makeString(L"a"), makeString(L"an"), makeString(L"and"),
                makeString(L"are"), makeString(L"as"), makeString(L"at"),
                makeString(L"be"), makeString(L"but"), makeString(L"by"),
                makeString(L"for"), makeString(L"if"), makeString(L"in"),
                makeString(L"into"), makeString(L"is"), makeString(L"it"),
                makeString(L"no"), makeString(L"not"), makeString(L"of"),
                makeString(L"on"), makeString(L"or"), makeString(L"such"),
                makeString(L"that"), makeString(L"the"), makeString(L"their"),
                makeString(L"then"), makeString(L"there"), makeString(L"these"),
                makeString(L"they"), makeString(L"this"), makeString(L"to"),
                makeString(L"was"), makeString(L"will"), makeString(L"with"))));

MockTokenFilter::MockTokenFilter(shared_ptr<TokenStream> input,
                                 shared_ptr<CharacterRunAutomaton> filter)
    : TokenFilter(input), filter(filter)
{
}

bool MockTokenFilter::incrementToken() 
{
  // TODO: fix me when posInc=false, to work like FilteringTokenFilter in that
  // case and not return initial token with posInc=0 ever

  // return the first non-stop word found
  skippedPositions = 0;
  while (input->incrementToken()) {
    if (!filter->run(termAtt->buffer(), 0, termAtt->length())) {
      posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                       skippedPositions);
      return true;
    }
    skippedPositions += posIncrAtt->getPositionIncrement();
  }
  // reached EOS -- return false
  return false;
}

void MockTokenFilter::end() 
{
  TokenFilter::end();
  posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                   skippedPositions);
}

void MockTokenFilter::reset() 
{
  TokenFilter::reset();
  skippedPositions = 0;
}
} // namespace org::apache::lucene::analysis