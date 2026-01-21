// нужно, для умного вывода всех типов в консоли
#pragma once
#include <functional>
#include <memory>
#include <string>

struct ITypeHandler;

struct TypeEntry {
    std::string name;
    std::function<std::unique_ptr<ITypeHandler>()> factory;
};
