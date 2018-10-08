using namespace std;

#include "Optimizer2.h"
#include "Cell.h"

namespace org::egothor::stemmer
{

Optimizer2::Optimizer2() {}

shared_ptr<Cell> Optimizer2::merge(shared_ptr<Cell> m, shared_ptr<Cell> e)
{
  if (m->cmd == e->cmd && m->ref == e->ref && m->skip == e->skip) {
    shared_ptr<Cell> c = make_shared<Cell>(m);
    c->cnt += e->cnt;
    return c;
  } else {
    return nullptr;
  }
}
} // namespace org::egothor::stemmer