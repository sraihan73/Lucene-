#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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
namespace org::apache::lucene::analysis::hunspell
{

// many hunspell dictionaries use this encoding, yet java does not have it?!?!
class ISO8859_14Decoder final : public CharsetDecoder
{
  GET_CLASS_NAME(ISO8859_14Decoder)

public:
  static std::deque<wchar_t> const TABLE;

  ISO8859_14Decoder();

protected:
  std::shared_ptr<CoderResult>
  decodeLoop(std::shared_ptr<ByteBuffer> in_,
             std::shared_ptr<CharBuffer> out) override;

protected:
  std::shared_ptr<ISO8859_14Decoder> shared_from_this()
  {
    return std::static_pointer_cast<ISO8859_14Decoder>(
        java.nio.charset.CharsetDecoder::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/hunspell/
