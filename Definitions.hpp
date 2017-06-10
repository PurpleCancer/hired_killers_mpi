#define ROOT 0

#define NUMBER_OF_COMPANIES 0
#define MAX_NUMBER_OF_COMPANIES 5

#define MIN_NUMBER_OF_KILLERS 1
#define MAX_NUMBER_OF_KILLERS 4


#define QUEUE   0
#define DEQUEUE 1
#define TAKE    2
#define RETURN  3
#define ACK     4

struct message_data
{
    int companyId;
    int rating;
};