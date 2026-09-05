#include "span_list.hpp"
#include <cassert>

namespace lmem {

SpanList::iterator::iterator(Span *span/*=nullptr*/) : span_(span) {}
Span *&SpanList::iterator::operator++() {
  span_ = span_->next;
  return span_;
}
Span *SpanList::iterator::operator++(int) {
  auto res = span_;
  span_ = span_->next;
  return res;
}
Span *&SpanList::iterator::operator--() {
  span_ = span_->prev;
  return span_;
}
Span *SpanList::iterator::operator--(int) {
  auto res = span_;
  span_ = span_->prev;
  return res;
}
Span *&SpanList::iterator::operator*() { return span_; }
Span *SpanList::iterator::operator->() { return span_; }
bool SpanList::iterator::operator!=(iterator const &other) const {
  return span_ != other.span_;
}
bool SpanList::iterator::operator==(iterator const &other) const {
  return span_ == other.span_;
}
SpanList::SpanList() {
  head_ = new Span();
  head_->next = head_;
  head_->prev = head_;
}

SpanList::~SpanList() { delete head_; }

SpanList::iterator SpanList::begin() { return head_->next; }

SpanList::iterator SpanList::end(){return head_->prev;}
// 插入到where之前
void SpanList::insert(Span *where, Span *data) {
  assert(where);
  assert(data);
  insert_(data, where->prev, where);
}

void SpanList::insert_(Span *new_data, Span *prev, Span *next) {
  new_data->next = next;
  new_data->prev = prev;
  prev->next = new_data;
  next->prev = new_data;
}

void SpanList::push_front(Span *span) { insert(*begin(), span); }

Span *SpanList::pop_front() {
  auto res = head_->next;
  erase(res);
  return res;
}
void SpanList::erase_(Span *prev, Span *next) {
  prev->next = next;
  next->prev = prev;
}

void SpanList::erase(Span *where) {
  assert(where != head_); // 不能删除头节点
  if (where == nullptr) {
    return;
  }
  erase_(where->prev, where->next);
}

void SpanList::lock() const { mutex_.lock(); }

void SpanList::unlock() const { mutex_.unlock(); }
bool SpanList::empty() const { return head_ == head_->next; }
} // namespace lmem