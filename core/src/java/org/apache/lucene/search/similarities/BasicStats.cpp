using namespace std;

#include "BasicStats.h"

namespace org::apache::lucene::search::similarities
{
using Terms = org::apache::lucene::index::Terms;

BasicStats::BasicStats(const wstring &field, float boost)
    : field(field), boost(boost)
{
}

int64_t BasicStats::getNumberOfDocuments() { return numberOfDocuments; }

void BasicStats::setNumberOfDocuments(int64_t numberOfDocuments)
{
  this->numberOfDocuments = numberOfDocuments;
}

int64_t BasicStats::getNumberOfFieldTokens() { return numberOfFieldTokens; }

void BasicStats::setNumberOfFieldTokens(int64_t numberOfFieldTokens)
{
  this->numberOfFieldTokens = numberOfFieldTokens;
}

float BasicStats::getAvgFieldLength() { return avgFieldLength; }

void BasicStats::setAvgFieldLength(float avgFieldLength)
{
  this->avgFieldLength = avgFieldLength;
}

int64_t BasicStats::getDocFreq() { return docFreq; }

void BasicStats::setDocFreq(int64_t docFreq) { this->docFreq = docFreq; }

int64_t BasicStats::getTotalTermFreq() { return totalTermFreq; }

void BasicStats::setTotalTermFreq(int64_t totalTermFreq)
{
  this->totalTermFreq = totalTermFreq;
}

float BasicStats::getBoost() { return boost; }
} // namespace org::apache::lucene::search::similarities