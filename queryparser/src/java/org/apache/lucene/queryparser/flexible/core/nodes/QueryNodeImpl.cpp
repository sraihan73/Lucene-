using namespace std;

#include "QueryNodeImpl.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using NLS = org::apache::lucene::queryparser::flexible::messages::NLS;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;
const wstring QueryNodeImpl::PLAINTEXT_FIELD_NAME = L"_plain";

void QueryNodeImpl::allocate()
{

  if (this->clauses.empty()) {
    this->clauses = deque<>();

  } else {
    this->clauses.clear();
  }
}

void QueryNodeImpl::add(shared_ptr<QueryNode> child)
{

  if (isLeaf() || this->clauses.empty() || child == nullptr) {
    throw invalid_argument(NLS::getLocalizedMessage(
        QueryParserMessages::NODE_ACTION_NOT_SUPPORTED));
  }

  this->clauses.push_back(child);
  (std::static_pointer_cast<QueryNodeImpl>(child))
      ->setParent(shared_from_this());
}

void QueryNodeImpl::add(deque<std::shared_ptr<QueryNode>> &children)
{

  if (isLeaf() || this->clauses.empty()) {
    throw invalid_argument(NLS::getLocalizedMessage(
        QueryParserMessages::NODE_ACTION_NOT_SUPPORTED));
  }

  for (auto child : children) {
    add(child);
  }
}

bool QueryNodeImpl::isLeaf() { return this->isLeaf_; }

void QueryNodeImpl::set(deque<std::shared_ptr<QueryNode>> &children)
{

  if (isLeaf() || this->clauses.empty()) {
    shared_ptr<ResourceBundle> bundle = ResourceBundle::getBundle(
        L"org.apache.lucene.queryParser.messages.QueryParserMessages",
        Locale::getDefault());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring message =
        bundle->getObject(L"Q0008E.NODE_ACTION_NOT_SUPPORTED")->toString();

    throw invalid_argument(message);
  }

  // reset parent value
  for (auto child : children) {
    child->removeFromParent();
  }

  deque<std::shared_ptr<QueryNode>> existingChildren =
      deque<std::shared_ptr<QueryNode>>(getChildren());
  for (auto existingChild : existingChildren) {
    existingChild->removeFromParent();
  }

  // allocate new children deque
  allocate();

  // add new children and set parent
  add(children);
}

shared_ptr<QueryNode>
QueryNodeImpl::cloneTree() 
{
  shared_ptr<QueryNodeImpl> clone =
      std::static_pointer_cast<QueryNodeImpl>(__super::clone());
  clone->isLeaf_ = this->isLeaf_;

  // Reset all tags
  clone->tags = unordered_map<wstring, any>();

  // copy children
  if (this->clauses.size() > 0) {
    deque<std::shared_ptr<QueryNode>> localClauses =
        deque<std::shared_ptr<QueryNode>>();
    for (auto clause : this->clauses) {
      localClauses.push_back(clause->cloneTree());
    }
    clone->clauses = localClauses;
  }

  return clone;
}

shared_ptr<QueryNode> QueryNodeImpl::clone() 
{
  return cloneTree();
}

void QueryNodeImpl::setLeaf(bool isLeaf) { this->isLeaf_ = isLeaf; }

deque<std::shared_ptr<QueryNode>> QueryNodeImpl::getChildren()
{
  if (isLeaf() || this->clauses.empty()) {
    return nullptr;
  }
  return deque<>(this->clauses);
}

void QueryNodeImpl::setTag(const wstring &tagName, any value)
{
  this->tags.emplace(tagName.toLowerCase(Locale::ROOT), value);
}

void QueryNodeImpl::unsetTag(const wstring &tagName)
{
  this->tags.erase(tagName.toLowerCase(Locale::ROOT));
}

bool QueryNodeImpl::containsTag(const wstring &tagName)
{
  return this->tags.find(tagName.toLowerCase(Locale::ROOT)) != this->tags.end();
}

any QueryNodeImpl::getTag(const wstring &tagName)
{
  return this->tags[tagName.toLowerCase(Locale::ROOT)];
}

void QueryNodeImpl::setParent(shared_ptr<QueryNode> parent)
{
  if (this->parent != parent) {
    this->removeFromParent();
    this->parent = parent;
  }
}

shared_ptr<QueryNode> QueryNodeImpl::getParent() { return this->parent; }

bool QueryNodeImpl::isRoot() { return getParent() == nullptr; }

bool QueryNodeImpl::isDefaultField(shared_ptr<std::wstring> fld)
{
  if (this->toQueryStringIgnoreFields) {
    return true;
  }
  if (fld == nullptr) {
    return true;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (QueryNodeImpl::PLAINTEXT_FIELD_NAME == StringUtils::toString(fld)) {
    return true;
  }
  return false;
}

wstring QueryNodeImpl::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return __super::toString();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public
// java.util.Map<std::wstring, Object> getTagMap()
unordered_map<wstring, any> QueryNodeImpl::getTagMap()
{
  return static_cast<unordered_map<wstring, any>>(this->tags.clone());
}

void QueryNodeImpl::removeChildren(shared_ptr<QueryNode> childNode)
{
  deque<std::shared_ptr<QueryNode>>::const_iterator it = this->clauses.begin();
  while (it != this->clauses.end()) {
    if (*it == childNode) {
      (*it)->remove();
    }
    it++;
  }
  childNode->removeFromParent();
}

void QueryNodeImpl::removeFromParent()
{
  if (this->parent != nullptr) {
    shared_ptr<QueryNode> parent = this->parent;
    this->parent.reset();
    parent->removeChildren(shared_from_this());
  }
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes