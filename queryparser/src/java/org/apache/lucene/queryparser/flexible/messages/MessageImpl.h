#pragma once
#include "stringbuilder.h"
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
 * Default implementation of Message interface.
 * For Native Language Support (NLS), system of software internationalization.
 */
class MessageImpl : public std::enable_shared_from_this<MessageImpl>,
                    public Message
{
  GET_CLASS_NAME(MessageImpl)

private:
  std::wstring key;

  std::deque<std::any> arguments = std::deque<std::any>(0);

public:
  MessageImpl(const std::wstring &key);

  MessageImpl(const std::wstring &key, std::deque<std::any> &args);

  std::deque<std::any> getArguments() override;

  std::wstring getKey() override;

  std::wstring getLocalizedMessage() override;

  std::wstring getLocalizedMessage(std::shared_ptr<Locale> locale) override;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::queryparser::flexible::messages
