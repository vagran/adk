/* /ADK/include/adk/bitmap.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file bitmap.h
 * Bitmap class implementation.
 */

#ifndef BITMAP_H_
#define BITMAP_H_

namespace adk {

namespace adk_internal {

/** Base class for bitmap implementation. Depending on template parameters it
 * provides either static or dynamically allocated bitmap storage.
 */
template <size_t numBits, class Allocator = std::allocator<int>,
          typename word_t = long>
class BitmapBase
{
protected:
    /** Return number of bits in the bitmap. */
    static constexpr size_t
    NumBits()
    {
        return numBits;
    }

    /** Return number of words in bitmap storage. */
    static constexpr size_t
    NumWords()
    {
        return RoundUp2(numBits, sizeof(word_t) * NBBY) / (sizeof(word_t) * NBBY);
    }

    /** Bitmap bits storage. */
    word_t _bits[NumWords()];

    BitmapBase()
    {
        memset(_bits, 0, sizeof(_bits));
    }
};

template <class Allocator, typename word_t>
class BitmapBase<-1, Allocator, word_t>
{
private:
    /** Number of bits in the bitmap. */
    size_t _numBits;
    /** Allocator for bitmap storage. */
    typedef typename Allocator::template rebind<word_t>::other WordAllocator;
    WordAllocator _wordAlloc;
protected:
    /** Bitmap bits storage. */
    word_t *_bits;

    /** Return number of bits in the bitmap. */
    size_t
    NumBits() const
    {
        return _numBits;
    }

    /** Return number of words in bitmap storage. */
    size_t
    NumWords() const
    {
        return RoundUp2(_numBits, sizeof(word_t) * NBBY) / (sizeof(word_t) * NBBY);
    }

    BitmapBase(size_t numBits): _numBits(numBits)
    {
        _bits = _wordAlloc.allocate(NumWords());
        memset(_bits, 0, NumWords() * sizeof(word_t));
    }

    ~BitmapBase()
    {
        _wordAlloc.deallocate(_bits, NumWords());
    }
};

} /* namespace adk_internal */

/** Class representing bitmap - set of bits.
 * @param numBits Number of bits in a bitmap. Use -1 to create dynamic bitmap
 *      if number of bits is not known at compile time. In such case run-time
 *      value of number of bits is passed to the class constructor.
 * @param Allocator Allocator class when using dynamic bitmap.
 * @param word_t Native word type to use for optimized bits operations.
 */
template <size_t numBits, class Allocator = std::allocator<int>, typename word_t = long>
class Bitmap: public adk_internal::BitmapBase<numBits, Allocator, word_t> {
private:
    /** Base class type. */
    typedef adk_internal::BitmapBase<numBits, Allocator, word_t> BaseT;

    /** Get index of word containing the specified bit. */
    static constexpr size_t
    _GetWordIdx(size_t bitIdx)
    {
        return bitIdx / (sizeof(word_t) * NBBY);
    }

    /** Get mask of word with the specified bit. */
    static constexpr word_t
    _GetMask(size_t bitIdx)
    {
        return static_cast<word_t>(1) << (bitIdx % (sizeof(word_t) * NBBY));
    }

public:
    Bitmap(): BaseT() {}

    Bitmap(size_t numDynamicBits): BaseT(numDynamicBits) {}

    /** Set bit with specified index. */
    void
    Set(size_t bitIdx)
    {
        ASSERT(bitIdx < BaseT::NumBits());
        BaseT::_bits[_GetWordIdx(bitIdx)] |= _GetMask(bitIdx);
    }

    /** Clear bit with specified index. */
    void
    Clear(size_t bitIdx)
    {
        ASSERT(bitIdx < BaseT::NumBits());
        BaseT::_bits[_GetWordIdx(bitIdx)] &= ~_GetMask(bitIdx);
    }

    /** Set or clear bit with specified index.
     *
     * @param bitIdx Index of bit to change.
     * @param value @a true to set, @a false to clear.
     */
    void
    SetValue(size_t bitIdx, bool value)
    {
        ASSERT(bitIdx < BaseT::NumBits());
        if (value) {
            Set(bitIdx);
        } else {
            Clear(bitIdx);
        }
    }

    /** Check if bit is set at specified position.
     *
     * @param bitIdx Null based bit index.
     * @return @a true if the bit is set, @a false otherwise.
     */
    bool
    IsSet(size_t bitIdx) const
    {
        ASSERT(bitIdx < BaseT::NumBits());
        return BaseT::_bits[_GetWordIdx(bitIdx)] & _GetMask(bitIdx);
    }

    /** Check if bit is clear at specified position.
     *
     * @param bitIdx Null based bit index.
     * @return @a true if the bit is clear, @a false otherwise.
     */
    bool
    IsClear(size_t bitIdx) const
    {
        ASSERT(bitIdx < BaseT::NumBits());
        return !(BaseT::_bits[_GetWordIdx(bitIdx)] & _GetMask(bitIdx));
    }

    /** Toggle bit with specified index. */
    void
    Toggle(size_t bitIdx)
    {
        ASSERT(bitIdx < BaseT::NumBits());
        BaseT::_bits[_GetWordIdx(bitIdx)] ^= _GetMask(bitIdx);
    }

    bool
    operator[](size_t bitIdx) const
    {
        ASSERT(bitIdx < BaseT::NumBits());
        return IsSet(bitIdx);
    }

    /** Clear all bits in the string. */
    void
    ClearAll()
    {
        memset(BaseT::_bits, 0, BaseT::NumWords() * sizeof(word_t));
    }

    /** Clear all bits in the string. */
    void
    SetAll() {
        memset(BaseT::_bits, 0xff, BaseT::NumWords() * sizeof(word_t));
    }

    /** Invert all bits in the string. */
    void
    Invert()
    {
        for (size_t wordIdx = 0;
             wordIdx < BaseT::NumWords() * sizeof(word_t);
             wordIdx++) {

            BaseT::_bits[wordIdx] ^= ~static_cast<word_t>(0);
        }
    }

    /** Get first set bit index.
     *
     * @return First set bit index, -1 if set bit not found.
     */
    size_t
    FirstSet() const
    {
        for (size_t wordIdx = 0; wordIdx < BaseT::NumWords(); wordIdx++) {
            word_t w = BaseT::_bits[wordIdx];
            if (!w) {
                continue;
            }
            size_t baseIdx = wordIdx * sizeof(word_t) * NBBY;
            for (size_t bitIdx = 0; baseIdx + bitIdx < BaseT::NumBits(); bitIdx++) {
                if (w & _GetMask(bitIdx)) {
                    return baseIdx + bitIdx;
                }
            }
        }
        return -1;
    }

    /** Get first cleared bit index.
     *
     * @return First cleared bit index, -1 if set bit not found.
     */
    size_t
    FirstClear() const
    {
        for (size_t wordIdx = 0; wordIdx < BaseT::NumWords(); wordIdx++) {
            word_t w = BaseT::_bits[wordIdx];
            if (w == ~static_cast<word_t>(0)) {
                continue;
            }
            size_t baseIdx = wordIdx * sizeof(word_t) * NBBY;
            for (size_t bitIdx = 0; baseIdx + bitIdx < BaseT::NumBits(); bitIdx++) {
                if (!(w & _GetMask(bitIdx))) {
                    return baseIdx + bitIdx;
                }
            }
        }
        return -1;
    }
};

} /* namespace adk */

#endif /* BITMAP_H_ */
