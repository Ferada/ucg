/*
 * Copyright 2015-2016 Gary R. Van Sickle (grvs@users.sourceforge.net).
 *
 * This file is part of UniversalCodeGrep.
 *
 * UniversalCodeGrep is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU General Public License as published by the Free
 * Software Foundation.
 *
 * UniversalCodeGrep is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * UniversalCodeGrep.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file */

#ifndef SYNC_QUEUE_H_
#define SYNC_QUEUE_H_

#include <mutex>
#include <condition_variable>
#include <queue>


enum class queue_op_status
{
	success,
	empty,
	full,
	closed,
	busy,
	timeout,
	not_ready
};

/**
 * Simple unbounded synchronized queue class.
 *
 * The interface implemented here is compatible with Boost's sync_queue<> implementation
 * documented here: http://www.boost.org/doc/libs/1_59_0/doc/html/thread/sds.html#thread.sds.synchronized_queues.
 */
template <typename ValueType>
class sync_queue
{
public:
	sync_queue() : m_closed(false) {};
	~sync_queue() {};

	void close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		m_closed = true;

		// Notify any threads waiting on the queue's condition variable that it's just been closed.
		m_cv.notify_all();
	}

	queue_op_status wait_push(const ValueType& x)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		// Is the queue closed?
		if(m_closed)
		{
			// Yes, fail the push.
			return queue_op_status::closed;
		}

		// Push via copy.
		m_underlying_queue.push(x);

		// Notify any threads waiting on the queue's condition variable that it now has something to pull.
		m_cv.notify_all();

		return queue_op_status::success;
	}

	queue_op_status wait_push(ValueType&& x)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		// Is the queue closed?
		if(m_closed)
		{
			// Yes, fail the push.
			return queue_op_status::closed;
		}

		// Push via move.
		m_underlying_queue.push(std::move(x));

		// Notify any threads waiting on the queue's condition variable that it now has something to pull.
		m_cv.notify_all();

		return queue_op_status::success;
	}

	queue_op_status wait_pull(ValueType& x)
	{
		// Using a unique_lock<> here vs. a lock_guard<> because we'll be using a condition variable, which needs
		// to unlock the mutex.
		std::unique_lock<std::mutex> lock(m_mutex);

		if(m_underlying_queue.empty() && m_closed)
		{
			// Queue is empty and has been closed, let the caller know.
			return queue_op_status::closed;
		}

		// Wait until the queue is not empty, or somebody closes the sync_queue<>.
		m_cv.wait(lock, [this](){ return !m_underlying_queue.empty() || m_closed; });

		// Check if we've be awoken to a closed and empty queue.
		if(m_underlying_queue.empty() && m_closed)
		{
			// We have, let the caller know.
			return queue_op_status::closed;
		}

		// Otherwise, we have something in the queue to pull off.
		x = m_underlying_queue.front();
		m_underlying_queue.pop();

		return queue_op_status::success;
	}

	queue_op_status wait_pull(ValueType&& x)
	{
		// Using a unique_lock<> here vs. a lock_guard<> because we'll be using a condition variable, which needs
		// to unlock the mutex.
		std::unique_lock<std::mutex> lock(m_mutex);

		if(m_underlying_queue.empty() && m_closed)
		{
			// Queue is empty and has been closed, let the caller know.
			return queue_op_status::closed;
		}

		// Wait until the queue is not empty, or somebody closes the sync_queue<>.
		m_cv.wait(lock, [this](){ return !m_underlying_queue.empty() || m_closed; });

		// Check if we've be awoken to a closed and empty queue.
		if(m_underlying_queue.empty() && m_closed)
		{
			// We have, let the caller know.
			return queue_op_status::closed;
		}

		// Otherwise, we have something in the queue to pull off.
		// Use move-assignment vs. copy-assignment for efficiency.
		// Note that C++11 std::queue<>::front() returns a non-const reference as well as a const one, so
		// std::move() will work here.
		x = std::move(m_underlying_queue.front());
		m_underlying_queue.pop();

		return queue_op_status::success;
	}

private:

	std::mutex m_mutex;

	std::condition_variable m_cv;

	bool m_closed;

	std::queue<ValueType> m_underlying_queue;

};

#endif /* SYNC_QUEUE_H_ */
