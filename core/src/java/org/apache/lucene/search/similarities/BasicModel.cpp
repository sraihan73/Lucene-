using namespace std;

#include "BasicModel.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

BasicModel::BasicModel() {}

shared_ptr<Explanation> BasicModel::explain(shared_ptr<BasicStats> stats,
                                            float tfn)
{
  return Explanation::match(
      score(stats, tfn), getClass().getSimpleName() + L", computed from: ",
      {Explanation::match(stats->getNumberOfDocuments(), L"numberOfDocuments"),
       Explanation::match(stats->getTotalTermFreq(), L"totalTermFreq")});
}
} // namespace org::apache::lucene::search::similarities