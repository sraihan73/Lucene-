#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
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
namespace org::apache::lucene::analysis
{

/** Internal class to enable reuse of the string reader by {@link
 * Analyzer#tokenStream(std::wstring,std::wstring)} */
class ReusableStringReader final : public Reader
{
private:
  int pos = 0, size = 0;
  std::wstring s = L"";

public:
  void setValue(const std::wstring &s);

  int read() override;

  int read(std::deque<wchar_t> &c, int off, int len) override;

  virtual ~ReusableStringReader();

protected:
  std::shared_ptr<ReusableStringReader> shared_from_this()
  {
    return std::static_pointer_cast<ReusableStringReader>(
        java.io.Reader::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
