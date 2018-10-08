using namespace std;

#include "OpenNLPLemmatizerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "OpenNLPTokenizer.h"
#include "tools/NLPLemmatizerOp.h"

namespace org::apache::lucene::analysis::opennlp
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NLPLemmatizerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPLemmatizerOp;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

OpenNLPLemmatizerFilter::OpenNLPLemmatizerFilter(
    shared_ptr<TokenStream> input, shared_ptr<NLPLemmatizerOp> lemmatizerOp)
    : org::apache::lucene::analysis::TokenFilter(input),
      lemmatizerOp(lemmatizerOp)
{
}

bool OpenNLPLemmatizerFilter::incrementToken() 
{
  if (!moreTokensAvailable) {
    clear();
    return false;
  }
  if (sentenceTokenAttrsIter == nullptr || !sentenceTokenAttrsIter->hasNext()) {
    nextSentence();
    if (sentenceTokens.empty()) { // zero non-keyword tokens
      clear();
      return false;
    }
    lemmas = lemmatizerOp->lemmatize(sentenceTokens, sentenceTokenTypes);
    lemmaNum = 0;
    sentenceTokenAttrsIter = sentenceTokenAttrs.begin();
  }
  clearAttributes();
  sentenceTokenAttrsIter->next().copyTo(shared_from_this());
  if (!keywordAtt->isKeyword()) {
    termAtt->setEmpty()->append(lemmas[lemmaNum++]);
  }
  return true;
}

void OpenNLPLemmatizerFilter::nextSentence() 
{
  deque<wstring> tokenList = deque<wstring>();
  deque<wstring> typeList = deque<wstring>();
  sentenceTokenAttrs.clear();
  bool endOfSentence = false;
  while (!endOfSentence && (moreTokensAvailable = input->incrementToken())) {
    if (!keywordAtt->isKeyword()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      tokenList.push_back(termAtt->toString());
      typeList.push_back(typeAtt->type());
    }
    endOfSentence =
        0 != (flagsAtt->getFlags() & OpenNLPTokenizer::EOS_FLAG_BIT);
    sentenceTokenAttrs.push_back(input->cloneAttributes());
  }
  sentenceTokens =
      tokenList.size() > 0
          ? tokenList.toArray(std::deque<wstring>(tokenList.size()))
          : nullptr;
  sentenceTokenTypes =
      typeList.size() > 0
          ? typeList.toArray(std::deque<wstring>(typeList.size()))
          : nullptr;
}

void OpenNLPLemmatizerFilter::reset() 
{
  TokenFilter::reset();
  moreTokensAvailable = true;
  clear();
}

void OpenNLPLemmatizerFilter::clear()
{
  sentenceTokenAttrs.clear();
  sentenceTokenAttrsIter.reset();
  sentenceTokens.clear();
  sentenceTokenTypes.clear();
  lemmas.clear();
  lemmaNum = 0;
}
} // namespace org::apache::lucene::analysis::opennlp