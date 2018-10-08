using namespace std;

#include "LambdaTTF.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

LambdaTTF::LambdaTTF() {}

float LambdaTTF::lambda(shared_ptr<BasicStats> stats)
{
  return (stats->getTotalTermFreq() + 1.0F) /
         (stats->getNumberOfDocuments() + 1.0F);
}

shared_ptr<Explanation> LambdaTTF::explain(shared_ptr<BasicStats> stats)
{
  return Explanation::match(
      lambda(stats), getClass().getSimpleName() + L", computed from: ",
      {Explanation::match(stats->getTotalTermFreq(), L"totalTermFreq"),
       Explanation::match(stats->getNumberOfDocuments(),
                          L"numberOfDocuments")});
}

wstring LambdaTTF::toString() { return L"L"; }
} // namespace org::apache::lucene::search::similarities