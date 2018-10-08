#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
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
namespace org::apache::lucene::queries::payloads
{

/**
 * Returns the maximum payload score seen, else 1 if there are no payloads on
 *the doc. <p> Is thread safe and completely reusable.
 *
 **/
class MaxPayloadFunction : public PayloadFunction
{
  GET_CLASS_NAME(MaxPayloadFunction)
public:
  float currentScore(int docId, const std::wstring &field, int start, int end,
                     int numPayloadsSeen, float currentScore,
                     float currentPayloadScore) override;

  float docScore(int docId, const std::wstring &field, int numPayloadsSeen,
                 float payloadScore) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

protected:
  std::shared_ptr<MaxPayloadFunction> shared_from_this()
  {
    return std::static_pointer_cast<MaxPayloadFunction>(
        PayloadFunction::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::payloads
