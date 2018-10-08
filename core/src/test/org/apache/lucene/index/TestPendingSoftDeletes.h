#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PendingSoftDeletes.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesFieldUpdates.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

class TestPendingSoftDeletes : public TestPendingDeletes
{
  GET_CLASS_NAME(TestPendingSoftDeletes)

protected:
  std::shared_ptr<PendingSoftDeletes>
  newPendingDeletes(std::shared_ptr<SegmentCommitInfo> commitInfo) override;

public:
  virtual void testHardDeleteSoftDeleted() ;

  virtual void testDeleteSoft() ;

  virtual void testApplyUpdates() ;

  virtual void testUpdateAppliedOnlyOnce() ;

  virtual void testResetOnUpdate() ;

private:
  std::shared_ptr<DocValuesFieldUpdates>
  singleUpdate(std::deque<int> &docsChanged, int maxDoc, bool hasValue);

private:
  class DocValuesFieldUpdatesAnonymousInnerClass : public DocValuesFieldUpdates
  {
    GET_CLASS_NAME(DocValuesFieldUpdatesAnonymousInnerClass)
  private:
    std::shared_ptr<TestPendingSoftDeletes> outerInstance;

    std::deque<int> docsChanged;
    bool hasValue = false;

  public:
    DocValuesFieldUpdatesAnonymousInnerClass(
        std::shared_ptr<TestPendingSoftDeletes> outerInstance, int maxDoc,
        std::deque<int> &docsChanged, bool hasValue);

    void add(int doc, int64_t value) override;

    void add(int doc, std::shared_ptr<BytesRef> value) override;

    void add(int docId, std::shared_ptr<Iterator> iterator) override;

    std::shared_ptr<Iterator> iterator() override;

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<DocValuesFieldUpdatesAnonymousInnerClass> outerInstance;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<DocValuesFieldUpdatesAnonymousInnerClass>
              outerInstance);

      std::shared_ptr<java::util::Iterator<int>> iter;
      int doc = 0;

      int nextDoc();

      int64_t longValue();

      std::shared_ptr<BytesRef> binaryValue();

      int docID();

      int64_t delGen();

      bool hasValue();
    };

  protected:
    std::shared_ptr<DocValuesFieldUpdatesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesFieldUpdatesAnonymousInnerClass>(
          DocValuesFieldUpdates::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPendingSoftDeletes> shared_from_this()
  {
    return std::static_pointer_cast<TestPendingSoftDeletes>(
        TestPendingDeletes::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
