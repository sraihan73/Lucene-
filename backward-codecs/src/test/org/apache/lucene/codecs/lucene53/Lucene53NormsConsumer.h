#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsConsumer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
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
class NormsProducer;
}
namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::codecs::lucene53
{

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene53.Lucene53NormsFormat.VERSION_CURRENT;

/**
 * Writer for {@link Lucene53NormsFormat}
 */
class Lucene53NormsConsumer : public NormsConsumer
{
  GET_CLASS_NAME(Lucene53NormsConsumer)
public:
  std::shared_ptr<IndexOutput> data, meta;
  const int maxDoc;

  Lucene53NormsConsumer(std::shared_ptr<SegmentWriteState> state,
                        const std::wstring &dataCodec,
                        const std::wstring &dataExtension,
                        const std::wstring &metaCodec,
                        const std::wstring &metaExtension) ;

  void addNormsField(
      std::shared_ptr<FieldInfo> field,
      std::shared_ptr<NormsProducer> normsProducer)  override;

private:
  void addNormsField(
      std::shared_ptr<FieldInfo> field,
      std::deque<std::shared_ptr<Number>> &values) ;

  void addConstant(int64_t constant) ;

  void
  addByte1(std::deque<std::shared_ptr<Number>> &values) ;

  void
  addByte2(std::deque<std::shared_ptr<Number>> &values) ;

  void
  addByte4(std::deque<std::shared_ptr<Number>> &values) ;

  void
  addByte8(std::deque<std::shared_ptr<Number>> &values) ;

public:
  virtual ~Lucene53NormsConsumer();

protected:
  std::shared_ptr<Lucene53NormsConsumer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene53NormsConsumer>(
        org.apache.lucene.codecs.NormsConsumer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene53
