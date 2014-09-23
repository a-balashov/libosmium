#ifndef OSMIUM_INDEX_MAP_SPARSE_TABLE_HPP
#define OSMIUM_INDEX_MAP_SPARSE_TABLE_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include <google/sparsetable>

#include <osmium/index/map.hpp>
#include <osmium/io/detail/read_write.hpp>

namespace osmium {

    namespace index {

        namespace map {

            /**
            * The SparseTable index stores elements in a Google sparsetable,
            * a data structure that can hold sparsly filled tables in a
            * very space efficient way. It will resize automatically.
            *
            * Use this index if the ID space is only sparsly
            * populated, such as when working with smaller OSM files (like
            * country extracts).
            *
            * This will only work on 64 bit machines.
            */
            template <typename TId, typename TValue>
            class SparseTable : public osmium::index::map::Map<TId, TValue> {

                TId m_grow_size;

                google::sparsetable<TValue> m_elements;

                static_assert(sizeof(typename google::sparsetable<TValue>::size_type) >= 8, "google::sparsetable needs 64bit machine");

            public:

                /**
                * Constructor.
                *
                * @param grow_size The initial size of the index (ie number of
                *                  elements that fit into the index).
                *                  The storage will grow by at least this size
                *                  every time it runs out of space.
                */
                explicit SparseTable(const TId grow_size=10000) :
                    m_grow_size(grow_size),
                    m_elements(grow_size) {
                }

                ~SparseTable() override final = default;

                void set(const TId id, const TValue value) override final {
                    if (id >= m_elements.size()) {
                        m_elements.resize(id + m_grow_size);
                    }
                    m_elements[id] = value;
                }

                const TValue get(const TId id) const override final {
                    if (id >= m_elements.size()) {
                        not_found_error(id);
                    }
                    if (m_elements[id] == osmium::index::empty_value<TValue>()) {
                        not_found_error(id);
                    }
                    return m_elements[id];
                }

                size_t size() const override final {
                    return m_elements.size();
                }

                size_t used_memory() const override final {
                    // unused elements use 1 bit, used elements sizeof(TValue) bytes
                    // http://google-sparsehash.googlecode.com/svn/trunk/doc/sparsetable.html
                    return (m_elements.size() / 8) + (m_elements.num_nonempty() * sizeof(TValue));
                }

                void clear() override final {
                    m_elements.clear();
                }

                void dump_as_list(const int fd) const {
                    std::vector<std::pair<TId, TValue>> v;
                    int n=0;
                    for (const TValue value : m_elements) {
                        if (value != osmium::index::empty_value<TValue>()) {
                            v.emplace_back(n, value);
                        }
                        ++n;
                    }
                    osmium::io::detail::reliable_write(fd, reinterpret_cast<const char*>(v.data()), sizeof(std::pair<TId, TValue>) * v.size());
                }

            }; // class SparseTable

        } // namespace map

    } // namespace index

} // namespace osmium

#endif // OSMIUM_INDEX_BYID_SPARSE_TABLE_HPP