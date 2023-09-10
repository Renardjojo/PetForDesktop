#pragma once

#include <string>
#include <unordered_map>

class ResourcesManager
{
protected:
    std::unordered_map<std::string, T> m_resources;

public:

    T* get(const std::string& key) noexcept
    {
        auto it = m_resources.find(key);
        if (it == m_resources.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    const T* get(const std::string& key) const noexcept
    {
        auto it = m_resources.find(key);
        if (it == m_resources.end())
        {
            Log::getInstance()->logWarning(stringFormat("Resource insert with key \"%s\" doesn't exist", key.c_str()));
            return nullptr;
        }

        return &it->second;
    }

    std::unordered_map<std::string, T>& getAll() noexcept
    {
        return m_resources;
    }

    const std::string* getKey(const T* data) const noexcept
    {
        for (auto&& res : m_resources)
        {
            if (&res.second == data)
                return &res.first;
        }
        return nullptr;
    }

    template <typename... Args>
    T& add(const std::string& key, Args&&... args) noexcept(std::is_nothrow_constructible_v<T>)
    {
        auto rst = m_resources.try_emplace(key, std::forward<Args>(args)...);
        return rst.first->second;
    }

    void remove(const std::string& key) noexcept(std::is_nothrow_destructible_v<T>)
    {
        m_resources.erase(key);
    }

    void clear() noexcept(std::is_nothrow_destructible_v<T>)
    {
        m_resources.clear();
    }
};