#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/join/BitSetProducer.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/similarities/BasicStats.h"

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
namespace org::apache::lucene::search::join
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using namespace org::apache::lucene::search;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBlockJoin : public LuceneTestCase
{
  GET_CLASS_NAME(TestBlockJoin)

  // One resume...
private:
  std::shared_ptr<Document> makeResume(const std::wstring &name,
                                       const std::wstring &country);

  // ... has multiple jobs
  std::shared_ptr<Document> makeJob(const std::wstring &skill, int year);

  // ... has multiple qualifications
  std::shared_ptr<Document> makeQualification(const std::wstring &qualification,
                                              int year);

public:
  virtual void testExtractTerms() ;

  virtual void testEmptyChildFilter() ;

  // You must use ToParentBlockJoinSearcher if you want to do BQ SHOULD queries:
  virtual void testBQShouldJoinedChild() ;

  virtual void testSimple() ;

protected:
  virtual std::shared_ptr<Query> skill(const std::wstring &skill);

public:
  virtual void testSimpleFilter() ;

private:
  void addSkillless(std::shared_ptr<RandomIndexWriter> w) ;

  std::shared_ptr<Document>
  getParentDoc(std::shared_ptr<IndexReader> reader,
               std::shared_ptr<BitSetProducer> parents,
               int childDocID) ;

public:
  virtual void testBoostBug() ;

private:
  std::deque<std::deque<std::wstring>> getRandomFields(int maxUniqueValues);

  std::shared_ptr<Term> randomParentTerm(std::deque<std::wstring> &values);

  std::shared_ptr<Term> randomChildTerm(std::deque<std::wstring> &values);

  std::shared_ptr<Sort> getRandomSort(const std::wstring &prefix,
                                      int numFields);

public:
  virtual void testRandom() ;

private:
  void compareChildHits(
      std::shared_ptr<IndexReader> r, std::shared_ptr<IndexReader> joinR,
      std::shared_ptr<TopDocs> results,
      std::shared_ptr<TopDocs> joinResults) ;

  void compareHits(std::shared_ptr<IndexReader> r,
                   std::shared_ptr<IndexReader> joinR,
                   std::shared_ptr<TopDocs> controlHits,
                   std::unordered_map<int, std::shared_ptr<TopDocs>>
                       &joinResults) ;

public:
  virtual void testMultiChildTypes() ;

  virtual void testAdvanceSingleParentSingleChild() ;

  virtual void testAdvanceSingleParentNoChild() ;

  // LUCENE-4968
  virtual void testChildQueryNeverMatches() ;

  virtual void
  testAdvanceSingleDeletedParentNoChild() ;

  virtual void testIntersectionWithRandomApproximation() ;

  // LUCENE-6588
  // delete documents to simulate FilteredQuery applying a filter as acceptDocs
  virtual void testParentScoringBug() ;

  virtual void testToChildBlockJoinQueryExplain() ;

  virtual void
  testToChildInitialAdvanceParentButNoKids() ;

  virtual void
  testMultiChildQueriesOfDiffParentLevels() ;

  virtual void testScoreMode() ;

private:
  class SimilarityBaseAnonymousInnerClass : public SimilarityBase
  {
    GET_CLASS_NAME(SimilarityBaseAnonymousInnerClass)
  private:
    std::shared_ptr<TestBlockJoin> outerInstance;

  public:
    SimilarityBaseAnonymousInnerClass(
        std::shared_ptr<TestBlockJoin> outerInstance);

    virtual std::wstring toString();

  protected:
    float score(std::shared_ptr<BasicStats> stats, float freq,
                float docLen) override;

  protected:
    std::shared_ptr<SimilarityBaseAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimilarityBaseAnonymousInnerClass>(
          org.apache.lucene.search.similarities
              .SimilarityBase::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestBlockJoin> shared_from_this()
  {
    return std::static_pointer_cast<TestBlockJoin>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
