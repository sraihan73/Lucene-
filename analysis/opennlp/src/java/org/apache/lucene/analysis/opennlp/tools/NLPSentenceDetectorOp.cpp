using namespace std;

#include "NLPSentenceDetectorOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::sentdetect::SentenceDetectorME;
using opennlp::tools::sentdetect::SentenceModel;
using opennlp::tools::util::Span;

NLPSentenceDetectorOp::NLPSentenceDetectorOp(
    shared_ptr<SentenceModel> model) 
    : sentenceSplitter(make_shared<SentenceDetectorME>(model))
{
}

NLPSentenceDetectorOp::NLPSentenceDetectorOp()
    : sentenceSplitter(sentenceSplitter.reset())
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<std::shared_ptr<Span>>
NLPSentenceDetectorOp::splitSentences(const wstring &line)
{
  if (sentenceSplitter != nullptr) {
    return sentenceSplitter->sentPosDetect(line);
  } else {
    std::deque<std::shared_ptr<Span>> shorty(1);
    shorty[0] = make_shared<Span>(0, line.length());
    return shorty;
  }
}
} // namespace org::apache::lucene::analysis::opennlp::tools