using namespace std;

#include "QueryNodeProcessorPipeline.h"

namespace org::apache::lucene::queryparser::flexible::core::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

QueryNodeProcessorPipeline::QueryNodeProcessorPipeline()
{
  // empty constructor
}

QueryNodeProcessorPipeline::QueryNodeProcessorPipeline(
    shared_ptr<QueryConfigHandler> queryConfigHandler)
{
  this->queryConfig = queryConfigHandler;
}

shared_ptr<QueryConfigHandler>
QueryNodeProcessorPipeline::getQueryConfigHandler()
{
  return this->queryConfig;
}

shared_ptr<QueryNode> QueryNodeProcessorPipeline::process(
    shared_ptr<QueryNode> queryTree) 
{

  for (auto processor : this->processors) {
    queryTree = processor->process(queryTree);
  }

  return queryTree;
}

void QueryNodeProcessorPipeline::setQueryConfigHandler(
    shared_ptr<QueryConfigHandler> queryConfigHandler)
{
  this->queryConfig = queryConfigHandler;

  for (auto processor : this->processors) {
    processor->setQueryConfigHandler(this->queryConfig);
  }
}

bool QueryNodeProcessorPipeline::add(shared_ptr<QueryNodeProcessor> processor)
{
  bool added = this->processors.push_back(processor);

  if (added) {
    processor->setQueryConfigHandler(this->queryConfig);
  }

  return added;
}

void QueryNodeProcessorPipeline::add(int index,
                                     shared_ptr<QueryNodeProcessor> processor)
{
  this->processors.add(index, processor);
  processor->setQueryConfigHandler(this->queryConfig);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public bool addAll(deque<?
// extends QueryNodeProcessor> c)
bool QueryNodeProcessorPipeline::addAll(shared_ptr<deque<T1>> c)
{
  bool anyAdded = this->processors.addAll(c);

  for (auto processor : c) {
    processor->setQueryConfigHandler(this->queryConfig);
  }

  return anyAdded;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public bool addAll(int index,
// deque<? extends QueryNodeProcessor> c)
bool QueryNodeProcessorPipeline::addAll(int index, shared_ptr<deque<T1>> c)
{
  bool anyAdded = this->processors.addAll(index, c);

  for (auto processor : c) {
    processor->setQueryConfigHandler(this->queryConfig);
  }

  return anyAdded;
}

void QueryNodeProcessorPipeline::clear() { this->processors.clear(); }

bool QueryNodeProcessorPipeline::contains(any o)
{
  return find(this->processors.begin(), this->processors.end(), o) !=
         this->processors.end();
}

template <typename T1>
bool QueryNodeProcessorPipeline::containsAll(shared_ptr<deque<T1>> c)
{
  return this->processors.containsAll(c);
}

shared_ptr<QueryNodeProcessor> QueryNodeProcessorPipeline::get(int index)
{
  return this->processors.get(index);
}

int QueryNodeProcessorPipeline::indexOf(any o)
{
  return this->processors.find(o);
}

bool QueryNodeProcessorPipeline::isEmpty() { return this->processors.empty(); }

shared_ptr<Iterator<std::shared_ptr<QueryNodeProcessor>>>
QueryNodeProcessorPipeline::iterator()
{
  return this->processors.begin();
}

int QueryNodeProcessorPipeline::lastIndexOf(any o)
{
  return this->processors.rfind(o);
}

shared_ptr<ListIterator<std::shared_ptr<QueryNodeProcessor>>>
QueryNodeProcessorPipeline::listIterator()
{
  return this->processors.begin();
}

shared_ptr<ListIterator<std::shared_ptr<QueryNodeProcessor>>>
QueryNodeProcessorPipeline::listIterator(int index)
{
  return this->processors.listIterator(index);
}

bool QueryNodeProcessorPipeline::remove(any o)
{
  return this->processors.remove(o);
}

shared_ptr<QueryNodeProcessor> QueryNodeProcessorPipeline::remove(int index)
{
  return this->processors.remove(index);
}

template <typename T1>
bool QueryNodeProcessorPipeline::removeAll(shared_ptr<deque<T1>> c)
{
  return this->processors.removeAll(c);
}

template <typename T1>
bool QueryNodeProcessorPipeline::retainAll(shared_ptr<deque<T1>> c)
{
  return this->processors.retainAll(c);
}

shared_ptr<QueryNodeProcessor>
QueryNodeProcessorPipeline::set(int index,
                                shared_ptr<QueryNodeProcessor> processor)
{
  shared_ptr<QueryNodeProcessor> oldProcessor =
      this->processors.set(index, processor);

  if (oldProcessor != processor) {
    processor->setQueryConfigHandler(this->queryConfig);
  }

  return oldProcessor;
}

int QueryNodeProcessorPipeline::size() { return this->processors.size(); }

deque<std::shared_ptr<QueryNodeProcessor>>
QueryNodeProcessorPipeline::subList(int fromIndex, int toIndex)
{
  return this->processors.subList(fromIndex, toIndex);
}

template <typename T>
std::deque<T> QueryNodeProcessorPipeline::toArray(std::deque<T> &array_)
{
  return this->processors.toArray(array_);
}

std::deque<any> QueryNodeProcessorPipeline::toArray()
{
  return this->processors.toArray();
}
} // namespace org::apache::lucene::queryparser::flexible::core::processors