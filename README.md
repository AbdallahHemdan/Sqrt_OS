# <font color='ff6c85 '>Mini OS</font>

## <font color='008080'>Team members </font>

|      Name      | Sec | BN |
|:--------------:|:---:|:--:|
|  Ahmed Sherif  |  1  | 3  |
| Ahmed Mohamed  |  1  | 6  |
|  Adel Mohamed  |  1  | 31 |
| Abdallah Ahmed |  2  | 1  |

## <font color='008080'>Data structures used</font>

|     Algo/Part     |       DS       |
|:-----------------:|:--------------:|
|   Processes WTA   |  Linked list   |
|    HPF - SRTN     | Priority queue |
| RR - Waiting list |     Queue      |
|       Buddy       |  Binary tree   |

## <font color='008080'>Algorithm explanation and results</font>

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
