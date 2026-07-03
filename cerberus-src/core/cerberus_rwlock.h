#pragma once

#include <pthread.h>

class RWLock
{
public:
	RWLock()  { pthread_rwlock_init(&rwlock_, nullptr); }
	~RWLock() { pthread_rwlock_destroy(&rwlock_); }

	void read_lock()    { pthread_rwlock_rdlock(&rwlock_); }
	void read_unlock()  { pthread_rwlock_unlock(&rwlock_); }
	void write_lock()   { pthread_rwlock_wrlock(&rwlock_); }
	void write_unlock() { pthread_rwlock_unlock(&rwlock_); }

private:
	pthread_rwlock_t rwlock_;
	RWLock(const RWLock&);
	RWLock& operator=(const RWLock&);
};

class ReadLockGuard
{
public:
	explicit ReadLockGuard(RWLock& rwlock) : rwlock_(rwlock) { rwlock_.read_lock(); }
	~ReadLockGuard() { rwlock_.read_unlock(); }
private:
	RWLock& rwlock_;
	ReadLockGuard(const ReadLockGuard&);
	ReadLockGuard& operator=(const ReadLockGuard&);
};

class WriteLockGuard
{
public:
	explicit WriteLockGuard(RWLock& rwlock) : rwlock_(rwlock) { rwlock_.write_lock(); }
	~WriteLockGuard() { rwlock_.write_unlock(); }
private:
	RWLock& rwlock_;
	WriteLockGuard(const WriteLockGuard&);
	WriteLockGuard& operator=(const WriteLockGuard&);
};
