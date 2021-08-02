#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <atomic>
#include <pthread.h>
#include<sys/prctl.h>
using std::string;
using std::vector;
using std::shared_ptr;

#define MyLog printf

typedef uint64_t hash_t;
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;
const char* requests[] = {
    "add", "remove","dump","quit"
};
const char* item_names[] = {
    "pear",
    "cherry",
    "strawberry",
    "banana",
    "watermelon",
    "apple",
    "orange",
    "mango",
    "plum",
    "pineapple",
    "litchi"
};
struct MyItem {
    MyItem(const string& n, uint32_t i) {
        name =n;
        id =i;
    }
    string name;
    uint32_t id;
};

shared_ptr<vector<shared_ptr<MyItem>>> g_sp_items;
uint32_t g_next_id{0};
std::atomic<bool> g_thread_running(true);
std::atomic<bool> g_dumping_on(false);

hash_t hash_(char const* str) {
  hash_t ret{basis};
  while (*str) {
      ret ^= *str;
      ret *= prime;
      str++;
    }
  return ret;
}

constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis) {
    return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime)
        : last_value;
}
void add_item(shared_ptr<MyItem> spItem) {
    g_sp_items->emplace_back(spItem);
}
void remove_item(uint32_t id) {
    for(auto it = g_sp_items->begin(); it!=g_sp_items->end();++it) {
        auto spItem = *it;
        if (spItem->id == id) {
            g_sp_items->erase(it);
            break;
        }
    }
    MyLog("%s failed to find item with id:%d", __func__, id);
}
void dump_items() {
    MyLog("\n%s\n",__func__);
    for(auto spItem: *g_sp_items) {
        MyLog("\t item:%d name:%s\n", spItem->id, spItem->name.c_str());
    }
}
bool is_requested(const char* request) {
  std::string temp = request;
  temp = "./" + temp;
  if (access(temp.c_str(), F_OK) == 0) {
      unlink(temp.c_str());
      MyLog("%s", temp.c_str());
      return true;
  }
  return false;
}
void handle_add_request(){
    MyLog("\n%s\n", __func__);
    uint32_t id = ++g_next_id;
    auto spItem = std::make_shared<MyItem>(item_names[id],id);
    add_item(spItem);
}
void handle_remove_request() {
    MyLog("\n%s\n", __func__);
    int32_t id = rand()%10;
    MyLog("\n%s,id=%d",__func__,id);
    remove_item(id);
}
void handle_dump_request() {
    MyLog("\n%s\n", __func__);
    g_dumping_on.store(true);
}
void handle_quit_request() {
    MyLog("\n%s\n", __func__);
    g_thread_running.store(false);
}
void handle_stop_dump_request() {
    MyLog("\n%s\n", __func__);
    g_dumping_on.store(false);
}
void wait_request() {
    const char* request = nullptr;
    int request_size = sizeof(requests)/sizeof(requests[0]);
    while(g_thread_running.load()) {
        int32_t index = -1;
        for(int32_t i = 0;i< request_size;i++) {
            if (is_requested(requests[i])) {
                index = i;
                break;
            }
        }
        if (index != -1) {
           switch(hash_(requests[index])){
               case hash_compile_time("add"):
                   handle_add_request();
                   break;
               case hash_compile_time("remove"):
                   handle_remove_request();
                   break;
               case hash_compile_time("dump"):
                   handle_dump_request();
                   break;
               case hash_compile_time("quit"):
                   handle_quit_request();
                   break;
               case hash_compile_time("stop_dump"):
                   handle_stop_dump_request();
                   break;
               default:
                   break;
           } 
        }
        usleep(1000000);
    }
}
void* thread_func(void* arg) {
   while (g_thread_running.load()){
       if (g_dumping_on.load()) {
          dump_items();       
       }
       usleep(1000000);
   } 
}
int main(int argc, char *argv[]) {
    MyLog("\n%s\n", argv[0]);
    g_sp_items = std::make_shared<vector<shared_ptr<MyItem>>>();
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, thread_func, NULL);
    if(ret!=0) {
        MyLog("\n%s create thread failed",__func__);
    }
    prctl(PR_SET_NAME,"thread1");
    wait_request();
    MyLog("\n%s wait for join\n", __func__);
    if (ret) {
        if(pthread_join(tid,NULL)) {
            MyLog("\n%s failed to join thread",__func__);
            abort();
        }
    }
    MyLog("quit normally");
    return 0;
}
