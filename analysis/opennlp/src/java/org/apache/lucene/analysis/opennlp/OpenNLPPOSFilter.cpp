using namespace std;

#include "OpenNLPPOSFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "OpenNLPTokenizer.h"
#include "tools/NLPPOSTaggerOp.h"

namespace org::apache::lucene::analysis::opennlp
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NLPPOSTaggerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPPOSTaggerOp;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

OpenNLPPOSFilter::OpenNLPPOSFilter(shared_ptr<TokenStream> input,
                                   shared_ptr<NLPPOSTaggerOp> posTaggerOp)
    : org::apache::lucene::analysis::TokenFilter(input),
      posTaggerOp(posTaggerOp)
{
}

bool OpenNLPPOSFilter::incrementToken() 
{
  if (!moreTokensAvailable) {
    clear();
    return false;
  }
  if (tokenNum ==
      sentenceTokenAttrs
          .size()) { // beginning of stream, or previous sentence exhausted
    std::deque<wstring> sentenceTokens = nextSentence();
    if (sentenceTokens.empty()) {
      clear();
      return false;
    }
    tags = posTaggerOp->getPOSTags(sentenceTokens);
    tokenNum = 0;
  }
  clearAttributes();
  sentenceTokenAttrs[tokenNum]->copyTo(shared_from_this());
  typeAtt->setType(tags[tokenNum++]);
  return true;
}

std::deque<wstring> OpenNLPPOSFilter::nextSentence() 
{
  deque<wstring> termList = deque<wstring>();
  sentenceTokenAttrs.clear();
  bool endOfSentence = false;
  while (!endOfSentence && (moreTokensAvailable = input->incrementToken())) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    termList.push_back(termAtt->toString());
    endOfSentence =
        0 != (flagsAtt->getFlags() & OpenNLPTokenizer::EOS_FLAG_BIT);
    sentenceTokenAttrs.push_back(input->cloneAttributes());
  }
  return termList.size() > 0
             ? termList.toArray(std::deque<wstring>(termList.size()))
             : nullptr;
}

void OpenNLPPOSFilter::reset() 
{
  TokenFilter::reset();
  moreTokensAvailable = true;
  clear();
}

void OpenNLPPOSFilter::clear()
{
  sentenceTokenAttrs.clear();
  tags.clear();
  tokenNum = 0;
}
} // namespace org::apache::lucene::analysis::opennlp