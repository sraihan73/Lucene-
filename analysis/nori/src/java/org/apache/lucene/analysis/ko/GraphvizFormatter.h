#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ko::dict
{
class ConnectionCosts;
}

namespace org::apache::lucene::analysis::ko
{
class KoreanTokenizer;
}
namespace org::apache::lucene::analysis::ko
{
class Position;
}
namespace org::apache::lucene::analysis::ko
{
class WrappedPositionArray;
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
namespace org::apache::lucene::analysis::ko
{

using Position = org::apache::lucene::analysis::ko::KoreanTokenizer::Position;
using WrappedPositionArray =
    org::apache::lucene::analysis::ko::KoreanTokenizer::WrappedPositionArray;
using ConnectionCosts =
    org::apache::lucene::analysis::ko::dict::ConnectionCosts;

// TODO: would be nice to show 2nd best path in a diff't
// color...

/**
 * Outputs the dot (graphviz) string for the viterbi lattice.
 */
class GraphvizFormatter : public std::enable_shared_from_this<GraphvizFormatter>
{
  GET_CLASS_NAME(GraphvizFormatter)

private:
  static const std::wstring BOS_LABEL;

  static const std::wstring EOS_LABEL;

  static const std::wstring FONT_NAME;

  const std::shared_ptr<ConnectionCosts> costs;

  const std::unordered_map<std::wstring, std::wstring> bestPathMap;

  const std::shared_ptr<StringBuilder> sb = std::make_shared<StringBuilder>();

public:
  GraphvizFormatter(std::shared_ptr<ConnectionCosts> costs);

  virtual std::wstring finish();

  // Backtraces another incremental fragment:
  virtual void onBacktrace(std::shared_ptr<KoreanTokenizer> tok,
                           std::shared_ptr<WrappedPositionArray> positions,
                           int lastBackTracePos,
                           std::shared_ptr<Position> endPosData, int fromIDX,
                           std::deque<wchar_t> &fragment, bool isEnd);

  // Records which arcs make up the best bath:
private:
  void setBestPathMap(std::shared_ptr<WrappedPositionArray> positions,
                      int startPos, std::shared_ptr<Position> endPosData,
                      int fromIDX);

  std::wstring formatNodes(std::shared_ptr<KoreanTokenizer> tok,
                           std::shared_ptr<WrappedPositionArray> positions,
                           int startPos, std::shared_ptr<Position> endPosData,
                           std::deque<wchar_t> &fragment);

  std::wstring formatHeader();

  std::wstring formatTrailer();

  std::wstring getNodeID(int pos, int idx);
};

} // namespace org::apache::lucene::analysis::ko
