#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::memory
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

/** In-memory docvalues format that does no (or very little)
 *  compression.  Indexed values are stored on disk, but
 *  then at search time all values are loaded into memory as
 *  simple java arrays.  For numeric values, it uses
 *  byte[], short[], int[], long[] as necessary to fit the
 *  range of the values.  For binary values, there is an int
 *  (4 bytes) overhead per value.
 *
 *  <p>Limitations:
 *  <ul>
 *    <li>For binary and sorted fields the total space
 *        required for all binary values cannot exceed about
 *        2.1 GB (see #MAX_TOTAL_BYTES_LENGTH).</li>
 *
 *    <li>For sorted set fields, the sum of the size of each
 *        document's set of values cannot exceed about 2.1 B
 *        values (see #MAX_SORTED_SET_ORDS).  For example,
 *        if every document has 10 values (10 instances of
 *        {@link SortedSetDocValuesField}) added, then no
 *        more than ~210 M documents can be added to one
 *        segment. </li>
 *  </ul> */

class DirectDocValuesFormat : public DocValuesFormat
{
  GET_CLASS_NAME(DirectDocValuesFormat)

  /** The sum of all byte lengths for binary field, or for
   *  the unique values in sorted or sorted set fields, cannot
   *  exceed this. */
public:
  static const int MAX_TOTAL_BYTES_LENGTH = ArrayUtil::MAX_ARRAY_LENGTH;

  /** The sum of the number of values across all documents
   *  in a sorted set field cannot exceed this. */
  static const int MAX_SORTED_SET_ORDS = ArrayUtil::MAX_ARRAY_LENGTH;

  /** Sole constructor. */
  DirectDocValuesFormat();

  std::shared_ptr<DocValuesConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<DocValuesProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  static const std::wstring DATA_CODEC;
  static const std::wstring DATA_EXTENSION;
  static const std::wstring METADATA_CODEC;
  static const std::wstring METADATA_EXTENSION;

protected:
  std::shared_ptr<DirectDocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<DirectDocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
