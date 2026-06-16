#pragma once
#include <expected>
#include <format>

namespace phys
{

void showMessage(std::unexpected<std::string> expected);
void showMessage(const char *msg);

template <typename... Args> void showMessageF(std::format_string<Args...> msg, Args &&...args)
{
    auto msgFormat = std::format(msg, std::forward<Args>(args)...);
    showMessage(msgFormat.c_str());
}

} // namespace phys
