using namespace std;

#include "NLPPOSTaggerOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::postag::POSModel;
using opennlp::tools::postag::POSTagger;
using opennlp::tools::postag::POSTaggerME;

NLPPOSTaggerOp::NLPPOSTaggerOp(shared_ptr<POSModel> model) 
{
  tagger = make_shared<POSTaggerME>(model);
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<wstring> NLPPOSTaggerOp::getPOSTags(std::deque<wstring> &words)
{
  return tagger->tag(words);
}
} // namespace org::apache::lucene::analysis::opennlp::tools