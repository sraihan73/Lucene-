#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
namespace org::apache::lucene::analysis::ja::util
{

/**
 * Utility class for english translations of morphological data,
 * used only for debugging.
 */
class ToStringUtil : public std::enable_shared_from_this<ToStringUtil>
{
  GET_CLASS_NAME(ToStringUtil)
  // a translation map_obj for parts of speech, only used for reflectWith
private:
  static const std::unordered_map<std::wstring, std::wstring> posTranslations;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static ToStringUtil::StaticConstructor staticConstructor;

  /**
   * Get the english form of a POS tag
   */
public:
  static std::wstring getPOSTranslation(const std::wstring &s);

  // a translation map_obj for inflection types, only used for reflectWith
private:
  static const std::unordered_map<std::wstring, std::wstring>
      inflTypeTranslations;

  /**
   * Get the english form of inflection type
   */
public:
  static std::wstring getInflectionTypeTranslation(const std::wstring &s);

  // a translation map_obj for inflection forms, only used for reflectWith
private:
  static const std::unordered_map<std::wstring, std::wstring>
      inflFormTranslations;

  /**
   * Get the english form of inflected form
   */
public:
  static std::wstring getInflectedFormTranslation(const std::wstring &s);

  /**
   * Romanize katakana with modified hepburn
   */
  static std::wstring getRomanization(const std::wstring &s);

  /**
   * Romanize katakana with modified hepburn
   */
  // TODO: now that this is used by readingsfilter and not just for
  // debugging, fix this to really be a scheme that works best with IMEs
  static void
  getRomanization(std::shared_ptr<Appendable> builder,
                  std::shared_ptr<std::wstring> s) ;
};

} // namespace org::apache::lucene::analysis::ja::util
