#pragma once

template<typename T>
struct Singleton: T
{
	static Singleton<T> &getInstance()
	{
		static Singleton<T> instance;

		return instance;
	}

	Singleton(): T() {}
	Singleton(const Singleton<T> &) = delete;
	Singleton<T> &operator =(const Singleton<T> &) = delete;
} ;
