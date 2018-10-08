using namespace std;

#include "TSTAutocomplete.h"

namespace org::apache::lucene::search::suggest::tst
{

TSTAutocomplete::TSTAutocomplete() {}

void TSTAutocomplete::balancedTree(std::deque<any> &tokens,
                                   std::deque<any> &vals, int lo, int hi,
                                   shared_ptr<TernaryTreeNode> root)
{
  if (lo > hi) {
    return;
  }
  int mid = (lo + hi) / 2;
  root = insert(root, any_cast<wstring>(tokens[mid]), vals[mid], 0);
  balancedTree(tokens, vals, lo, mid - 1, root);
  balancedTree(tokens, vals, mid + 1, hi, root);
}

shared_ptr<TernaryTreeNode>
TSTAutocomplete::insert(shared_ptr<TernaryTreeNode> currentNode,
                        shared_ptr<std::wstring> s, any val, int x)
{
  if (s == nullptr || s->length() <= x) {
    return currentNode;
  }
  if (currentNode == nullptr) {
    shared_ptr<TernaryTreeNode> newNode = make_shared<TernaryTreeNode>();
    newNode->splitchar = s->charAt(x);
    currentNode = newNode;
    if (x < s->length() - 1) {
      currentNode->eqKid = insert(currentNode->eqKid, s, val, x + 1);
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      currentNode->token = s->toString();
      currentNode->val = val;
      return currentNode;
    }
  } else if (currentNode->splitchar > s->charAt(x)) {
    currentNode->loKid = insert(currentNode->loKid, s, val, x);
  } else if (currentNode->splitchar == s->charAt(x)) {
    if (x < s->length() - 1) {
      currentNode->eqKid = insert(currentNode->eqKid, s, val, x + 1);
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      currentNode->token = s->toString();
      currentNode->val = val;
      return currentNode;
    }
  } else {
    currentNode->hiKid = insert(currentNode->hiKid, s, val, x);
  }
  return currentNode;
}

deque<std::shared_ptr<TernaryTreeNode>>
TSTAutocomplete::prefixCompletion(shared_ptr<TernaryTreeNode> root,
                                  shared_ptr<std::wstring> s, int x)
{

  shared_ptr<TernaryTreeNode> p = root;
  deque<std::shared_ptr<TernaryTreeNode>> suggest =
      deque<std::shared_ptr<TernaryTreeNode>>();

  while (p != nullptr) {
    if (s->charAt(x) < p->splitchar) {
      p = p->loKid;
    } else if (s->charAt(x) == p->splitchar) {
      if (x == s->length() - 1) {
        break;
      } else {
        x++;
      }
      p = p->eqKid;
    } else {
      p = p->hiKid;
    }
  }

  if (p == nullptr) {
    return suggest;
  }
  if (p->eqKid == nullptr && p->token == L"") {
    return suggest;
  }
  if (p->eqKid == nullptr && p->token != L"") {
    suggest.push_back(p);
    return suggest;
  }

  if (p->token != L"") {
    suggest.push_back(p);
  }
  p = p->eqKid;

  stack<std::shared_ptr<TernaryTreeNode>> st =
      stack<std::shared_ptr<TernaryTreeNode>>();
  st.push(p);
  while (!st.empty()) {
    shared_ptr<TernaryTreeNode> top = st.top();
    st.pop();
    if (top->token != L"") {
      suggest.push_back(top);
    }
    if (top->eqKid != nullptr) {
      st.push(top->eqKid);
    }
    if (top->loKid != nullptr) {
      st.push(top->loKid);
    }
    if (top->hiKid != nullptr) {
      st.push(top->hiKid);
    }
  }
  return suggest;
}
} // namespace org::apache::lucene::search::suggest::tst