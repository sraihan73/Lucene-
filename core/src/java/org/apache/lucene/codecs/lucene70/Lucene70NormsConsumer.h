#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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
namespace org::apache::lucene::codecs::lucene70
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70NormsFormat.VERSION_CURRENT;

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/**
 * Writer for {@link Lucene70NormsFormat}
 */
class Lucene70NormsConsumer final : public NormsConsumer
{
  GET_CLASS_NAME(Lucene70NormsConsumer)
public:
  std::shared_ptr<IndexOutput> data, meta;
  const int maxDoc;

  Lucene70NormsConsumer(std::shared_ptr<SegmentWriteState> state,
                        const std::wstring &dataCodec,
                        const std::wstring &dataExtension,
                        const std::wstring &metaCodec,
                        const std::wstring &metaExtension) ;

  virtual ~Lucene70NormsConsumer();

  void addNormsField(
      std::shared_ptr<FieldInfo> field,
      std::shared_ptr<NormsProducer> normsProducer)  override;

private:
  int numBytesPerValue(int64_t min, int64_t max);

  void writeValues(std::shared_ptr<NumericDocValues> values,
                   int numBytesPerValue,
                   std::shared_ptr<IndexOutput> out) throw(IOException,
                                                           AssertionError);

protected:
  std::shared_ptr<Lucene70NormsConsumer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene70NormsConsumer>(
        org.apache.lucene.codecs.NormsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene70/
