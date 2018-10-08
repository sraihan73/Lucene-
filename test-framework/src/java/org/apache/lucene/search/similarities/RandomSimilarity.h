#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class BM25Similarity;
}

namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class BasicModel;
}
namespace org::apache::lucene::search::similarities
{
class AfterEffect;
}
namespace org::apache::lucene::search::similarities
{
class Normalization;
}
namespace org::apache::lucene::search::similarities
{
class Distribution;
}
namespace org::apache::lucene::search::similarities
{
class Lambda;
}
namespace org::apache::lucene::search::similarities
{
class Independence;
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
namespace org::apache::lucene::search::similarities
{

/**
 * Similarity implementation that randomizes Similarity implementations
 * per-field.
 * <p>
 * The choices are 'sticky', so the selected algorithm is always used
 * for the same field.
 */
class RandomSimilarity : public PerFieldSimilarityWrapper
{
  GET_CLASS_NAME(RandomSimilarity)
public:
  const std::shared_ptr<BM25Similarity> defaultSim =
      std::make_shared<BM25Similarity>();
  const std::deque<std::shared_ptr<Similarity>> knownSims;
  std::unordered_map<std::wstring, std::shared_ptr<Similarity>>
      previousMappings =
          std::unordered_map<std::wstring, std::shared_ptr<Similarity>>();
  const int perFieldSeed;
  const bool shouldQueryNorm;

  RandomSimilarity(std::shared_ptr<Random> random);

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Similarity> get(const std::wstring &field) override;

  // all the similarities that we rotate through
  /** The DFR basic models to test. */
  static std::deque<std::shared_ptr<BasicModel>> BASIC_MODELS;
  /** The DFR aftereffects to test. */
  static std::deque<std::shared_ptr<AfterEffect>> AFTER_EFFECTS;
  /** The DFR normalizations to test. */
  static std::deque<std::shared_ptr<Normalization>> NORMALIZATIONS;
  /** The distributions for IB. */
  static std::deque<std::shared_ptr<Distribution>> DISTRIBUTIONS;
  /** Lambdas for IB. */
  static std::deque<std::shared_ptr<Lambda>> LAMBDAS;
  /** Independence measures for DFI */
  static std::deque<std::shared_ptr<Independence>> INDEPENDENCE_MEASURES;
  static std::deque<std::shared_ptr<Similarity>> allSims;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static RandomSimilarity::StaticConstructor staticConstructor;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::wstring toString();

protected:
  std::shared_ptr<RandomSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<RandomSimilarity>(
        PerFieldSimilarityWrapper::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
