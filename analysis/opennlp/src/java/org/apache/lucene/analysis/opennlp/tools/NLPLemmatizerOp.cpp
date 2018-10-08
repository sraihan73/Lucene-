using namespace std;

#include "NLPLemmatizerOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::lemmatizer::DictionaryLemmatizer;
using opennlp::tools::lemmatizer::LemmatizerME;
using opennlp::tools::lemmatizer::LemmatizerModel;

NLPLemmatizerOp::NLPLemmatizerOp(
    shared_ptr<InputStream> dictionary,
    shared_ptr<LemmatizerModel> lemmatizerModel) 
    : dictionaryLemmatizer(dictionary == nullptr
                               ? nullptr
                               : make_shared<DictionaryLemmatizer>(dictionary)),
      lemmatizerME(lemmatizerModel == nullptr
                       ? nullptr
                       : make_shared<LemmatizerME>(lemmatizerModel))
{
  assert((dictionary != nullptr || lemmatizerModel != nullptr,
          L"At least one parameter must be non-null"));
}

std::deque<wstring> NLPLemmatizerOp::lemmatize(std::deque<wstring> &words,
                                                std::deque<wstring> &postags)
{
  std::deque<wstring> lemmas;
  std::deque<wstring> maxEntLemmas;
  if (dictionaryLemmatizer != nullptr) {
    lemmas = dictionaryLemmatizer->lemmatize(words, postags);
    for (int i = 0; i < lemmas.size(); ++i) {
      if (lemmas[i] == L"O") { // this word is not in the dictionary
        if (lemmatizerME !=
            nullptr) { // fall back to the MaxEnt lemmatizer if it's enabled
          if (maxEntLemmas.empty()) {
            maxEntLemmas = lemmatizerME->lemmatize(words, postags);
          }
          if (L"_" == maxEntLemmas[i]) {
            lemmas[i] =
                words[i]; // put back the original word if no lemma is found
          } else {
            lemmas[i] = maxEntLemmas[i];
          }
        } else { // there is no MaxEnt lemmatizer
          lemmas[i] =
              words[i]; // put back the original word if no lemma is found
        }
      }
    }
  } else { // there is only a MaxEnt lemmatizer
    maxEntLemmas = lemmatizerME->lemmatize(words, postags);
    for (int i = 0; i < maxEntLemmas.size(); ++i) {
      if (L"_" == maxEntLemmas[i]) {
        maxEntLemmas[i] =
            words[i]; // put back the original word if no lemma is found
      }
    }
    lemmas = maxEntLemmas;
  }
  return lemmas;
}
} // namespace org::apache::lucene::analysis::opennlp::tools