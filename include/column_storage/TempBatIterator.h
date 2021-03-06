// Copyright (c) 2010 Benjamin Schlegel
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Copyright (c) 2016-2017 Till Kolditz
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/***
 * @author Benjamin Schlegel
 */
#ifndef TEMPBATITERATOR_H
#define TEMPBATITERATOR_H

#include <vector>

#include <ColumnStore.h>
#include <column_storage/BatIterator.h>
#include <column_storage/TempBat.h>

namespace ahead {

    template<typename Head, typename Tail>
    class TempBATIterator :
            public BATIterator<Head, Tail> {

    private:
        using head_t = typename BAT<Head, Tail>::head_t;
        using tail_t = typename BAT<Head, Tail>::tail_t;
        using coldesc_head_t = typename BAT<Head, Tail>::coldesc_head_t;
        using coldesc_tail_t = typename BAT<Head, Tail>::coldesc_tail_t;
        typedef typename coldesc_head_t::container_t container_head_t;
        typedef typename coldesc_tail_t::container_t container_tail_t;
        typedef typename container_head_t::iterator iterator_head_t;
        typedef typename container_tail_t::iterator iterator_tail_t;

        std::shared_ptr<container_head_t> cHead;
        std::shared_ptr<container_tail_t> cTail;
        iterator_head_t iterHead;
        iterator_head_t iterHeadEnd;
        iterator_tail_t iterTail;
        iterator_tail_t iterTailEnd;

    public:
        typedef TempBATIterator<Head, Tail> self_t;

        TempBATIterator(
                coldesc_head_t& head,
                coldesc_tail_t & tail)
                : cHead(head.container),
                  cTail(tail.container),
                  iterHead(cHead->begin()),
                  iterHeadEnd(cHead->end()),
                  iterTail(cTail->begin()),
                  iterTailEnd(cTail->end()) {
        }

        TempBATIterator(
                const TempBATIterator<Head, Tail> & iter)
                : cHead(iter.cHead),
                  cTail(iter.cTail),
                  iterHead(iter.iterHead),
                  iterHeadEnd(iter.iterHeadEnd),
                  iterTail(iter.iterTail),
                  iterTailEnd(iter.iterTailEnd) {
        }

        virtual ~TempBATIterator() {
        }

        TempBATIterator& operator=(
                const TempBATIterator & copy) {
            new (this) TempBATIterator(copy);
            return *this;
        }

        virtual void next() override {
            iterHead++;
            iterTail++;
        }

        virtual TempBATIterator& operator++() override {
            next();
            return *this;
        }

        virtual TempBATIterator& operator+=(
                oid_t i) override {
            std::advance(iterHead, i);
            std::advance(iterTail, i);
            return *this;
        }

        virtual std::optional<oid_t> position() override {
            if (hasNext()) {
                return std::optional<oid_t>(static_cast<oid_t>(cHead->size() - (iterHeadEnd - iterHead)));
            }
            return std::optional<oid_t>();
        }

        virtual void position(
                oid_t index) override {
            iterHead = cHead->begin();
            iterTail = cTail->begin();
            std::advance(iterHead, index);
            std::advance(iterTail, index);
        }

        virtual bool hasNext() override {
            return (iterHead < iterHeadEnd) & (iterTail < iterTailEnd);
        }

        virtual head_t head() override {
            return *iterHead;
        }

        virtual tail_t tail() override {
            return *iterTail;
        }

        virtual size_t size() override {
            return cHead->size();
        }

        virtual size_t consumption() override {
            return cHead->capacity() * sizeof(head_t) + cTail->capacity() * sizeof(tail_t);
        }
    };

