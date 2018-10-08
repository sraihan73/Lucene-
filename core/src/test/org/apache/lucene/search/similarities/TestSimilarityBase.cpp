using namespace std;

#include "TestSimilarityBase.h"

namespace org::apache::lucene::search::similarities
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Store = org::apache::lucene::document::Field::Store;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using TopDocs = org::apache::lucene::search::TopDocs;
using SimWeight =
    org::apache::lucene::search::similarities::Similarity::SimWeight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Version = org::apache::lucene::util::Version;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
wstring TestSimilarityBase::FIELD_BODY = L"body";
wstring TestSimilarityBase::FIELD_ID = L"id";
float TestSimilarityBase::FLOAT_EPSILON = 1e-5.0f;
std::deque<std::shared_ptr<BasicModel>> TestSimilarityBase::BASIC_MODELS = {
    make_shared<BasicModelBE>(), make_shared<BasicModelD>(),
    make_shared<BasicModelG>(),  make_shared<BasicModelIF>(),
    make_shared<BasicModelIn>(), make_shared<BasicModelIne>(),
    make_shared<BasicModelP>()};
std::deque<std::shared_ptr<AfterEffect>> TestSimilarityBase::AFTER_EFFECTS = {
    make_shared<AfterEffectB>(), make_shared<AfterEffectL>(),
    make_shared<AfterEffect::NoAfterEffect>()};
std::deque<std::shared_ptr<Normalization>> TestSimilarityBase::NORMALIZATIONS =
    {make_shared<NormalizationH1>(), make_shared<NormalizationH2>(),
     make_shared<NormalizationH3>(), make_shared<NormalizationZ>(),
     make_shared<Normalization::NoNormalization>()};
std::deque<std::shared_ptr<Distribution>> TestSimilarityBase::DISTRIBUTIONS = {
    make_shared<DistributionLL>(), make_shared<DistributionSPL>()};
std::deque<std::shared_ptr<Lambda>> TestSimilarityBase::LAMBDAS = {
    make_shared<LambdaDF>(), make_shared<LambdaTTF>()};
std::deque<std::shared_ptr<Independence>>
    TestSimilarityBase::INDEPENDENCE_MEASURES = {
        make_shared<IndependenceStandardized>(),
        make_shared<IndependenceSaturated>(),
        make_shared<IndependenceChiSquared>()};

