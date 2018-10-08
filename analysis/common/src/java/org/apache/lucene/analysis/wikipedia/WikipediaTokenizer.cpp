using namespace std;

#include "WikipediaTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "WikipediaTokenizerImpl.h"

namespace org::apache::lucene::analysis::wikipedia
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
const wstring WikipediaTokenizer::INTERNAL_LINK = L"il";
const wstring WikipediaTokenizer::EXTERNAL_LINK = L"el";
const wstring WikipediaTokenizer::EXTERNAL_LINK_URL = L"elu";
const wstring WikipediaTokenizer::CITATION = L"ci";
const wstring WikipediaTokenizer::CATEGORY = L"c";
const wstring WikipediaTokenizer::BOLD = L"b";
const wstring WikipediaTokenizer::ITALICS = L"i";
const wstring WikipediaTokenizer::BOLD_ITALICS = L"bi";
const wstring WikipediaTokenizer::HEADING = L"h";
const wstring WikipediaTokenizer::SUB_HEADING = L"sh";
std::deque<wstring> const WikipediaTokenizer::TOKEN_TYPES =
    std::deque<wstring>{L"<ALPHANUM>", L"<APOSTROPHE>", L"<ACRONYM>",
                         L"<COMPANY>",  L"<EMAIL>",      L"<HOST>",
                         L"<NUM>",      L"<CJ>",         INTERNAL_LINK,
                         EXTERNAL_LINK, CITATION,        CATEGORY,
                         BOLD,          ITALICS,         BOLD_ITALICS,
                         HEADING,       SUB_HEADING,     EXTERNAL_LINK_URL};

WikipediaTokenizer::WikipediaTokenizer()
    : WikipediaTokenizer(TOKENS_ONLY, Collections::emptySet<std::wstring>())
{
}

WikipediaTokenizer::WikipediaTokenizer(
    int tokenOutput, shared_ptr<Set<wstring>> untokenizedTypes)
    : scanner(make_shared<WikipediaTokenizerImpl>(this->input))
{
  init(tokenOutput, untokenizedTypes);
}

WikipediaTokenizer::WikipediaTokenizer(
    shared_ptr<AttributeFactory> factory, int tokenOutput,
    shared_ptr<Set<wstring>> untokenizedTypes)
    : org::apache::lucene::analysis::Tokenizer(factory),
      scanner(make_shared<WikipediaTokenizerImpl>(this->input))
{
  init(tokenOutput, untokenizedTypes);
}

void WikipediaTokenizer::init(int tokenOutput,
                              shared_ptr<Set<wstring>> untokenizedTypes)
{
  // TODO: cutover to enum
  if (tokenOutput != TOKENS_ONLY && tokenOutput != UNTOKENIZED_ONLY &&
      tokenOutput != BOTH) {
    throw invalid_argument(
        L"tokenOutput must be TOKENS_ONLY, UNTOKENIZED_ONLY or BOTH");
  }
  this->tokenOutput = tokenOutput;
  this->untokenizedTypes = untokenizedTypes;
}

bool WikipediaTokenizer::incrementToken() 
{
  if (tokens != nullptr && tokens->hasNext()) {
    shared_ptr<AttributeSource::State> state = tokens->next();
    restoreState(state);
    return true;
  }
  clearAttributes();
  int tokenType = scanner->getNextToken();

  if (tokenType == WikipediaTokenizerImpl::YYEOF) {
    return false;
  }
  wstring type = WikipediaTokenizerImpl::TOKEN_TYPES[tokenType];
  if (tokenOutput == TOKENS_ONLY || untokenizedTypes->contains(type) == false) {
    setupToken();
  } else if (tokenOutput == UNTOKENIZED_ONLY &&
             untokenizedTypes->contains(type) == true) {
    collapseTokens(tokenType);

  } else if (tokenOutput == BOTH) {
    // collapse into a single token, add it to tokens AND output the individual
    // tokens output the untokenized Token first
    collapseAndSaveTokens(tokenType, type);
  }
  int posinc = scanner->getPositionIncrement();
  if (first && posinc == 0) {
    posinc = 1; // don't emit posinc=0 for the first token!
  }
  posIncrAtt->setPositionIncrement(posinc);
  typeAtt->setType(type);
  first = false;
  return true;
}

