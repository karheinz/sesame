//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
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
#ifndef MSGPACK_TYPE_SESAME_STRING_HPP
#define MSGPACK_TYPE_SESAME_STRING_HPP

#include "types.hpp"
#include "msgpack/versioning.hpp"
#include "msgpack/adaptor/adaptor_base.hpp"
#include "msgpack/adaptor/check_container_size.hpp"
#include "sesame/utils/string.hpp"

#include <string>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <>
struct convert<String> {
    msgpack::object const& operator()(msgpack::object const& o, String& v) const {
        switch (o.type) {
        case msgpack::type::BIN:
            v.assign(o.via.bin.ptr, o.via.bin.size);
            break;
        case msgpack::type::STR:
            v.assign(o.via.str.ptr, o.via.str.size);
            break;
        default:
            throw msgpack::type_error();
            break;
        }

        v = sesame::utils::fromUtf8( v );
        return o;
    }
};

template <>
struct pack<String> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const String& s) const {
        String v( sesame::utils::toUtf8( s ) );
        uint32_t size = checked_get_container_size(v.size());
        o.pack_str(size);
        o.pack_str_body(v.data(), size);
        return o;
    }
};

template <>
struct object<String> {
    void operator()(msgpack::object& o, const String& s) const {
        String v( sesame::utils::toUtf8( s ) );
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::STR;
        o.via.str.ptr = v.data();
        o.via.str.size = size;
    }
};

template <>
struct object_with_zone<String> {
    void operator()(msgpack::object::with_zone& o, const String& s) const {
        String v( sesame::utils::toUtf8( s ) );
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::STR;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size));
        o.via.str.ptr = ptr;
        o.via.str.size = size;
        std::memcpy(ptr, v.data(), v.size());
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace msgpack

#endif // MSGPACK_TYPE_SESAME_STRING_HPP
