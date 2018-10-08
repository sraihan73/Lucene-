using namespace std;

#include "OpenNLPChunkerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "OpenNLPTokenizer.h"
#include "tools/NLPChunkerOp.h"

namespace org::apache::lucene::analysis::opennlp
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NLPChunkerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPChunkerOp;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

OpenNLPChunkerFilter::OpenNLPChunkerFilter(shared_ptr<TokenStream> input,
                                           shared_ptr<NLPChunkerOp> chunkerOp)
    : org::apache::lucene::analysis::TokenFilter(input), chunkerOp(chunkerOp)
{
}

bool OpenNLPChunkerFilter::incrementToken() 
{
  if (!moreTokensAvailable) {
    clear();
    return false;
  }
  if (tokenNum == sentenceTokenAttrs.size()) {
    nextSentence();
    if (sentenceTerms.empty()) {
      clear();
      return false;
    }
    assignTokenTypes(
        chunkerOp->getChunks(sentenceTerms, sentenceTermPOSTags, nullptr));
    tokenNum = 0;
  }
  clearAttributes();
  sentenceTokenAttrs[tokenNum++]->copyTo(shared_from_this());
  return true;
}

void OpenNLPChunkerFilter::nextSentence() 
{
  deque<wstring> termList = deque<wstring>();
  deque<wstring> posTagList = deque<wstring>();
  sentenceTokenAttrs.clear();
  bool endOfSentence = false;
  while (!endOfSentence && (moreTokensAvailable = input->incrementToken())) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    termList.push_back(termAtt->toString());
    posTagList.push_back(typeAtt->type());
    endOfSentence =
        0 != (flagsAtt->getFlags() & OpenNLPTokenizer::EOS_FLAG_BIT);
    sentenceTokenAttrs.push_back(input->cloneAttributes());
  }
  sentenceTerms = termList.size() > 0
                      ? termList.toArray(std::deque<wstring>(termList.size()))
                      : nullptr;
  sentenceTermPOSTags =
      posTagList.size() > 0
          ? posTagList.toArray(std::deque<wstring>(posTagList.size()))
          : nullptr;
}

void OpenNLPChunkerFilter::assignTokenTypes(std::deque<wstring> &tags)
{
  for (int i = 0; i < tags.size(); ++i) {
    sentenceTokenAttrs[i]->getAttribute(TypeAttribute::typeid).setType(tags[i]);
  }
}

void OpenNLPChunkerFilter::reset() 
{
  TokenFilter::reset();
  moreTokensAvailable = true;
  clear();
}

void OpenNLPChunkerFilter::clear()
{
  sentenceTokenAttrs.clear();
  sentenceTerms.clear();
  sentenceTermPOSTags.clear();
  tokenNum = 0;
}
} // namespace org::apache::lucene::analysis::opennlp