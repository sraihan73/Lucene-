using namespace std;

#include "Axiomatic.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

Axiomatic::Axiomatic(float s, int queryLen, float k)
    : s(s), k(k), queryLen(queryLen)
{
  if (Float::isFinite(s) == false || isnan(s) || s < 0 || s > 1) {
    throw invalid_argument(L"illegal s value: " + to_wstring(s) +
                           L", must be between 0 and 1");
  }
  if (Float::isFinite(k) == false || isnan(k) || k < 0 || k > 1) {
    throw invalid_argument(L"illegal k value: " + to_wstring(k) +
                           L", must be between 0 and 1");
  }
  if (queryLen < 0 || queryLen > numeric_limits<int>::max()) {
    throw invalid_argument(L"illegal query length value: " +
                           to_wstring(queryLen) +
                           L", must be larger 0 and smaller than MAX_INT");
  }
}

Axiomatic::Axiomatic(float s) : Axiomatic(s, 1, 0.35f) {}

Axiomatic::Axiomatic(float s, int queryLen) : Axiomatic(s, queryLen, 0.35f) {}

Axiomatic::Axiomatic() : Axiomatic(0.25f, 1, 0.35f) {}

float Axiomatic::score(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return tf(stats, freq, docLen) * ln(stats, freq, docLen) *
             tfln(stats, freq, docLen) * idf(stats, freq, docLen) -
         gamma(stats, freq, docLen);
}

void Axiomatic::explain(deque<std::shared_ptr<Explanation>> &subs,
                        shared_ptr<BasicStats> stats, int doc, float freq,
                        float docLen)
{
  if (stats->getBoost() != 1.0f) {
    subs.push_back(Explanation::match(stats->getBoost(), L"boost"));
  }

  subs.push_back(Explanation::match(this->k, L"k"));
  subs.push_back(Explanation::match(this->s, L"s"));
  subs.push_back(Explanation::match(this->queryLen, L"queryLen"));
  subs.push_back(Explanation::match(tf(stats, freq, docLen), L"tf"));
  subs.push_back(Explanation::match(ln(stats, freq, docLen), L"ln"));
  subs.push_back(Explanation::match(tfln(stats, freq, docLen), L"tfln"));
  subs.push_back(Explanation::match(idf(stats, freq, docLen), L"idf"));
  subs.push_back(Explanation::match(gamma(stats, freq, docLen), L"gamma"));
  SimilarityBase::explain(subs, stats, doc, freq, docLen);
}
} // namespace org::apache::lucene::search::similarities