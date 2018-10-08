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
namespace org::apache::lucene::analysis::util
{

/**
 * A StringBuilder that allows one to access the array.
 */
class OpenStringBuilder
    : public std::enable_shared_from_this<OpenStringBuilder>,
      public Appendable,
      public std::wstring
{
  GET_CLASS_NAME(OpenStringBuilder)
protected:
  std::deque<wchar_t> buf;
  int len = 0;

public:
  OpenStringBuilder();

  OpenStringBuilder(int size);

  OpenStringBuilder(std::deque<wchar_t> &arr, int len);

  virtual void setLength(int len);

  virtual void set(std::deque<wchar_t> &arr, int end);

  virtual std::deque<wchar_t> getArray();
  virtual int size();
  int length() override;
  virtual int capacity();

  std::shared_ptr<Appendable>
  append(std::shared_ptr<std::wstring> csq) override;

  std::shared_ptr<Appendable> append(std::shared_ptr<std::wstring> csq,
                                     int start, int end) override;

  std::shared_ptr<Appendable> append(wchar_t c) override;

  wchar_t charAt(int index) override;

  virtual void setCharAt(int index, wchar_t ch);

  std::shared_ptr<std::wstring> subSequence(int start, int end) override;

  virtual void unsafeWrite(wchar_t b);

  virtual void unsafeWrite(int b);

  virtual void unsafeWrite(std::deque<wchar_t> &b, int off, int len);

protected:
  virtual void resize(int len);

public:
  virtual void reserve(int num);

  virtual void write(wchar_t b);

  virtual void write(int b);

  void write(std::deque<wchar_t> &b);

  virtual void write(std::deque<wchar_t> &b, int off, int len);

  void write(std::shared_ptr<OpenStringBuilder> arr);

  virtual void write(const std::wstring &s);

  virtual void flush();

  void reset();

  virtual std::deque<wchar_t> toCharArray();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
