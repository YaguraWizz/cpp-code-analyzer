#include <iostream>
#include <algorithm>
#include <iterator>

#define TEMP
#define TEMP_ARGS(w, x)

#define AVERAGE(arr, size)                  \
    ({                                      \
        double sum = 0.0;                   \
        for (int i = 0; i < size; ++i) {    \
            sum += arr[i];                  \
        }                                   \
        sum / size;                         \
    })
  

static void UsedValue();

int main() {
	int value1;
	int value2;
	bool value3;
	return 0;
}

static void UsedValue(){
	bool value4 = true, value5;
}