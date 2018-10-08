using namespace std;

#include "IntTaxonomyFacets.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using DimConfig = org::apache::lucene::facet::FacetsConfig::DimConfig;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using TopOrdAndIntQueue = org::apache::lucene::facet::TopOrdAndIntQueue;
using com::carrotsearch::hppc::IntIntScatterMap;
using com::carrotsearch::hppc::cursors::IntIntCursor;

IntTaxonomyFacets::IntTaxonomyFacets(
    const wstring &indexFieldName, shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc) 
    : TaxonomyFacets(indexFieldName, taxoReader, config)
{

  if (useHashTable(fc, taxoReader)) {
    sparseValues = make_shared<IntIntScatterMap>();
    values.clear();
  } else {
    sparseValues.reset();
    values = std::deque<int>(taxoReader->getSize());
  }
}

bool IntTaxonomyFacets::useHashTable(shared_ptr<FacetsCollector> fc,
                                     shared_ptr<TaxonomyReader> taxoReader)
{
  if (taxoReader->getSize() < 1024) {
    // small number of unique values: use an array
    return false;
  }

  if (fc == nullptr) {
    // counting all docs: use an array
    return false;
  }

  int maxDoc = 0;
  int sumTotalHits = 0;
  for (auto docs : fc->getMatchingDocs()) {
    sumTotalHits += docs->totalHits;
    maxDoc += docs->context->reader()->maxDoc();
  }

  // if our result set is < 10% of the index, we collect sparsely (use hash
  // map_obj):
  return sumTotalHits < maxDoc / 10;
}

void IntTaxonomyFacets::increment(int ordinal) { increment(ordinal, 1); }

void IntTaxonomyFacets::increment(int ordinal, int amount)
{
  if (sparseValues != nullptr) {
    sparseValues->addTo(ordinal, amount);
  } else {
    values[ordinal] += amount;
  }
}

int IntTaxonomyFacets::getValue(int ordinal)
{
  if (sparseValues != nullptr) {
    return sparseValues->get(ordinal);
  } else {
    return values[ordinal];
  }
}

void IntTaxonomyFacets::rollup() 
{
  // Rollup any necessary dims:
  std::deque<int> children;
  for (auto ent : config->getDimConfigs()) {
    wstring dim = ent.first;
    shared_ptr<DimConfig> ft = ent.second;
    if (ft->hierarchical && ft->multiValued == false) {
      int dimRootOrd = taxoReader->getOrdinal(make_shared<FacetLabel>(dim));
      // It can be -1 if this field was declared in the
      // config but never indexed:
      if (dimRootOrd > 0) {
        if (children.empty()) {
          // lazy init
          children = getChildren();
        }
        increment(dimRootOrd, rollup(children[dimRootOrd]));
      }
    }
  }
}

int IntTaxonomyFacets::rollup(int ord) 
{
  std::deque<int> children = getChildren();
  std::deque<int> siblings = getSiblings();
  int sum = 0;
  while (ord != TaxonomyReader::INVALID_ORDINAL) {
    increment(ord, rollup(children[ord]));
    sum += getValue(ord);
    ord = siblings[ord];
  }
  return sum;
}

shared_ptr<Number>
IntTaxonomyFacets::getSpecificValue(const wstring &dim,
                                    deque<wstring> &path) 
{
  shared_ptr<DimConfig> dimConfig = verifyDim(dim);
  if (path->length == 0) {
    if (dimConfig->hierarchical && dimConfig->multiValued == false) {
      // ok: rolled up at search time
    } else if (dimConfig->requireDimCount && dimConfig->multiValued) {
      // ok: we indexed all ords at index time
    } else {
      throw invalid_argument(L"cannot return dimension-level value alone; use "
                             L"getTopChildren instead");
    }
  }
  int ord = taxoReader->getOrdinal(make_shared<FacetLabel>(dim, path));
  if (ord < 0) {
    return -1;
  }
  return getValue(ord);
}

shared_ptr<FacetResult>
IntTaxonomyFacets::getTopChildren(int topN, const wstring &dim,
                                  deque<wstring> &path) 
{
  if (topN <= 0) {
    throw invalid_argument(L"topN must be > 0 (got: " + to_wstring(topN) +
                           L")");
  }
  shared_ptr<DimConfig> dimConfig = verifyDim(dim);
  shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(dim, path);
  int dimOrd = taxoReader->getOrdinal(cp);
  if (dimOrd == -1) {
    return nullptr;
  }

  shared_ptr<TopOrdAndIntQueue> q =
      make_shared<TopOrdAndIntQueue>(min(taxoReader->getSize(), topN));

  int bottomValue = 0;

  int totValue = 0;
  int childCount = 0;

  shared_ptr<TopOrdAndIntQueue::OrdAndValue> reuse = nullptr;

  // TODO: would be faster if we had a "get the following children" API?  then
  // we can make a single pass over the hashmap

  if (sparseValues != nullptr) {
    for (auto c : sparseValues) {
      int count = c->value;
      int ord = c->key;
      if (parents[ord] == dimOrd && count > 0) {
        totValue += count;
        childCount++;
        if (count > bottomValue) {
          if (reuse == nullptr) {
            reuse = make_shared<TopOrdAndIntQueue::OrdAndValue>();
          }
          reuse->ord = ord;
          reuse->value = count;
          reuse = q->insertWithOverflow(reuse);
          if (q->size() == topN) {
            bottomValue = q->top()->value;
          }
        }
      }
    }
  } else {
    std::deque<int> children = getChildren();
    std::deque<int> siblings = getSiblings();
    int ord = children[dimOrd];
    while (ord != TaxonomyReader::INVALID_ORDINAL) {
      int value = values[ord];
      if (value > 0) {
        totValue += value;
        childCount++;
        if (value > bottomValue) {
          if (reuse == nullptr) {
            reuse = make_shared<TopOrdAndIntQueue::OrdAndValue>();
          }
          reuse->ord = ord;
          reuse->value = value;
          reuse = q->insertWithOverflow(reuse);
          if (q->size() == topN) {
            bottomValue = q->top()->value;
          }
        }
      }

      ord = siblings[ord];
    }
  }

  if (totValue == 0) {
    return nullptr;
  }

  if (dimConfig->multiValued) {
    if (dimConfig->requireDimCount) {
      totValue = getValue(dimOrd);
    } else {
      // Our sum'd value is not correct, in general:
      totValue = -1;
    }
  } else {
    // Our sum'd dim value is accurate, so we keep it
  }

  std::deque<std::shared_ptr<LabelAndValue>> labelValues(q->size());
  for (int i = labelValues.size() - 1; i >= 0; i--) {
    shared_ptr<TopOrdAndIntQueue::OrdAndValue> ordAndValue = q->pop();
    shared_ptr<FacetLabel> child = taxoReader->getPath(ordAndValue->ord);
    labelValues[i] = make_shared<LabelAndValue>(child->components[cp->length],
                                                ordAndValue->value);
  }

  return make_shared<FacetResult>(dim, path, totValue, labelValues, childCount);
}
} // namespace org::apache::lucene::facet::taxonomy