/*
 * Copyright (c) 2018-2019 Confetti Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
*/
#ifdef __ANDROID__

#include <assert.h>
#include "../Interfaces/IThread.h"
#include "../Interfaces/IOperatingSystem.h"
#include "../Interfaces/ILogManager.h"
#include "../Interfaces/IMemoryManager.h"

#include <pthread.h>
#include <unistd.h>

AtomicUint::AtomicUint()
{
    mAtomicInt = 0;
}

AtomicUint::~AtomicUint()
{

}

void AtomicUint::AtomicStore(unsigned int new_val)
{
    int origin_val ;
    origin_val = mAtomicInt ;
    do { origin_val = __sync_val_compare_and_swap ((volatile unsigned int*)&mAtomicInt, origin_val, new_val);} while(origin_val != new_val);
}

unsigned int AtomicUint::AtomicIncrement()
{
    return __sync_fetch_and_add((volatile unsigned int*)&mAtomicInt, 1);
}

unsigned int AtomicUint::AtomicDecrement()
{
    return __sync_fetch_and_add((volatile unsigned int*)&mAtomicInt, -1);
}


Mutex::Mutex() { pHandle = PTHREAD_MUTEX_INITIALIZER; }

Mutex::~Mutex() { pthread_mutex_destroy(&pHandle); }

void Mutex::Acquire() { pthread_mutex_lock(&pHandle); }

void Mutex::Release() { pthread_mutex_unlock(&pHandle); }

void* ThreadFunctionStatic(void* data)
{
	ThreadDesc* pItem = static_cast<ThreadDesc*>(data);
	pItem->pFunc(pItem->pData);
	return 0;
}

ConditionVariable::ConditionVariable()
{
	pHandle = PTHREAD_COND_INITIALIZER;
	int res = pthread_cond_init(&pHandle, NULL);
	assert(res == 0);
}

ConditionVariable::~ConditionVariable() { pthread_cond_destroy(&pHandle); }

void ConditionVariable::Wait(const Mutex& mutex)
{
	pthread_mutex_t* mutexHandle = (pthread_mutex_t*)&mutex.pHandle;
	pthread_cond_wait(&pHandle, mutexHandle);
}

void ConditionVariable::Wait(const Mutex& mutex, unsigned int ms)
{
	timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ms * 1000;

	pthread_mutex_t* mutexHandle = (pthread_mutex_t*)&mutex.pHandle;
	pthread_cond_timedwait(&pHandle, mutexHandle, &ts);
}

void ConditionVariable::Set() { pthread_cond_signal(&pHandle); }

void ConditionVariable::SetAll() { pthread_cond_broadcast(&pHandle); }

ThreadID Thread::mainThreadID;

void Thread::SetMainThread() { mainThreadID = GetCurrentThreadID(); }

ThreadID Thread::GetCurrentThreadID() { return pthread_self(); }

bool Thread::IsMainThread() { return GetCurrentThreadID() == mainThreadID; }

ThreadHandle _createThread(ThreadDesc* pData)
{
	pthread_t handle;
	pthread_create(&handle, NULL, ThreadFunctionStatic, pData);
	return (ThreadHandle)handle;
}

void _destroyThread(ThreadHandle handle)
{
	ASSERT(handle != (long)NULL);
	// Wait for thread to join, need to make sure thread stops running otherwise it is not properly destroyed
	pthread_join(handle, NULL);
}

ThreadHandle create_thread(ThreadDesc* pData)
{
	pthread_t handle;
	pthread_create(&handle,NULL,ThreadFunctionStatic,pData);
	return (ThreadHandle)handle;
}

void destroy_thread(ThreadHandle handle)
{
	ASSERT(handle!= (long)NULL);
	// Wait for thread to join, need to make sure thread
	//stops running otherwise it is not properly destroyed
	pthread_join(handle, NULL);
}

void Thread::Sleep(unsigned mSec)
{
	  usleep(mSec*1000);
}

// threading class (Static functions)
unsigned int Thread::GetNumCPUCores(void)
{
	unsigned int ncpu;
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
	return ncpu;
}
#endif    //if __ANDROID__