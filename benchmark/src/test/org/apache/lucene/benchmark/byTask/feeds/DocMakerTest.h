#pragma once
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/ContentSource.h"
#include "../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
}

namespace org::apache::lucene::benchmark::byTask::feeds
{
class NoMoreDataException;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using Document = org::apache::lucene::document::Document;

/** Tests the functionality of {@link DocMaker}. */
class DocMakerTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(DocMakerTest)

public:
  class OneDocSource final : public ContentSource
  {
    GET_CLASS_NAME(OneDocSource)

  private:
    bool finish = false;

  public:
    virtual ~OneDocSource();

    std::shared_ptr<DocData> getNextDocData(
        std::shared_ptr<DocData> docData)  override;

  protected:
    std::shared_ptr<OneDocSource> shared_from_this()
    {
      return std::static_pointer_cast<OneDocSource>(
          ContentSource::shared_from_this());
    }
  };

private:
  void doTestIndexProperties(bool setIndexProps, bool indexPropsVal,
                             int numExpectedResults) ;

  std::shared_ptr<Document>
  createTestNormsDocument(bool setNormsProp, bool normsPropVal,
                          bool setBodyNormsProp,
                          bool bodyNormsVal) ;

  /* Tests doc.index.props property. */
public:
  virtual void testIndexProperties() ;

  /* Tests doc.tokenized.norms and doc.body.tokenized.norms properties. */
  virtual void testNorms() ;

  virtual void testDocMakerLeak() ;

protected:
  std::shared_ptr<DocMakerTest> shared_from_this()
  {
    return std::static_pointer_cast<DocMakerTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
