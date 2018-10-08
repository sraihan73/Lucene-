using namespace std;

#include "AnalysisOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

AnalysisOffsetStrategy::AnalysisOffsetStrategy(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &queryTerms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
    shared_ptr<Analyzer> analyzer)
    : FieldOffsetStrategy(field, queryTerms, phraseHelper, automata),
      analyzer(analyzer)
{
  if (analyzer->getOffsetGap(field) !=
      1) { // note: 1 is the default. It is RARELY changed.
    throw invalid_argument(
        L"offset gap of the provided analyzer should be 1 (field " + field +
        L")");
  }
}

UnifiedHighlighter::OffsetSource AnalysisOffsetStrategy::getOffsetSource()
{
  return UnifiedHighlighter::OffsetSource::ANALYSIS;
}

shared_ptr<TokenStream>
AnalysisOffsetStrategy::tokenStream(const wstring &content) 
{
  // If there is no splitChar in content then we needn't wrap:
  int splitCharIdx = (int)content.find(UnifiedHighlighter::MULTIVAL_SEP_CHAR);
  if (splitCharIdx == -1) {
    return analyzer->tokenStream(field, content);
  }

  shared_ptr<TokenStream> subTokenStream =
      analyzer->tokenStream(field, content.substr(0, splitCharIdx));

  return make_shared<MultiValueTokenStream>(
      subTokenStream, field, analyzer, content,
      UnifiedHighlighter::MULTIVAL_SEP_CHAR, splitCharIdx);
}

AnalysisOffsetStrategy::MultiValueTokenStream::MultiValueTokenStream(
    shared_ptr<TokenStream> subTokenStream, const wstring &fieldName,
    shared_ptr<Analyzer> indexAnalyzer, const wstring &content,
    wchar_t splitChar, int splitCharIdx)
    : org::apache::lucene::analysis::TokenFilter(subTokenStream),
      fieldName(fieldName), indexAnalyzer(indexAnalyzer), content(content),
      splitChar(splitChar)
{
  this->endValIdx = splitCharIdx;
}

void AnalysisOffsetStrategy::MultiValueTokenStream::reset() 
{
  if (startValIdx != 0) {
    throw make_shared<IllegalStateException>(
        L"This TokenStream wasn't developed to be re-used.");
    // ... although we could if a need for it arises.
  }
  TokenFilter::reset();
}

bool AnalysisOffsetStrategy::MultiValueTokenStream::incrementToken() throw(
    IOException)
{
  while (true) {

    if (input->incrementToken()) {
      // Position tracking:
      if (remainingPosInc >
          0) { // usually true first token of additional values (not first val)
        posIncAtt->setPositionIncrement(remainingPosInc +
                                        posIncAtt->getPositionIncrement());
        remainingPosInc = 0; // reset
      }
      // Offset tracking:
      offsetAtt->setOffset(startValIdx + offsetAtt->startOffset(),
                           startValIdx + offsetAtt->endOffset());
      return true;
    }

    if (endValIdx == content.length()) { // no more
      return false;
    }

    input->end(); // might adjust position increment
    remainingPosInc += posIncAtt->getPositionIncrement();
    delete input;
    remainingPosInc += indexAnalyzer->getPositionIncrementGap(fieldName);

    // Get new tokenStream based on next segment divided by the splitChar
    startValIdx = endValIdx + 1;
    endValIdx = (int)content.find(splitChar, startValIdx);
    if (endValIdx == -1) { // EOF
      endValIdx = content.length();
    }
    shared_ptr<TokenStream> tokenStream = indexAnalyzer->tokenStream(
        fieldName, content.substr(startValIdx, endValIdx - startValIdx));
    if (tokenStream !=
        input) { // (input is defined in TokenFilter set in the constructor)
      // This is a grand trick we do -- knowing that the analyzer's re-use
      // strategy is going to produce the very same tokenStream instance and
      // thus have the same AttributeSource as this wrapping TokenStream since
      // we used it as our input in the constructor. Were this not the case,
      // we'd have to copy every attribute of interest since we can't alter the
      // AttributeSource of this wrapping TokenStream post-construction (it's
      // all private/final). If this is a problem, we could do that instead;
      // maybe with a custom CharTermAttribute that allows us to easily set the
      // char[] reference without literally copying char by char.
      throw make_shared<IllegalStateException>(
          L"Require TokenStream re-use.  Unsupported re-use strategy?: " +
          indexAnalyzer->getReuseStrategy());
    }
    tokenStream->reset();
  } // while loop to increment token of this new value
}

void AnalysisOffsetStrategy::MultiValueTokenStream::end() 
{
  TokenFilter::end();
  // Offset tracking:
  offsetAtt->setOffset(startValIdx + offsetAtt->startOffset(),
                       startValIdx + offsetAtt->endOffset());
}
} // namespace org::apache::lucene::search::uhighlight