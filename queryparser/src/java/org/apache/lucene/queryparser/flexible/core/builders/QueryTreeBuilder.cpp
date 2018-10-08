using namespace std;

#include "QueryTreeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::core::builders
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using EscapeQuerySyntaxImpl = org::apache::lucene::queryparser::flexible::
    standard::parser::EscapeQuerySyntaxImpl;
const wstring QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID =
    QueryTreeBuilder::typeid->getName();

QueryTreeBuilder::QueryTreeBuilder()
{
  // empty constructor
}

void QueryTreeBuilder::setBuilder(shared_ptr<std::wstring> fieldName,
                                  shared_ptr<QueryBuilder> builder)
{

  if (this->fieldNameBuilders.empty()) {
    this->fieldNameBuilders =
        unordered_map<wstring, std::shared_ptr<QueryBuilder>>();
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->fieldNameBuilders.emplace(fieldName->toString(), builder);
}

void QueryTreeBuilder::setBuilder(type_info queryNodeClass,
                                  shared_ptr<QueryBuilder> builder)
{

  if (this->queryNodeBuilders.empty()) {
    this->queryNodeBuilders =
        unordered_map<type_info, std::shared_ptr<QueryBuilder>>();
  }

  this->queryNodeBuilders.emplace(queryNodeClass, builder);
}

void QueryTreeBuilder::process(shared_ptr<QueryNode> node) throw(
    QueryNodeException)
{

  if (node != nullptr) {
    shared_ptr<QueryBuilder> builder = getBuilder(node);

    if (!(std::dynamic_pointer_cast<QueryTreeBuilder>(builder) != nullptr)) {
      deque<std::shared_ptr<QueryNode>> children = node->getChildren();

      if (children.size() > 0) {

        for (auto child : children) {
          process(child);
        }
      }
    }

    processNode(node, builder);
  }
}

shared_ptr<QueryBuilder>
QueryTreeBuilder::getBuilder(shared_ptr<QueryNode> node)
{
  shared_ptr<QueryBuilder> builder = nullptr;

  if (this->fieldNameBuilders.size() > 0 &&
      std::dynamic_pointer_cast<FieldableNode>(node) != nullptr) {
    shared_ptr<std::wstring> field =
        (std::static_pointer_cast<FieldableNode>(node))->getField();

    if (field != nullptr) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      field = field->toString();
    }

    builder = this->fieldNameBuilders[field];
  }

  if (builder == nullptr && this->queryNodeBuilders.size() > 0) {

    type_info clazz = node->getClass();

    do {
      builder = getQueryBuilder(clazz);

      if (builder == nullptr) {
        std::deque<type_info> classes = clazz.getInterfaces();

        for (auto actualClass : classes) {
          builder = getQueryBuilder(actualClass);

          if (builder != nullptr) {
            break;
          }
        }
      }

    } while (builder == nullptr && (clazz = clazz.getSuperclass()) != nullptr);
  }

  return builder;
}

void QueryTreeBuilder::processNode(
    shared_ptr<QueryNode> node,
    shared_ptr<QueryBuilder> builder) 
{

  if (builder == nullptr) {

    throw make_shared<QueryNodeException>(make_shared<MessageImpl>(
        QueryParserMessages::LUCENE_QUERY_CONVERSION_ERROR,
        node->toQueryString(make_shared<EscapeQuerySyntaxImpl>()),
        node->getClassName()));
  }

  any obj = builder->build(node);

  if (obj != nullptr) {
    node->setTag(QUERY_TREE_BUILDER_TAGID, obj);
  }
}

shared_ptr<QueryBuilder> QueryTreeBuilder::getQueryBuilder(type_info clazz)
{

  if (QueryNode::typeid->isAssignableFrom(clazz)) {
    return this->queryNodeBuilders[clazz];
  }

  return nullptr;
}

any QueryTreeBuilder::build(shared_ptr<QueryNode> queryNode) throw(
    QueryNodeException)
{
  process(queryNode);

  return queryNode->getTag(QUERY_TREE_BUILDER_TAGID);
}
} // namespace org::apache::lucene::queryparser::flexible::core::builders