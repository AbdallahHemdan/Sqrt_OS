# <font color='ff6c85 '>Mini OS</font>

<div align="center">
  
  [![GitHub contributors](https://img.shields.io/github/contributors/AdelRizq/mini-OS)](https://github.com/AdelRizq/mini-OS/contributors)
  [![GitHub issues](https://img.shields.io/github/issues/AdelRizq/mini-OS)](https://github.com/AdelRizq/mini-OS/issues)
  [![GitHub forks](https://img.shields.io/github/forks/AdelRizq/mini-OS)](https://github.com/AdelRizq/mini-OS/network)
  [![GitHub stars](https://img.shields.io/github/stars/AdelRizq/mini-OS)](https://github.com/AdelRizq/mini-OS/stargazers)
  [![GitHub license](https://img.shields.io/github/license/AdelRizq/mini-OS)](https://github.com/AdelRizq/mini-OS/blob/master/LICENSE)
  <img src="https://img.shields.io/github/languages/count/AdelRizq/mini-OS" />
  <img src="https://img.shields.io/github/languages/top/AdelRizq/mini-OS" />
  <img src="https://img.shields.io/github/languages/code-size/AdelRizq/mini-OS" />
  <img src="https://img.shields.io/github/issues-pr-raw/AdelRizq/mini-OS" />

</div>

## <font color='008080'>👨‍🎓 Introduction</font>

 > A CPU scheduler determines an order for the execution of its scheduled processes; it
 > decides which process will run according to a certain data structure that keeps track
 > of the processes in the system and their status.

 > A process, upon creation, has one of the three states: Running, Ready, Blocked (doing
> I/O, using other resources than CPU or waiting on unavailable resource).

## <font color='008080'>🖥 System Description</font>

> Consider a Computer with 1-CPU and fixed size memory (1024 ==> can be changed).
> Our scheduler implemented the complementary components as sketched in the following diagrams.

<div align='center'><img src="assets/diagram.png"></div>

## <font color='008080'>🛒 Data structures used</font>

|     Algo/Part     |       DS       |
|:-----------------:|:--------------:|
|   Processes WTA   |  Linked list   |
|    HPF - SRTN     | Priority queue |
| RR - Waiting list |     Queue      |
|       Buddy       |  Binary tree   |

## <font color='008080'>🧐 Algorithm explanation and results</font>

1. Phase 1
    - Algo Explanation:
        1. Recieve and push all processes came in the current second
        2. Check the running process if it finishes or not (Check its remaining time through the according shared memory)
        3. Check if the ready queue is not empty:
           - True: based on the algo, we decide that we will switch or not
        4. Wait till the next second
    - Results:
      - HPF
        <div align='center'><img src="assets/HPF_result_P1.png"></div>
      - SRTN
        <div align='center'><img src="assets/SRTN_result_P1.png"></div>
      - RR
        <div align='center'><img src="assets/RR_result_P1.png"></div>
2. Phase 2
    - Algo Explanation:
        1. Recieve processes came in the current second and based on the memory free space:
            1. In case of free space: Allocate space for it and push it in the ready queue
            2. Else: Push it in the waiting list
        2. Check the running process if it finishes or not:
            1. True: Free its allocated space and check the waiting list
            2. False: Pass (continue the code flow)
    - Results:
      - HPF
        <div align='center'><img src="assets/HPF_result_P2.png"></div>
      - SRTN
        <div align='center'><img src="assets/SRTN_result_P2.png"></div>
      - RR
        <div align='center'><img src="assets/RR_result_P2.png"></div>
