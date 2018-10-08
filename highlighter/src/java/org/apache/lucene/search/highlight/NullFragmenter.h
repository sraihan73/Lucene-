#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * {@link Fragmenter} implementation which does not fragment the text.
 * This is useful for highlighting the entire content of a document or field.
 */
class NullFragmenter : public std::enable_shared_from_this<NullFragmenter>,
                       public Fragmenter
{
  GET_CLASS_NAME(NullFragmenter)
public:
  void start(const std::wstring &s,
             std::shared_ptr<TokenStream> tokenStream) override;

  bool isNewFragment() override;
};

} // namespace org::apache::lucene::search::highlight
