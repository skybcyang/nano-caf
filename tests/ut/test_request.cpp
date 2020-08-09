//
// Created by Darwin Yuan on 2020/8/8.
//

#include <catch.hpp>
#include <nano-caf/core/msg/request.h>
#include <nano-caf/core/actor/type_actor_handle.h>
#include <nano-caf/core/actor/behavior_based_actor.h>
#include <iostream>

namespace {
   using namespace NANO_CAF_NS;

   __CAF_actor_interface(media_session,
       (open,  (const long&) -> long),
       (close, (const long&) -> void)
   );

   struct not_atom {};
   struct fake_atom : atom_signature {};

   template<typename  T> struct S;

   //S<detail::method_atoms<media_session>> s;

   struct media_session_actor : behavior_based_actor {
      auto get_behavior() -> behavior override {
         return {
            [&](media_session::open_atom, long value) -> long {
               std::cout << "open received: " << value << std::endl;
               return 109;
            },
            [&](media_session::close_atom, long value) {
               std::cout << "close received: " << value << std::endl;
            },
            [&](exit_msg_atom, exit_reason reason) {
               std::cout << "exit received" << std::endl;
            },
         };
      }
   };

   TEST_CASE("actor interface") {
      REQUIRE(2 == media_session::total_methods);
      REQUIRE(std::is_same_v<type_list<media_session::open_atom, long>,
         media_session::open_atom::type::pattern_type>);
      REQUIRE(std::is_same_v<type_list<long>, media_session::open_atom::type::args_type>);
      REQUIRE(std::is_same_v<type_list<long>, media_session::open_atom::type::args_type>);

      actor_system system;
      system.start(1);
      REQUIRE(system.get_num_of_actors() == 0);

      type_actor_handle<media_session> me = system.spawn_type<media_session, media_session_actor>();
      me.request(media_session::open, (long)10).wait().match(
         [](auto status) { REQUIRE(false); },
         [](auto result) { REQUIRE(result == 109); });

      me.request(media_session::close, (long)10).wait().match(
         [](auto status) { REQUIRE(false); },
         [](auto result) { REQUIRE(true);  });

      REQUIRE(enq_result::ok == me.send(media_session::open, (long)10));
      REQUIRE(enq_result::ok == me.send(media_session::close, (long)20));
      REQUIRE(enq_result::ok == me.exit());

      me.wait_for_exit();
      me.release();
      system.shutdown();
   }
}