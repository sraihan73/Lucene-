using namespace std;

#include "FacetTestCase.h"

namespace org::apache::lucene::facet
{
using CachedOrdinalsReader =
    org::apache::lucene::facet::taxonomy::CachedOrdinalsReader;
using DocValuesOrdinalsReader =
    org::apache::lucene::facet::taxonomy::DocValuesOrdinalsReader;
using FastTaxonomyFacetCounts =
    org::apache::lucene::facet::taxonomy::FastTaxonomyFacetCounts;
using OrdinalsReader = org::apache::lucene::facet::taxonomy::OrdinalsReader;
using TaxonomyFacetCounts =
    org::apache::lucene::facet::taxonomy::TaxonomyFacetCounts;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Facets> FacetTestCase::getTaxonomyFacetCounts(
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> c) 
{
  return getTaxonomyFacetCounts(taxoReader, config, c,
                                FacetsConfig::DEFAULT_INDEX_FIELD_NAME);
}

shared_ptr<Facets> FacetTestCase::getTaxonomyFacetCounts(
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> c,
    const wstring &indexFieldName) 
{
  shared_ptr<Facets> facets;
  if (random()->nextBoolean()) {
    facets = make_shared<FastTaxonomyFacetCounts>(indexFieldName, taxoReader,
                                                  config, c);
  } else {
    shared_ptr<OrdinalsReader> ordsReader =
        make_shared<DocValuesOrdinalsReader>(indexFieldName);
    if (random()->nextBoolean()) {
      ordsReader = make_shared<CachedOrdinalsReader>(ordsReader);
    }
    facets =
        make_shared<TaxonomyFacetCounts>(ordsReader, taxoReader, config, c);
  }

  return facets;
}

std::deque<wstring> FacetTestCase::getRandomTokens(int count)
{
  std::deque<wstring> tokens(count);
  for (int i = 0; i < tokens.size(); i++) {
    tokens[i] = TestUtil::randomRealisticUnicodeString(random(), 1, 10);
    // tokens[i] = _TestUtil.randomSimpleString(random(), 1, 10);
  }
  return tokens;
}

wstring FacetTestCase::pickToken(std::deque<wstring> &tokens)
{
  for (int i = 0; i < tokens.size(); i++) {
    if (random()->nextBoolean()) {
      return tokens[i];
    }
  }

  // Move long tail onto first token:
  return tokens[0];
}

deque<std::shared_ptr<TestDoc>>
FacetTestCase::getRandomDocs(std::deque<wstring> &tokens, int count,
                             int numDims)
{
  deque<std::shared_ptr<TestDoc>> docs = deque<std::shared_ptr<TestDoc>>();
  for (int i = 0; i < count; i++) {
    shared_ptr<TestDoc> doc = make_shared<TestDoc>();
    docs.push_back(doc);
    doc->content = pickToken(tokens);
    doc->dims = std::deque<wstring>(numDims);
    for (int j = 0; j < numDims; j++) {
      doc->dims[j] = pickToken(tokens);
      if (random()->nextInt(10) < 3) {
        break;
      }
    }
    if (VERBOSE) {
      wcout << L"  doc " << i << L": content=" << doc->content << endl;
      for (int j = 0; j < numDims; j++) {
        if (doc->dims[j] != L"") {
          wcout << L"    dim[" << j << L"]=" << doc->dims[j] << endl;
        }
      }
    }
  }

  return docs;
}

void FacetTestCase::sortTies(deque<std::shared_ptr<FacetResult>> &results)
{
  for (auto result : results) {
    sortTies(result->labelValues);
  }
}

void FacetTestCase::sortTies(
    std::deque<std::shared_ptr<LabelAndValue>> &labelValues)
{
  double lastValue = -1;
  int numInRow = 0;
  int i = 0;
  while (i <= labelValues.size()) {
    if (i < labelValues.size() &&
        labelValues[i]->value.doubleValue() == lastValue) {
      numInRow++;
    } else {
      if (numInRow > 1) {
        Arrays::sort(
            labelValues, i - numInRow, i,
            make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));
      }
      numInRow = 1;
      if (i < labelValues.size()) {
        lastValue = labelValues[i]->value.doubleValue();
      }
    }
    i++;
  }
}

