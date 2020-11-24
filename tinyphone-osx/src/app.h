
#ifdef __cplusplus
#include <string>
extern "C" {
#endif

struct UIAccountInfo{
    char * name;
    char * status;
    int active;
    int primary;
};
typedef struct UIAccountInfo UIAccountInfo;

struct UIAccountInfoArray {
    int count;
    UIAccountInfo accounts[10];
};

void Start();
void Stop();
struct UIAccountInfoArray Accounts();

#ifdef __cplusplus
}
#endif
