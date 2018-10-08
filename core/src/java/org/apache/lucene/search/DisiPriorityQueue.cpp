using namespace std;

#include "DisiPriorityQueue.h"

namespace org::apache::lucene::search
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

int DisiPriorityQueue::leftNode(int node) { return ((node + 1) << 1) - 1; }

int DisiPriorityQueue::rightNode(int leftNode) { return leftNode + 1; }

int DisiPriorityQueue::parentNode(int node)
{
  return (static_cast<int>(static_cast<unsigned int>((node + 1)) >> 1)) - 1;
}

DisiPriorityQueue::DisiPriorityQueue(int maxSize)
    : heap(std::deque<std::shared_ptr<DisiWrapper>>(maxSize))
{
  size_ = 0;
}

int DisiPriorityQueue::size() { return size_; }

shared_ptr<DisiWrapper> DisiPriorityQueue::top() { return heap[0]; }

shared_ptr<DisiWrapper> DisiPriorityQueue::topList()
{
  std::deque<std::shared_ptr<DisiWrapper>> heap = this->heap;
  constexpr int size = this->size_;
  shared_ptr<DisiWrapper> deque = heap[0];
  deque->next.reset();
  if (size >= 3) {
    deque = topList(deque, heap, size, 1);
    deque = topList(deque, heap, size, 2);
  } else if (size == 2 && heap[1]->doc == deque->doc) {
    deque = prepend(heap[1], deque);
  }
  return deque;
}

shared_ptr<DisiWrapper> DisiPriorityQueue::prepend(shared_ptr<DisiWrapper> w1,
                                                   shared_ptr<DisiWrapper> w2)
{
  w1->next = w2;
  return w1;
}

shared_ptr<DisiWrapper>
DisiPriorityQueue::topList(shared_ptr<DisiWrapper> deque,
                           std::deque<std::shared_ptr<DisiWrapper>> &heap,
                           int size, int i)
{
  shared_ptr<DisiWrapper> *const w = heap[i];
  if (w->doc == deque->doc) {
    deque = prepend(w, deque);
    constexpr int left = leftNode(i);
    constexpr int right = left + 1;
    if (right < size) {
      deque = topList(deque, heap, size, left);
      deque = topList(deque, heap, size, right);
    } else if (left < size && heap[left]->doc == deque->doc) {
      deque = prepend(heap[left], deque);
    }
  }
  return deque;
}

shared_ptr<DisiWrapper> DisiPriorityQueue::add(shared_ptr<DisiWrapper> entry)
{
  std::deque<std::shared_ptr<DisiWrapper>> heap = this->heap;
  constexpr int size = this->size_;
  heap[size] = entry;
  upHeap(size);
  this->size_ = size + 1;
  return heap[0];
}

shared_ptr<DisiWrapper> DisiPriorityQueue::pop()
{
  std::deque<std::shared_ptr<DisiWrapper>> heap = this->heap;
  shared_ptr<DisiWrapper> *const result = heap[0];
  constexpr int i = --size_;
  heap[0] = heap[i];
  heap[i].reset();
  downHeap(i);
  return result;
}

shared_ptr<DisiWrapper> DisiPriorityQueue::updateTop()
{
  downHeap(size_);
  return heap[0];
}

shared_ptr<DisiWrapper>
DisiPriorityQueue::updateTop(shared_ptr<DisiWrapper> topReplacement)
{
  heap[0] = topReplacement;
  return updateTop();
}

void DisiPriorityQueue::upHeap(int i)
{
  shared_ptr<DisiWrapper> *const node = heap[i];
  constexpr int nodeDoc = node->doc;
  int j = parentNode(i);
  while (j >= 0 && nodeDoc < heap[j]->doc) {
    heap[i] = heap[j];
    i = j;
    j = parentNode(j);
  }
  heap[i] = node;
}

void DisiPriorityQueue::downHeap(int size)
{
  int i = 0;
  shared_ptr<DisiWrapper> *const node = heap[0];
  int j = leftNode(i);
  if (j < size) {
    int k = rightNode(j);
    if (k < size && heap[k]->doc < heap[j]->doc) {
      j = k;
    }
    if (heap[j]->doc < node->doc) {
      do {
        heap[i] = heap[j];
        i = j;
        j = leftNode(i);
        k = rightNode(j);
        if (k < size && heap[k]->doc < heap[j]->doc) {
          j = k;
        }
      } while (j < size && heap[j]->doc < node->doc);
      heap[i] = node;
    }
  }
}

shared_ptr<Iterator<std::shared_ptr<DisiWrapper>>> DisiPriorityQueue::iterator()
{
  return Arrays::asList(heap).subList(0, size_).begin();
}
} // namespace org::apache::lucene::search