FacetTestCase::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<FacetTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FacetTestCase::ComparatorAnonymousInnerClass::compare(
    shared_ptr<LabelAndValue> a, shared_ptr<LabelAndValue> b)
{
  assert(a->value->doubleValue() == b->value->doubleValue());
  return (make_shared<BytesRef>(a->label))
      ->compareTo(make_shared<BytesRef>(b->label));
}

void FacetTestCase::sortLabelValues(
    deque<std::shared_ptr<LabelAndValue>> &labelValues)
{
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(labelValues, new
  // java.util.Comparator<LabelAndValue>()
  sort(labelValues.begin(), labelValues.end(),
       make_shared<ComparatorAnonymousInnerClass2>(shared_from_this()));
}

FacetTestCase::ComparatorAnonymousInnerClass2::ComparatorAnonymousInnerClass2(
    shared_ptr<FacetTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FacetTestCase::ComparatorAnonymousInnerClass2::compare(
    shared_ptr<LabelAndValue> a, shared_ptr<LabelAndValue> b)
{
  if (a->value->doubleValue() > b->value->doubleValue()) {
    return -1;
  } else if (a->value->doubleValue() < b->value->doubleValue()) {
    return 1;
  } else {
    return (make_shared<BytesRef>(a->label))
        ->compareTo(make_shared<BytesRef>(b->label));
  }
}

void FacetTestCase::sortFacetResults(
    deque<std::shared_ptr<FacetResult>> &results)
{
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(results, new
  // java.util.Comparator<FacetResult>()
  sort(results.begin(), results.end(),
       make_shared<ComparatorAnonymousInnerClass3>(shared_from_this()));
}

FacetTestCase::ComparatorAnonymousInnerClass3::ComparatorAnonymousInnerClass3(
    shared_ptr<FacetTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FacetTestCase::ComparatorAnonymousInnerClass3::compare(
    shared_ptr<FacetResult> a, shared_ptr<FacetResult> b)
{
  if (a->value->doubleValue() > b->value->doubleValue()) {
    return -1;
  } else if (b->value->doubleValue() > a->value->doubleValue()) {
    return 1;
  } else {
    return 0;
  }
}

void FacetTestCase::assertFloatValuesEquals(
    deque<std::shared_ptr<FacetResult>> &a,
    deque<std::shared_ptr<FacetResult>> &b)
{
  TestUtil::assertEquals(a.size(), b.size());
  float lastValue = numeric_limits<float>::infinity();
  unordered_map<wstring, std::shared_ptr<FacetResult>> aByDim =
      unordered_map<wstring, std::shared_ptr<FacetResult>>();
  for (int i = 0; i < a.size(); i++) {
    assertTrue(a[i]->value.floatValue() <= lastValue);
    lastValue = a[i]->value.floatValue();
    aByDim.emplace(a[i]->dim, a[i]);
  }
  lastValue = numeric_limits<float>::infinity();
  unordered_map<wstring, std::shared_ptr<FacetResult>> bByDim =
      unordered_map<wstring, std::shared_ptr<FacetResult>>();
  for (int i = 0; i < b.size(); i++) {
    bByDim.emplace(b[i]->dim, b[i]);
    assertTrue(b[i]->value.floatValue() <= lastValue);
    lastValue = b[i]->value.floatValue();
  }
  for (auto dim : aByDim) {
    assertFloatValuesEquals(aByDim[dim.first], bByDim[dim.first]);
  }
}

void FacetTestCase::assertFloatValuesEquals(shared_ptr<FacetResult> a,
                                            shared_ptr<FacetResult> b)
{
  TestUtil::assertEquals(a->dim, b->dim);
  assertTrue(Arrays::equals(a->path, b->path));
  TestUtil::assertEquals(a->childCount, b->childCount);
  assertEquals(a->value->floatValue(), b->value->floatValue(),
               a->value->floatValue() / 1e5);
  TestUtil::assertEquals(a->labelValues.size(), b->labelValues.size());
  for (int i = 0; i < a->labelValues.size(); i++) {
    TestUtil::assertEquals(a->labelValues[i]->label, b->labelValues[i]->label);
    assertEquals(a->labelValues[i]->value.floatValue(),
                 b->labelValues[i]->value.floatValue(),
                 a->labelValues[i]->value.floatValue() / 1e5);
  }
}
} // namespace org::apache::lucene::facet