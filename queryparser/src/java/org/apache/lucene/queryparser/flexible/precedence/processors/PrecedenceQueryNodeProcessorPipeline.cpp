using namespace std;

#include "PrecedenceQueryNodeProcessorPipeline.h"

namespace org::apache::lucene::queryparser::flexible::precedence::processors
{
using PrecedenceQueryParser = org::apache::lucene::queryparser::flexible::
    precedence::PrecedenceQueryParser;
using BooleanQuery2ModifierNodeProcessor = org::apache::lucene::queryparser::
    flexible::standard::processors::BooleanQuery2ModifierNodeProcessor;
using StandardQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::standard::processors::StandardQueryNodeProcessorPipeline;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;

PrecedenceQueryNodeProcessorPipeline::PrecedenceQueryNodeProcessorPipeline(
    shared_ptr<QueryConfigHandler> queryConfig)
    : org::apache::lucene::queryparser::flexible::standard::processors::
          StandardQueryNodeProcessorPipeline(queryConfig)
{

  for (int i = 0; i < size(); i++) {

    if (get(i)->getClass().equals(BooleanQuery2ModifierNodeProcessor::typeid)) {
      remove(i--);
    }
  }

  add(make_shared<BooleanModifiersQueryNodeProcessor>());
}
} // namespace
  // org::apache::lucene::queryparser::flexible::precedence::processors