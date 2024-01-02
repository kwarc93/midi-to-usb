/*
 * fast_queue.hpp
 *
 *
 *  Created on: 7 sty 2023
 *      Author: kwarc
 */

#ifndef FAST_QUEUE_H_
#define FAST_QUEUE_H_


/* Single producer - single consumer queue with no locks. N must be power of 2. */
template<typename T, uint8_t N>
class fast_queue
{
public:
    fast_queue() : read_idx {0}, write_idx {0} {}
    ~fast_queue() {}

    bool empty() const
    {
        return this->read_idx == this->write_idx;
    }

    constexpr uint8_t size() const
    {
        return this->write_idx - this->read_idx;
    }

    constexpr uint8_t max_size() const
    {
    	/* When read_idx == write_idx queue is empty, so storage is one less than N */
        return N - 1;
    }

    bool push(const T &element)
    {
        if (this->size() == this->max_size())
            return false;

        this->elements[this->write_idx++] = element;
        this->write_idx &= this->wrap_mask;
        return true;
    }

    bool pop(T &element)
    {
        if (this->empty())
            return false;

        element = this->elements[this->read_idx++];
        this->read_idx &= this->wrap_mask;
        return true;
    }

private:
    uint8_t read_idx, write_idx;
    T elements[N];
    static constexpr uint8_t wrap_mask = N - 1;

    static_assert(N && ((N & (N - 1)) == 0), "N should be power of 2");
};


#endif /* FAST_QUEUE_HPP_ */
