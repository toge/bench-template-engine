#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "frozenchars.hpp"
#include "glaze/glaze.hpp"
#include <nlohmann/json.hpp>

using namespace frozenchars::literals;
namespace fop = frozenchars::ops;

struct User {
  std::string name;
  std::string email;
  int age;
};

struct UserData {
  std::vector<User> users;
  std::string base_url;
  std::string param1_key;
  std::string param1_val;
  std::string param2_key;
  std::string param2_val;
  std::string param3_key;
  std::string param3_val;
  std::string param4_key;
  std::string param4_val;
  std::string param5_key;
  std::string param5_val;
  std::string title;
  std::string description;
  std::string link1_text;
  std::string link1_url;
  std::string link2_text;
  std::string link2_url;
  std::string link3_text;
  std::string link3_url;
};

struct ConfigData {
  std::vector<std::pair<std::string, std::string>> entries;
};

[[nodiscard]] inline auto make_sample_users() -> std::vector<User> {
  return {
    {"Alice Smith", "alice@example.com", 30},
    {"Bob Johnson", "bob@example.com", 25},
    {"Carol Williams", "carol@example.com", 35},
    {"David Brown", "david@example.com", 28},
    {"Eve Davis", "eve@example.com", 32},
    {"Frank Miller", "frank@example.com", 40},
    {"Grace Wilson", "grace@example.com", 27},
    {"Henry Moore", "henry@example.com", 45},
    {"Ivy Taylor", "ivy@example.com", 22},
    {"Jack Anderson", "jack@example.com", 38},
  };
}

[[nodiscard]] inline auto make_sample_userdata() -> UserData {
  return UserData{
    .users = make_sample_users(),
    .base_url = "https://example.com/api/search",
    .param1_key = "q",
    .param1_val = "hello world",
    .param2_key = "page",
    .param2_val = "1",
    .param3_key = "limit",
    .param3_val = "10",
    .param4_key = "sort",
    .param4_val = "name",
    .param5_key = "order",
    .param5_val = "asc",
    .title = "Sample Document",
    .description = "This is a sample document for benchmarking.",
    .link1_text = "Home",
    .link1_url = "https://example.com/",
    .link2_text = "About",
    .link2_url = "https://example.com/about",
    .link3_text = "Contact",
    .link3_url = "https://example.com/contact",
  };
}

[[nodiscard]] inline auto make_sample_config() -> ConfigData {
  return ConfigData{
    .entries = {
      {"server.host", "localhost"},
      {"server.port", "8080"},
      {"database.driver", "sqlite3"},
      {"database.path", "/data/app.db"},
      {"cache.enabled", "true"},
      {"cache.ttl", "3600"},
      {"log.level", "info"},
      {"log.file", "/var/log/app.log"},
      {"auth.secret", "changeme"},
      {"auth.token_ttl", "86400"},
    }
  };
}

namespace bench {

template <typename T>
void DoNotOptimize(T const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

} // namespace bench
