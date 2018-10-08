using namespace std;

#include "NLPChunkerOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::chunker::ChunkerME;
using opennlp::tools::chunker::ChunkerModel;

NLPChunkerOp::NLPChunkerOp(shared_ptr<ChunkerModel> chunkerModel) throw(
    IOException)
{
  chunker = make_shared<opennlp::tools->chunker->ChunkerME>(chunkerModel);
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<wstring> NLPChunkerOp::getChunks(std::deque<wstring> &words,
                                             std::deque<wstring> &tags,
                                             std::deque<double> &probs)
{
  std::deque<wstring> chunks = chunker->chunk(words, tags);
  if (probs.size() > 0) {
    chunker->probs(probs);
  }
  return chunks;
}
} // namespace org::apache::lucene::analysis::opennlp::tools