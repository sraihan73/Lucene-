#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}

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
namespace org::apache::lucene::codecs::lucene50
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests partial enumeration (only pulling a subset of the indexed data)
 */
class TestBlockPostingsFormat3 : public LuceneTestCase
{
  GET_CLASS_NAME(TestBlockPostingsFormat3)
private:
  const int MAXDOC = TEST_NIGHTLY ? Lucene50PostingsFormat::BLOCK_SIZE * 20
                                  : Lucene50PostingsFormat::BLOCK_SIZE * 3;

  // creates 8 fields with different options and does "duels" of fields against
  // each other
public:
  virtual void test() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestBlockPostingsFormat3> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestBlockPostingsFormat3> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  void verify(std::shared_ptr<Directory> dir) ;

  // following code is almost an exact dup of code from TestDuelingCodecs:
  // sorry!

public:
  virtual void assertTerms(std::shared_ptr<Terms> leftTerms,
                           std::shared_ptr<Terms> rightTerms,
                           bool deep) ;

private:
  void assertTermsSeeking(
      std::shared_ptr<Terms> leftTerms,
      std::shared_ptr<Terms> rightTerms) ;

  /**
   * checks collection-level statistics on Terms
   */
public:
  virtual void assertTermsStatistics(
      std::shared_ptr<Terms> leftTerms,
      std::shared_ptr<Terms> rightTerms) ;

  /**
   * checks the terms enum sequentially
   * if deep is false, it does a 'shallow' test that doesnt go down to the
   * docsenums
   */
  virtual void assertTermsEnum(std::shared_ptr<TermsEnum> leftTermsEnum,
                               std::shared_ptr<TermsEnum> rightTermsEnum,
                               bool deep,
                               bool hasPositions) ;

  /**
   * checks term-level statistics
   */
  virtual void assertTermStats(
      std::shared_ptr<TermsEnum> leftTermsEnum,
      std::shared_ptr<TermsEnum> rightTermsEnum) ;

  /**
   * checks docs + freqs + positions + payloads, sequentially
   */
  virtual void assertDocsAndPositionsEnum(
      std::shared_ptr<PostingsEnum> leftDocs,
      std::shared_ptr<PostingsEnum> rightDocs) ;

  /**
   * checks docs + freqs, sequentially
   */
  virtual void assertDocsEnum(
      std::shared_ptr<PostingsEnum> leftDocs,
      std::shared_ptr<PostingsEnum> rightDocs) ;

  /**
   * checks advancing docs
   */
  virtual void assertDocsSkipping(
      int docFreq, std::shared_ptr<PostingsEnum> leftDocs,
      std::shared_ptr<PostingsEnum> rightDocs) ;

  /**
   * checks advancing docs + positions
   */
  virtual void assertPositionsSkipping(
      int docFreq, std::shared_ptr<PostingsEnum> leftDocs,
      std::shared_ptr<PostingsEnum> rightDocs) ;

private:
  class RandomBits : public std::enable_shared_from_this<RandomBits>,
                     public Bits
  {
    GET_CLASS_NAME(RandomBits)
  public:
    std::shared_ptr<FixedBitSet> bits;

    RandomBits(int maxDoc, double pctLive, std::shared_ptr<Random> random);

    bool get(int index) override;

    int length() override;
  };

protected:
  std::shared_ptr<TestBlockPostingsFormat3> shared_from_this()
  {
    return std::static_pointer_cast<TestBlockPostingsFormat3>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene50
