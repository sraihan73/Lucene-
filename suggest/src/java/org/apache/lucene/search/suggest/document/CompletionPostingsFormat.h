#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
}

namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
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
namespace org::apache::lucene::search::suggest::document
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * <p>
 * A {@link PostingsFormat} which supports document suggestion based on
 * indexed {@link SuggestField}s.
 * Document suggestion is based on an weighted FST which map_obj analyzed
 * terms of a {@link SuggestField} to its surface form and document id.
 * </p>
 * <p>
 * Files:
 * <ul>
 *   <li><tt>.lkp</tt>: <a href="#Completiondictionary">Completion
 * Dictionary</a></li> <li><tt>.cmp</tt>: <a href="#Completionindex">Completion
 * Index</a></li>
 * </ul>
 * <p>
 * <a name="Completionictionary"></a>
 * <h3>Completion Dictionary</h3>
 * <p>The .lkp file contains an FST for each suggest field
 * </p>
 * <ul>
 *   <li>CompletionDict (.lkp) --&gt; Header, FST<sup>NumSuggestFields</sup>,
 * Footer</li> <li>Header --&gt; {@link CodecUtil#writeHeader CodecHeader}</li>
 *   <!-- TODO: should the FST output be mentioned at all? -->
 *   <li>FST --&gt; {@link FST FST&lt;Long, BytesRef&gt;}</li>
 *   <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes:</p>
 * <ul>
 *   <li>Header is a {@link CodecUtil#writeHeader CodecHeader} storing the
 * version information for the Completion implementation.</li> <li>FST maps all
 * analyzed forms to surface forms of a SuggestField</li>
 * </ul>
 * <a name="Completionindex"></a>
 * <h3>Completion Index</h3>
 * <p>The .cmp file contains an index into the completion dictionary, so that it
 * can be accessed randomly.</p> <ul> <li>CompletionIndex (.cmp) --&gt; Header,
 * NumSuggestFields, Entry<sup>NumSuggestFields</sup>, Footer</li> <li>Header
 * --&gt; {@link CodecUtil#writeHeader CodecHeader}</li> <li>NumSuggestFields
 * --&gt; {@link DataOutput#writeVInt Uint32}</li> <li>Entry --&gt; FieldNumber,
 * CompletionDictionaryOffset, MinWeight, MaxWeight, Type</li> <li>FieldNumber
 * --&gt; {@link DataOutput#writeVInt Uint32}</li>
 *   <li>CompletionDictionaryOffset --&gt; {@link DataOutput#writeVLong
 * Uint64}</li> <li>MinWeight --&gt; {@link DataOutput#writeVLong  Uint64}</li>
 *   <li>MaxWeight --&gt; {@link DataOutput#writeVLong  Uint64}</li>
 *   <li>Type --&gt; {@link DataOutput#writeByte  Byte}</li>
 *   <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes:</p>
 * <ul>
 *   <li>Header is a {@link CodecUtil#writeHeader CodecHeader} storing the
 * version information for the Completion implementation.</li>
 *   <li>NumSuggestFields is the number of suggest fields indexed</li>
 *   <li>FieldNumber is the fields number from {@link FieldInfos}. (.fnm)</li>
 *   <li>CompletionDictionaryOffset is the file offset of a field's FST in
 * CompletionDictionary (.lkp)</li> <li>MinWeight and MaxWeight are the global
 * minimum and maximum weight for the field</li> <li>Type indicates if the
 * suggester has context or not</li>
 * </ul>
 *
 * @lucene.experimental
 */
class CompletionPostingsFormat : public PostingsFormat
{
  GET_CLASS_NAME(CompletionPostingsFormat)

public:
  static const std::wstring CODEC_NAME;
  static constexpr int COMPLETION_CODEC_VERSION = 1;
  static constexpr int COMPLETION_VERSION_CURRENT = COMPLETION_CODEC_VERSION;
  static const std::wstring INDEX_EXTENSION;
  static const std::wstring DICT_EXTENSION;

  /**
   * Used only by core Lucene at read-time via Service Provider instantiation
   */
  CompletionPostingsFormat();

  /**
   * Concrete implementation should specify the delegating postings format
   */
protected:
  virtual std::shared_ptr<PostingsFormat> delegatePostingsFormat() = 0;

public:
  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

protected:
  std::shared_ptr<CompletionPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CompletionPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
