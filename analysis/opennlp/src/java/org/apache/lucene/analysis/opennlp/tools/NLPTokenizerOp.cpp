using namespace std;

#include "NLPTokenizerOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::tokenize::Tokenizer;
using opennlp::tools::tokenize::TokenizerME;
using opennlp::tools::tokenize::TokenizerModel;
using opennlp::tools::util::Span;

NLPTokenizerOp::NLPTokenizerOp(shared_ptr<TokenizerModel> model)
    : tokenizer(make_shared<TokenizerME>(model))
{
}

NLPTokenizerOp::NLPTokenizerOp() : tokenizer(tokenizer.reset()) {}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<std::shared_ptr<Span>>
NLPTokenizerOp::getTerms(const wstring &sentence)
{
  if (tokenizer == nullptr) {
    std::deque<std::shared_ptr<Span>> span1(1);
    span1[0] = make_shared<Span>(0, sentence.length());
    return span1;
  }
  return tokenizer->tokenizePos(sentence);
}
} // namespace org::apache::lucene::analysis::opennlp::tools