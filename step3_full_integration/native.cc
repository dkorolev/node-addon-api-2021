#include "extension/extension_using_current.h"
#include "wrapper.h"

Napi::Value DemoAsyncIntegration(const Napi::CallbackInfo& cbinfo) {
  NodeJSContext ctx(cbinfo);

  int const port = cbinfo[0].As<Napi::Number>();
  NodeJSFunction f_timer = ctx.ExtractJSFunction(cbinfo[1]);
  NodeJSFunction f_route = ctx.ExtractJSFunction(cbinfo[2]);

  ctx.RunAsync([port, f_timer, f_route]() {
    ExternalService e(port, [&](const std::string& s) { f_route(s); });
    for (int i = 1; i <= 10; ++i) {
      f_timer(i);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    e.WaitUntilDeleted();
  });

  return ctx.GetPromise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports["demoAsyncIntegration"] = Napi::Function::New(env, DemoAsyncIntegration);
  return exports;
}

NODE_API_MODULE(addon, Init)
