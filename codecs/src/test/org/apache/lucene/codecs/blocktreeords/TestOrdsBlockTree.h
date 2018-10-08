#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/BasePostingsFormatTestCase.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/codecs/blocktreeords/BlockTreeOrdsPostingsFormat.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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
namespace org::apache::lucene::codecs::blocktreeords
{

using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TestUtil = org::apache::lucene::util::TestUtil;

class TestOrdsBlockTree : public BasePostingsFormatTestCase
{
  GET_CLASS_NAME(TestOrdsBlockTree)
private:
  const std::shared_ptr<Codec> codec = TestUtil::alwaysPostingsFormat(
      std::make_shared<BlockTreeOrdsPostingsFormat>());

protected:
  std::shared_ptr<Codec> getCodec() override;

public:
  virtual void testBasic() ;

  virtual void testTwoBlocks() ;

  virtual void testThreeBlocks() ;

private:
  void testEnum(std::shared_ptr<TermsEnum> te,
                std::deque<std::wstring> &terms) ;

public:
  virtual void testFloorBlocks() ;

  virtual void testNonRootFloorBlocks() ;

  virtual void testSeveralNonRootBlocks() ;

  virtual void testSeekCeilNotFound() ;

protected:
  std::shared_ptr<TestOrdsBlockTree> shared_from_this()
  {
    return std::static_pointer_cast<TestOrdsBlockTree>(
        org.apache.lucene.index.BasePostingsFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/blocktreeords/
