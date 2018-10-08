#pragma once
#include "BinaryDictionary.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::dict
{
class TokenInfoFST;
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
namespace org::apache::lucene::analysis::ja::dict
{

/**
 * Binary dictionary implementation for a known-word dictionary model:
 * Words are encoded into an FST mapping to a deque of wordIDs.
 */
class TokenInfoDictionary final : public BinaryDictionary
{
  GET_CLASS_NAME(TokenInfoDictionary)

public:
  static const std::wstring FST_FILENAME_SUFFIX;

private:
  const std::shared_ptr<TokenInfoFST> fst;

  TokenInfoDictionary() ;

public:
  std::shared_ptr<TokenInfoFST> getFST();

  static std::shared_ptr<TokenInfoDictionary> getInstance();

private:
  class SingletonHolder : public std::enable_shared_from_this<SingletonHolder>
  {
    GET_CLASS_NAME(SingletonHolder)
  public:
    static const std::shared_ptr<TokenInfoDictionary> INSTANCE;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static SingletonHolder::StaticConstructor staticConstructor;
  };

protected:
  std::shared_ptr<TokenInfoDictionary> shared_from_this()
  {
    return std::static_pointer_cast<TokenInfoDictionary>(
        BinaryDictionary::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja::dict
