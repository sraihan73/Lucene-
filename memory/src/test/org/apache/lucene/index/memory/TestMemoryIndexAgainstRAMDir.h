#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/memory/MemoryIndex.h"

#include  "core/src/java/org/apache/lucene/index/CompositeReader.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"
#include  "core/src/java/org/apache/lucene/util/Allocator.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"

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
namespace org::apache::lucene::index::memory
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using namespace org::apache::lucene::index;
using Directory = org::apache::lucene::store::Directory;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using Allocator = org::apache::lucene::util::ByteBlockPool::Allocator;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.hamcrest.CoreMatchers.equalTo;

/**
 * Verifies that Lucene MemoryIndex and RAMDirectory have the same behaviour,
 * returning the same results for queries on some randomish indexes.
 */
class TestMemoryIndexAgainstRAMDir : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMemoryIndexAgainstRAMDir)
private:
  std::shared_ptr<Set<std::wstring>> queries =
      std::unordered_set<std::wstring>();

public:
  static const int ITERATIONS = 100 * RANDOM_MULTIPLIER;

  void setUp()  override;

  /**
   * read a set of queries from a resource file
   */
private:
  std::shared_ptr<Set<std::wstring>>
  readQueries(const std::wstring &resource) ;

  /**
   * runs random tests, up to ITERATIONS times.
   */
public:
  virtual void testRandomQueries() ;

  /**
   * Build a randomish document for both RAMDirectory and MemoryIndex,
   * and run all the queries against it.
   */
  virtual void assertAgainstRAMDirectory(
      std::shared_ptr<MemoryIndex> memory) ;

private:
  void
  duellReaders(std::shared_ptr<CompositeReader> other,
               std::shared_ptr<LeafReader> memIndexReader) ;

  /**
   * Run all queries against both the RAMDirectory and MemoryIndex, ensuring
   * they are the same.
   */
public:
  virtual void assertAllQueries(
      std::shared_ptr<MemoryIndex> memory, std::shared_ptr<Directory> ramdir,
      std::shared_ptr<Analyzer> analyzer) ;

  /**
   * Return a random analyzer (Simple, Stop, Standard) to analyze the terms.
   */
private:
  std::shared_ptr<Analyzer> randomAnalyzer();

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMemoryIndexAgainstRAMDir> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestMemoryIndexAgainstRAMDir> outerInstance);

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

  // a tokenfilter that makes all terms starting with 't' empty strings
public:
  class CrazyTokenFilter final : public TokenFilter
  {
    GET_CLASS_NAME(CrazyTokenFilter)
  public:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

    CrazyTokenFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<CrazyTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<CrazyTokenFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

  /**
   * Some terms to be indexed, in addition to random words.
   * These terms are commonly used in the queries.
   */
private:
  static std::deque<std::wstring> const TEST_TERMS;

  /**
   * half of the time, returns a random term from TEST_TERMS.
   * the other half of the time, returns a random unicode string.
   */
  std::wstring randomTerm();

public:
  virtual void testDocsEnumStart() ;

private:
  std::shared_ptr<ByteBlockPool::Allocator> randomByteBlockAllocator();

  std::shared_ptr<MemoryIndex> randomMemoryIndex();

public:
  virtual void testDocsAndPositionsEnumStart() ;

  // LUCENE-3831
  virtual void testNullPointerException() ;

  // LUCENE-3831
  virtual void testPassesIfWrapped() ;

  virtual void testSameFieldAddedMultipleTimes() ;

  virtual void testNonExistentField() ;

  virtual void
  testDocValuesMemoryIndexVsNormalIndex() ;

  virtual void testNormsWithDocValues() ;

  virtual void
  testPointValuesMemoryIndexVsNormalIndex() ;

  virtual void testDuellMemIndex() ;

  // LUCENE-4880
  virtual void testEmptyString() ;

  virtual void
  testDuelMemoryIndexCoreDirectoryWithArrayField() ;

protected:
  virtual void
  compareTermVectors(std::shared_ptr<Terms> terms,
                     std::shared_ptr<Terms> memTerms,
                     const std::wstring &field_name) ;

protected:
  std::shared_ptr<TestMemoryIndexAgainstRAMDir> shared_from_this()
  {
    return std::static_pointer_cast<TestMemoryIndexAgainstRAMDir>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/memory/
