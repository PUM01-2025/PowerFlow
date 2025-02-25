#include <add.h>

#include <cassert>

int main() {
    assert(add(5, 5) == 10);
    assert(add(1, 1) == 2);
    assert(add(0, -1) == -1);
    return 0;
}
