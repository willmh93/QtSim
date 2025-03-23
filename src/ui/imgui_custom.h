#pragma once
#include <unordered_map>

//#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

struct StartValue
{
    void* initial;
    int size;
};

struct ImInputManager
{
    static std::unordered_map<void*, StartValue> starting_map;

    static void updateWithStartingValues()
    {
        for (auto& item : ImInputManager::starting_map)
            memcpy(item.first, item.second.initial, item.second.size);
    }
};

template<typename T>
T* start(T* ptr)
{
    if (ImInputManager::starting_map.count(ptr) == 0)
    {
        ImInputManager::starting_map[ptr] = { new T(*ptr), sizeof(T) };
    }
    return reinterpret_cast<T*>(ImInputManager::starting_map[ptr].initial);
}