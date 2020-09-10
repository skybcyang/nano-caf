//
// Created by Darwin Yuan on 2020/7/27.
//

#ifndef NANO_CAF_FUTURE_CALLBACK_H
#define NANO_CAF_FUTURE_CALLBACK_H

#include <nano-caf/nano-caf-ns.h>
#include <nano-caf/core/msg/message.h>
#include <nano-caf/util/callable_trait.h>
#include <nano-caf/util/result_t.h>
#include <type_traits>
#include <functional>
#include <optional>

NANO_CAF_NS_BEGIN

struct future_callback {
   virtual auto invoke() noexcept -> bool = 0;
   virtual ~future_callback() = default;
};

template<typename F>
struct generic_future_callback : future_callback {
   generic_future_callback(F&& f) : f(std::move(f)) {}

   auto invoke() noexcept -> bool override {
      return f();
   }

private:
   F f;
};

template<typename ... Args>
struct future_set {
   template<typename ... Futures>
   future_set(Futures&& ... futures) : futures_{std::forward<Futures>(futures)...} {}

   template<typename Tp>
   static constexpr auto is_future_done(const Tp& future) noexcept -> bool {
      return future.wait_for(std::chrono::nanoseconds{0}) == std::future_status::ready;
   }

   template<size_t ... I>
   auto done(std::index_sequence<I...>) const -> bool {
      return (is_future_done(std::get<I>(futures_)) && ...);
   }

   template<size_t ... I, typename F>
   auto invoke(F&& f, std::index_sequence<I...> is) const -> bool {
      if(!done(is)) return false;
      f(std::get<I>(futures_).get()...);
      return true;
   }

   template<typename F>
   auto invoke(F&& f) const -> bool {
      return invoke(std::forward<F>(f), std::make_index_sequence<sizeof...(Args)>{});
   }

private:
   mutable std::tuple<Args...> futures_;
};

namespace detail {
   template<typename F, typename ... Args>
   auto with_futures(F&& f, Args&& ...args) -> result_t<future_callback*> {
      static_assert(std::is_invocable_r_v<void, F, decltype(std::declval<std::decay_t<Args>>().get())...>);
      using seq_type = std::make_index_sequence<sizeof...(Args)>;
      future_set<std::decay_t<Args>...> futures{std::forward<Args>(args)...};
      if(futures.invoke(std::forward<F>(f), seq_type{})) return status_t::ok;

      auto callback = new generic_future_callback(
         [futures = std::move(futures), func = std::move(f)]() mutable -> bool {
            return futures.invoke(func);
         });
      if(callback == nullptr) {
         return status_t::out_of_mem;
      }
      return callback;
   }
}

namespace detail {
   template<typename T, typename FUTURE>
   struct request_then {
      request_then(T& registry, FUTURE& future)
         : registry_{std::move(registry)}
         , either_future_{std::move(future)} {}

   private:
      template<typename F_HANDLER, typename F_FAIL>
      auto then_(F_HANDLER&& f_handler, F_FAIL&& f_fail) -> status_t {
         if(either_future_.is_ok()) {
            auto l = [handler = std::move(f_handler), fail = std::move(f_fail)](auto result) {
               result.match(handler, fail);
            };
            return with_futures(std::move(l), std::move(*either_future_))
               .with_value([&](auto future_cb) { return registry_(future_cb); });
         } else {
            return either_future_.failure();
         }
      }

   public:
      template<typename F_HANDLER, typename F_FAIL>
      auto then(F_HANDLER&& f_handler, F_FAIL&& f_fail) -> status_t {
         auto status = then_(std::forward<F_HANDLER>(f_handler), std::forward<F_FAIL>(f_fail));
         if(status != status_t::ok) {
            f_fail(status);
         }
         return status;
      }

      template<typename F_HANDLER>
      auto on_success(F_HANDLER&& f_handler) -> status_t {
         return then_(std::forward<F_HANDLER>(f_handler), [](status_t){});
      }

      template<typename F_FAIL>
      auto on_fail(F_FAIL&& f_fail) -> status_t {
         auto status = then_([](auto){}, std::forward<F_FAIL>(f_fail));
         if(status != status_t::ok) {
            f_fail(status);
         }
         return status;
      }

   private:
      T registry_;
      FUTURE either_future_;
   };
}

namespace async {
   template<typename T>
   constexpr bool is_future_type = false;

   template<typename T>
   constexpr bool is_future_type<std::shared_future<T>> = true;

   template<typename T>
   constexpr bool is_future_type<std::future<T>> = true;

   template<typename T>
   auto is_future_valid(const T& f) -> std::enable_if_t<is_future_type<T>, bool> {
      return f.valid();
   }

   template<typename T>
   auto is_future_valid(const result_t<T>& f) -> std::enable_if_t<is_future_type<T>, bool> {
      return f.match(
         [](auto& f) { return f.valid(); },
         [](auto) { return false; });
   }

   template<typename T>
   auto get_future(T& f) -> std::enable_if_t<is_future_type<T>, T&>{
      return f;
   }

   template<typename T>
   auto get_future(result_t<T>& f) -> std::enable_if_t<is_future_type<T>, T&> {
      return *f;
   }
}

NANO_CAF_NS_END

#endif //NANO_CAF_FUTURE_CALLBACK_H
