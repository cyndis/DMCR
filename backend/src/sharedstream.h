#ifndef SHARED_STREAM_H_
#define SHARED_STREAM_H_

#include <list>
#include <condition_variable>
#include <chrono>
#include <exception>

namespace dmcr {

/*
 * StreamTimeoutException is an exception class that is thrown if a timeout
 * was specified in SharedStream<T>::pull and has been reached. 
 */

class StreamTimeoutException : public std::exception
{
    public:
    const char* what() const throw() {
        return "Stream timeout occurred";
    };
};

/*  
 *  SharedStream<T> is an abstract data queue that enables multiple producers 
 *  to write to a common stream handle.
 *
 *  Call push(T)-method to write to the end of the stream and pull() to 
 *  retrieve them from the other end. An optional timeout parameter may be 
 *  specified.
*/

template <class T>
class SharedStream
{
public:

    // Push an instance of T to the back of the stream
    void push(const T& value)
    {
        {
            // Limit method re-entry
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push_back(value);
        }
        // Wake up one waiting listener
        m_cond.notify_one();
    }

    // A blocking call that waits for content to arrive
    // Optional parameter: maximum time duration to wait for
    // Default value is to wait forever
    template <class Rep=int, class Period=std::ratio<1> >
    T pull(
        const std::chrono::duration<Rep, Period>& duration
        = std::chrono::seconds(0))
    {
        // Limit method re-entry
        std::unique_lock<std::mutex> lock(m_mutex);

        // Predicate that determines whether the queue has content
        auto predicate = [this](){
            return !m_queue.empty();
        };
        // Wait until stream has content or for 'duration' time units if 
        // nonzero time given. Mutex is unlocked while waiting.
        if (duration == std::chrono::duration<Rep,Period>::zero())
            m_cond.wait(lock, predicate);
        else
        {
            if(!m_cond.wait_for(lock, duration, predicate))
            {
                // Timeout, nothing to do here!
                throw StreamTimeoutException();
            }
        }

        // Mutex is now locked again
        // Pop content like a boss
        T ret = std::move(m_queue.front()); 
        m_queue.pop_front();
        return ret;
    }

    // Enable normal construction, move construction and move assignment
    SharedStream() = default;
    SharedStream(SharedStream&& other) = default;
    SharedStream& operator=(SharedStream&& other) = default;

    // Prohibit copying and assignment
    SharedStream(const SharedStream&) = delete; 
    SharedStream & operator=(const SharedStream&) = delete;

private:
    
    std::mutex              m_mutex;
    std::condition_variable m_cond;
    std::list<T>            m_queue;
};


};

#endif

