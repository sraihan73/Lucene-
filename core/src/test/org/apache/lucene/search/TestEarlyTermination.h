#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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

using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestEarlyTermination : public LuceneTestCase
{
  GET_CLASS_NAME(TestEarlyTermination)

public:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<RandomIndexWriter> writer;

  void setUp()  override;

  void tearDown()  override;

  virtual void testEarlyTermination() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestEarlyTermination> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestEarlyTermination> outerInstance);

    bool collectionTerminated = false;

    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestEarlyTermination> shared_from_this()
  {
    return std::static_pointer_cast<TestEarlyTermination>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
