#pragma once
#include <Windows.h>

class CCSetion
{
    CRITICAL_SECTION m_cs;

public:
    CCSetion()
    {
        InitializeCriticalSection(&m_cs);
    }

    ~CCSetion()
    {
        DeleteCriticalSection(&m_cs);
    }

    void Lock()
    {
        EnterCriticalSection(&m_cs);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_cs);
    }
};

class CAutoLockCS
{
    CCSetion& m_Lock;

public:
    explicit CAutoLockCS(CCSetion& cs) : m_Lock(cs)
    {
        m_Lock.Lock();
    }

    ~CAutoLockCS()
    {
        m_Lock.Unlock();
    }
};