void TestSimilarityBase::setUp() 
{
  LuceneTestCase::setUp();

  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  for (int i = 0; i < docs.size(); i++) {
    shared_ptr<Document> d = make_shared<Document>();
    shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
    ft->setIndexOptions(IndexOptions::NONE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(newField(FIELD_ID, Integer::toString(i), ft));
    d->push_back(newTextField(FIELD_BODY, docs[i], Field::Store::YES));
    writer->addDocument(d);
  }

  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;

  sims = deque<>();
  for (auto basicModel : BASIC_MODELS) {
    for (auto afterEffect : AFTER_EFFECTS) {
      for (auto normalization : NORMALIZATIONS) {
        sims.push_back(
            make_shared<DFRSimilarity>(basicModel, afterEffect, normalization));
      }
    }
  }
  for (auto distribution : DISTRIBUTIONS) {
    for (auto lambda : LAMBDAS) {
      for (auto normalization : NORMALIZATIONS) {
        sims.push_back(
            make_shared<IBSimilarity>(distribution, lambda, normalization));
      }
    }
  }
  sims.push_back(make_shared<LMDirichletSimilarity>());
  sims.push_back(make_shared<LMJelinekMercerSimilarity>(0.1f));
  sims.push_back(make_shared<LMJelinekMercerSimilarity>(0.7f));
  for (auto independence : INDEPENDENCE_MEASURES) {
    sims.push_back(make_shared<DFISimilarity>(independence));
  }
}

int TestSimilarityBase::NUMBER_OF_DOCUMENTS = 100;
int64_t TestSimilarityBase::NUMBER_OF_FIELD_TOKENS = 5000;
float TestSimilarityBase::AVG_FIELD_LENGTH = 50;
int TestSimilarityBase::DOC_FREQ = 10;
int64_t TestSimilarityBase::TOTAL_TERM_FREQ = 70;
float TestSimilarityBase::FREQ = 7;
int TestSimilarityBase::DOC_LEN = 40;

shared_ptr<BasicStats> TestSimilarityBase::createStats()
{
  shared_ptr<BasicStats> stats = make_shared<BasicStats>(L"spoof", 1.0f);
  stats->setNumberOfDocuments(NUMBER_OF_DOCUMENTS);
  stats->setNumberOfFieldTokens(NUMBER_OF_FIELD_TOKENS);
  stats->setAvgFieldLength(AVG_FIELD_LENGTH);
  stats->setDocFreq(DOC_FREQ);
  stats->setTotalTermFreq(TOTAL_TERM_FREQ);
  return stats;
}

shared_ptr<CollectionStatistics>
TestSimilarityBase::toCollectionStats(shared_ptr<BasicStats> stats)
{
  return make_shared<CollectionStatistics>(stats->field,
                                           stats->getNumberOfDocuments(), -1,
                                           stats->getNumberOfFieldTokens(), -1);
}

shared_ptr<TermStatistics>
TestSimilarityBase::toTermStats(shared_ptr<BasicStats> stats)
{
  return make_shared<TermStatistics>(make_shared<BytesRef>(L"spoofyText"),
                                     stats->getDocFreq(),
                                     stats->getTotalTermFreq());
}

void TestSimilarityBase::unitTestCore(shared_ptr<BasicStats> stats, float freq,
                                      int docLen)
{
  for (auto sim : sims) {
    shared_ptr<BasicStats> realStats =
        std::static_pointer_cast<BasicStats>(sim->computeWeight(
            stats->getBoost(), toCollectionStats(stats), {toTermStats(stats)}));
    float score = sim->score(realStats, freq, docLen);
    float explScore =
        sim->explain(realStats, 1, Explanation::match(freq, L"freq"), docLen)
            ->getValue();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertFalse(L"Score infinite: " + sim->toString(), isinf(score));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertFalse(L"Score NaN: " + sim->toString(), isnan(score));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(L"Score negative: " + sim->toString(), score >= 0);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"score() and explain() return different values: " +
                     sim->toString(),
                 score, explScore, FLOAT_EPSILON);
  }
}

void TestSimilarityBase::testDefault() 
{
  unitTestCore(createStats(), FREQ, DOC_LEN);
}

void TestSimilarityBase::testSparseDocuments() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setNumberOfFieldTokens(stats->getNumberOfDocuments());
  stats->setTotalTermFreq(stats->getDocFreq());
  stats->setAvgFieldLength(static_cast<float>(stats->getNumberOfFieldTokens()) /
                           stats->getNumberOfDocuments());
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testVerySparseDocuments() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setNumberOfFieldTokens(stats->getNumberOfDocuments() * 2 / 3);
  stats->setTotalTermFreq(stats->getDocFreq());
  stats->setAvgFieldLength(static_cast<float>(stats->getNumberOfFieldTokens()) /
                           stats->getNumberOfDocuments());
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testOneDocument() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setNumberOfDocuments(1);
  stats->setNumberOfFieldTokens(DOC_LEN);
  stats->setAvgFieldLength(DOC_LEN);
  stats->setDocFreq(1);
  stats->setTotalTermFreq(static_cast<int>(FREQ));
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testAllDocumentsRelevant() 
{
  shared_ptr<BasicStats> stats = createStats();
  float mult = (0.0f + stats->getNumberOfDocuments()) / stats->getDocFreq();
  stats->setTotalTermFreq(static_cast<int>(stats->getTotalTermFreq() * mult));
  stats->setDocFreq(stats->getNumberOfDocuments());
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testMostDocumentsRelevant() 
{
  shared_ptr<BasicStats> stats = createStats();
  float mult = (0.6f * stats->getNumberOfDocuments()) / stats->getDocFreq();
  stats->setTotalTermFreq(static_cast<int>(stats->getTotalTermFreq() * mult));
  stats->setDocFreq(static_cast<int>(stats->getNumberOfDocuments() * 0.6));
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testOnlyOneRelevantDocument() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setDocFreq(1);
  stats->setTotalTermFreq(static_cast<int>(FREQ) + 3);
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testAllTermsRelevant() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setTotalTermFreq(stats->getNumberOfFieldTokens());
  unitTestCore(stats, DOC_LEN, DOC_LEN);
  stats->setAvgFieldLength(DOC_LEN + 10);
  unitTestCore(stats, DOC_LEN, DOC_LEN);
}

void TestSimilarityBase::testMoreTermsThanDocuments() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setTotalTermFreq(stats->getTotalTermFreq() +
                          stats->getNumberOfDocuments());
  unitTestCore(stats, 2 * FREQ, DOC_LEN);
}

