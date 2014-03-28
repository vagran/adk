/* /ADK/include/adk/hash.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file hash.h
 * Hash class declarations.
 */

#ifndef HASH_H_
#define HASH_H_

namespace adk {

/** Lookup (non-cryptographic) hash calculation class. It is based on Bob
 * Jenkins hash algorithm (lookup3).
 */
class Hash {
public:
    /** Create hash object.
     *
     * @param initial Initial value for a hash. It can be used as salt.
     */
    Hash(u32 initial = 0)
    {
        _a = _b = _c = INITIAL_VALUE + initial;
    }

    /** Reset the calculator state to the initial one.
     *
     * @param initial Initial value for a hash. It can be used as salt.
     */
    void
    Reset(u32 initial = 0)
    {
        _a = _b = _c = INITIAL_VALUE + initial;
        _resid = 0;
        _length = 0;
    }

    /** Apply hash reversible mix operation to three 32-bits words.
     * @param a The first word.
     * @param b The second word.
     * @param c The third word.
     */
    static void
    Mix(u32 &a, u32 &b, u32 &c);

    /** Final mixing of three 32-bits words.
     *
     * @param a The first word.
     * @param b The second word.
     * @param c The third word. Result is returned in this argument.
     */
    static void
    Final(u32 &a, u32 &b, u32 &c);

    /** Feed input data to hash calculator. This method can be called any number
     * of times providing next portion of input data. At any time @ref Get32 or
     * @ref Get64 methods can be called to get current value of hash.
     *
     * @param data Next portion of input data.
     * @param size Size in bytes of provided data.
     */
    void
    Feed(const void *data, size_t size);

    /** Get 32-bits hash value based on data fed so far.
     *
     * @return 32-bits hash value.
     */
    u32
    Get32()
    {
        u32 a, b, c;
        _Finalize(a, b, c);
        return c;
    }

    /** Get 64-bits hash value based on data fed so far.
     *
     * @return 64-bits hash value.
     */
    u64
    Get64()
    {
        u32 a, b, c;
        _Finalize(a, b, c);
        return (static_cast<u64>(b) << 32) | c;
    }

    /** Get 32-bits hash value based on data fed so far. This operation is not
     * destructive - data still can be fed on input to get hash values
     * incrementally.
     *
     * @return 32-bits hash value.
     */
    operator u32()
    {
        return Get32();
    }

    /** Get 64-bits hash value based on data fed so far.
     *
     * @return 64-bits hash value.
     */
    operator u64()
    {
        return Get64();
    }

    /** Get total length of data fed to the calculator input so far.
     *
     * @return Total length of data fed to the calculator input so far
     */
    size_t
    GetLength()
    {
        return _length;
    }

private:
    enum {
        /** Initial value for accumulators based on golden ratio. */
        INITIAL_VALUE = 0x9e3779b8,
    };
    /** Hash calculation accumulators. */
    u32 _a, _b, _c;
    /** When non-full 96 bits chunk is read some accumulators are partially
     * affected. This value indicates how many bytes were applied to the
     * accumulators. When it reaches 12 they mixed and this counter is zeroed.
     */
    size_t _resid = 0;
    /** Total length of data consumed by the calculator so far. */
    size_t _length = 0;

    /** Finalize hash calculation based on current calculator state. Arguments
     * should not be initialized - the method returns result in them. The
     * current state of the calculator is not affected so the calculation still
     * can continue after that.
     *
     * @param a The first value for storing result.
     * @param b The second value for storing result.
     * @param c The third value for storing result.
     */
    void
    _Finalize(u32 &a, u32 &b, u32 &c);
};

} /* namespace adk */

#endif /* HASH_H_ */
