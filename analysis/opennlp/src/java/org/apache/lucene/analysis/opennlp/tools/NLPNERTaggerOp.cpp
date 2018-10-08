using namespace std;

#include "NLPNERTaggerOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::namefind::NameFinderME;
using opennlp::tools::namefind::TokenNameFinder;
using opennlp::tools::namefind::TokenNameFinderModel;
using opennlp::tools::util::Span;

NLPNERTaggerOp::NLPNERTaggerOp(shared_ptr<TokenNameFinderModel> model)
    : nameFinder(make_shared<NameFinderME>(model))
{
}

std::deque<std::shared_ptr<Span>>
NLPNERTaggerOp::getNames(std::deque<wstring> &words)
{
  std::deque<std::shared_ptr<Span>> names = nameFinder->find(words);
  return names;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void NLPNERTaggerOp::reset() { nameFinder->clearAdaptiveData(); }
} // namespace org::apache::lucene::analysis::opennlp::tools