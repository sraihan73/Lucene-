#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/uhighlight/UnifiedHighlighter.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/OffsetsEnum.h"

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
namespace org::apache::lucene::search::uhighlight
{

using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Never returns offsets. Used when the query would highlight nothing.
 *
 * @lucene.internal
 */
class NoOpOffsetStrategy : public FieldOffsetStrategy
{
  GET_CLASS_NAME(NoOpOffsetStrategy)

public:
  static const std::shared_ptr<NoOpOffsetStrategy> INSTANCE;

private:
  NoOpOffsetStrategy();

public:
  UnifiedHighlighter::OffsetSource getOffsetSource() override;

  std::shared_ptr<OffsetsEnum>
  getOffsetsEnum(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &content)  override;

protected:
  std::shared_ptr<NoOpOffsetStrategy> shared_from_this()
  {
    return std::static_pointer_cast<NoOpOffsetStrategy>(
        FieldOffsetStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
