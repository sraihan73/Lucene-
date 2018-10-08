using namespace std;

#include "ClassicTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "ClassicTokenizerImpl.h"

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
std::deque<wstring> const ClassicTokenizer::TOKEN_TYPES = std::deque<wstring>{
    L"<ALPHANUM>", L"<APOSTROPHE>", L"<ACRONYM>", L"<COMPANY>",    L"<EMAIL>",
    L"<HOST>",     L"<NUM>",        L"<CJ>",      L"<ACRONYM_DEP>"};

void ClassicTokenizer::setMaxTokenLength(int length)
{
  if (length < 1) {
    throw invalid_argument(L"maxTokenLength must be greater than zero");
  }
  this->maxTokenLength = length;
}

int ClassicTokenizer::getMaxTokenLength() { return maxTokenLength; }

ClassicTokenizer::ClassicTokenizer() { init(); }

ClassicTokenizer::ClassicTokenizer(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::analysis::Tokenizer(factory)
{
  init();
}

void ClassicTokenizer::init()
{
  this->scanner = make_shared<ClassicTokenizerImpl>(input);
}

bool ClassicTokenizer::incrementToken() 
{
  clearAttributes();
  skippedPositions = 0;

  while (true) {
    int tokenType = scanner->getNextToken();

    if (tokenType == ClassicTokenizerImpl::YYEOF) {
      return false;
    }

    if (scanner->yylength() <= maxTokenLength) {
      posIncrAtt->setPositionIncrement(skippedPositions + 1);
      scanner->getText(termAtt);
      constexpr int start = scanner->yychar();
      offsetAtt->setOffset(correctOffset(start),
                           correctOffset(start + termAtt->length()));

      if (tokenType == ClassicTokenizer::ACRONYM_DEP) {
        typeAtt->setType(ClassicTokenizer::TOKEN_TYPES[ClassicTokenizer::HOST]);
        termAtt->setLength(termAtt->length() - 1); // remove extra '.'
      } else {
        typeAtt->setType(ClassicTokenizer::TOKEN_TYPES[tokenType]);
      }
      return true;
    } else {
      // When we skip a too-long term, we still increment the
      // position increment
      skippedPositions++;
    }
  }
}

void ClassicTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  int finalOffset = correctOffset(scanner->yychar() + scanner->yylength());
  offsetAtt->setOffset(finalOffset, finalOffset);
  // adjust any skipped tokens
  posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() +
                                   skippedPositions);
}

ClassicTokenizer::~ClassicTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  scanner->yyreset(input);
}

void ClassicTokenizer::reset() 
{
  Tokenizer::reset();
  scanner->yyreset(input);
  skippedPositions = 0;
}
} // namespace org::apache::lucene::analysis::standard