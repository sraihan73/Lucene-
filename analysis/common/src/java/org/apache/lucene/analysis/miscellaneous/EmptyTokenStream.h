#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * An always exhausted token stream.
 */
class EmptyTokenStream final : public TokenStream
{
  GET_CLASS_NAME(EmptyTokenStream)

public:
  bool incrementToken() override final;

protected:
  std::shared_ptr<EmptyTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<EmptyTokenStream>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
