using namespace std;

#include "FloatTaxonomyFacets.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DimConfig = org::apache::lucene::facet::FacetsConfig::DimConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using TopOrdAndFloatQueue = org::apache::lucene::facet::TopOrdAndFloatQueue;

FloatTaxonomyFacets::FloatTaxonomyFacets(
    const wstring &indexFieldName, shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config) 
    : TaxonomyFacets(indexFieldName, taxoReader, config),
      values(std::deque<float>(taxoReader->getSize()))
{
}

void FloatTaxonomyFacets::rollup() 
{
  // Rollup any necessary dims:
  std::deque<int> children = getChildren();
  for (auto ent : config->getDimConfigs()) {
    wstring dim = ent.first;
    shared_ptr<FacetsConfig::DimConfig> ft = ent.second;
    if (ft->hierarchical && ft->multiValued == false) {
      int dimRootOrd = taxoReader->getOrdinal(make_shared<FacetLabel>(dim));
      assert(dimRootOrd > 0);
      values[dimRootOrd] += rollup(children[dimRootOrd]);
    }
  }
}

float FloatTaxonomyFacets::rollup(int ord) 
{
  std::deque<int> children = getChildren();
  std::deque<int> siblings = getSiblings();
  float sum = 0;
  while (ord != TaxonomyReader::INVALID_ORDINAL) {
    float childValue = values[ord] + rollup(children[ord]);
    values[ord] = childValue;
    sum += childValue;
    ord = siblings[ord];
  }
  return sum;
}

shared_ptr<Number>
FloatTaxonomyFacets::getSpecificValue(const wstring &dim,
                                      deque<wstring> &path) 
{
  shared_ptr<FacetsConfig::DimConfig> dimConfig = verifyDim(dim);
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
  return values[ord];
}

shared_ptr<FacetResult>
FloatTaxonomyFacets::getTopChildren(int topN, const wstring &dim,
                                    deque<wstring> &path) 
{
  if (topN <= 0) {
    throw invalid_argument(L"topN must be > 0 (got: " + to_wstring(topN) +
                           L")");
  }
  shared_ptr<FacetsConfig::DimConfig> dimConfig = verifyDim(dim);
  shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(dim, path);
  int dimOrd = taxoReader->getOrdinal(cp);
  if (dimOrd == -1) {
    return nullptr;
  }

  shared_ptr<TopOrdAndFloatQueue> q =
      make_shared<TopOrdAndFloatQueue>(min(taxoReader->getSize(), topN));
  float bottomValue = 0;

  std::deque<int> children = getChildren();
  std::deque<int> siblings = getSiblings();

  int ord = children[dimOrd];
  float sumValues = 0;
  int childCount = 0;

  shared_ptr<TopOrdAndFloatQueue::OrdAndValue> reuse = nullptr;
  while (ord != TaxonomyReader::INVALID_ORDINAL) {
    if (values[ord] > 0) {
      sumValues += values[ord];
      childCount++;
      if (values[ord] > bottomValue) {
        if (reuse == nullptr) {
          reuse = make_shared<TopOrdAndFloatQueue::OrdAndValue>();
        }
        reuse->ord = ord;
        reuse->value = values[ord];
        reuse = q->insertWithOverflow(reuse);
        if (q->size() == topN) {
          bottomValue = q->top()->value;
        }
      }
    }

    ord = siblings[ord];
  }

  if (sumValues == 0) {
    return nullptr;
  }

  if (dimConfig->multiValued) {
    if (dimConfig->requireDimCount) {
      sumValues = values[dimOrd];
    } else {
      // Our sum'd count is not correct, in general:
      sumValues = -1;
    }
  } else {
    // Our sum'd dim count is accurate, so we keep it
  }

  std::deque<std::shared_ptr<LabelAndValue>> labelValues(q->size());
  for (int i = labelValues.size() - 1; i >= 0; i--) {
    shared_ptr<TopOrdAndFloatQueue::OrdAndValue> ordAndValue = q->pop();
    shared_ptr<FacetLabel> child = taxoReader->getPath(ordAndValue->ord);
    labelValues[i] = make_shared<LabelAndValue>(child->components[cp->length],
                                                ordAndValue->value);
  }

  return make_shared<FacetResult>(dim, path, sumValues, labelValues,
                                  childCount);
}
} // namespace org::apache::lucene::facet::taxonomy