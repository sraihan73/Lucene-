#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class TermRangeQuery;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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
namespace org::apache::lucene::search
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTermRangeQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermRangeQuery)

private:
  int docCount = 0;
  std::shared_ptr<Directory> dir;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testExclusive() ;

  virtual void testInclusive() ;

  virtual void testAllDocs() ;

  /** This test should not be here, but it tests the fuzzy query rewrite mode
   * (TOP_TERMS_SCORING_BOOLEAN_REWRITE) with constant score and checks, that
   * only the lower end of terms is put into the range */
  virtual void testTopTermsRewrite() ;

private:
  void checkBooleanTerms(std::shared_ptr<IndexSearcher> searcher,
                         std::shared_ptr<TermRangeQuery> query,
                         std::deque<std::wstring> &terms) ;

public:
  virtual void testEqualsHashcode();

private:
  class SingleCharAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(SingleCharAnalyzer)

  private:
    class SingleCharTokenizer : public Tokenizer
    {
      GET_CLASS_NAME(SingleCharTokenizer)
    public:
      std::deque<wchar_t> buffer = std::deque<wchar_t>(1);
      bool done = false;
      std::shared_ptr<CharTermAttribute> termAtt;

      SingleCharTokenizer();

      bool incrementToken()  override;

      void reset()  override;

    protected:
      std::shared_ptr<SingleCharTokenizer> shared_from_this()
      {
        return std::static_pointer_cast<SingleCharTokenizer>(
            org.apache.lucene.analysis.Tokenizer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<SingleCharAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<SingleCharAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  void initializeIndex(std::deque<std::wstring> &values) ;

  void initializeIndex(std::deque<std::wstring> &values,
                       std::shared_ptr<Analyzer> analyzer) ;

  // shouldnt create an analyzer for every doc?
  void addDoc(const std::wstring &content) ;

  void insertDoc(std::shared_ptr<IndexWriter> writer,
                 const std::wstring &content) ;

  // LUCENE-38
public:
  virtual void testExclusiveLowerNull() ;

  // LUCENE-38
  virtual void testInclusiveLowerNull() ;

protected:
  std::shared_ptr<TestTermRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestTermRangeQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
