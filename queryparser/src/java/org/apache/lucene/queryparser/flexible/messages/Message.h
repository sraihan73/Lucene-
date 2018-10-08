#pragma once
#include "stringhelper.h"
#include <any>
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
namespace org::apache::lucene::queryparser::flexible::messages
{

/**
 * Message Interface for a lazy loading.
 * For Native Language Support (NLS), system of software internationalization.
 */
class Message
{
  GET_CLASS_NAME(Message)

public:
  virtual std::wstring getKey() = 0;

  virtual std::deque<std::any> getArguments() = 0;

  virtual std::wstring getLocalizedMessage() = 0;

  virtual std::wstring getLocalizedMessage(std::shared_ptr<Locale> locale) = 0;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/messages/
