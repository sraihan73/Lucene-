#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::codecs::simpletext
{
class SimpleTextBKDReader;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class PointValues;
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
namespace org::apache::lucene::codecs::simpletext
{

using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_FP;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BYTES_PER_DIM;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.DOC_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.FIELD_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.FIELD_FP;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.FIELD_FP_NAME;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.INDEX_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_LEAF_POINTS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_VALUE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MIN_VALUE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.NUM_DIMS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.POINT_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_DIM;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_VALUE;

class SimpleTextPointsReader : public PointsReader
{
  GET_CLASS_NAME(SimpleTextPointsReader)

private:
  const std::shared_ptr<IndexInput> dataIn;

public:
  const std::shared_ptr<SegmentReadState> readState;
  const std::unordered_map<std::wstring, std::shared_ptr<SimpleTextBKDReader>>
      readers = std::unordered_map<std::wstring,
                                   std::shared_ptr<SimpleTextBKDReader>>();
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();

  SimpleTextPointsReader(std::shared_ptr<SegmentReadState> readState) throw(
      IOException);

private:
  std::shared_ptr<SimpleTextBKDReader>
  initReader(int64_t fp) ;

  void readLine(std::shared_ptr<IndexInput> in_) ;

  bool startsWith(std::shared_ptr<BytesRef> prefix);

  int parseInt(std::shared_ptr<BytesRef> prefix);

  int64_t parseLong(std::shared_ptr<BytesRef> prefix);

  std::wstring stripPrefix(std::shared_ptr<BytesRef> prefix);

public:
  std::shared_ptr<PointValues>
  getValues(const std::wstring &fieldName)  override;

  void checkIntegrity()  override;

  int64_t ramBytesUsed() override;

  virtual ~SimpleTextPointsReader();

  virtual std::wstring toString();

protected:
  std::shared_ptr<SimpleTextPointsReader> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextPointsReader>(
        org.apache.lucene.codecs.PointsReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
