#pragma once
#include "exceptionhelper.h"
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
namespace org::apache::lucene::queryparser::surround::parser
{

/** An efficient implementation of JavaCC's CharStream interface.  <p>Note that
 * this does not do line-number counting, but instead keeps track of the
 * character position of the token in the input, as required by Lucene's {@link
 * org.apache.lucene.analysis.tokenattributes.OffsetAttribute} API. */
class FastCharStream final
    : public std::enable_shared_from_this<FastCharStream>,
      public CharStream
{
  GET_CLASS_NAME(FastCharStream)
public:
  std::deque<wchar_t> buffer;

  int bufferLength = 0;   // end of valid chars
  int bufferPosition = 0; // next char to read

  int tokenStart = 0;  // offset in buffer
  int bufferStart = 0; // position in file of buffer

  std::shared_ptr<Reader> input; // source of chars

  /** Constructs from a Reader. */
  FastCharStream(std::shared_ptr<Reader> r);

  wchar_t readChar()  override;

private:
  void refill() ;

public:
  wchar_t BeginToken()  override;

  void backup(int amount) override;

  std::wstring GetImage() override;

  std::deque<wchar_t> GetSuffix(int len) override;

  void Done() override;

  int getColumn() override;
  int getLine() override;
  int getEndColumn() override;
  int getEndLine() override;
  int getBeginColumn() override;
  int getBeginLine() override;
};

} // namespace org::apache::lucene::queryparser::surround::parser
