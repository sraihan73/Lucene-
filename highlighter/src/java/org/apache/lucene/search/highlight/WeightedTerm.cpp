using namespace std;

#include "WeightedTerm.h"

namespace org::apache::lucene::search::highlight
{

WeightedTerm::WeightedTerm(float weight, const wstring &term)
{
  this->weight = weight;
  this->term = term;
}

wstring WeightedTerm::getTerm() { return term; }

float WeightedTerm::getWeight() { return weight; }

void WeightedTerm::setTerm(const wstring &term) { this->term = term; }

void WeightedTerm::setWeight(float weight) { this->weight = weight; }
} // namespace org::apache::lucene::search::highlight