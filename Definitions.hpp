#define ROOT 0

#define NUMBER_OF_COMPANIES 2
#define MAX_NUMBER_OF_COMPANIES 5

#define MIN_NUMBER_OF_KILLERS 2
#define MAX_NUMBER_OF_KILLERS 3

#define MIN_SLEEP_MSEC 3000000
#define MAX_SLEEP_MSEC 10000000


#define MSG_QUEUE   0
#define MSG_DEQUEUE 1
#define MSG_TAKE    2
#define MSG_RETURN  3
#define MSG_ACK     4

struct message_data
{
    int companyId;
    int rating;
    int lamportClock;
};