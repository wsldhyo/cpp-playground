#include "test_stack.hpp"
int main(int argc, char* argv[]){
    test_mtx_stack_concurrency(8, 5000);
    return 0;
}