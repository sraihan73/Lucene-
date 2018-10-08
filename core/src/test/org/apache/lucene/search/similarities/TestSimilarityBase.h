#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/BasicModel.h"

#include  "core/src/java/org/apache/lucene/search/similarities/AfterEffect.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Normalization.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Distribution.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Lambda.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Independence.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimilarityBase.h"
#include  "core/src/java/org/apache/lucene/search/similarities/BasicStats.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search::similarities
{

using IndexReader = org::apache::lucene::index::IndexReader;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests the {@link SimilarityBase}-based Similarities. Contains unit tests and
 * integration tests for all Similarities and correctness tests for a select
 * few.
 * <p>This class maintains a deque of
 * {@code SimilarityBase} subclasses. Each test case performs its test on all
GET_CLASS_NAME(maintains)
 * items in the deque. If a test case fails, the name of the Similarity that
 * caused the failure is returned as part of the assertion error message.</p>
 * <p>Unit testing is performed by constructing statistics manually and calling
 * the {@link SimilarityBase#score(BasicStats, float, float)} method of the
 * Similarities. The statistics represent corner cases of corpus distributions.
 * </p>
 * <p>For the integration tests, a small (8-document) collection is indexed. The
 * tests verify that for a specific query, all relevant documents are returned
 * in the correct order. The collection consists of two poems of English poet
 * <a href="http://en.wikipedia.org/wiki/William_blake">William Blake</a>.</p>
 * <p>Note: the deque of Similarities is maintained by hand. If a new Similarity
 * is added to the {@code org.apache.lucene.search.similarities} package, the
 * deque should be updated accordingly.</p>
 * <p>
 * In the correctness tests, the score is verified against the result of manual
 * computation. Since it would be impossible to test all Similarities
 * (e.g. all possible DFR combinations, all parameter values for LM), only
 * the best performing setups in the original papers are verified.
 * </p>
 */
class TestSimilarityBase : public LuceneTestCase
{
  GET_CLASS_NAME(TestSimilarityBase)
private:
  static std::wstring FIELD_BODY;
  static std::wstring FIELD_ID;
  /** The tolerance range for float equality. */
  static float FLOAT_EPSILON;
  /** The DFR basic models to test. */
public:
  static std::deque<std::shared_ptr<BasicModel>> BASIC_MODELS;
  /** The DFR aftereffects to test. */
  static std::deque<std::shared_ptr<AfterEffect>> AFTER_EFFECTS;
  /** The DFR normalizations to test. */
  static std::deque<std::shared_ptr<Normalization>> NORMALIZATIONS;
  /** The distributions for IB. */
  static std::deque<std::shared_ptr<Distribution>> DISTRIBUTIONS;
  /** Lambdas for IB. */
  static std::deque<std::shared_ptr<Lambda>> LAMBDAS;
  /** Independence measures for DFI */
  static std::deque<std::shared_ptr<Independence>> INDEPENDENCE_MEASURES;

private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  /** The deque of similarities to test. */
  std::deque<std::shared_ptr<SimilarityBase>> sims;

public:
  void setUp()  override;

  // ------------------------------- Unit tests --------------------------------

  /** The default number of documents in the unit tests. */
private:
  static int NUMBER_OF_DOCUMENTS;
  /** The default total number of tokens in the field in the unit tests. */
  static int64_t NUMBER_OF_FIELD_TOKENS;
  /** The default average field length in the unit tests. */
  static float AVG_FIELD_LENGTH;
  /** The default document frequency in the unit tests. */
  static int DOC_FREQ;
  /**
   * The default total number of occurrences of this term across all documents
   * in the unit tests.
   */
  static int64_t TOTAL_TERM_FREQ;

  /** The default tf in the unit tests. */
  static float FREQ;
  /** The default document length in the unit tests. */
  static int DOC_LEN;

  /** Creates the default statistics object that the specific tests modify. */
  std::shared_ptr<BasicStats> createStats();

  std::shared_ptr<CollectionStatistics>
  toCollectionStats(std::shared_ptr<BasicStats> stats);

  std::shared_ptr<TermStatistics>
  toTermStats(std::shared_ptr<BasicStats> stats);
  /**
   * The generic test core called by all unit test methods. It calls the
   * {@link SimilarityBase#score(BasicStats, float, float)} method of all
   * Similarities in {@link #sims} and checks if the score is valid; i.e. it
   * is a finite positive real number.
   */
  void unitTestCore(std::shared_ptr<BasicStats> stats, float freq, int docLen);

  /** Runs the unit test with the default statistics. */
public:
  virtual void testDefault() ;

  /**
   * Tests correct behavior when
   * {@code numberOfDocuments = numberOfFieldTokens}.
   */
  virtual void testSparseDocuments() ;

  /**
   * Tests correct behavior when
   * {@code numberOfDocuments > numberOfFieldTokens}.
   */
  virtual void testVerySparseDocuments() ;

  /**
   * Tests correct behavior when
   * {@code NumberOfDocuments = 1}.
   */
  virtual void testOneDocument() ;

  /**
   * Tests correct behavior when
   * {@code docFreq = numberOfDocuments}.
   */
  virtual void testAllDocumentsRelevant() ;

  /**
   * Tests correct behavior when
   * {@code docFreq > numberOfDocuments / 2}.
   */
  virtual void testMostDocumentsRelevant() ;

  /**
   * Tests correct behavior when
   * {@code docFreq = 1}.
   */
  virtual void testOnlyOneRelevantDocument() ;

  /**
   * Tests correct behavior when
   * {@code totalTermFreq = numberOfFieldTokens}.
   */
  virtual void testAllTermsRelevant() ;

  /**
   * Tests correct behavior when
   * {@code totalTermFreq > numberOfDocuments}.
   */
  virtual void testMoreTermsThanDocuments() ;

  /**
   * Tests correct behavior when
   * {@code totalTermFreq = numberOfDocuments}.
   */
  virtual void testNumberOfTermsAsDocuments() ;

  /**
   * Tests correct behavior when {@code totalTermFreq = 1}.
   */
  virtual void testOneTerm() ;

  /**
   * Tests correct behavior when {@code totalTermFreq = freq}.
   */
  virtual void testOneRelevantDocument() ;

  /**
   * Tests correct behavior when {@code numberOfFieldTokens = freq}.
   */
  virtual void testAllTermsRelevantOnlyOneDocument() ;

  /**
   * Tests correct behavior when there is only one document with a single term
   * in the collection.
   */
  virtual void testOnlyOneTermOneDocument() ;

  /**
   * Tests correct behavior when there is only one term in the field, but
   * more than one documents.
   */
  virtual void testOnlyOneTerm() ;

  /**
   * Tests correct behavior when {@code avgFieldLength = docLen}.
   */
  virtual void testDocumentLengthAverage() ;

  // ---------------------------- Correctness tests ----------------------------

  /** Correctness test for the Dirichlet LM model. */
  virtual void testLMDirichlet() ;

  /** Correctness test for the Jelinek-Mercer LM model. */
  virtual void testLMJelinekMercer() ;

  /**
   * Correctness test for the LL IB model with DF-based lambda and
   * no normalization.
   */
  virtual void testLLForIB() ;

  /**
   * Correctness test for the SPL IB model with TTF-based lambda and
   * no normalization.
   */
  virtual void testSPLForIB() ;

  /** Correctness test for the PL2 DFR model. */
  virtual void testPL2() ;

  /** Correctness test for the IneB2 DFR model. */
  virtual void testIneB2() ;

  /** Correctness test for the GL1 DFR model. */
  virtual void testGL1() ;

  /** Correctness test for the BEB1 DFR model. */
  virtual void testBEB1() ;

  /** Correctness test for the D DFR model (basic model only). */
  virtual void testD() ;

  /** Correctness test for the In2 DFR model with no aftereffect. */
  virtual void testIn2() ;

  /** Correctness test for the IFB DFR model with no normalization. */
  virtual void testIFB() ;

  /**
   * The generic test core called by all correctness test methods. It calls the
   * {@link SimilarityBase#score(BasicStats, float, float)} method of all
   * Similarities in {@link #sims} and compares the score against the manually
   * computed {@code gold}.
   */
private:
  void correctnessTestCore(std::shared_ptr<SimilarityBase> sim, float gold);

  // ---------------------------- Integration tests ----------------------------

  /** The "collection" for the integration tests. */
public:
  std::deque<std::wstring> docs = std::deque<std::wstring>{
      L"Tiger, tiger burning bright   In the forest of the night   What "
      L"immortal hand or eye   Could frame thy fearful symmetry ?",
      L"In what distant depths or skies   Burnt the fire of thine eyes ?   On "
      L"what wings dare he aspire ?   What the hands the seize the fire ?",
      L"And what shoulder and what art   Could twist the sinews of thy heart ? "
      L"  And when thy heart began to beat What dread hand ? And what dread "
      L"feet ?",
      L"What the hammer? What the chain ?   In what furnace was thy brain ?   "
      L"What the anvil ? And what dread grasp   Dare its deadly terrors clasp "
      L"?",
      L"And when the stars threw down their spears   And water'd heaven with "
      L"their tear   Did he smile his work to see ?   Did he, who made the "
      L"lamb, made thee ?",
      L"Tiger, tiger burning bright   In the forest of the night   What "
      L"immortal hand or eye   Dare frame thy fearful symmetry ?",
      L"Cruelty has a human heart   And jealousy a human face   Terror the "
      L"human form divine   And Secrecy the human dress .",
      L"The human dress is forg'd iron   The human form a fiery forge   The "
      L"human face a furnace seal'd   The human heart its fiery gorge ."};

  /**
   * Tests whether all similarities return three documents for the query word
   * "heart".
   */
  virtual void testHeartList() ;

  /** Test whether all similarities return document 3 before documents 7 and 8.
   */
  virtual void testHeartRanking() ;

  void tearDown()  override;

  // LUCENE-5221
  virtual void testDiscountOverlapsBoost() ;

  virtual void testLengthEncodingBackwardCompatibility() ;

protected:
  std::shared_ptr<TestSimilarityBase> shared_from_this()
  {
    return std::static_pointer_cast<TestSimilarityBase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
