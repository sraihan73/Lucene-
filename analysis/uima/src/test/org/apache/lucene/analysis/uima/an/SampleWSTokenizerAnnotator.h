#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::analysis::uima::an
{

using org::apache::uima::UimaContext;
using org::apache::uima::analysis_component::JCasAnnotator_ImplBase;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::jcas::JCas;
using org::apache::uima::resource::ResourceInitializationException;

/**
 * Dummy implementation of a UIMA based whitespace tokenizer
 */
class SampleWSTokenizerAnnotator : public JCasAnnotator_ImplBase
{
  GET_CLASS_NAME(SampleWSTokenizerAnnotator)

private:
  static const std::wstring TOKEN_TYPE;
  static const std::wstring SENTENCE_TYPE;
  std::wstring lineEnd;
  static const std::wstring WHITESPACE;

public:
  void initialize(std::shared_ptr<UimaContext> aContext) throw(
      ResourceInitializationException) override;

  void process(std::shared_ptr<JCas> jCas) throw(
      AnalysisEngineProcessException) override;

protected:
  std::shared_ptr<SampleWSTokenizerAnnotator> shared_from_this()
  {
    return std::static_pointer_cast<SampleWSTokenizerAnnotator>(
        org.apache.uima.analysis_component
            .JCasAnnotator_ImplBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::uima::an
