#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataOutput;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class IntersectVisitor;
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
namespace org::apache::lucene::util::bkd
{

using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;

class DocIdsWriter : public std::enable_shared_from_this<DocIdsWriter>
{
  GET_CLASS_NAME(DocIdsWriter)

private:
  DocIdsWriter();

public:
  static void writeDocIds(std::deque<int> &docIds, int start, int count,
                          std::shared_ptr<DataOutput> out) ;

  /** Read {@code count} integers into {@code docIDs}. */
  static void readInts(std::shared_ptr<IndexInput> in_, int count,
                       std::deque<int> &docIDs) ;

private:
  static void readDeltaVInts(std::shared_ptr<IndexInput> in_, int count,
                             std::deque<int> &docIDs) ;

public:
  template <typename T>
  static void readInts32(std::shared_ptr<IndexInput> in_, int count,
                         std::deque<int> &docIDs) ;

private:
  static void readInts24(std::shared_ptr<IndexInput> in_, int count,
                         std::deque<int> &docIDs) ;

  /** Read {@code count} integers and feed the result directly to {@link
   * IntersectVisitor#visit(int)}. */
public:
  static void
  readInts(std::shared_ptr<IndexInput> in_, int count,
           std::shared_ptr<IntersectVisitor> visitor) ;

private:
  static void
  readDeltaVInts(std::shared_ptr<IndexInput> in_, int count,
                 std::shared_ptr<IntersectVisitor> visitor) ;

public:
  static void
  readInts32(std::shared_ptr<IndexInput> in_, int count,
             std::shared_ptr<IntersectVisitor> visitor) ;

private:
  static void
  readInts24(std::shared_ptr<IndexInput> in_, int count,
             std::shared_ptr<IntersectVisitor> visitor) ;
};

} // namespace org::apache::lucene::util::bkd