//
// Created by Darwin Yuan on 2020/8/8.
//

#ifndef NANO_CAF_TYPED_ACTOR_HANDLE_H
#define NANO_CAF_TYPED_ACTOR_HANDLE_H

#include <nano-caf/core/actor/requester.h>

NANO_CAF_NS_BEGIN

template<typename ACTOR_INTERFACE>
struct typed_actor_handle : private actor_handle {
   typed_actor_handle(intrusive_actor_ptr ptr = nullptr) : actor_handle{ptr} {}

   using actor_handle::wait_for_exit;
   using actor_handle::release;
   using actor_handle::exit;

   template<typename METHOD_ATOM, message::category CATEGORY = message::normal, typename ... Args,
      typename = std::enable_if_t<requester::is_msg_valid<METHOD_ATOM, ACTOR_INTERFACE, Args...>>>
   auto send(Args&& ... args) {
      return actor_handle::send<typename METHOD_ATOM::msg_type, CATEGORY>(std::forward<Args>(args)...);
   }

   template<typename METHOD_ATOM, message::category CATEGORY = message::normal, typename ... Args,
      typename = std::enable_if_t<requester::is_msg_valid<METHOD_ATOM, ACTOR_INTERFACE, Args...>>>
   auto send(intrusive_actor_ptr sender, Args&& ... args) {
      return actor_handle::send<typename METHOD_ATOM::msg_type, CATEGORY>(sender, std::forward<Args>(args)...);
   }

   template<typename METHOD_ATOM, message::category CATEGORY = message::normal, typename ... Args,
      typename = std::enable_if_t<requester::is_msg_valid<METHOD_ATOM, ACTOR_INTERFACE, Args...>>>
   auto request(Args&& ... args) {
      auto l = [&](auto&& handler) mutable {
         return actor_handle::request<typename METHOD_ATOM::msg_type, CATEGORY>(
               std::forward<decltype(handler)>(handler),
               std::forward<Args>(args)...);
      };
      return requester::then_rsp<METHOD_ATOM, decltype(l), requester::wait_rsp>(std::move(l));
   }

   template<typename METHOD_ATOM, message::category CATEGORY = message::normal, typename ... Args,
      typename = std::enable_if_t<requester::is_msg_valid<METHOD_ATOM, ACTOR_INTERFACE, Args...>>>
   auto request(intrusive_actor_ptr from, Args&& ... args) {
      auto l = [&, from](auto&& handler) mutable {
         return actor_handle::request<typename METHOD_ATOM::msg_type, CATEGORY>(
            from,
            std::forward<decltype(handler)>(handler),
            std::forward<Args>(args)...);
      };
      return requester::then_rsp<METHOD_ATOM, decltype(l), requester::request_rsp_base>(std::move(l));
   }
};

NANO_CAF_NS_END

#endif //NANO_CAF_TYPED_ACTOR_HANDLE_H
