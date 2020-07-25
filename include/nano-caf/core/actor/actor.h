//
// Created by Darwin Yuan on 2020/7/25.
//

#ifndef NANO_CAF_ACTOR_H
#define NANO_CAF_ACTOR_H

#include <nano-caf/core/actor/actor_control_block.h>
#include <nano-caf/core/actor/actor_handle.h>
#include <nano-caf/core/actor/exit_reason.h>
#include <nano-caf/core/actor_system.h>

NANO_CAF_NS_BEGIN

struct actor {
   virtual ~actor() = default;

protected:
   template<typename T, typename ... Args>
   inline auto send(actor_handle& to, message_id const& id, Args&& ... args) {
      return to.send<T>(self(), id, std::forward<Args>(args)...);
   }

   template<typename T, typename ... Ts>
   inline auto spawn(Ts&& ... args) noexcept -> actor_handle {
      return self().system().spawn<T>(std::forward<Ts>(args)...);
   }

   virtual auto exit(exit_reason) -> void = 0;

   virtual auto handle_message(const message_element& msg) noexcept -> void = 0;

private:
   virtual auto self() -> actor_control_block& = 0;
};

NANO_CAF_NS_END

#endif //NANO_CAF_ACTOR_H