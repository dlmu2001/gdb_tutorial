#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdint.h>

using std::string;

#define MyLog printf

typedef uint64_t hash_t;
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;
const char* requests[] = {
    "add", "remove","dump"
};

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
}
void handle_remove_request() {
    MyLog("\n%s\n", __func__);
}
void handle_dump_request() {
    MyLog("\n%s\n", __func__);
}
void wait_request() {
    const char* request = nullptr;
    int request_size = sizeof(requests)/sizeof(requests[0]);
    while(true) {
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
               default:
                   break;
           } 
        }
        usleep(1000000);
    }
}

int main(int argc, char *argv[]) {
    MyLog("\n%s\n", argv[0]);
    wait_request();
}
