// Clone `https://github.com/c5t/current` alongside this repo to run this test.

#include "extension_using_current.h"

#include "../../../current/blocks/http/api.h"
#include "../../../current/bricks/sync/waitable_atomic.h"

struct ExternalServiceImpl final : ExternalServiceSuper {
  std::function<void(const std::string&)> callback_;
  current::WaitableAtomic<bool> deleted_{false};
  HTTPRoutesScope scope_;

  ExternalServiceImpl(uint16_t port, std::function<void(const std::string&)> cb)
      : callback_(cb), scope_(HTTP(static_cast<current::net::BarePort>(port)).Register("/", [this](Request r) {
          std::ostringstream os;
          os << r.method;
          if (!r.body.empty()) {
            os << ": " << r.body;
          }
          std::string s = os.str();
          callback_(s);
          s += '\n';
          r(s);
          if (r.method == "DELETE") {
            *deleted_.MutableScopedAccessor() = true;
          }
        })) {}

  void WaitUntilDeleted() const override {
    deleted_.Wait([](bool deleted) { return deleted; });
  }
};

ExternalService::ExternalService(uint16_t port, std::function<void(const std::string&)> cb)
    : impl_(std::make_unique<ExternalServiceImpl>(port, cb)) {}

