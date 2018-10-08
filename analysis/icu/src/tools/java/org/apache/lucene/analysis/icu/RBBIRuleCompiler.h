#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::icu
{

/**
 * Command-line utility to converts RuleBasedBreakIterator (.rbbi) files into
 * binary compiled form (.brk).
 */
class RBBIRuleCompiler : public std::enable_shared_from_this<RBBIRuleCompiler>
{
  GET_CLASS_NAME(RBBIRuleCompiler)

public:
  static std::wstring
  getRules(std::shared_ptr<File> ruleFile) ;

  static void compile(std::shared_ptr<File> srcDir,
                      std::shared_ptr<File> destDir) ;

private:
  class FilenameFilterAnonymousInnerClass : public FilenameFilter
  {
    GET_CLASS_NAME(FilenameFilterAnonymousInnerClass)
  public:
    FilenameFilterAnonymousInnerClass();

    bool accept(std::shared_ptr<File> dir, const std::wstring &name);

  protected:
    std::shared_ptr<FilenameFilterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilenameFilterAnonymousInnerClass>(
          java.io.FilenameFilter::shared_from_this());
    }
  };

  static void main(std::deque<std::wstring> &args) ;
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/
