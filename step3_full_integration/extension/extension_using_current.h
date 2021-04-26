// Clone `https://github.com/c5t/current` alongside this repo to run this test.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

struct ExternalServiceSuper {
  ~ExternalServiceSuper() = default;
  virtual void WaitUntilDeleted() const = 0;
};

class ExternalService final {
 private:
  std::unique_ptr<ExternalServiceSuper> impl_;

 public:
  ExternalService(uint16_t port, std::function<void(const std::string&)> cb);
  void WaitUntilDeleted() const { impl_->WaitUntilDeleted(); }
};
