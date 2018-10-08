using namespace std;

#include "TestRandomSamplingFacetsCollector.h"

namespace org::apache::lucene::facet
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FastTaxonomyFacetCounts =
    org::apache::lucene::facet::taxonomy::FastTaxonomyFacetCounts;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiCollector = org::apache::lucene::search::MultiCollector;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
std::deque<float> const TestRandomSamplingFacetsCollector::CHI_SQUARE_VALUES =
    std::deque<float>{
        0.0f,    3.841f,  5.991f,  7.815f,  9.488f,  11.07f,  12.592f, 14.067f,
        15.507f, 16.919f, 18.307f, 19.675f, 21.026f, 22.362f, 23.685f, 24.996f,
        26.296f, 27.587f, 28.869f, 30.144f, 31.41f,  32.671f, 33.924f, 35.172f,
        36.415f, 37.652f, 38.885f, 40.113f, 41.337f, 42.557f, 43.773f, 44.985f,
        46.194f, 47.4f,   48.602f, 49.802f, 50.998f, 52.192f, 53.384f, 54.572f,
        55.758f, 56.942f, 58.124f, 59.304f, 60.481f, 61.656f, 62.83f,  64.001f,
        65.171f, 66.339f, 67.505f};

void TestRandomSamplingFacetsCollector::testRandomSampling() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<Random> random = TestRandomSamplingFacetsCollector::random();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random, dir);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  constexpr int numCategories = 10;
  int numDocs = atLeast(10000);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(
        L"EvenOdd", (i % 2 == 0) ? L"even" : L"odd", Store::NO));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<FacetField>(
        L"iMod10", Integer::toString(i % numCategories)));
    writer->addDocument(config->build(taxoWriter, doc));
  }
  writer->forceMerge(CHI_SQUARE_VALUES.size() - 1);

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);
  IOUtils::close({writer, taxoWriter});

  // Test empty results
  shared_ptr<RandomSamplingFacetsCollector> collectRandomZeroResults =
      make_shared<RandomSamplingFacetsCollector>(numDocs / 10,
                                                 random->nextLong());

  // There should be no divisions by zero
  searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"EvenOdd", L"NeverMatches")),
      collectRandomZeroResults);

  // There should be no divisions by zero and no null result
  assertNotNull(collectRandomZeroResults->getMatchingDocs());

  // There should be no results at all
  for (auto doc : collectRandomZeroResults->getMatchingDocs()) {
    assertEquals(0, doc->totalHits);
  }

  // Now start searching and retrieve results.

  // Use a query to select half of the documents.
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(L"EvenOdd", L"even"));

  shared_ptr<RandomSamplingFacetsCollector> random10Percent =
      make_shared<RandomSamplingFacetsCollector>(
          numDocs / 10,
          random->nextLong()); // 10% of total docs, 20% of the hits

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  searcher->search(query, MultiCollector::wrap({fc, random10Percent}));

  const deque<std::shared_ptr<MatchingDocs>> matchingDocs =
      random10Percent->getMatchingDocs();

  // count the total hits and sampled docs, also store the number of sampled
  // docs per segment
  int totalSampledDocs = 0, totalHits = 0;
  std::deque<int> numSampledDocs(matchingDocs.size());
  //    System.out.println("numSegments=" + numSampledDocs.length);
  for (int i = 0; i < numSampledDocs.size(); i++) {
    shared_ptr<MatchingDocs> md = matchingDocs[i];
    shared_ptr<DocIdSetIterator> *const iter = md->bits->begin();
    while (iter->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      ++numSampledDocs[i];
    }
    totalSampledDocs += numSampledDocs[i];
    totalHits += md->totalHits;
  }

  // compute the chi-square value for the sampled documents' distribution
  float chi_square = 0;
  for (int i = 0; i < numSampledDocs.size(); i++) {
    shared_ptr<MatchingDocs> md = matchingDocs[i];
    float ei = static_cast<float>(md->totalHits) / totalHits;
    if (ei > 0.0f) {
      float oi = static_cast<float>(numSampledDocs[i]) / totalSampledDocs;
      chi_square += (pow(ei - oi, 2) / ei);
    }
  }

  // Verify that the chi-square value isn't too big. According to
  // http://en.wikipedia.org/wiki/Chi-squared_distribution#Table_of_.CF.872_value_vs_p-value,
  // we basically verify that there is a really small chance of hitting a very
  // bad sample (p-value < 0.05), for n-degrees of freedom. The number 'n'
  // depends on the number of segments.
  assertTrue(L"chisquare not statistically significant enough: " +
                 to_wstring(chi_square),
             chi_square < CHI_SQUARE_VALUES[numSampledDocs.size()]);

  // Test amortized counts - should be 5X the sampled count, but maximum
  // numDocs/10
  shared_ptr<FastTaxonomyFacetCounts> *const random10FacetCounts =
      make_shared<FastTaxonomyFacetCounts>(taxoReader, config, random10Percent);
  shared_ptr<FacetResult> *const random10Result =
      random10FacetCounts->getTopChildren(10, L"iMod10");
  shared_ptr<FacetResult> *const amortized10Result =
      random10Percent->amortizeFacetCounts(random10Result, config, searcher);
  for (int i = 0; i < amortized10Result->labelValues.size(); i++) {
    shared_ptr<LabelAndValue> amortized = amortized10Result->labelValues[i];
    shared_ptr<LabelAndValue> sampled = random10Result->labelValues[i];
    // since numDocs may not divide by 10 exactly, allow for some slack in the
    // amortized count
    assertEquals(amortized->value->floatValue(),
                 min(5 * sampled->value->floatValue(), numDocs / 10.0f), 1.0);
  }

  IOUtils::close({searcher->getIndexReader(), taxoReader, dir, taxoDir});
}
} // namespace org::apache::lucene::facet