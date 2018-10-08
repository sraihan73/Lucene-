#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
}

namespace org::apache::lucene::index
{
class TermsHash;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::index
{
class TermsHashPerField;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
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
namespace org::apache::lucene::index
{

class FreqProxTermsWriter final : public TermsHash
{
  GET_CLASS_NAME(FreqProxTermsWriter)

public:
  FreqProxTermsWriter(std::shared_ptr<DocumentsWriterPerThread> docWriter,
                      std::shared_ptr<TermsHash> termVectors);

private:
  void applyDeletes(std::shared_ptr<SegmentWriteState> state,
                    std::shared_ptr<Fields> fields) ;

public:
  void
  flush(std::unordered_map<std::wstring, std::shared_ptr<TermsHashPerField>>
            &fieldsToFlush,
        std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap)  override;

  std::shared_ptr<TermsHashPerField>
  addField(std::shared_ptr<FieldInvertState> invertState,
           std::shared_ptr<FieldInfo> fieldInfo) override;

protected:
  std::shared_ptr<FreqProxTermsWriter> shared_from_this()
  {
    return std::static_pointer_cast<FreqProxTermsWriter>(
        TermsHash::shared_from_this());
  }
};

} // namespace org::apache::lucene::index