    template<>
    class TempBATIterator<v2_void_t, v2_void_t> :
            public BATIterator<v2_void_t, v2_void_t> {

    private:
        typedef v2_void_t Head, Tail;
        using head_t = typename BAT<Head, Tail>::head_t;
        using tail_t = typename BAT<Head, Tail>::tail_t;
        using coldesc_head_t = typename BAT<Head, Tail>::coldesc_head_t;
        using coldesc_tail_t = typename BAT<Head, Tail>::coldesc_tail_t;

        oid_t seqbase_head;
        oid_t oid_head;
        oid_t seqbase_tail;
        oid_t oid_tail;
        oid_t count;
        oid_t pos;

    public:
        typedef TempBATIterator<v2_void_t, v2_void_t> self_t;

        TempBATIterator(
                coldesc_head_t & head,
                coldesc_tail_t & tail,
                oid_t count)
                : seqbase_head(head.metaData.seqbase),
                  oid_head(seqbase_head),
                  seqbase_tail(tail.metaData.seqbase),
                  oid_tail(seqbase_tail),
                  count(count),
                  pos(0) {
        }

        TempBATIterator(
                const TempBATIterator<v2_void_t, v2_void_t> & iter)
                : seqbase_head(iter.seqbase_head),
                  oid_head(iter.oid_head),
                  seqbase_tail(iter.seqbase_tail),
                  oid_tail(iter.oid_tail),
                  count(iter.count),
                  pos(iter.pos) {
        }

        virtual ~TempBATIterator() {
        }

        TempBATIterator& operator=(
                const TempBATIterator & copy) {
            new (this) TempBATIterator(copy);
            return *this;
        }

        virtual void next() override {
            ++oid_head;
            ++oid_tail;
            ++pos;
        }

        virtual TempBATIterator& operator++() override {
            next();
            return *this;
        }

        virtual TempBATIterator& operator+=(
                oid_t i) override {
            oid_head += i;
            oid_tail += i;
            pos += i;
            return *this;
        }

        virtual std::optional<oid_t> position() override {
            if (hasNext()) {
                return std::optional<oid_t>(static_cast<oid_t>(pos));
            }
            return std::optional<oid_t>();
        }

        virtual void position(
                oid_t index) override {
            oid_head = seqbase_head + index;
            oid_tail = seqbase_tail + index;
        }

        virtual bool hasNext() override {
            return pos < count;
        }

        virtual head_t head() override {
            return oid_head;
        }

        virtual tail_t tail() override {
            return oid_tail;
        }

        virtual size_t size() override {
            return 0;
        }

        virtual size_t consumption() override {
            return 0;
        }
    };

    template<typename Head>
    class TempBATIterator<Head, v2_void_t> :
            public BATIterator<Head, v2_void_t> {

    private:
        typedef v2_void_t Tail;
        using head_t = typename BAT<Head, Tail>::head_t;
        using tail_t = typename BAT<Head, Tail>::tail_t;
        using coldesc_head_t = typename BAT<Head, Tail>::coldesc_head_t;
        using coldesc_tail_t = typename BAT<Head, Tail>::coldesc_tail_t;
        typedef typename coldesc_head_t::container_t container_head_t;
        typedef typename container_head_t::iterator iterator_head_t;

        std::shared_ptr<container_head_t> cHead;
        iterator_head_t iterHead;
        iterator_head_t iterHeadEnd;
        oid_t seqbase;
        oid_t oid_tail;

    public:
        typedef TempBATIterator<Head, v2_void_t> self_t;

        TempBATIterator(
                coldesc_head_t& head,
                coldesc_tail_t & tail)
                : cHead(head.container),
                  iterHead(cHead->begin()),
                  iterHeadEnd(cHead->end()),
                  seqbase(tail.metaData.seqbase),
                  oid_tail(seqbase) {
        }

        TempBATIterator(
                const TempBATIterator<Head, v2_void_t> & iter)
                : cHead(iter.cHead),
                  iterHead(iter.iterHead),
                  iterHeadEnd(iter.iterHeadEnd),
                  seqbase(iter.seqbase),
                  oid_tail(iter.oid_tail) {
        }

        virtual ~TempBATIterator() {
        }

        TempBATIterator& operator=(
                const TempBATIterator & copy) {
            new (this) TempBATIterator(copy);
            return *this;
        }

        virtual void next() override {
            std::advance(iterHead, 1);
            ++oid_tail;
        }

        virtual TempBATIterator& operator++() override {
            next();
            return *this;
        }

        virtual TempBATIterator& operator+=(
                oid_t i) override {
            std::advance(iterHead, i);
            oid_tail += i;
            return *this;
        }

        virtual std::optional<oid_t> position() override {
            if (hasNext()) {
                return std::optional<oid_t>(iterHead - cHead->begin());
            }
            return std::optional<oid_t>();
        }

        virtual void position(
                oid_t index) override {
            iterHead = cHead->begin();
            std::advance(iterHead, index);
            oid_tail = seqbase + index;
        }

        virtual bool hasNext() override {
            return iterHead < iterHeadEnd;
        }

        virtual head_t head() override {
            return *iterHead;
        }

        virtual tail_t tail() override {
            return oid_tail;
        }

        virtual size_t size() override {
            return cHead->size();
        }

        virtual size_t consumption() override {
            return cHead->capacity() * sizeof(head_t);
        }
    };

