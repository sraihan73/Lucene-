#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::analysis::ko::dict
{

/**
 * n-gram connection cost data
 */
class ConnectionCosts final
    : public std::enable_shared_from_this<ConnectionCosts>
{
  GET_CLASS_NAME(ConnectionCosts)

public:
  static const std::wstring FILENAME_SUFFIX;
  static const std::wstring HEADER;
  static constexpr int VERSION = 1;

private:
  const std::shared_ptr<ByteBuffer> buffer;
  const int forwardSize;

  ConnectionCosts() ;

public:
  int get(int forwardId, int backwardId);

  static std::shared_ptr<ConnectionCosts> getInstance();

private:
  class SingletonHolder : public std::enable_shared_from_this<SingletonHolder>
  {
    GET_CLASS_NAME(SingletonHolder)
  public:
    static const std::shared_ptr<ConnectionCosts> INSTANCE;

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
};

} // namespace org::apache::lucene::analysis::ko::dict
