using namespace std;

#include "VariableContext.h"

namespace org::apache::lucene::expressions::js
{

VariableContext::VariableContext(Type c, const wstring &s, int i)
    : type(c), text(s), integer(i)
{
}

std::deque<std::shared_ptr<VariableContext>>
VariableContext::parse(const wstring &variable)
{
  std::deque<wchar_t> text = variable.toCharArray();
  deque<std::shared_ptr<VariableContext>> contexts =
      deque<std::shared_ptr<VariableContext>>();
  int i = addMember(
      text, 0, contexts); // base variable is a "member" of the global namespace
  while (i < text.size()) {
    if (text[i] == L'[') {
      if (text[++i] == L'\'') {
        i = addStringIndex(text, i, contexts);
      } else {
        i = addIntIndex(text, i, contexts);
      }
      ++i;   // move past end bracket
    } else { // text[i] == '.', ie object member
      i = addMember(text, i + 1, contexts);
    }
  }
  return contexts.toArray(
      std::deque<std::shared_ptr<VariableContext>>(contexts.size()));
}

int VariableContext::addMember(
    std::deque<wchar_t> &text, int i,
    deque<std::shared_ptr<VariableContext>> &contexts)
{
  int j = i + 1;
  while (j < text.size() && text[j] != L'[' && text[j] != L'.' &&
         text[j] != L'(') {
    ++j; // find first array, member access, or method call
  }
  if (j + 1 < text.size() && text[j] == L'(' && text[j + 1] == L')') {
    contexts.push_back(make_shared<VariableContext>(
        Type::METHOD, wstring(text, i, j - i), -1));
    j += 2; // move past the parenthesis
  } else {
    contexts.push_back(make_shared<VariableContext>(
        Type::MEMBER, wstring(text, i, j - i), -1));
  }
  return j;
}

int VariableContext::addStringIndex(
    std::deque<wchar_t> &text, int i,
    deque<std::shared_ptr<VariableContext>> &contexts)
{
  ++i; // move past quote
  int j = i;
  while (text[j] != L'\'') { // find end of single quoted string
    if (text[j] == L'\\') {
      ++j; // skip over escapes
    }
    ++j;
  }
  shared_ptr<StringBuilder> buf =
      make_shared<StringBuilder>(j - i); // space for string, without end quote
  while (i < j) { // copy string to buffer (without begin/end quotes)
    if (text[i] == L'\\') {
      ++i; // unescape escapes
    }
    buf->append(text[i]);
    ++i;
  }
  contexts.push_back(
      make_shared<VariableContext>(Type::STR_INDEX, buf->toString(), -1));
  return j + 1; // move past quote, return end bracket location
}

int VariableContext::addIntIndex(
    std::deque<wchar_t> &text, int i,
    deque<std::shared_ptr<VariableContext>> &contexts)
{
  int j = i + 1;
  while (text[j] != L']') {
    ++j; // find end of array access
  }
  int index = stoi(wstring(text, i, j - i));
  contexts.push_back(
      make_shared<VariableContext>(Type::INT_INDEX, nullptr, index));
  return j;
}
} // namespace org::apache::lucene::expressions::js