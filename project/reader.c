#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

int initsem(key_t semkey, int n);
int p(int semid);
int v(int semid);
void wait1(int semid,int lockid,pid_t pid);
void signal1(int semid,int lockid,pid_t pid);
int printToFile(char* filename,int value,pid_t pid);
int scanFromFile(char* filename);

typedef union _semun {
     int val;
     struct semid_ds *buf;
     ushort *array;
} semun;

// argv[0] : self , argv[1] : first sleep, argv[2] : second sleep
void main(int argc, char* argv[]){
    //sem key values
    key_t key_lock = 0x333, key_okToRead = 0x334, key_okToWrite = 0x335;
    // id through initsem
    int lockid,oktrid,oktwid;
    // process pid
    pid_t pid;
   
    pid = getpid();
    lockid = initsem(key_lock,1);
    oktrid = initsem(key_okToRead,0);
    oktwid = initsem(key_okToWrite,0);
    
    // failed to get semaphore
    if(lockid < 0 || oktrid <0 || oktwid < 0){
        exit(1);
    }
    //first sleep
    sleep(atoi(argv[1]));  
    //acquire lock
    p(lockid);
    
    //second sleep 1/2
    sleep(atoi(argv[2])/2);
    //if can't read, go to sleep
    while(scanFromFile("AW.txt")+scanFromFile("WW.txt") > 0){
        wait1(oktrid, lockid,pid);
    }
    //read
    printToFile("AR.txt",scanFromFile("AR.txt")+1,pid);
    //second sleep 2/2
    if(atoi(argv[2])%2 == 1) sleep((int)atoi(argv[2])/2+1);
    else sleep((int)atoi(argv[2])/2);
    //relase lock
    v(lockid);
    
    //acquire lock
    p(lockid);
    // complete to read
    printToFile("AR.txt",scanFromFile("AR.txt")-1,pid);
    //if sleep for the other, wake up
    if(scanFromFile("AR.txt")==0 && scanFromFile("WW.txt") > 0)
        signal1(oktwid,lockid,pid);
    //relase lock
    v(lockid);
}

// get semaphore id
int initsem(key_t semkey,int n){
    int status = 0, semid;
    if ((semid = semget (semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
    {
       if (errno == EEXIST)
                semid = semget (semkey, 1, 0);
   }
   else
   {
       semun arg;
       arg.val = n;
       status = semctl(semid, 0, SETVAL, arg);
   }
   if (semid == -1 || status == -1)
   {
       perror("initsem failed");
       return (-1);
   }
   return (semid);
}

//semaphore p method
int p(int semid){
    struct sembuf p_buf;
    p_buf.sem_num = 0;
    p_buf.sem_op = -1;
    p_buf.sem_flg = SEM_UNDO;
    if (semop(semid, &p_buf, 1) == -1)
    {   
        perror ("p(semid) failed");
        exit(1);
    }
    return (0);   
}

//semaphore v method
int v (int semid)
{
    struct sembuf v_buf;
    v_buf.sem_num = 0;
    v_buf.sem_op = 1;
    v_buf.sem_flg = SEM_UNDO;
    if (semop(semid, &v_buf, 1) == -1)
    {
        perror ("v(semid) failed");
        exit(1);
    }
    return (0);
}

//print parameter value, pid and time to the file, filename
int printToFile(char* filename,int value,pid_t pid){
    FILE *fp;
    struct timeval tv;
    double time_in_mill;
    if(fp = fopen(filename,"a+")){
	gettimeofday(&tv,NULL);
	time_in_mill = (tv.tv_sec)*1000+(tv.tv_usec)/1000;
	fprintf(fp,"%d %d %f\n",value,pid,time_in_mill);
    }
    fclose(fp);
    return 0;
}

//scan value, pid and time from the file, filename
int scanFromFile(char* filename){
    FILE *fp;
    int value=0;
    int pid=0;
    float c;
       
    if(fp = fopen(filename,"r")){}
    else{
        fp = fopen(filename,"w+");
        fprintf(fp,"%d %d 0\n",value,pid);
    }
    while(!feof(fp))
    fscanf(fp,"%d %d %f",&value,&pid,&c);
    fclose(fp);
    return value;
}

//condition variable wait method
void wait1(int semid,int lockid,pid_t pid){
    if(scanFromFile("WW.txt")+scanFromFile("AW.txt") > 0)
    	printToFile("WR.txt",scanFromFile("WR.txt")+1,pid);
    v(lockid);
    p(semid);
    p(lockid);
}

//condition variable signal method
void signal1(int semid,int lockid,pid_t pid){
    if(scanFromFile("WW.txt") > 0){
        v(semid);
        printToFile("WW.txt",scanFromFile("WW.txt")-1,pid);
    }
}
