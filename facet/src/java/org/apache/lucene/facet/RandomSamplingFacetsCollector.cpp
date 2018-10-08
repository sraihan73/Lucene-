using namespace std;

#include "RandomSamplingFacetsCollector.h"

namespace org::apache::lucene::facet
{
using DimConfig = org::apache::lucene::facet::FacetsConfig::DimConfig;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using BitDocIdSet = org::apache::lucene::util::BitDocIdSet;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

RandomSamplingFacetsCollector::XORShift64Random::XORShift64Random(
    int64_t seed)
{
  x = seed == 0 ? 0xdeadbeef : seed;
}

int64_t RandomSamplingFacetsCollector::XORShift64Random::randomLong()
{
  x ^= (x << 21);
  x ^= (static_cast<int64_t>(static_cast<uint64_t>(x) >> 35));
  x ^= (x << 4);
  return x;
}

int RandomSamplingFacetsCollector::XORShift64Random::nextInt(int n)
{
  int res = static_cast<int>(randomLong() % n);
  return (res < 0) ? -res : res;
}

RandomSamplingFacetsCollector::RandomSamplingFacetsCollector(int sampleSize)
    : RandomSamplingFacetsCollector(sampleSize, 0)
{
}

RandomSamplingFacetsCollector::RandomSamplingFacetsCollector(int sampleSize,
                                                             int64_t seed)
    : FacetsCollector(false), sampleSize(sampleSize),
      random(make_shared<XORShift64Random>(seed))
{
  this->sampledDocs.clear();
}

deque<std::shared_ptr<MatchingDocs>>
RandomSamplingFacetsCollector::getMatchingDocs()
{
  deque<std::shared_ptr<MatchingDocs>> matchingDocs =
      FacetsCollector::getMatchingDocs();

  if (totalHits == NOT_CALCULATED) {
    totalHits = 0;
    for (auto md : matchingDocs) {
      totalHits += md->totalHits;
    }
  }

  if (totalHits <= sampleSize) {
    return matchingDocs;
  }

  if (sampledDocs.empty()) {
    samplingRate = (1.0 * sampleSize) / totalHits;
    sampledDocs = createSampledDocs(matchingDocs);
  }
  return sampledDocs;
}

deque<std::shared_ptr<MatchingDocs>>
RandomSamplingFacetsCollector::getOriginalMatchingDocs()
{
  return FacetsCollector::getMatchingDocs();
}

deque<std::shared_ptr<MatchingDocs>>
RandomSamplingFacetsCollector::createSampledDocs(
    deque<std::shared_ptr<MatchingDocs>> &matchingDocsList)
{
  deque<std::shared_ptr<MatchingDocs>> sampledDocsList =
      deque<std::shared_ptr<MatchingDocs>>(matchingDocsList.size());
  for (auto docs : matchingDocsList) {
    sampledDocsList.push_back(createSample(docs));
  }
  return sampledDocsList;
}

shared_ptr<MatchingDocs>
RandomSamplingFacetsCollector::createSample(shared_ptr<MatchingDocs> docs)
{
  int maxdoc = docs->context->reader()->maxDoc();

  // TODO: we could try the WAH8DocIdSet here as well, as the results will be
  // sparse
  shared_ptr<FixedBitSet> sampleDocs = make_shared<FixedBitSet>(maxdoc);

  int binSize = static_cast<int>(1.0 / samplingRate);

  try {
    int counter = 0;
    int limit, randomIndex;
    if (leftoverBin != NOT_CALCULATED) {
      limit = leftoverBin;
      // either NOT_CALCULATED, which means we already sampled from that bin,
      // or the next document to sample
      randomIndex = leftoverIndex;
    } else {
      limit = binSize;
      randomIndex = random->nextInt(binSize);
    }
    shared_ptr<DocIdSetIterator> *const it = docs->bits->begin();
    for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = it->nextDoc()) {
      if (counter == randomIndex) {
        sampleDocs->set(doc);
      }
      counter++;
      if (counter >= limit) {
        counter = 0;
        limit = binSize;
        randomIndex = random->nextInt(binSize);
      }
    }

    if (counter == 0) {
      // we either exhausted the bin and the iterator at the same time, or
      // this segment had no results. in the latter case we might want to
      // carry leftover to the next segment as is, but that complicates the
      // code and doesn't seem so important.
      leftoverBin = leftoverIndex = NOT_CALCULATED;
    } else {
      leftoverBin = limit - counter;
      if (randomIndex > counter) {
        // the document to sample is in the next bin
        leftoverIndex = randomIndex - counter;
      } else if (randomIndex < counter) {
        // we sampled a document from the bin, so just skip over remaining
        // documents in the bin in the next segment.
        leftoverIndex = NOT_CALCULATED;
      }
    }

    return make_shared<MatchingDocs>(docs->context,
                                     make_shared<BitDocIdSet>(sampleDocs),
                                     docs->totalHits, nullptr);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

shared_ptr<FacetResult> RandomSamplingFacetsCollector::amortizeFacetCounts(
    shared_ptr<FacetResult> res, shared_ptr<FacetsConfig> config,
    shared_ptr<IndexSearcher> searcher) 
{
  if (res == nullptr || totalHits <= sampleSize) {
    return res;
  }

  std::deque<std::shared_ptr<LabelAndValue>> fixedLabelValues(
      res->labelValues.size());
  shared_ptr<IndexReader> reader = searcher->getIndexReader();
  shared_ptr<DimConfig> dimConfig = config->getDimConfig(res->dim);

  // +2 to prepend dimension, append child label
  std::deque<wstring> childPath(res->path.size() + 2);
  childPath[0] = res->dim;

  System::arraycopy(res->path, 0, childPath, 1, res->path.size()); // reuse

  for (int i = 0; i < res->labelValues.size(); i++) {
    childPath[res->path.size() + 1] = res->labelValues[i]->label;
    wstring fullPath = FacetsConfig::pathToString(childPath, childPath.size());
    int max =
        reader->docFreq(make_shared<Term>(dimConfig->indexFieldName, fullPath));
    int correctedCount = static_cast<int>(
        res->labelValues[i]->value.doubleValue() / samplingRate);
    correctedCount = min(max, correctedCount);
    fixedLabelValues[i] =
        make_shared<LabelAndValue>(res->labelValues[i]->label, correctedCount);
  }

  // cap the total count on the total number of non-deleted documents in the
  // reader
  int correctedTotalCount = res->value->intValue();
  if (correctedTotalCount > 0) {
    correctedTotalCount =
        min(reader->numDocs(),
            static_cast<int>(res->value->doubleValue() / samplingRate));
  }

  return make_shared<FacetResult>(res->dim, res->path, correctedTotalCount,
                                  fixedLabelValues, res->childCount);
}

double RandomSamplingFacetsCollector::getSamplingRate() { return samplingRate; }
} // namespace org::apache::lucene::facet