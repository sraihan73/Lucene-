#pragma once
#include "Codec.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldInfosFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/LiveDocsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/CompoundFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsFormat.h"

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

/**
 * A codec that forwards all its method calls to another codec.
 * <p>
 * Extend this class when you need to reuse the functionality of an existing
 * codec. For example, if you want to build a codec that redefines LuceneMN's
 * {@link LiveDocsFormat}:
 * <pre class="prettyprint">
 *   public final class CustomCodec extends FilterCodec {
GET_CLASS_NAME(="prettyprint">)
 *
 *     public CustomCodec() {
 *       super("CustomCodec", new LuceneMNCodec());
 *     }
 *
 *     public LiveDocsFormat liveDocsFormat() {
 *       return new CustomLiveDocsFormat();
 *     }
 *
 *   }
 * </pre>
 *
 * <p><em>Please note:</em> Don't call {@link Codec#forName} from
 * the no-arg constructor of your own codec. When the SPI framework
 * loads your own Codec as SPI component, SPI has not yet fully initialized!
 * If you want to extend another Codec, instantiate it directly by calling
 * its constructor.
 *
 * @lucene.experimental
 */
class FilterCodec : public Codec
{
  GET_CLASS_NAME(FilterCodec)

  /** The codec to filter. */
protected:
  const std::shared_ptr<Codec> delegate_;

  /** Sole constructor. When subclassing this codec,
   * create a no-arg ctor and pass the delegate codec
   * and a unique name to this ctor.
   */
  FilterCodec(const std::wstring &name, std::shared_ptr<Codec> delegate_);

public:
  std::shared_ptr<DocValuesFormat> docValuesFormat() override;

  std::shared_ptr<FieldInfosFormat> fieldInfosFormat() override;

  std::shared_ptr<LiveDocsFormat> liveDocsFormat() override;

  std::shared_ptr<NormsFormat> normsFormat() override;

  std::shared_ptr<PostingsFormat> postingsFormat() override;

  std::shared_ptr<SegmentInfoFormat> segmentInfoFormat() override;

  std::shared_ptr<StoredFieldsFormat> storedFieldsFormat() override;

  std::shared_ptr<TermVectorsFormat> termVectorsFormat() override;

  std::shared_ptr<CompoundFormat> compoundFormat() override;

  std::shared_ptr<PointsFormat> pointsFormat() override;

protected:
  std::shared_ptr<FilterCodec> shared_from_this()
  {
    return std::static_pointer_cast<FilterCodec>(Codec::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/