void TestSimilarityBase::testNumberOfTermsAsDocuments() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setTotalTermFreq(stats->getNumberOfDocuments());
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testOneTerm() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setDocFreq(1);
  stats->setTotalTermFreq(1);
  unitTestCore(stats, 1, DOC_LEN);
}

void TestSimilarityBase::testOneRelevantDocument() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setDocFreq(1);
  stats->setTotalTermFreq(static_cast<int>(FREQ));
  unitTestCore(stats, FREQ, DOC_LEN);
}

void TestSimilarityBase::testAllTermsRelevantOnlyOneDocument() throw(
    IOException)
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setNumberOfDocuments(10);
  stats->setNumberOfFieldTokens(50);
  stats->setAvgFieldLength(5);
  stats->setDocFreq(1);
  stats->setTotalTermFreq(50);
  unitTestCore(stats, 50, 50);
}

void TestSimilarityBase::testOnlyOneTermOneDocument() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setNumberOfDocuments(1);
  stats->setNumberOfFieldTokens(1);
  stats->setAvgFieldLength(1);
  stats->setDocFreq(1);
  stats->setTotalTermFreq(1);
  unitTestCore(stats, 1, 1);
}

void TestSimilarityBase::testOnlyOneTerm() 
{
  shared_ptr<BasicStats> stats = createStats();
  stats->setNumberOfFieldTokens(1);
  stats->setAvgFieldLength(1.0f / stats->getNumberOfDocuments());
  stats->setDocFreq(1);
  stats->setTotalTermFreq(1);
  unitTestCore(stats, 1, DOC_LEN);
}

void TestSimilarityBase::testDocumentLengthAverage() 
{
  shared_ptr<BasicStats> stats = createStats();
  unitTestCore(stats, FREQ, static_cast<int>(stats->getAvgFieldLength()));
}

void TestSimilarityBase::testLMDirichlet() 
{
  float p = (FREQ + 2000.0f * (TOTAL_TERM_FREQ + 1) /
                        (NUMBER_OF_FIELD_TOKENS + 1.0f)) /
            (DOC_LEN + 2000.0f);
  float a = 2000.0f / (DOC_LEN + 2000.0f);
  float gold = static_cast<float>(
      log(p / (a * (TOTAL_TERM_FREQ + 1) / (NUMBER_OF_FIELD_TOKENS + 1.0f))) +
      log(a));
  correctnessTestCore(make_shared<LMDirichletSimilarity>(), gold);
}

void TestSimilarityBase::testLMJelinekMercer() 
{
  float p = (1 - 0.1f) * FREQ / DOC_LEN +
            0.1f * (TOTAL_TERM_FREQ + 1) / (NUMBER_OF_FIELD_TOKENS + 1.0f);
  float gold = static_cast<float>(log(
      p / (0.1f * (TOTAL_TERM_FREQ + 1) / (NUMBER_OF_FIELD_TOKENS + 1.0f))));
  correctnessTestCore(make_shared<LMJelinekMercerSimilarity>(0.1f), gold);
}

void TestSimilarityBase::testLLForIB() 
{
  shared_ptr<SimilarityBase> sim = make_shared<IBSimilarity>(
      make_shared<DistributionLL>(), make_shared<LambdaDF>(),
      make_shared<Normalization::NoNormalization>());
  correctnessTestCore(sim, 4.178574562072754f);
}

void TestSimilarityBase::testSPLForIB() 
{
  shared_ptr<SimilarityBase> sim = make_shared<IBSimilarity>(
      make_shared<DistributionSPL>(), make_shared<LambdaTTF>(),
      make_shared<Normalization::NoNormalization>());
  correctnessTestCore(sim, 2.2387237548828125f);
}

