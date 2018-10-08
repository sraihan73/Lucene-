#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
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
namespace org::apache::lucene::queries::payloads
{

using namespace org::apache::lucene::analysis;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 *
 *
 **/
class PayloadHelper : public std::enable_shared_from_this<PayloadHelper>
{
  GET_CLASS_NAME(PayloadHelper)

private:
  std::deque<char> payloadField = std::deque<char>{1};
  std::deque<char> payloadMultiField1 = std::deque<char>{2};
  std::deque<char> payloadMultiField2 = std::deque<char>{4};

public:
  static const std::wstring NO_PAYLOAD_FIELD;
  static const std::wstring MULTI_FIELD;
  static const std::wstring FIELD;

  std::shared_ptr<IndexReader> reader;

public:
  class PayloadAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)
  private:
    std::shared_ptr<PayloadHelper> outerInstance;

  public:
    PayloadAnalyzer(std::shared_ptr<PayloadHelper> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

public:
  class PayloadFilter final : public TokenFilter
  {
    GET_CLASS_NAME(PayloadFilter)
  private:
    std::shared_ptr<PayloadHelper> outerInstance;

    const std::wstring fieldName;
    int numSeen = 0;
    const std::shared_ptr<PayloadAttribute> payloadAtt;

  public:
    PayloadFilter(std::shared_ptr<PayloadHelper> outerInstance,
                  std::shared_ptr<TokenStream> input,
                  const std::wstring &fieldName);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<PayloadFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadFilter>(
          TokenFilter::shared_from_this());
    }
  };

  /**
   * Sets up a RAMDirectory, and adds documents (using English.intToEnglish())
   * with two fields: field and multiField and analyzes them using the
   * PayloadAnalyzer
   * @param similarity The Similarity class to use in the Searcher
   * @param numDocs The num docs to add
   * @return An IndexSearcher
   */
  // TODO: randomize
public:
  virtual std::shared_ptr<IndexSearcher>
  setUp(std::shared_ptr<Random> random, std::shared_ptr<Similarity> similarity,
        int numDocs) ;

  virtual void tearDown() ;
};

} // namespace org::apache::lucene::queries::payloads
