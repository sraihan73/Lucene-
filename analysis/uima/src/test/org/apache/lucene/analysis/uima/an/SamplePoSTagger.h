#pragma once
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
namespace org::apache::lucene::analysis::uima::an
{

using org::apache::uima::analysis_component::JCasAnnotator_ImplBase;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::jcas::JCas;

/**
 * Dummy implementation of a PoS tagger to add part of speech as token types
 */
class SamplePoSTagger : public JCasAnnotator_ImplBase
{
  GET_CLASS_NAME(SamplePoSTagger)

private:
  static const std::wstring NUM;
  static const std::wstring WORD;
  static const std::wstring TYPE_NAME;
  static const std::wstring FEATURE_NAME;

public:
  void process(std::shared_ptr<JCas> jcas) throw(
      AnalysisEngineProcessException) override;

private:
  std::wstring extractPoS(const std::wstring &text);

protected:
  std::shared_ptr<SamplePoSTagger> shared_from_this()
  {
    return std::static_pointer_cast<SamplePoSTagger>(
        org.apache.uima.analysis_component
            .JCasAnnotator_ImplBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/uima/an/
