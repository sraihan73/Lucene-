#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
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

using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

using namespace org::apache::lucene::codecs::simpletext;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextStoredFieldsWriter.*;

/**
 * reads plaintext stored fields
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextStoredFieldsReader : public StoredFieldsReader
{
  GET_CLASS_NAME(SimpleTextStoredFieldsReader)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(
          SimpleTextStoredFieldsReader::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(BytesRef::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(CharsRef::typeid);

  std::deque<int64_t> offsets; // docid -> offset in .fld file
  std::shared_ptr<IndexInput> in_;
  std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  std::shared_ptr<CharsRefBuilder> scratchUTF16 =
      std::make_shared<CharsRefBuilder>();
  const std::shared_ptr<FieldInfos> fieldInfos;

public:
  SimpleTextStoredFieldsReader(
      std::shared_ptr<Directory> directory, std::shared_ptr<SegmentInfo> si,
      std::shared_ptr<FieldInfos> fn,
      std::shared_ptr<IOContext> context) ;

  // used by clone
  SimpleTextStoredFieldsReader(std::deque<int64_t> &offsets,
                               std::shared_ptr<IndexInput> in_,
                               std::shared_ptr<FieldInfos> fieldInfos);

  // we don't actually write a .fdx-like index, instead we read the
  // stored fields file in entirety up-front and save the offsets
  // so we can seek to the documents later.
private:
  void readIndex(int size) ;

public:
  void visitDocument(int n, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override;

private:
  void
  readField(std::shared_ptr<BytesRef> type,
            std::shared_ptr<FieldInfo> fieldInfo,
            std::shared_ptr<StoredFieldVisitor> visitor) ;

public:
  std::shared_ptr<StoredFieldsReader> clone() override;

  virtual ~SimpleTextStoredFieldsReader();

private:
  void readLine() ;

  int parseIntAt(int offset);

  bool equalsAt(std::shared_ptr<BytesRef> a, std::shared_ptr<BytesRef> b,
                int bOffset);

public:
  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

protected:
  std::shared_ptr<SimpleTextStoredFieldsReader> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextStoredFieldsReader>(
        org.apache.lucene.codecs.StoredFieldsReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
