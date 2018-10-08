#pragma once
#include "../index/PointValues.h"
#include "PointsReader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PointValues;
}

namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class IntersectVisitor;
}
namespace org::apache::lucene::index
{
class DocMap;
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
namespace org::apache::lucene::codecs
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;

/** Abstract API to write points
 *
 * @lucene.experimental
 */

class PointsWriter : public std::enable_shared_from_this<PointsWriter>
{
  GET_CLASS_NAME(PointsWriter)
  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  PointsWriter();

  /** Write all values contained in the provided reader */
public:
  virtual void writeField(std::shared_ptr<FieldInfo> fieldInfo,
                          std::shared_ptr<PointsReader> values) = 0;

  /** Default naive merge implementation for one field: it just re-indexes all
   * the values from the incoming segment.  The default codec overrides this for
   * 1D fields and uses a faster but more complex implementation. */
protected:
  virtual void
  mergeOneField(std::shared_ptr<MergeState> mergeState,
                std::shared_ptr<FieldInfo> fieldInfo) ;

private:
  class PointsReaderAnonymousInnerClass : public PointsReader
  {
    GET_CLASS_NAME(PointsReaderAnonymousInnerClass)
  private:
    std::shared_ptr<PointsWriter> outerInstance;

    std::shared_ptr<MergeState> mergeState;
    std::shared_ptr<FieldInfo> fieldInfo;
    int64_t finalMaxPointCount = 0;
    int finalDocCount = 0;

  public:
    PointsReaderAnonymousInnerClass(std::shared_ptr<PointsWriter> outerInstance,
                                    std::shared_ptr<MergeState> mergeState,
                                    std::shared_ptr<FieldInfo> fieldInfo,
                                    int64_t finalMaxPointCount,
                                    int finalDocCount);

    int64_t ramBytesUsed() override;

    virtual ~PointsReaderAnonymousInnerClass();
    std::shared_ptr<PointValues>
    getValues(const std::wstring &fieldName) override;

  private:
    class PointValuesAnonymousInnerClass : public PointValues
    {
      GET_CLASS_NAME(PointValuesAnonymousInnerClass)
    private:
      std::shared_ptr<PointsReaderAnonymousInnerClass> outerInstance;

      std::wstring fieldName;

    public:
      PointValuesAnonymousInnerClass(
          std::shared_ptr<PointsReaderAnonymousInnerClass> outerInstance,
          const std::wstring &fieldName);

      void intersect(std::shared_ptr<PointValues::IntersectVisitor>
                         mergedVisitor)  override;

    private:
      class IntersectVisitorAnonymousInnerClass
          : public std::enable_shared_from_this<
                IntersectVisitorAnonymousInnerClass>,
            public IntersectVisitor
      {
        GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
      private:
        std::shared_ptr<PointValuesAnonymousInnerClass> outerInstance;

        std::shared_ptr<PointValues::IntersectVisitor> mergedVisitor;
        std::shared_ptr<MergeState::DocMap> docMap;

      public:
        IntersectVisitorAnonymousInnerClass(
            std::shared_ptr<PointValuesAnonymousInnerClass> outerInstance,
            std::shared_ptr<PointValues::IntersectVisitor> mergedVisitor,
            std::shared_ptr<MergeState::DocMap> docMap);

        void visit(int docID) override;

        void visit(int docID,
                   std::deque<char> &packedValue)  override;

        Relation compare(std::deque<char> &minPackedValue,
                         std::deque<char> &maxPackedValue) override;
      };

    public:
      int64_t estimatePointCount(
          std::shared_ptr<PointValues::IntersectVisitor> visitor) override;

      std::deque<char> getMinPackedValue() override;

      std::deque<char> getMaxPackedValue() override;

      int getNumDimensions() override;

      int getBytesPerDimension() override;

      int64_t size() override;

      int getDocCount() override;

    protected:
      std::shared_ptr<PointValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PointValuesAnonymousInnerClass>(
            org.apache.lucene.index.PointValues::shared_from_this());
      }
    };

  public:
    void checkIntegrity()  override;

  protected:
    std::shared_ptr<PointsReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointsReaderAnonymousInnerClass>(
          PointsReader::shared_from_this());
    }
  };

  /** Default merge implementation to merge incoming points readers by visiting
   * all their points and adding to this writer */
public:
  virtual void merge(std::shared_ptr<MergeState> mergeState) ;

  /** Called once at the end before close */
  virtual void finish() = 0;
};

} // namespace org::apache::lucene::codecs