void TestSimilarityBase::testPL2() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelP>(), make_shared<AfterEffectL>(),
      make_shared<NormalizationH2>());
  float tfn = static_cast<float>(
      FREQ *
      SimilarityBase::log2(1 + AVG_FIELD_LENGTH / DOC_LEN)); // 8.1894750101
  float l = 1.0f / (tfn + 1.0f);                             // 0.108820144666
  float lambda =
      (1.0f + TOTAL_TERM_FREQ) / (1.0f + NUMBER_OF_DOCUMENTS); // 0.7029703
  float p = static_cast<float>(
      tfn * SimilarityBase::log2(tfn / lambda) +
      (lambda + 1 / (12 * tfn) - tfn) * SimilarityBase::log2(M_E) +
      0.5 * SimilarityBase::log2(2 * M_PI * tfn)); // 21.065619
  float gold = l * p;                              // 2.2923636
  correctnessTestCore(sim, gold);
}

void TestSimilarityBase::testIneB2() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelIne>(), make_shared<AfterEffectB>(),
      make_shared<NormalizationH2>());
  correctnessTestCore(sim, 5.747603416442871f);
}

void TestSimilarityBase::testGL1() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelG>(), make_shared<AfterEffectL>(),
      make_shared<NormalizationH1>());
  correctnessTestCore(sim, 1.6390540599822998f);
}

void TestSimilarityBase::testBEB1() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelBE>(), make_shared<AfterEffectB>(),
      make_shared<NormalizationH1>());
  float tfn = FREQ * AVG_FIELD_LENGTH / DOC_LEN; // 8.75
  float b =
      (TOTAL_TERM_FREQ + 1 + 1) / ((DOC_FREQ + 1) * (tfn + 1)); // 0.67132866
  double f = TOTAL_TERM_FREQ + 1 + tfn;
  double n = f + NUMBER_OF_DOCUMENTS;
  double n1 = n + f - 1;       // 258.5
  double m1 = n + f - tfn - 2; // 248.75
  double n2 = f;               // 79.75
  double m2 = f - tfn;         // 71.0
  float be = static_cast<float>(
      -SimilarityBase::log2(n - 1) - SimilarityBase::log2(M_E) +
      ((m1 + 0.5f) * SimilarityBase::log2(n1 / m1) +
       (n1 - m1) * SimilarityBase::log2(n1)) -
      ((m2 + 0.5f) * SimilarityBase::log2(n2 / m2) +
       (n2 - m2) * SimilarityBase::log2(n2))); // 67.26544321004599
                                               // 15.7720995
  float gold = b * be;                         // 10.588263
  correctnessTestCore(sim, gold);
}

void TestSimilarityBase::testD() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelD>(), make_shared<AfterEffect::NoAfterEffect>(),
      make_shared<Normalization::NoNormalization>());
  double totalTermFreqNorm = TOTAL_TERM_FREQ + FREQ + 1;
  double p = 1.0 / (NUMBER_OF_DOCUMENTS + 1); // 0.009900990099009901
  double phi = FREQ / totalTermFreqNorm;      // 0.08974358974358974
  double D = phi * SimilarityBase::log2(phi / p) +
             (1 - phi) * SimilarityBase::log2((1 - phi) / (1 - p));
  float gold = static_cast<float>(
      totalTermFreqNorm * D +
      0.5 * SimilarityBase::log2(1 + 2 * M_PI * FREQ * (1 - phi))); // 16.328257
  correctnessTestCore(sim, gold);
}

void TestSimilarityBase::testIn2() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelIn>(), make_shared<AfterEffect::NoAfterEffect>(),
      make_shared<NormalizationH2>());
  float tfn = static_cast<float>(
      FREQ * SimilarityBase::log2(1 + AVG_FIELD_LENGTH / DOC_LEN));
  float gold = static_cast<float>(
      tfn * SimilarityBase::log2((NUMBER_OF_DOCUMENTS + 1) / (DOC_FREQ + 0.5)));
  correctnessTestCore(sim, gold);
}

void TestSimilarityBase::testIFB() 
{
  shared_ptr<SimilarityBase> sim = make_shared<DFRSimilarity>(
      make_shared<BasicModelIF>(), make_shared<AfterEffectB>(),
      make_shared<Normalization::NoNormalization>());
  float B = (TOTAL_TERM_FREQ + 1 + 1) / ((DOC_FREQ + 1) * (FREQ + 1)); // 0.8875
  float IF = static_cast<float>(
      FREQ * SimilarityBase::log2(1 + (NUMBER_OF_DOCUMENTS + 1) /
                                          (TOTAL_TERM_FREQ + 0.5)));
  float gold = B * IF; // 7.96761458307
  correctnessTestCore(sim, gold);
}

