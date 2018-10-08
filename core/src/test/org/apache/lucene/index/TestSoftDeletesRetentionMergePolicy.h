#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

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

using Field = org::apache::lucene::document::Field;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSoftDeletesRetentionMergePolicy : public LuceneTestCase
{
  GET_CLASS_NAME(TestSoftDeletesRetentionMergePolicy)

public:
  virtual void testForceMergeFullyDeleted() ;

  virtual void testKeepFullyDeletedSegments() ;

  virtual void testFieldBasedRetention() ;

  virtual void testKeepAllDocsAcrossMerges() ;

  /**
   * tests soft deletes that carry over deleted documents on merge for history
   * rentention.
   */
  virtual void testSoftDeleteWithRetention() throw(IOException,
                                                   InterruptedException);

private:
  class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
  {
    GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
  private:
    std::shared_ptr<TestSoftDeletesRetentionMergePolicy> outerInstance;

    std::shared_ptr<org::apache::lucene::index::LeafReader> leafReader;

  public:
    FilterLeafReaderAnonymousInnerClass(
        std::shared_ptr<TestSoftDeletesRetentionMergePolicy> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LeafReader> leafReader);

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    std::shared_ptr<Bits> getLiveDocs() override;

    int numDocs() override;

  protected:
    std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
          FilterLeafReader::shared_from_this());
    }
  };

public:
  virtual void testForceMergeDeletes() ;

  virtual void testDropFullySoftDeletedSegment() ;

  virtual void testSoftDeleteWhileMergeSurvives() ;

  /*
   * This test is trying to hard-delete a particular document while the segment
   * is merged which is already soft-deleted This requires special logic inside
   * IndexWriter#carryOverHardDeletes since docMaps are not created for this
   * document.
   */
  virtual void testDeleteDocWhileMergeThatIsSoftDeleted() ;

  virtual void testUndeleteDocument() ;

  static void doUpdate(std::shared_ptr<Term> doc,
                       std::shared_ptr<IndexWriter> writer,
                       std::deque<Field> &fields) ;

private:
  class NoDeletesSubReaderWrapper final
      : public FilterDirectoryReader::SubReaderWrapper
  {
    GET_CLASS_NAME(NoDeletesSubReaderWrapper)

  public:
    std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) override;

  private:
    class FilterLeafReaderAnonymousInnerClass2 : public FilterLeafReader
    {
      GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass2)
    private:
      std::shared_ptr<NoDeletesSubReaderWrapper> outerInstance;

    public:
      FilterLeafReaderAnonymousInnerClass2(
          std::shared_ptr<NoDeletesSubReaderWrapper> outerInstance,
          std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

      int numDocs() override;

      std::shared_ptr<Bits> getLiveDocs() override;

      std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

      std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    protected:
      std::shared_ptr<FilterLeafReaderAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass2>(
            FilterLeafReader::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<NoDeletesSubReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<NoDeletesSubReaderWrapper>(
          FilterDirectoryReader.SubReaderWrapper::shared_from_this());
    }
  };

private:
  class NoDeletesWrapper final : public FilterDirectoryReader
  {
    GET_CLASS_NAME(NoDeletesWrapper)

  public:
    NoDeletesWrapper(std::shared_ptr<DirectoryReader> in_) ;

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<NoDeletesWrapper> shared_from_this()
    {
      return std::static_pointer_cast<NoDeletesWrapper>(
          FilterDirectoryReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSoftDeletesRetentionMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestSoftDeletesRetentionMergePolicy>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
