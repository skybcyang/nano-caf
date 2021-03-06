//
// Created by Darwin Yuan on 2020/7/27.
//

#ifndef NANO_CAF_ASYNC_OBJECT_H
#define NANO_CAF_ASYNC_OBJECT_H

#include <nano-caf/nano-caf-ns.h>
#include <nano-caf/core/thread_pool/resumable.h>
#include <nano-caf/core/msg/predefined-msgs.h>
#include <nano-caf/core/actor/intrusive_actor_ptr.h>
#include <nano-caf/core/await/promise.h>
#include <nano-caf/core/await/detail/future_object.h>
#include <nano-caf/core/actor/actor_handle.h>
#include <type_traits>
#include <functional>
#include <future>

NANO_CAF_NS_BEGIN

template <typename F, typename R, typename ... Args>
struct async_object : resumable  {
   async_object(weak_actor_ptr sender, F&& f, Args&& ... args)
      : f_{std::move(f)}
      , args_{std::move(args) ...}
      , sender_{std::move(sender)}
      {}

   virtual auto resume() noexcept -> bool override {
      promise_.set_value(std::move(std::apply(f_, std::move(args_))), sender_.lock());
      return true;
   }

   auto get_future(on_actor_context& context) {
      return promise_.get_future(context);
   }

private:
   F f_;
   std::tuple<Args...> args_;
   promise<R> promise_;
   weak_actor_ptr sender_;
};

template<typename R, typename F, typename ... Args, typename = std::enable_if_t<std::is_invocable_r_v<R, F, Args...>>>
auto make_async_object(weak_actor_ptr sender, F&& callable, Args&& ... args) {
   return new async_object<std::decay_t<F>, R, Args...>
      { std::move(sender), std::forward<F>(callable), std::forward<Args>(args) ... };
}

NANO_CAF_NS_END

#endif //NANO_CAF_ASYNC_OBJECT_H
