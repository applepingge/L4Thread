#include<stdint.h>
class utcb_t
{
    public:
    uint64_t threadid;
};

class tcb_t
{
    public:
    uint64_t tid;
    uint64_t * stack;
    utcb_t uid;
};

class prio_queue_t
{
    public:
    tcb_t * prio_queue[10];
    uint64_t priv[10];
};

prio_queue_t q;
void test()
{
    uint32_t i;
    for(i = 0; i < 10; i++)
    {
        q.prio_queue[i] = (tcb_t *) 0;
    }
    //for(i = 0; i < 10; i++)
    //{
    //    q.priv[i] = 10;
    //}
}