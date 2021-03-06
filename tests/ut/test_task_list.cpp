//
// Created by Darwin Yuan on 2020/7/23.
//

#include <catch.hpp>
#include <nano-caf/core/actor/task_list.h>
#include <nano-caf/core/msg/make_message.h>
#include "test_msgs.h"

namespace {
   using namespace NANO_CAF_NS;

   TEST_CASE("an empty task list") {
      task_list list{};
      THEN("its total task size should be 0") {
         REQUIRE(list.total_task_size() == 0);
      }
      THEN("it should be an empty list") {
         REQUIRE(list.empty() == true);
      }
      THEN("try to get next elem, should return nullptr") {
         size_t deficit = 100;
         REQUIRE(list.next(deficit) == nullptr);
         AND_THEN("the deficit becomes 0") {
            REQUIRE(deficit == 0);
         }
      }
      WHEN("push back a new element") {
         list.push_back(make_message<test_message>(1));
         THEN("its total task size should be 1") {
            REQUIRE(list.total_task_size() == 1);
         }
         THEN("it should not be an empty list") {
            REQUIRE(list.empty() == false);
         }
         AND_WHEN("push back another element") {
            list.push_back(make_message<test_message>(2));
            THEN("its total task size should be 2") {
               REQUIRE(list.total_task_size() == 2);
            }
            THEN("it should not be an empty list") {
               REQUIRE_FALSE(list.empty());
            }
         }
      }
   }

