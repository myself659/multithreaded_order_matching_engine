#ifndef _QUEUE_MPSC_H_
#define _QUEUE_MPSC_H_

#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>

#include <core/concurrent/lock.hpp>
#include <core/memory/aligned_container_policy.hpp>

namespace core
{

// Unbounded multi producer single consumer queue
template<typename T>
class QueueMPSC : public boost::noncopyable, AlignedContainerPolicy<T>
{
    public:

        struct QueueMPSCNode
        {
            T m_data;
            QueueMPSCNode* m_next;
            QueueMPSCNode() : m_next{nullptr}{}
        };

        QueueMPSC() : m_mutex("QueueMPSC")
        {
            // Create a new empty node so we avoid a lock for accessing head in enqueue
            QueueMPSCNode* dummy = new QueueMPSCNode;

            //Tail and head point to it
            m_head = m_tail = dummy;
        }

        ~QueueMPSC()
        {
            while (m_head)
            {
                auto temp = m_head;
                m_head = m_head->m_next;
                delete temp;
            }
        }

        void push(T data)
        {
            QueueMPSCNode* newNode = new QueueMPSCNode;
            newNode->m_data = data;
            std::unique_lock<core::Lock> l(m_mutex);
            /////////////////////////////
            m_tail->m_next = newNode;
            m_tail = newNode;
            /////////////////////////////
            l.unlock();
            m_noData.notify_one();
        }

        bool isEmpty()
        {
            bool retVal = true;
            std::lock_guard<core::Lock> l(m_mutex);
            if ( m_head->m_next != nullptr )
            {
                retVal = false;
            }
            return retVal;
        }

        QueueMPSCNode* flush()
        {
            QueueMPSCNode* ret = nullptr;

            if (isEmpty() == true)
            {
                return ret;
            }

            std::unique_lock<core::Lock> l(m_mutex);
            m_noData.wait(l, [this](){return m_head->m_next != nullptr; });
            /////////////////////////////
            //JUST SWAP THE POINTERS
            ret = m_head->m_next;
            m_head->m_next = nullptr;
            m_tail = m_head;
            /////////////////////////////
            l.unlock();

            return ret;
        }

    private:
        core::Lock m_mutex;
        std::condition_variable_any m_noData;

        QueueMPSCNode* m_head;
        QueueMPSCNode* m_tail;

        // Move ctor deletion
        QueueMPSC(QueueMPSC&& other) = delete;
        // Move assignment operator deletion
        QueueMPSC& operator=(QueueMPSC&& other) = delete;
};

}//namespace

#endif