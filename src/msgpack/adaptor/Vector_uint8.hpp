//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2014-2015 KONDO Takatoshi
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_TYPE_SESAME_VECTOR_UINT8_HPP
#define MSGPACK_TYPE_SESAME_VECTOR_UINT8_HPP

#include "msgpack/versioning.hpp"
#include "msgpack/adaptor/adaptor_base.hpp"
#include "msgpack/adaptor/check_container_size.hpp"

#include <vector>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <>
struct convert<Vector<uint8_t> > {
    msgpack::object const& operator()(msgpack::object const& o, Vector<uint8_t>& v) const {
        switch (o.type) {
        case msgpack::type::BIN:
            v.resize(o.via.bin.size);
            std::memcpy(&v.front(), o.via.bin.ptr, o.via.bin.size);
            break;
        case msgpack::type::STR:
            v.resize(o.via.str.size);
            std::memcpy(&v.front(), o.via.str.ptr, o.via.str.size);
            break;
        default:
            throw msgpack::type_error();
            break;
        }
        return o;
    }
};

template <>
struct pack<Vector<uint8_t> > {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const Vector<uint8_t>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_bin(size);
        o.pack_bin_body(reinterpret_cast<const char*>(&v.front()), size);

        return o;
    }
};

template <>
struct object<Vector<uint8_t> > {
    void operator()(msgpack::object& o, const Vector<uint8_t>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::BIN;
        o.via.bin.ptr = reinterpret_cast<const char*>(&v.front());
        o.via.bin.size = size;
    }
};

template <>
struct object_with_zone<Vector<uint8_t> > {
    void operator()(msgpack::object::with_zone& o, const Vector<uint8_t>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::BIN;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size));
        o.via.bin.ptr = ptr;
        o.via.bin.size = size;
        std::memcpy(ptr, &v.front(), size);
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace msgpack

#endif // MSGPACK_TYPE_SESAME_VECTOR_UINT8_HPP
