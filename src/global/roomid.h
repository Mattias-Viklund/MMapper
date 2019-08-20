#pragma once
// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors
// Author: Nils Schimmelmann <nschimme@gmail.com> (Jahara)

#include <climits>
#include <cstdint>
#include <memory>
#include <set>
#include <vector>

struct RoomId final
{
private:
    uint32_t value = 0;

public:
    RoomId() = default;
    constexpr explicit RoomId(uint32_t value) noexcept
        : value{value}
    {}
    inline constexpr explicit operator uint32_t() const { return value; }
    inline constexpr uint32_t asUint32() const { return value; }
    inline constexpr bool operator<(RoomId rhs) const { return value < rhs.value; }
    inline constexpr bool operator>(RoomId rhs) const { return value > rhs.value; }
    inline constexpr bool operator<=(RoomId rhs) const { return value <= rhs.value; }
    inline constexpr bool operator>=(RoomId rhs) const { return value >= rhs.value; }
    inline constexpr bool operator==(RoomId rhs) const { return value == rhs.value; }
    inline constexpr bool operator!=(RoomId rhs) const { return value != rhs.value; }

public:
    friend inline uint32_t qHash(RoomId id) { return id.asUint32(); }
};
static constexpr const RoomId INVALID_ROOMID{UINT_MAX};
static constexpr const RoomId DEFAULT_ROOMID{0};

using RoomIdSet = std::set<RoomId>;

template<typename T>
class roomid_vector : private std::vector<T>
{
private:
    using base = std::vector<T>;

public:
    using std::vector<T>::vector;

public:
    auto operator[](RoomId roomId) -> decltype(auto) { return base::at(roomId.asUint32()); }
    auto operator[](RoomId roomId) const -> decltype(auto) { return base::at(roomId.asUint32()); }

public:
    using base::begin;
    using base::end;

public:
    using base::resize;
    using base::size;
};

class Room;
using RoomIndex = roomid_vector<std::shared_ptr<Room>>;

class RoomRecipient;
using RoomLocks = roomid_vector<std::set<RoomRecipient *>>;

class RoomCollection;
using SharedRoomCollection = std::shared_ptr<RoomCollection>;
using RoomHomes = roomid_vector<SharedRoomCollection>;
