//
// Created by Darwin Yuan on 2020/8/2.
//

#include <nano-caf/core/msg/message.h>
#include <nano-caf/core/actor/sched_actor.h>
#include <nano-caf/core/actor_system.h>
#include <nano-caf/core/actor/actor.h>
#include <iostream>
#include <nanobench.h>
#include <random>
#include "../ut/test_msgs.h"

using namespace NANO_CAF_NS;

struct future_actor : actor {
   const size_t value = 10;
   size_t final_result = 0;

   auto add(size_t a, size_t b) {
      //std::cout << "calc future 1" << std::endl;
      std::random_device r;
      std::default_random_engine regen{r()};
      std::uniform_int_distribution<size_t> uniform(0, 1000);

      unsigned long result = 0;
      for(unsigned long i = 0; i < 100000; i++) {
         result += (a * b + value) + uniform(regen);
      }

      return result;
   }

   auto on_init() noexcept -> void override {
      auto future1 = async(&future_actor::add, this, 5, 3);
      if(!future1.is_ok()) {
         spdlog::error("future1 failed = {}", future1.failure());
         exit(exit_reason::unhandled_exception);
      }

      auto future2 = async([this]() {
         //std::cout << "calc future 2" << std::endl;
         size_t result = 0;
          size_t a = 20;
          size_t b = 4;
         std::random_device r;
         std::default_random_engine regen{r()};
         std::uniform_int_distribution<size_t> uniform(0, 1000);

         for(size_t i = 0; i < 100000; i++) {
            result += (a * b + value) + uniform(regen);
         }

         return result;
      });
      if(!future2.is_ok()) {
         spdlog::error("future2 failed = {}", future2.failure());
         exit(exit_reason::unhandled_exception);
      }

      auto future3 = async([this]() {
         //std::cout << "calc future 3" << std::endl;
         size_t result = 0;
          size_t a = 20;
          size_t b = 42;
         std::random_device r;
         std::default_random_engine regen{r()};
         std::uniform_int_distribution<size_t> uniform(0, 1000);

         for(size_t i = 0; i < 100000; i++) {
            result += (a * b + value) + uniform(regen);
         }

         return result;
      });

      if(!future3.is_ok()) {
         spdlog::error("future3 failed = {}", future3.failure());
         exit(exit_reason::unhandled_exception);
      }

      auto result4 = with(future1, future2, future3)(
         [this](auto r1, auto  r2, auto r3) {
            final_result = r1 + r2 + r3;
            //std::cout << "all futures done = " << final_result << std::endl;
            exit(exit_reason::normal);
         });

      if(result4 != status_t::ok) {
         spdlog::error("result4 failed = {}", result4);
         exit(exit_reason::unhandled_exception);
      }

      auto result1 = with(future1)([]([[maybe_unused]]auto r1) {
         //std::cout << "async future1 done = " << r1 << std::endl;
      });

      if(result1 != status_t::ok) {
         spdlog::error("result1 failed = {}", result1);
         exit(exit_reason::unhandled_exception);
      }

      auto result2 = with(future2)([]([[maybe_unused]] auto r2) {
         //std::cout << "async future2 done = " << r2 << std::endl;
      });

      if(result2 != status_t::ok) {
         spdlog::error("result2 failed = {}", result2);
         exit(exit_reason::unhandled_exception);
      }

      auto result3 = with(future3)([]([[maybe_unused]] auto r3) {
         //std::cout << "async future3 done = " << r3 << std::endl;
      });

      if(result3 != status_t::ok) {
         spdlog::error("result3 failed = {}", result3);
         exit(exit_reason::unhandled_exception);
      }


   }

   auto handle_message(message&) noexcept -> task_result override {
      return task_result::resume;
   }
};

void run_on_thread(size_t num_of_threads, char const*) {
   actor_system system;
   system.start(num_of_threads);

//   ankerl::nanobench::Bench().minEpochIterations(109).run(name, [&] {
     for(size_t i=0; i<1000; i++) {
        auto me = system.spawn<future_actor>();
        //me.send<test_message>(1);
        me.wait_for_exit();
        me.release();
     }

//   });

   system.shutdown();

   std::cout << "--------------------------------" << std::endl;
   for(size_t i=0; i<num_of_threads; i++) {
      std::cout << "worker[" <<i<<"] = " << system.sched_jobs(i) << " jobs" << std::endl;
   }
   std::cout << "================================" << std::endl;
}

#define __(n) n, "3 tasks on " #n " threads"

int main() {
   run_on_thread(__(1));
   run_on_thread(__(2));
   run_on_thread(__(3));
   run_on_thread(__(4));

   return 0;
}