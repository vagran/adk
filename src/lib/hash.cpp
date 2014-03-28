/* /ADK/src/lib/hash.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file hash.cpp
 * Hash class implementation.
 */

#include <adk.h>

using namespace adk;

void
Hash::Mix(u32 &a, u32 &b, u32 &c)
{
    a -= c; a ^= RotL(c, 4);    c += b;
    b -= a; b ^= RotL(a, 6);    a += c;
    c -= b; c ^= RotL(b, 8);    b += a;
    a -= c; a ^= RotL(c, 16);   c += b;
    b -= a; b ^= RotL(a, 19);   a += c;
    c -= b; c ^= RotL(b, 4);    b += a;
}

void
Hash::Final(u32 &a, u32 &b, u32 &c)
{
    c ^= b; c -= RotL(b, 14);
    a ^= c; a -= RotL(c, 11);
    b ^= a; b -= RotL(a, 25);
    c ^= b; c -= RotL(b, 16);
    a ^= c; a -= RotL(c, 4);
    b ^= a; b -= RotL(a, 14);
    c ^= b; c -= RotL(b, 24);
}

void
Hash::_Finalize(u32 &a, u32 &b, u32 &c)
{
    a = _a + _length;
    b = _b + _length;
    c = _c + _length;
    Final(a, b, c);
}

void
Hash::Feed(const void *data, size_t size)
{
    const u8 *key = static_cast<const u8 *>(data);

    /* Fill remainder. */
    while (_resid) {
        switch (_resid) {
        case 1:
            _a += static_cast<u32>(*key) << 8;
            break;
        case 2:
            _a += static_cast<u32>(*key) << 16;
            break;
        case 3:
            _a += static_cast<u32>(*key) << 24;
            break;
        case 4:
            _b += static_cast<u32>(*key);
            break;
        case 5:
            _b += static_cast<u32>(*key) << 8;
            break;
        case 6:
            _b += static_cast<u32>(*key) << 16;
            break;
        case 7:
            _b += static_cast<u32>(*key) << 24;
            break;
        case 8:
            _c += static_cast<u32>(*key);
            break;
        case 9:
            _c += static_cast<u32>(*key) << 8;
            break;
        case 10:
            _c += static_cast<u32>(*key) << 16;
            break;
        case 11:
            _c += static_cast<u32>(*key) << 24;
            break;
        }
        key++;
        _length++;
        _resid++;
        size--;
        if (_resid == 12) {
            Mix(_a, _b, _c);
            _resid = 0;
        }
        if (!size) {
            return;
        }
    }

    /* Apply main part. */
    while (size >= 12) {
        _a += GetUnaligned<u32>(&key[0]);
        _b += GetUnaligned<u32>(&key[4]);
        _c += GetUnaligned<u32>(&key[8]);
        Mix(_a, _b, _c);
        key += 12;
        _length += 12;
        size -= 12;
    }

    /* Apply trailing remainder. */
    switch (size) {
    case 11:
        _c += static_cast<u32>(key[10]) << 16;
    case 10:
        _c += static_cast<u32>(key[9]) << 8;
    case 9:
        _c += static_cast<u32>(key[8]);
    case 8:
        _b += static_cast<u32>(key[7]) << 24;
    case 7:
        _b += static_cast<u32>(key[6]) << 16;
    case 6:
        _b += static_cast<u32>(key[5]) << 8;
    case 5:
        _b += static_cast<u32>(key[4]);
    case 4:
        _a += static_cast<u32>(key[3]) << 24;
    case 3:
        _a += static_cast<u32>(key[2]) << 16;
    case 2:
        _a += static_cast<u32>(key[1]) << 8;
    case 1:
        _a += static_cast<u32>(key[0]);
    }
    _resid = size;
    _length += size;
}
