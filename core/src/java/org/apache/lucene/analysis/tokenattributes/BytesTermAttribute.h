#pragma once
#include "TermToBytesRefAttribute.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::analysis::tokenattributes
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * This attribute can be used if you have the raw term bytes to be indexed.
 * It can be used as replacement for {@link CharTermAttribute}, if binary
 * terms should be indexed.
 * @lucene.internal
 */
class BytesTermAttribute : public TermToBytesRefAttribute
{
  GET_CLASS_NAME(BytesTermAttribute)
  /** Sets the {@link BytesRef} of the term */
public:
  virtual void setBytesRef(std::shared_ptr<BytesRef> bytes) = 0;
};
} // namespace org::apache::lucene::analysis::tokenattributes
