/*
 * fast_queue.hpp
 *
 *
 *  Created on: 7 sty 2023
 *      Author: kwarc
 */

#ifndef FAST_QUEUE_H_
#define FAST_QUEUE_H_


/* Single Producer - Single Consumer queue with no locks */
template<typename T, size_t N>
class fast_queue
{
public:
    fast_queue() : read_idx {0}, write_idx {0} {}
    ~fast_queue() {}

    bool empty() const
    {
        return this->read_idx == this->write_idx;
    }

    constexpr size_t size() const
    {
        return this->write_idx - this->read_idx;
    }

    constexpr size_t max_size() const
    {
        return N;
    }

    bool push(const T &element)
    {
        if (this->size() == N)
            return false;

        this->elements[this->write_idx++] = element;
        if (this->write_idx == (N + 1))
        	this->write_idx = 0;
        return true;
    }

    bool pop(T &element)
    {
        if (this->empty())
            return false;

        element = this->elements[this->read_idx++];
        if (this->read_idx == (N + 1))
        	this->read_idx = 0;
        return true;
    }

private:
    /* When read_idx == write_idx queue is empty, so storage should be +1 size */
    size_t read_idx, write_idx;
    T elements[N + 1];
};


#endif /* FAST_QUEUE_HPP_ */
