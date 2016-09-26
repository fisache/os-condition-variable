# 프로젝트 설명
- 세마포어를 이용하여 Lock과 Condition variable을 만들어 Read & Write 문제를 해결합니다.

# 개발 환경
- OS : Mac
- Compiler : gcc
- Build System :
- Development Tool : vi

# 역할
- File에 Read, Write하는 코드와 세마포어를 이용해 Lock과 Condition variable을 만들었습니다.

# 사용한 기술
- Semaphore를 3개를 이용해 각각 P(), V()함수를 구현해 Lock과 Read, Write condition을 구현합니다.

# 설명

```c
typedef union _semun {
     int val;
     struct semid_ds *buf;
     ushort *array;
} semun;
```
세마포어 구조체 <br />

Reader.c
```c
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
```

<pre>
Condition Variable 2개 OkToRead 와 OkToWrite를 구현할 때 필요한 method인 wait 과 signal을 구현하였다. 
이외에도 broadcast도 있지만 reader에서는 사용되지 않기 때문에 구현하지 않았다. wait은 조건을 만족하지 않았을
경우 TCB에 저장한 후 relase lock을 하고 sleep을 하는 method이다. reader에서 조건은 read가 되지 못하는 
경우를 말하는데 조건을 만족하지 못하는 경우는 Writer가 기다리거나 쓰는 중일 때 이다. signal은 반대로 조건이 
변화하여 조건이 맞는 다른 저장되어있는 TCB를 깨울 때 사용되는 method이다. Reader에서는 read가 끝나는 것이 
변화된 조건이다. read가 끝나게 되면 TCB queue중에 우선 순위가 높은 write를 깨운다.
</pre>

Write.c
```c
//condition variable wait method
void wait1(int semid,int lockid,pid_t pid){
    if(scanFromFile("AR.txt")+scanFromFile("AW.txt")>0)
        printToFile("WW.txt",scanFromFile("WW.txt")+1,pid);
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
//condition variable broadcast method
void broadcast1(int semid,int lockid,pid_t pid){
    int num = scanFromFile("WR.txt");
    if(scanFromFile("WR.txt") > 0){
        while(num){
            num--;
            v(semid);
        }
        printToFile("WR.txt",0,pid);
    }
}
```

<pre>
Writer에서는 Reader의 2개 method 이외에도 broadcast method도 구현하였다.
Writer의 wait은 write 하지 못할 경우 발생되는데 그 경우는 다른 프로세스가 
read 또는 write 중일 때이다. 따라서 Reader wait method와 마찬가지로 
release lock을 한 후 sleep을 하게 된다. signal의 조건 변화는 write가 끝났을
때를 말한다. write가 끝나면 signal은 sleep 상태의 writer 중 하나를 깨우게 된다. 
broadcast method는 sleep 상태의 TCB를 모두 깨우는 method이다. read의 경우는
다 같이 읽는 행위가 가능하여 read 조건이 맞을 경우 broadcast를 사용한다. read가
가능한 경우는 기다리거나 쓰는 write가 없을 경우 발생되며, 발생되었을 때 모든 read를 깨운다.
</pre>
# 결론

![ScreenShot](https://github.com/fisache/os-condition-variable/blob/master/os.png)

<pre>
실행 결과 기록된 표를 보면 가장 먼저 실행 된 프로세스는 4085의 PID를 가진 Reader이며 
가장 늦게 끝난 프로세스는 4089의 PID를 가진 Reader이다. 또한 read, write 순서대로 
보면 Read > Read > Write > Write > Read > Read 순서이다. 발생 된 총 시간을 보면 
첫 번째 AR이 시작되기까지 1초(reader 1 5) 가 걸렸기 때문에 프로세스가 시작된 시간은 
38026이며 마지막 프로세스가 끝난 시간은 61031이다. 따라서 6개의 프로세스가 전부 완료될 
때까지 걸린 시간은 23005, 약 23초이다. 프로세스가 발생되는 순서를 보면 앞에서 실행된 
Read 2개는 Write가 실행되기 전에 진행되었고 뒤에서 실행된 Read 2개는 Wirte와 같이 
wait 상태였지만 우선순위 상 Write가 더 앞이기 때문에 Write 2개가 모두 실행 된 후 실행되었다.

</pre>
