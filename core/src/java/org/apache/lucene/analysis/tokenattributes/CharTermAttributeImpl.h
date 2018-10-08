#pragma once
#include "../../util/ArrayUtil.h"
#include "../../util/AttributeImpl.h"
#include "CharTermAttribute.h"
#include "TermToBytesRefAttribute.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class AttributeReflector;
}
namespace org::apache::lucene::util
{
class AttributeImpl;
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

using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/** Default implementation of {@link CharTermAttribute}. */
class CharTermAttributeImpl : public AttributeImpl,
                              public CharTermAttribute,
                              public TermToBytesRefAttribute,
                              public Cloneable
{
  GET_CLASS_NAME(CharTermAttributeImpl)
private:
  static int MIN_BUFFER_SIZE;

  std::deque<wchar_t> termBuffer = std::deque<wchar_t>(
      ArrayUtil::oversize(MIN_BUFFER_SIZE, Character::BYTES));
  int termLength = 0;

  /** May be used by subclasses to convert to different charsets / encodings for
   * implementing {@link #getBytesRef()}. */
protected:
  std::shared_ptr<BytesRefBuilder> builder =
      std::make_shared<BytesRefBuilder>();

  /** Initialize this attribute with empty term text */
public:
  CharTermAttributeImpl();

  void copyBuffer(std::deque<wchar_t> &buffer, int offset,
                  int length) override;

  std::deque<wchar_t> buffer() override;

  std::deque<wchar_t> resizeBuffer(int newSize) override;

private:
  void growTermBuffer(int newSize);

public:
  std::shared_ptr<CharTermAttribute> setLength(int length) override;

  std::shared_ptr<CharTermAttribute> setEmpty() override;

  // *** TermToBytesRefAttribute interface ***

  std::shared_ptr<BytesRef> getBytesRef() override;

  // *** std::wstring interface ***
  int length() override;

  wchar_t charAt(int index) override;

  std::shared_ptr<std::wstring> subSequence(int const start,
                                            int const end) override;

  // *** Appendable interface ***

  std::shared_ptr<CharTermAttribute>
  append(std::shared_ptr<std::wstring> csq) override;

  std::shared_ptr<CharTermAttribute> append(std::shared_ptr<std::wstring> csq,
                                            int start, int end) override;

  std::shared_ptr<CharTermAttribute> append(wchar_t c) override;

  // *** For performance some convenience methods in addition to CSQ's ***

  std::shared_ptr<CharTermAttribute> append(const std::wstring &s) override;

  std::shared_ptr<CharTermAttribute>
  append(std::shared_ptr<StringBuilder> s) override;

  std::shared_ptr<CharTermAttribute>
  append(std::shared_ptr<CharTermAttribute> ta) override;

private:
  std::shared_ptr<CharTermAttribute> appendNull();

  // *** AttributeImpl ***

public:
  virtual int hashCode();

  void clear() override;

  std::shared_ptr<CharTermAttributeImpl> clone() override;

  bool equals(std::any other) override;

  /**
   * Returns solely the term text as specified by the
   * {@link std::wstring} interface.
   */
  virtual std::wstring toString();

  void reflectWith(AttributeReflector reflector) override;

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

protected:
  std::shared_ptr<CharTermAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<CharTermAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::tokenattributes
