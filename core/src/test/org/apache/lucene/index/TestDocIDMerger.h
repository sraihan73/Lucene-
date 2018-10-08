#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergeState;
}

namespace org::apache::lucene::index
{
class DocMap;
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

class TestDocIDMerger : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocIDMerger)

private:
  class TestSubUnsorted : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(TestSubUnsorted)
  private:
    int docID = -1;

  public:
    const int valueStart;
    const int maxDoc;

    TestSubUnsorted(std::shared_ptr<MergeState::DocMap> docMap, int maxDoc,
                    int valueStart);

    int nextDoc() override;

    virtual int getValue();

  protected:
    std::shared_ptr<TestSubUnsorted> shared_from_this()
    {
      return std::static_pointer_cast<TestSubUnsorted>(
          DocIDMerger.Sub::shared_from_this());
    }
  };

public:
  virtual void testNoSort() ;

private:
  class DocMapAnonymousInnerClass : public MergeState::DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocIDMerger> outerInstance;

    int docBase = 0;

  public:
    DocMapAnonymousInnerClass(std::shared_ptr<TestDocIDMerger> outerInstance,
                              int docBase);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass>(
          MergeState.DocMap::shared_from_this());
    }
  };

private:
  class TestSubSorted : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(TestSubSorted)
  private:
    int docID = -1;

  public:
    const int maxDoc;
    const int index;

    TestSubSorted(std::shared_ptr<MergeState::DocMap> docMap, int maxDoc,
                  int index);

    int nextDoc() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TestSubSorted> shared_from_this()
    {
      return std::static_pointer_cast<TestSubSorted>(
          DocIDMerger.Sub::shared_from_this());
    }
  };

public:
  virtual void testWithSort() ;

private:
  class DocMapAnonymousInnerClass : public MergeState::DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocIDMerger> outerInstance;

    std::shared_ptr<FixedBitSet> liveDocs;
    std::deque<int> docMap;

  public:
    DocMapAnonymousInnerClass(std::shared_ptr<TestDocIDMerger> outerInstance,
                              std::shared_ptr<FixedBitSet> liveDocs,
                              std::deque<int> &docMap);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass>(
          MergeState.DocMap::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDocIDMerger> shared_from_this()
  {
    return std::static_pointer_cast<TestDocIDMerger>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
