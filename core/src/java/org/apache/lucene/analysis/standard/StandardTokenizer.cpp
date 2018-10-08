using namespace std;

#include "StandardTokenizer.h"
#include "../../util/AttributeFactory.h"
#include "../tokenattributes/CharTermAttribute.h"
#include "../tokenattributes/OffsetAttribute.h"
#include "../tokenattributes/PositionIncrementAttribute.h"
#include "../tokenattributes/TypeAttribute.h"
#include "StandardTokenizerImpl.h"

namespace org::apache::lucene::analysis::standard
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
std::deque<wstring> const StandardTokenizer::TOKEN_TYPES =
    std::deque<wstring>{L"<ALPHANUM>",    L"<NUM>",      L"<SOUTHEAST_ASIAN>",
                         L"<IDEOGRAPHIC>", L"<HIRAGANA>", L"<KATAKANA>",
                         L"<HANGUL>",      L"<EMOJI>"};

void StandardTokenizer::setMaxTokenLength(int length)
{
  if (length < 1) {
    throw invalid_argument(L"maxTokenLength must be greater than zero");
  } else if (length > MAX_TOKEN_LENGTH_LIMIT) {
    throw invalid_argument(L"maxTokenLength may not exceed " +
                           to_wstring(MAX_TOKEN_LENGTH_LIMIT));
  }
  if (length != maxTokenLength) {
    maxTokenLength = length;
    scanner->setBufferSize(length);
  }
}

int StandardTokenizer::getMaxTokenLength() { return maxTokenLength; }

StandardTokenizer::StandardTokenizer() { init(); }

StandardTokenizer::StandardTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::Tokenizer(factory)
{
  init();
}

void StandardTokenizer::init()
{
  this->scanner = make_shared<StandardTokenizerImpl>(input);
}

bool StandardTokenizer::incrementToken() 
{
  clearAttributes();
  skippedPositions = 0;

  while (true) {
    int tokenType = scanner->getNextToken();

    if (tokenType == StandardTokenizerImpl::YYEOF) {
      return false;
    }

    if (scanner->yylength() <= maxTokenLength) {
      posIncrAtt->setPositionIncrement(skippedPositions + 1);
      scanner->getText(termAtt);
      constexpr int start = scanner->yychar();
      offsetAtt->setOffset(correctOffset(start),
                           correctOffset(start + termAtt->length()));
      typeAtt->setType(StandardTokenizer::TOKEN_TYPES[tokenType]);
      return true;
    } else {
      // When we skip a too-long term, we still increment the
      // position increment
      skippedPositions++;
    }
  }
}

void StandardTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  int finalOffset = correctOffset(scanner->yychar() + scanner->yylength());
  offsetAtt->setOffset(finalOffset, finalOffset);
  // adjust any skipped tokens
  posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                   skippedPositions);
}

StandardTokenizer::~StandardTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  scanner->yyreset(input);
}

void StandardTokenizer::reset() 
{
  Tokenizer::reset();
  scanner->yyreset(input);
  skippedPositions = 0;
}
} // namespace org::apache::lucene::analysis::standard