#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
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
namespace org::apache::lucene::codecs::cheapbastard
{

using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using TestUtil = org::apache::lucene::util::TestUtil;

/** Codec that tries to use as little ram as possible because he spent all his
 * money on beer */
// TODO: better name :)
// but if we named it "LowMemory" in codecs/ package, it would be irresistible
// like optimize()!
class CheapBastardCodec : public FilterCodec
{
  GET_CLASS_NAME(CheapBastardCodec)

  // TODO: would be better to have no terms index at all and bsearch a terms
  // dict
private:
  const std::shared_ptr<PostingsFormat> postings =
      TestUtil::getDefaultPostingsFormat(100, 200);

public:
  CheapBastardCodec();

  std::shared_ptr<PostingsFormat> postingsFormat() override;

protected:
  std::shared_ptr<CheapBastardCodec> shared_from_this()
  {
    return std::static_pointer_cast<CheapBastardCodec>(
        org.apache.lucene.codecs.FilterCodec::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cheapbastard
