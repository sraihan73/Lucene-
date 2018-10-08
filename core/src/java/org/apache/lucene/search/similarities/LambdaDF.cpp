using namespace std;

#include "LambdaDF.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

LambdaDF::LambdaDF() {}

float LambdaDF::lambda(shared_ptr<BasicStats> stats)
{
  return (stats->getDocFreq() + 1.0F) / (stats->getNumberOfDocuments() + 1.0F);
}

shared_ptr<Explanation> LambdaDF::explain(shared_ptr<BasicStats> stats)
{
  return Explanation::match(
      lambda(stats), getClass().getSimpleName() + L", computed from: ",
      {Explanation::match(stats->getDocFreq(), L"docFreq"),
       Explanation::match(stats->getNumberOfDocuments(),
                          L"numberOfDocuments")});
}

wstring LambdaDF::toString() { return L"D"; }
} // namespace org::apache::lucene::search::similarities