using namespace std;

#include "Optimizer.h"
#include "Cell.h"
#include "Row.h"
#include "Trie.h"

namespace org::egothor::stemmer
{

Optimizer::Optimizer() {}

shared_ptr<Trie> Optimizer::optimize(shared_ptr<Trie> orig)
{
  deque<std::shared_ptr<std::wstring>> &cmds = orig->cmds;
  deque<std::shared_ptr<Row>> rows = deque<std::shared_ptr<Row>>();
  deque<std::shared_ptr<Row>> &orows = orig->rows;
  std::deque<int> remap(orows.size());

  for (int j = orows.size() - 1; j >= 0; j--) {
    shared_ptr<Row> now =
        make_shared<Remap>(shared_from_this(), orows[j], remap);
    bool merged = false;

    for (int i = 0; i < rows.size(); i++) {
      shared_ptr<Row> q = merge(now, rows[i]);
      if (q != nullptr) {
        rows[i] = q;
        merged = true;
        remap[j] = i;
        break;
      }
    }

    if (merged == false) {
      remap[j] = rows.size();
      rows.push_back(now);
    }
  }

  int root = remap[orig->root];
  Arrays::fill(remap, -1);
  rows = removeGaps(root, rows, deque<std::shared_ptr<Row>>(), remap);

  return make_shared<Trie>(orig->forward, remap[root], cmds, rows);
}

shared_ptr<Row> Optimizer::merge(shared_ptr<Row> master,
                                 shared_ptr<Row> existing)
{
  Iterator<wchar_t> i = master->cells.keySet().begin();
  shared_ptr<Row> n = make_shared<Row>();
  for (; i->hasNext();) {
    optional<wchar_t> ch = i->next();
    // XXX also must handle Cnt and Skip !!
    shared_ptr<Cell> a = master->cells[ch];
    shared_ptr<Cell> b = existing->cells[ch];

    shared_ptr<Cell> s = (b == nullptr) ? make_shared<Cell>(a) : merge(a, b);
    if (s == nullptr) {
      return nullptr;
    }
    n->cells.emplace(ch, s);
  }
  i = existing->cells.keySet().begin();
  for (; i->hasNext();) {
    optional<wchar_t> ch = i->next();
    if (master->at(ch) != nullptr) {
      continue;
    }
    n->cells.emplace(ch, existing->at(ch));
  }
  return n;
}

shared_ptr<Cell> Optimizer::merge(shared_ptr<Cell> m, shared_ptr<Cell> e)
{
  shared_ptr<Cell> n = make_shared<Cell>();

  if (m->skip != e->skip) {
    return nullptr;
  }

  if (m->cmd >= 0) {
    if (e->cmd >= 0) {
      if (m->cmd == e->cmd) {
        n->cmd = m->cmd;
      } else {
        return nullptr;
      }
    } else {
      n->cmd = m->cmd;
    }
  } else {
    n->cmd = e->cmd;
  }
  if (m->ref >= 0) {
    if (e->ref >= 0) {
      if (m->ref == e->ref) {
        if (m->skip == e->skip) {
          n->ref = m->ref;
        } else {
          return nullptr;
        }
      } else {
        return nullptr;
      }
    } else {
      n->ref = m->ref;
    }
  } else {
    n->ref = e->ref;
  }
  n->cnt = m->cnt + e->cnt;
  n->skip = m->skip;
  return n;
}
} // namespace org::egothor::stemmer