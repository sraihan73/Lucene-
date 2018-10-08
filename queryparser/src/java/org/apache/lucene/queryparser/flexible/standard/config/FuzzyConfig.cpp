using namespace std;

#include "FuzzyConfig.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;

FuzzyConfig::FuzzyConfig() {}

int FuzzyConfig::getPrefixLength() { return prefixLength; }

void FuzzyConfig::setPrefixLength(int prefixLength)
{
  this->prefixLength = prefixLength;
}

float FuzzyConfig::getMinSimilarity() { return minSimilarity; }

void FuzzyConfig::setMinSimilarity(float minSimilarity)
{
  this->minSimilarity = minSimilarity;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config