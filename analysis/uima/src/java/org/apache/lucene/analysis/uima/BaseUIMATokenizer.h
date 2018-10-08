#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeFactory;
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
namespace org::apache::lucene::analysis::uima
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using org::apache::uima::analysis_engine::AnalysisEngine;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::CAS;
using org::apache::uima::cas::FSIterator;
using org::apache::uima::cas::text::AnnotationFS;
using org::apache::uima::resource::ResourceInitializationException;

/**
 * Abstract base implementation of a {@link Tokenizer} which is able to analyze
 * the given input with a UIMA {@link AnalysisEngine}
 */
class BaseUIMATokenizer : public Tokenizer
{
  GET_CLASS_NAME(BaseUIMATokenizer)

protected:
  std::shared_ptr<FSIterator<std::shared_ptr<AnnotationFS>>> iterator;

private:
  const std::wstring descriptorPath;
  const std::unordered_map<std::wstring, std::any> configurationParameters;

protected:
  std::shared_ptr<AnalysisEngine> ae;
  std::shared_ptr<CAS> cas;

  BaseUIMATokenizer(
      std::shared_ptr<AttributeFactory> factory,
      const std::wstring &descriptorPath,
      std::unordered_map<std::wstring, std::any> &configurationParameters);

  /**
   * analyzes the tokenizer input using the given analysis engine
   * <p>
   * {@link #cas} will be filled with  extracted metadata (UIMA annotations,
   * feature structures)
   *
   * @throws IOException If there is a low-level I/O error.
   */
  virtual void analyzeInput() throw(ResourceInitializationException,
                                    AnalysisEngineProcessException,
                                    IOException);

  /**
   * initialize the FSIterator which is used to build tokens at each
   * incrementToken() method call
   *
   * @throws IOException If there is a low-level I/O error.
   */
  virtual void initializeIterator() = 0;

private:
  std::wstring toString(std::shared_ptr<Reader> reader) ;

public:
  void reset()  override;

protected:
  std::shared_ptr<BaseUIMATokenizer> shared_from_this()
  {
    return std::static_pointer_cast<BaseUIMATokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::uima