   TEST_CASE("an non-empty task list") {
      task_list list{};
      list.push_back(make_message<test_message>(1));
      list.push_back(make_message<test_message>(2));
      size_t deficit = 100;
      WHEN("pop front a element, should get the first one") {
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 1);
         AND_WHEN("pop front a element again, should get the 2nd one") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_WHEN("pop front a element again, should get nullptr") {
               REQUIRE(list.next(deficit) == nullptr);
            }AND_THEN("it should not be an empty list") {
               REQUIRE(list.empty() == true);
            }
         }
      }
   }

   TEST_CASE("push front to an emtpy task list") {
      task_list list{};

      WHEN("push_front a new elem") {
         list.push_front(make_message<test_message>(1));
         THEN("it should contains 1 elem") {
            REQUIRE(1 == list.total_task_size());
         }
         THEN("its first element should be the pushed one") {
            size_t deficit = 100;
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 1);
            AND_THEN("the deficit should be decreased") {
               REQUIRE(deficit == 99);
            }
            AND_THEN("it does not has 2nd element") {
               REQUIRE(list.empty() == true);
               REQUIRE(list.next(deficit) == nullptr);
            }
         }
      }
      WHEN("push_front 2 new elements") {
         list.push_front(make_message<test_message>(1));
         list.push_front(make_message<test_message>(2));
         THEN("it should contains 2 elems") {
            REQUIRE(2 == list.total_task_size());
         }
         THEN("its first element should be the last pushed one") {
            size_t deficit = 100;
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_THEN("the deficit should be decreased") {
               REQUIRE(deficit == 99);
            }
            THEN("its 2nd element should be the 1st pushed one") {
               auto ptr = list.next(deficit);
               REQUIRE(ptr != nullptr);
               REQUIRE(ptr->body<test_message>()->value == 1);
               AND_THEN("it does not has 3rd element") {
                  REQUIRE(list.empty() == true);
                  REQUIRE(list.next(deficit) == nullptr);
                  AND_THEN("the deficit should be 0") {
                     REQUIRE(deficit == 0);
                  }
               }
            }
         }
      }
   }

   TEST_CASE("append an empty task list") {
      task_list list{};
      list.push_back(make_message<test_message>(1));
      list.push_back(make_message<test_message>(2));
      task_list list2{};
      list.append_list(list2);

      THEN("its task size remains 2") {
         REQUIRE(list.total_task_size() == 2);
      }
      AND_THEN("its first element is still 1") {
         size_t deficit = 100;
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 1);
         AND_THEN("its 2nd element is still 2") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_THEN("it does not has 3rd element") {
               REQUIRE(list.empty() == true);
               REQUIRE(list.next(deficit) == nullptr);
            }
         }
      }
   }

   TEST_CASE("an empty list appends an non-empty task list") {
      task_list list{};
      REQUIRE(list.total_task_size() == 0);

      task_list list2{};
      list2.push_back(make_message<test_message>(1));
      list2.push_back(make_message<test_message>(2));
      list.append_list(list2);
      size_t deficit = 100;
      THEN("its task size become 2") {
         REQUIRE(list.total_task_size() == 2);
      }
      THEN("the task size of 2nd list becomes 0") {
         REQUIRE(list2.total_task_size() == 0);
      }
      THEN("the 2nd list becomes empty") {
         REQUIRE(list2.empty() == true);
      }
      AND_WHEN("try to pop front from the 2nd list, returns nullptr") {
         REQUIRE(list2.next(deficit) == nullptr);
      }
      AND_THEN("pop front, should return the 1st elem of 2nd list") {
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 1);
         AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_THEN("it does not has 3rd element") {
               REQUIRE(list.empty() == true);
               REQUIRE(list.next(deficit) == nullptr);
            }
         }
      }
   }



   TEST_CASE("an empty list prepends an non-empty task list") {
      task_list list{};
      REQUIRE(list.total_task_size() == 0);

      task_list list2{};
      list2.push_back(make_message<test_message>(1));
      list2.push_back(make_message<test_message>(2));
      list.append_list(list2);
      size_t deficit = 100;
      THEN("its task size become 2") {
         REQUIRE(list.total_task_size() == 2);
      }
      THEN("the task size of 2nd list becomes 0") {
         REQUIRE(list2.total_task_size() == 0);
      }
      THEN("the 2nd list becomes empty") {
         REQUIRE(list2.empty() == true);
      }
      AND_WHEN("try to pop front from the 2nd list, returns nullptr") {
         REQUIRE(list2.next(deficit) == nullptr);
      }
      AND_THEN("pop front, should return the 1st elem of 2nd list") {
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 1);
         AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_THEN("it does not has 3rd element") {
               REQUIRE(list.empty() == true);
               REQUIRE(list.next(deficit) == nullptr);
            }
         }
      }
   }

   TEST_CASE("push front a non empty list") {
      task_list list{};
      list.push_back(make_message<test_message>(1));
      list.push_back(make_message<test_message>(2));
      list.push_front(make_message<test_message>(3));
      list.push_front(make_message<test_message>(4));

      THEN("its task size become 4") {
         REQUIRE(list.total_task_size() == 4);
      }
      AND_THEN("pop front, should return the 1st elem of 2nd list") {
         size_t deficit = 100;
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 4);
         AND_THEN("the deficit should be decreased") {
            REQUIRE(deficit == 99);
         }
         AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 3);
            AND_THEN("the deficit should be decreased") {
               REQUIRE(deficit == 98);
            }
            AND_THEN("pop front a element again, should get the 1st elem of 2nd list") {
               auto ptr = list.next(deficit);
               REQUIRE(ptr != nullptr);
               REQUIRE(ptr->body<test_message>()->value == 1);
               AND_THEN("the deficit should be decreased") {
                  REQUIRE(deficit == 97);
               }
               AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
                  auto ptr = list.next(deficit);
                  REQUIRE(ptr != nullptr);
                  REQUIRE(ptr->body<test_message>()->value == 2);
                  AND_THEN("the deficit should be decreased") {
                     REQUIRE(deficit == 96);
                  }
                  AND_THEN("it does not has 5th element") {
                     REQUIRE(list.empty() == true);
                     REQUIRE(list.next(deficit) == nullptr);
                  }
               }
            }
         }
      }
   }

   TEST_CASE("a non-empty list appends an non-empty task list") {
      task_list list{};
      list.push_back(make_message<test_message>(1));
      list.push_back(make_message<test_message>(2));

      task_list list2{};
      list2.push_back(make_message<test_message>(3));
      list2.push_back(make_message<test_message>(4));

      list.append_list(list2);


      THEN("its task size become 4") {
         REQUIRE(list.total_task_size() == 4);
      }
      THEN("the task size of 2nd list becomes 0") {
         REQUIRE(list2.total_task_size() == 0);
      }
      THEN("the 2nd list becomes empty") {
         REQUIRE(list2.empty() == true);
      }
      AND_WHEN("try to pop front from the 2nd list, returns nullptr") {
         size_t deficit = 100;
         REQUIRE(list2.next(deficit) == nullptr);
      }

      AND_THEN("pop front, should return the 1st elem of 2nd list") {
         size_t deficit = 100;
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 1);
         AND_THEN("the deficit should be decreased") {
            REQUIRE(deficit == 99);
         }
         AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_THEN("the deficit should be decreased") {
               REQUIRE(deficit == 98);
            }
            AND_THEN("pop front a element again, should get the 1st elem of 2nd list") {
               auto ptr = list.next(deficit);
               REQUIRE(ptr != nullptr);
               REQUIRE(ptr->body<test_message>()->value == 3);
               AND_THEN("the deficit should be decreased") {
                  REQUIRE(deficit == 97);
               }
               AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
                  auto ptr = list.next(deficit);
                  REQUIRE(ptr != nullptr);
                  REQUIRE(ptr->body<test_message>()->value == 4);
                  AND_THEN("the deficit should be decreased") {
                     REQUIRE(deficit == 96);
                  }
                  AND_THEN("it does not has 5th element") {
                     REQUIRE(list.empty() == true);
                     REQUIRE(list.next(deficit) == nullptr);
                  }
               }
            }
         }
      }
   }

   TEST_CASE("a non-empty list prepends an non-empty task list") {
      task_list list{};
      list.push_back(make_message<test_message>(3));
      list.push_back(make_message<test_message>(4));

      task_list list2{};
      list2.push_back(make_message<test_message>(1));
      list2.push_back(make_message<test_message>(2));

      list.prepend_list(list2);

      THEN("its task size become 4") {
         REQUIRE(list.total_task_size() == 4);
      }
      THEN("the task size of 2nd list becomes 0") {
         REQUIRE(list2.total_task_size() == 0);
      }
      THEN("the 2nd list becomes empty") {
         REQUIRE(list2.empty() == true);
      }
      AND_WHEN("try to pop front from the 2nd list, returns nullptr") {
         size_t deficit = 100;
         REQUIRE(list2.next(deficit) == nullptr);
      }

      AND_THEN("if deficit is 0, should not be able to get any element") {
         size_t deficit = 0;
         auto ptr = list.next(deficit);
         REQUIRE(ptr == nullptr);
         REQUIRE(list.total_task_size() == 4);
      }

      AND_THEN("pop front, should return the 1st elem of 2nd list") {
         size_t deficit = 100;
         auto ptr = list.next(deficit);
         REQUIRE(ptr != nullptr);
         REQUIRE(ptr->body<test_message>()->value == 1);
         AND_THEN("the deficit should be decreased") {
            REQUIRE(deficit == 99);
         }
         AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
            auto ptr = list.next(deficit);
            REQUIRE(ptr != nullptr);
            REQUIRE(ptr->body<test_message>()->value == 2);
            AND_THEN("the deficit should be decreased") {
               REQUIRE(deficit == 98);
            }
            AND_THEN("pop front a element again, should get the 1st elem of 2nd list") {
               auto ptr = list.next(deficit);
               REQUIRE(ptr != nullptr);
               REQUIRE(ptr->body<test_message>()->value == 3);
               AND_THEN("the deficit should be decreased") {
                  REQUIRE(deficit == 97);
               }
               AND_THEN("pop front a element again, should get the 2nd elem of 2nd list") {
                  auto ptr = list.next(deficit);
                  REQUIRE(ptr != nullptr);
                  REQUIRE(ptr->body<test_message>()->value == 4);
                  AND_THEN("the deficit should be decreased") {
                     REQUIRE(deficit == 96);
                  }
                  AND_THEN("it does not has 5th element") {
                     REQUIRE(list.empty() == true);
                     REQUIRE(list.next(deficit) == nullptr);
                  }
               }
            }
         }
      }
   }
}