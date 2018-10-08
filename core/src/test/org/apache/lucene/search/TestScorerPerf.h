#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"

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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestScorerPerf : public LuceneTestCase
{
  GET_CLASS_NAME(TestScorerPerf)
public:
  bool validate = true; // set to false when doing performance testing

  std::deque<std::shared_ptr<FixedBitSet>> sets;
  std::deque<std::shared_ptr<Term>> terms;
  std::shared_ptr<IndexSearcher> s;
  std::shared_ptr<IndexReader> r;
  std::shared_ptr<Directory> d;

  // TODO: this should be setUp()....
  virtual void createDummySearcher() ;

  virtual void
  createRandomTerms(int nDocs, int nTerms, double power,
                    std::shared_ptr<Directory> dir) ;

  virtual std::shared_ptr<FixedBitSet> randBitSet(int sz, int numBitsToSet);

  virtual std::deque<std::shared_ptr<FixedBitSet>> randBitSets(int numSets,
                                                                int setSize);

public:
  class CountingHitCollector : public SimpleCollector
  {
    GET_CLASS_NAME(CountingHitCollector)
  public:
    int count = 0;
    int sum = 0;

  protected:
    int docBase = 0;

  public:
    void collect(int doc) override;

    virtual int getCount();
    virtual int getSum();

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<CountingHitCollector> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollector>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  class MatchingHitCollector : public CountingHitCollector
  {
    GET_CLASS_NAME(MatchingHitCollector)
  public:
    std::shared_ptr<FixedBitSet> answer;
    int pos = -1;
    MatchingHitCollector(std::shared_ptr<FixedBitSet> answer);

    virtual void collect(int doc, float score);

  protected:
    std::shared_ptr<MatchingHitCollector> shared_from_this()
    {
      return std::static_pointer_cast<MatchingHitCollector>(
          CountingHitCollector::shared_from_this());
    }
  };

private:
  class BitSetQuery : public Query
  {
    GET_CLASS_NAME(BitSetQuery)

  private:
    const std::shared_ptr<FixedBitSet> docs;

  public:
    BitSetQuery(std::shared_ptr<FixedBitSet> docs);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<BitSetQuery> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<BitSetQuery> outerInstance, float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any other) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<BitSetQuery> shared_from_this()
    {
      return std::static_pointer_cast<BitSetQuery>(Query::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<FixedBitSet>
  addClause(std::shared_ptr<BooleanQuery::Builder> bq,
            std::shared_ptr<FixedBitSet> result);

  virtual int doConjunctions(int iter, int maxClauses) ;

  virtual int doNestedConjunctions(int iter, int maxOuterClauses,
                                   int maxClauses) ;

  virtual int doTermConjunctions(std::shared_ptr<IndexSearcher> s,
                                 int termsInIndex, int maxClauses,
                                 int iter) ;

  virtual int doNestedTermConjunctions(std::shared_ptr<IndexSearcher> s,
                                       int termsInIndex, int maxOuterClauses,
                                       int maxClauses,
                                       int iter) ;

  virtual int doSloppyPhrase(std::shared_ptr<IndexSearcher> s, int termsInIndex,
                             int maxClauses, int iter) ;

  virtual void testConjunctions() ;

  /***
  int bigIter=10;

  public void testConjunctionPerf() throws Exception {
    r = newRandom();
    createDummySearcher();
    validate=false;
    sets=randBitSets(32,1000000);
    for (int i=0; i<bigIter; i++) {
      long start = System.currentTimeMillis();
      doConjunctions(500,6);
      long end = System.currentTimeMillis();
      if (VERBOSE) System.out.println("milliseconds="+(end-start));
    }
    s.close();
  }

  public void testNestedConjunctionPerf() throws Exception {
    r = newRandom();
    createDummySearcher();
    validate=false;
    sets=randBitSets(32,1000000);
    for (int i=0; i<bigIter; i++) {
      long start = System.currentTimeMillis();
      doNestedConjunctions(500,3,3);
      long end = System.currentTimeMillis();
      if (VERBOSE) System.out.println("milliseconds="+(end-start));
    }
    s.close();
  }


  public void testConjunctionTerms() throws Exception {
    r = newRandom();
    validate=false;
    RAMDirectory dir = new RAMDirectory();
    if (VERBOSE) System.out.println("Creating index");
    createRandomTerms(100000,25,.5, dir);
    s = newSearcher(dir, true);
    if (VERBOSE) System.out.println("Starting performance test");
    for (int i=0; i<bigIter; i++) {
      long start = System.currentTimeMillis();
      doTermConjunctions(s,25,5,1000);
      long end = System.currentTimeMillis();
      if (VERBOSE) System.out.println("milliseconds="+(end-start));
    }
    s.close();
  }

  public void testNestedConjunctionTerms() throws Exception {
    r = newRandom();
    validate=false;
    RAMDirectory dir = new RAMDirectory();
    if (VERBOSE) System.out.println("Creating index");
    createRandomTerms(100000,25,.2, dir);
    s = newSearcher(dir, true);
    if (VERBOSE) System.out.println("Starting performance test");
    for (int i=0; i<bigIter; i++) {
      long start = System.currentTimeMillis();
      doNestedTermConjunctions(s,25,3,3,200);
      long end = System.currentTimeMillis();
      if (VERBOSE) System.out.println("milliseconds="+(end-start));
    }
    s.close();
  }


  public void testSloppyPhrasePerf() throws Exception {
    r = newRandom();
    validate=false;
    RAMDirectory dir = new RAMDirectory();
    if (VERBOSE) System.out.println("Creating index");
    createRandomTerms(100000,25,2,dir);
    s = newSearcher(dir, true);
    if (VERBOSE) System.out.println("Starting performance test");
    for (int i=0; i<bigIter; i++) {
      long start = System.currentTimeMillis();
      doSloppyPhrase(s,25,2,1000);
      long end = System.currentTimeMillis();
      if (VERBOSE) System.out.println("milliseconds="+(end-start));
    }
    s.close();
  }
   ***/

protected:
  std::shared_ptr<TestScorerPerf> shared_from_this()
  {
    return std::static_pointer_cast<TestScorerPerf>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
