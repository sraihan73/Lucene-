#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DocConsumer;
}

namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
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
namespace org::apache::lucene::index
{

using IndexingChain =
    org::apache::lucene::index::DocumentsWriterPerThread::IndexingChain;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterConfig : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterConfig)

private:
  class MySimilarity final : public ClassicSimilarity
  {
    GET_CLASS_NAME(MySimilarity)
    // Does not implement anything - used only for type checking on
    // IndexWriterConfig.

  protected:
    std::shared_ptr<MySimilarity> shared_from_this()
    {
      return std::static_pointer_cast<MySimilarity>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

private:
  class MyIndexingChain final : public IndexingChain
  {
    GET_CLASS_NAME(MyIndexingChain)
    // Does not implement anything - used only for type checking on
    // IndexWriterConfig.

  public:
    std::shared_ptr<DocConsumer> getChain(
        std::shared_ptr<DocumentsWriterPerThread> documentsWriter) override;

  protected:
    std::shared_ptr<MyIndexingChain> shared_from_this()
    {
      return std::static_pointer_cast<MyIndexingChain>(
          org.apache.lucene.index.DocumentsWriterPerThread
              .IndexingChain::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDefaults() throws Exception
  virtual void testDefaults() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSettersChaining() throws Exception
  virtual void testSettersChaining() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReuse() throws Exception
  virtual void testReuse() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOverrideGetters() throws Exception
  virtual void testOverrideGetters() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testConstants() throws Exception
  virtual void testConstants() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testToString() throws Exception
  virtual void testToString() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testInvalidValues() throws Exception
  virtual void testInvalidValues() ;

  virtual void testLiveChangeToCFS() ;

protected:
  std::shared_ptr<TestIndexWriterConfig> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterConfig>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