void TestSimilarityBase::correctnessTestCore(shared_ptr<SimilarityBase> sim,
                                             float gold)
{
  shared_ptr<BasicStats> stats = createStats();
  shared_ptr<BasicStats> realStats =
      std::static_pointer_cast<BasicStats>(sim->computeWeight(
          stats->getBoost(), toCollectionStats(stats), {toTermStats(stats)}));
  float score = sim->score(realStats, FREQ, DOC_LEN);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(sim->toString() + L" score not correct.", gold, score,
               FLOAT_EPSILON);
}

void TestSimilarityBase::testHeartList() 
{
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(FIELD_BODY, L"heart"));

  for (auto sim : sims) {
    searcher->setSimilarity(sim);
    shared_ptr<TopDocs> topDocs = searcher->search(q, 1000);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"Failed: " + sim->toString(), 3, topDocs->totalHits);
  }
}

void TestSimilarityBase::testHeartRanking() 
{
  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(FIELD_BODY, L"heart"));

  for (auto sim : sims) {
    searcher->setSimilarity(sim);
    shared_ptr<TopDocs> topDocs = searcher->search(q, 1000);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"Failed: " + sim->toString(), L"2",
                 reader->document(topDocs->scoreDocs[0]->doc)[FIELD_ID]);
  }
}

void TestSimilarityBase::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestSimilarityBase::testDiscountOverlapsBoost() 
{
  shared_ptr<BM25Similarity> expected = make_shared<BM25Similarity>();
  shared_ptr<SimilarityBase> actual = make_shared<DFRSimilarity>(
      make_shared<BasicModelIne>(), make_shared<AfterEffectB>(),
      make_shared<NormalizationH2>());
  expected->setDiscountOverlaps(false);
  actual->setDiscountOverlaps(false);
  shared_ptr<FieldInvertState> state =
      make_shared<FieldInvertState>(Version::LATEST->major, L"foo");
  state->setLength(5);
  state->setNumOverlap(2);
  assertEquals(expected->computeNorm(state), actual->computeNorm(state));
  expected->setDiscountOverlaps(true);
  actual->setDiscountOverlaps(true);
  assertEquals(expected->computeNorm(state), actual->computeNorm(state));
}

void TestSimilarityBase::testLengthEncodingBackwardCompatibility() throw(
    IOException)
{
  shared_ptr<Similarity> similarity = RandomPicks::randomFrom(random(), sims);
  for (auto indexCreatedVersionMajor :
       std::deque<int>{Version::LUCENE_6_0_0->major, Version::LATEST->major}) {
    for (auto length : std::deque<int>{
             1, 2,
             4}) { // these length values are encoded accurately on both cases
      shared_ptr<Directory> dir = newDirectory();
      // set the version on the directory
      (make_shared<SegmentInfos>(indexCreatedVersionMajor))->commit(dir);
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, newIndexWriterConfig()->setSimilarity(similarity));
      shared_ptr<Document> doc = make_shared<Document>();
      wstring value = IntStream::range(0, length)
                          .mapToObj([&](any i) { L"b"; })
                          .collect(Collectors::joining(L" "));
      doc->push_back(make_shared<TextField>(L"foo", value, Field::Store::NO));
      w->addDocument(doc);
      shared_ptr<IndexReader> reader = DirectoryReader::open(w);
      shared_ptr<IndexSearcher> searcher = newSearcher(reader);
      searcher->setSimilarity(similarity);
      shared_ptr<Term> term = make_shared<Term>(L"foo", L"b");
      shared_ptr<TermContext> context =
          TermContext::build(reader->getContext(), term);
      shared_ptr<SimWeight> simWeight = similarity->computeWeight(
          1.0f, searcher->collectionStatistics(L"foo"),
          {searcher->termStatistics(term, context)});
      shared_ptr<SimilarityBase::BasicSimScorer> simScorer =
          std::static_pointer_cast<SimilarityBase::BasicSimScorer>(
              similarity->simScorer(simWeight, reader->leaves()[0]));
      float docLength = simScorer->getLengthValue(0);
      assertEquals(length, static_cast<int>(docLength));

      delete w;
      delete reader;
      delete dir;
    }
  }
}
} // namespace org::apache::lucene::search::similarities