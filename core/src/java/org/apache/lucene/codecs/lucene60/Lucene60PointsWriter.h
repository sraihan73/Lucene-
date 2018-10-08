#pragma once
#include "../../index/PointValues.h"
#include "../PointsWriter.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util::bkd
{
class BKDWriter;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class MergeState;
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
namespace org::apache::lucene::codecs::lucene60
{

using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/** Writes dimensional values */
class Lucene60PointsWriter : public PointsWriter
{
  GET_CLASS_NAME(Lucene60PointsWriter)

  /** Output used to write the BKD tree data file */
protected:
  const std::shared_ptr<IndexOutput> dataOut;

  /** Maps field name to file pointer in the data file where the BKD index is
   * located. */
  const std::unordered_map<std::wstring, int64_t> indexFPs =
      std::unordered_map<std::wstring, int64_t>();

public:
  const std::shared_ptr<SegmentWriteState> writeState;
  const int maxPointsInLeafNode;
  const double maxMBSortInHeap;

private:
  bool finished = false;

  /** Full constructor */
public:
  Lucene60PointsWriter(std::shared_ptr<SegmentWriteState> writeState,
                       int maxPointsInLeafNode,
                       double maxMBSortInHeap) ;

  /** Uses the defaults values for {@code maxPointsInLeafNode} (1024) and {@code
   * maxMBSortInHeap} (16.0) */
  Lucene60PointsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
      IOException);

  void
  writeField(std::shared_ptr<FieldInfo> fieldInfo,
             std::shared_ptr<PointsReader> reader)  override;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene60PointsWriter> outerInstance;

    std::shared_ptr<BKDWriter> writer;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<Lucene60PointsWriter> outerInstance,
        std::shared_ptr<BKDWriter> writer);

    void visit(int docID) override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

public:
  void
  merge(std::shared_ptr<MergeState> mergeState)  override;

  void finish()  override;

  virtual ~Lucene60PointsWriter();

protected:
  std::shared_ptr<Lucene60PointsWriter> shared_from_this()
  {
    return std::static_pointer_cast<Lucene60PointsWriter>(
        org.apache.lucene.codecs.PointsWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene60
