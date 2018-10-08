using namespace std;

#include "PrecedenceQueryParser.h"

namespace org::apache::lucene::queryparser::flexible::precedence
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using StandardQueryParser =
    org::apache::lucene::queryparser::flexible::standard::StandardQueryParser;
using StandardQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::standard::processors::StandardQueryNodeProcessorPipeline;
using PrecedenceQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::precedence::processors::PrecedenceQueryNodeProcessorPipeline;

PrecedenceQueryParser::PrecedenceQueryParser()
{
  setQueryNodeProcessor(make_shared<PrecedenceQueryNodeProcessorPipeline>(
      getQueryConfigHandler()));
}

PrecedenceQueryParser::PrecedenceQueryParser(shared_ptr<Analyzer> analyer)
    : org::apache::lucene::queryparser::flexible::standard::StandardQueryParser(
          analyer)
{

  setQueryNodeProcessor(make_shared<PrecedenceQueryNodeProcessorPipeline>(
      getQueryConfigHandler()));
}
} // namespace org::apache::lucene::queryparser::flexible::precedence