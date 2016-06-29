
#ifndef SFRAME_RING_QUEUE_H
#define SFRAME_RING_QUEUE_H

#include <inttypes.h>
#include "Lock.h"
#include "Singleton.h"

namespace sframe {

// ���ζ���
template<typename T, int Queue_Capacity>
class RingQueue : public noncopyable
{
public:
	RingQueue() : _head(0), _tail(0), _len(0) { }

	~RingQueue() {}

	// ѹ��
	bool Push(const T & val)
	{
		if (_len >= Queue_Capacity)
		{
			return false;
		}

		_queue[_tail] = val;
		_tail = (_tail + 1) % Queue_Capacity;
		_len++;

		return true;
	}

	// ����
	bool Pop(T * val)
	{
		if (_len <= 0)
		{
			return false;
		}

		if (val)
		{
			(*val) = _queue[_head];
		}
		_head = (_head + 1) % Queue_Capacity;
		_len--;

		return true;
	}

private:
	T _queue[Queue_Capacity];
	int32_t _head;
	int32_t _tail;
	int32_t _len;
};

}

#endif
