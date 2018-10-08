using namespace std;

#include "UAX29URLEmailTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "UAX29URLEmailTokenizerImpl.h"

namespace org::apache::lucene::analysis::standard
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
std::deque<wstring> const UAX29URLEmailTokenizer::TOKEN_TYPES =
    std::deque<wstring>{
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::ALPHANUM],
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::NUM],
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::SOUTHEAST_ASIAN],
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::IDEOGRAPHIC],
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::HIRAGANA],
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::KATAKANA],
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::HANGUL],
        L"<URL>",
        L"<EMAIL>"};

void UAX29URLEmailTokenizer::setMaxTokenLength(int length)
{
  if (length < 1) {
    throw invalid_argument(L"maxTokenLength must be greater than zero");
  } else if (length > MAX_TOKEN_LENGTH_LIMIT) {
    throw invalid_argument(L"maxTokenLength may not exceed " +
                           to_wstring(MAX_TOKEN_LENGTH_LIMIT));
  }
  if (length != maxTokenLength) {
    this->maxTokenLength = length;
    scanner->setBufferSize(length);
  }
}

int UAX29URLEmailTokenizer::getMaxTokenLength() { return maxTokenLength; }

UAX29URLEmailTokenizer::UAX29URLEmailTokenizer() : scanner(getScanner()) {}

UAX29URLEmailTokenizer::UAX29URLEmailTokenizer(
    shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::Tokenizer(factory), scanner(getScanner())
{
}

shared_ptr<UAX29URLEmailTokenizerImpl> UAX29URLEmailTokenizer::getScanner()
{
  return make_shared<UAX29URLEmailTokenizerImpl>(input);
}

bool UAX29URLEmailTokenizer::incrementToken() 
{
  clearAttributes();
  skippedPositions = 0;

  while (true) {
    int tokenType = scanner->getNextToken();

    if (tokenType == UAX29URLEmailTokenizerImpl::YYEOF) {
      return false;
    }

    if (scanner->yylength() <= maxTokenLength) {
      posIncrAtt->setPositionIncrement(skippedPositions + 1);
      scanner->getText(termAtt);
      constexpr int start = scanner->yychar();
      offsetAtt->setOffset(correctOffset(start),
                           correctOffset(start + termAtt->length()));
      typeAtt->setType(TOKEN_TYPES[tokenType]);
      return true;
    } else {
      // When we skip a too-long term, we still increment the
      // position increment
      skippedPositions++;
    }
  }
}

void UAX29URLEmailTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  int finalOffset = correctOffset(scanner->yychar() + scanner->yylength());
  offsetAtt->setOffset(finalOffset, finalOffset);
  // adjust any skipped tokens
  posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                   skippedPositions);
}

UAX29URLEmailTokenizer::~UAX29URLEmailTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  scanner->yyreset(input);
}

void UAX29URLEmailTokenizer::reset() 
{
  Tokenizer::reset();
  scanner->yyreset(input);
  skippedPositions = 0;
}
} // namespace org::apache::lucene::analysis::standard