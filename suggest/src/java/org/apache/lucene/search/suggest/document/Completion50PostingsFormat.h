#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
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

using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;

/**
 * {@link org.apache.lucene.search.suggest.document.CompletionPostingsFormat}
 * for {@link org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat}
 *
 * @lucene.experimental
 */
class Completion50PostingsFormat : public CompletionPostingsFormat
{
  GET_CLASS_NAME(Completion50PostingsFormat)

  /**
   * Sole Constructor
   */
public:
  Completion50PostingsFormat();

protected:
  std::shared_ptr<PostingsFormat> delegatePostingsFormat() override;

protected:
  std::shared_ptr<Completion50PostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<Completion50PostingsFormat>(
        CompletionPostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