void WikipediaTokenizer::collapseAndSaveTokens(
    int tokenType, const wstring &type) 
{
  // collapse
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>(32);
  int numAdded = scanner->setText(buffer);
  // TODO: how to know how much whitespace to add
  int theStart = scanner->yychar();
  int lastPos = theStart + numAdded;
  int tmpTokType;
  int numSeen = 0;
  deque<std::shared_ptr<AttributeSource::State>> tmp =
      deque<std::shared_ptr<AttributeSource::State>>();
  setupSavedToken(0, type);
  tmp.push_back(captureState());
  // while we can get a token and that token is the same type and we have not
  // transitioned to a new wiki-item of the same type
  while ((tmpTokType = scanner->getNextToken()) !=
             WikipediaTokenizerImpl::YYEOF &&
         tmpTokType == tokenType && scanner->getNumWikiTokensSeen() > numSeen) {
    int currPos = scanner->yychar();
    // append whitespace
    for (int i = 0; i < (currPos - lastPos); i++) {
      buffer->append(L' ');
    }
    numAdded = scanner->setText(buffer);
    setupSavedToken(scanner->getPositionIncrement(), type);
    tmp.push_back(captureState());
    numSeen++;
    lastPos = currPos + numAdded;
  }
  // trim the buffer
  // TODO: this is inefficient
  wstring s = buffer->toString()->trim();
  termAtt->setEmpty()->append(s);
  offsetAtt->setOffset(correctOffset(theStart),
                       correctOffset(theStart + s.length()));
  flagsAtt->setFlags(UNTOKENIZED_TOKEN_FLAG);
  // The way the loop is written, we will have proceeded to the next token.  We
  // need to pushback the scanner to lastPos
  if (tmpTokType != WikipediaTokenizerImpl::YYEOF) {
    scanner->yypushback(scanner->yylength());
  }
  tokens = tmp.begin();
}

void WikipediaTokenizer::setupSavedToken(int positionInc, const wstring &type)
{
  setupToken();
  posIncrAtt->setPositionIncrement(positionInc);
  typeAtt->setType(type);
}

void WikipediaTokenizer::collapseTokens(int tokenType) 
{
  // collapse
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>(32);
  int numAdded = scanner->setText(buffer);
  // TODO: how to know how much whitespace to add
  int theStart = scanner->yychar();
  int lastPos = theStart + numAdded;
  int tmpTokType;
  int numSeen = 0;
  // while we can get a token and that token is the same type and we have not
  // transitioned to a new wiki-item of the same type
  while ((tmpTokType = scanner->getNextToken()) !=
             WikipediaTokenizerImpl::YYEOF &&
         tmpTokType == tokenType && scanner->getNumWikiTokensSeen() > numSeen) {
    int currPos = scanner->yychar();
    // append whitespace
    for (int i = 0; i < (currPos - lastPos); i++) {
      buffer->append(L' ');
    }
    numAdded = scanner->setText(buffer);
    numSeen++;
    lastPos = currPos + numAdded;
  }
  // trim the buffer
  // TODO: this is inefficient
  wstring s = buffer->toString()->trim();
  termAtt->setEmpty()->append(s);
  offsetAtt->setOffset(correctOffset(theStart),
                       correctOffset(theStart + s.length()));
  flagsAtt->setFlags(UNTOKENIZED_TOKEN_FLAG);
  // The way the loop is written, we will have proceeded to the next token.  We
  // need to pushback the scanner to lastPos
  if (tmpTokType != WikipediaTokenizerImpl::YYEOF) {
    scanner->yypushback(scanner->yylength());
  } else {
    tokens.reset();
  }
}

void WikipediaTokenizer::setupToken()
{
  scanner->getText(termAtt);
  constexpr int start = scanner->yychar();
  offsetAtt->setOffset(correctOffset(start),
                       correctOffset(start + termAtt->length()));
}

WikipediaTokenizer::~WikipediaTokenizer()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  scanner->yyreset(input);
}

void WikipediaTokenizer::reset() 
{
  Tokenizer::reset();
  scanner->yyreset(input);
  tokens.reset();
  scanner->reset();
  first = true;
}

void WikipediaTokenizer::end() 
{
  Tokenizer::end();
  // set final offset
  constexpr int finalOffset =
      correctOffset(scanner->yychar() + scanner->yylength());
  this->offsetAtt->setOffset(finalOffset, finalOffset);
}
} // namespace org::apache::lucene::analysis::wikipedia