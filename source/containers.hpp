#pragma once

#include "types.hpp"
#include <cstring>

template <typename T>
struct Array
{
    inline static T Empty = {};

    T *data;
    u64 len;
    u64 cap;

    Array<T>() { WARN("Empty array initialized"); }
    Array<T>(T *_data, u64 _len, u64 _cap) : data{_data}, len{_len}, cap{_cap} {}

    ~Array<T>() = default;

    static Array<T> New(u64 size)
    {
        return Array<T>((T *)malloc(size), 0, size);
    }

    void Push(T &element)
    {
        if (len >= cap)
        {
            WARN("Array length exceeded");
            return;
        }

        data[len] = element;
        len++;
    }

    void Push(T const &element)
    {
        if (len >= cap)
        {
            return;
        }

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
        {
            WARN("Array empty");
            return Array<T>::Empty;
        }

        len -= 1;
        return data[len];
    }

    T &Last()
    {
        if (len == 0)
        {
            WARN("Array empty");
            return Array<T>::Empty;
        }

        return data[len - 1];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
        {
            WARN("Array length exceeded");
            return Array<T>::Empty;
        }
        return data[id];
    }

    // -------- Iterator support --------
    T *begin() { return data; }
    T *end() { return data + len; }

    const T *begin() const { return data; }
    const T *end() const { return data + len; }
};

struct String : Array<u8>
{
    operator cstr() { return (cstr)(this->data); }
    String(cstr str) : Array<u8>((u8 *)(str), strlen(str), len) {}
};

template <typename T, unsigned int N>
struct StackArray
{
    T data[N];
    u64 len;
    u64 cap = N;

    void Push(T &element)
    {
        if (len >= N)
        {
            WARN("StackArray length exceeded");
            return;
        }

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
        {
            WARN("StackArray is empty");
            return data[0];
        }

        len -= 1;
        return data[len];
    }

    T &Last()
    {
        if (len == 0)
        {
            WARN("StackArray is empty");
            return data[0];
        }

        return data[len - 1];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
        {
            WARN("StackArray is empty");
            return data[0];
        }

        return data[id];
    }

    // -------- Iterator support --------
    T *begin() { return data; }
    T *end() { return data + len; }

    const T *begin() const { return data; }
    const T *end() const { return data + len; }
};