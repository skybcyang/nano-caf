//
// Created by Darwin Yuan on 2020/7/24.
//

#ifndef NANO_CAF_WORKER_H
#define NANO_CAF_WORKER_H

#include <nano-caf/nano-caf-ns.h>
#include <nano-caf/core/thread_safe_list.h>
#include <cstddef>
#include <memory>


NANO_CAF_NS_BEGIN

struct resumable;

struct worker : private thread_safe_list {
   auto external_enqueue(resumable *job) -> void;
   auto launch() -> void;
   auto stop() -> void;

   auto wait_done() -> void;
private:
   auto run() -> void;
   auto resume_job(resumable*) -> bool;
   auto cleanup() -> void;
   auto goto_bed() -> void;
   auto wakeup_worker() -> void;

private:
   std::thread thread_{};
   std::mutex lock_{};
   std::condition_variable cv_{};
   bool sleeping{false};
};

NANO_CAF_NS_END

#endif //NANO_CAF_WORKER_H