    template<typename Tail>
    class TempBATIterator<v2_void_t, Tail> :
            public BATIterator<v2_void_t, Tail> {

    private:
        typedef typename v2_void_t::type_t head_t;
        typedef typename Tail::type_t tail_t;
        typedef typename TempBAT<v2_void_t, Tail>::coldesc_tail_t::container_t container_tail_t;
        typedef ColumnDescriptor<v2_void_t, void> coldesc_head_t;
        typedef ColumnDescriptor<Tail, container_tail_t> coldesc_tail_t;
        typedef typename container_tail_t::iterator iterator_tail_t;

        std::shared_ptr<container_tail_t> cTail;
        iterator_tail_t iterTail;
        iterator_tail_t iterTailEnd;
        oid_t seqbase;
        oid_t oid_head;

    public:
        typedef TempBATIterator<v2_void_t, Tail> self_t;

        TempBATIterator(
                coldesc_head_t& head,
                coldesc_tail_t & tail)
                : cTail(tail.container),
                  iterTail(cTail->begin()),
                  iterTailEnd(cTail->end()),
                  seqbase(head.metaData.seqbase),
                  oid_head(seqbase) {
        }

        TempBATIterator(
                const TempBATIterator<v2_void_t, Tail> & iter)
                : cTail(iter.cTail),
                  iterTail(iter.iterTail),
                  iterTailEnd(iter.iterTailEnd),
                  seqbase(iter.seqbase),
                  oid_head(iter.oid_head) {
        }

        virtual ~TempBATIterator() {
        }

        TempBATIterator& operator=(
                const TempBATIterator & copy) {
            new (this) TempBATIterator(copy);
            return *this;
        }

        virtual void next() override {
            iterTail++;
            ++oid_head;
        }

        virtual TempBATIterator& operator++() override {
            next();
            return *this;
        }

        virtual TempBATIterator& operator+=(
                oid_t i) override {
            oid_head += i;
            std::advance(iterTail, i);
            return *this;
        }

        virtual std::optional<oid_t> position() override {
            if (hasNext()) {
                return std::optional<oid_t>(static_cast<oid_t>(iterTail - cTail->begin()));
            }
            return std::optional<oid_t>();
        }

        virtual void position(
                oid_t index) override {
            iterTail = cTail->begin();
            std::advance(iterTail, index);
            oid_head = seqbase + index;
        }

        virtual bool hasNext() override {
            return iterTail < iterTailEnd;
        }

        virtual head_t head() override {
            return oid_head;
        }

        virtual tail_t tail() override {
            return *iterTail;
        }

        virtual size_t size() override {
            return cTail->size();
        }

        virtual size_t consumption() override {
            return cTail->capacity() * sizeof(tail_t);
        }
    };

}

#endif
