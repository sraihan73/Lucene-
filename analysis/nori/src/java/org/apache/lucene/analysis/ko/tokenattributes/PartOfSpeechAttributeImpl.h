#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include "../POS.h"
#include "PartOfSpeechAttribute.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/Token.h"

#include  "core/src/java/org/apache/lucene/analysis/ko/POS.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/dict/Dictionary.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/dict/Morpheme.h"
#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

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
namespace org::apache::lucene::analysis::ko::tokenattributes
{

using Type = org::apache::lucene::analysis::ko::POS::Type;
using Tag = org::apache::lucene::analysis::ko::POS::Tag;
using Token = org::apache::lucene::analysis::ko::Token;
using Morpheme = org::apache::lucene::analysis::ko::dict::Dictionary::Morpheme;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

/**
 * Part of Speech attributes for Korean.
 * @lucene.experimental
 */
class PartOfSpeechAttributeImpl : public AttributeImpl,
                                  public PartOfSpeechAttribute,
                                  public Cloneable
{
  GET_CLASS_NAME(PartOfSpeechAttributeImpl)
private:
  std::shared_ptr<Token> token;

public:
  Type getPOSType() override;

  Tag getLeftPOS() override;

  Tag getRightPOS() override;

  std::deque<std::shared_ptr<Morpheme>> getMorphemes() override;

  void setToken(std::shared_ptr<Token> token) override;

  void clear() override;

  void reflectWith(AttributeReflector reflector) override;

private:
  std::wstring
  displayMorphemes(std::deque<std::shared_ptr<Morpheme>> &morphemes);

public:
  void copyTo(std::shared_ptr<AttributeImpl> target) override;

protected:
  std::shared_ptr<PartOfSpeechAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<PartOfSpeechAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/tokenattributes/
