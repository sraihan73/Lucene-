#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class TermStats;
}

namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::codecs::blockterms
{

using TermStats = org::apache::lucene::codecs::TermStats;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Base class for terms index implementations to plug
 * into {@link BlockTermsWriter}.
GET_CLASS_NAME(for)
 *
 * @see TermsIndexReaderBase
 * @lucene.experimental
 */
class TermsIndexWriterBase
    : public std::enable_shared_from_this<TermsIndexWriterBase>
{
  GET_CLASS_NAME(TermsIndexWriterBase)

  /**
   * Terms index API for a single field.
   */
public:
  class FieldWriter : public std::enable_shared_from_this<FieldWriter>
  {
    GET_CLASS_NAME(FieldWriter)
  private:
    std::shared_ptr<TermsIndexWriterBase> outerInstance;

  public:
    FieldWriter(std::shared_ptr<TermsIndexWriterBase> outerInstance);

    virtual bool checkIndexTerm(std::shared_ptr<BytesRef> text,
                                std::shared_ptr<TermStats> stats) = 0;
    virtual void add(std::shared_ptr<BytesRef> text,
                     std::shared_ptr<TermStats> stats,
                     int64_t termsFilePointer) = 0;
    virtual void finish(int64_t termsFilePointer) = 0;
  };

public:
  virtual std::shared_ptr<FieldWriter>
  addField(std::shared_ptr<FieldInfo> fieldInfo,
           int64_t termsFilePointer) = 0;
};

} // namespace org::apache::lucene::codecs::blockterms
