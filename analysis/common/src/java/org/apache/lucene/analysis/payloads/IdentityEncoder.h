#pragma once
#include "AbstractEncoder.h"
#include "PayloadEncoder.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::analysis::payloads
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 *  Does nothing other than convert the char array to a byte array using the
 *specified encoding.
 *
 **/
class IdentityEncoder : public AbstractEncoder, public PayloadEncoder
{
  GET_CLASS_NAME(IdentityEncoder)
protected:
  std::shared_ptr<Charset> charset = StandardCharsets::UTF_8;

public:
  IdentityEncoder();

  IdentityEncoder(std::shared_ptr<Charset> charset);

  std::shared_ptr<BytesRef> encode(std::deque<wchar_t> &buffer, int offset,
                                   int length) override;

protected:
  std::shared_ptr<IdentityEncoder> shared_from_this()
  {
    return std::static_pointer_cast<IdentityEncoder>(
        AbstractEncoder::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/payloads/
