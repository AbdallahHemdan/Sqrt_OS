<div align="center">
<a href="https://github.com/AbdallahHemdan/Sqrt_OS" rel="noopener">
  
  ![Component 14](https://user-images.githubusercontent.com/40190772/104843823-48f18c00-58d5-11eb-946f-510a76db56b3.png)

</div>

<h3 align="center">Sqrt OS</h3>

<div align="center">
  
  [![GitHub contributors](https://img.shields.io/github/contributors/AbdallahHemdan/Sqrt_OS)](https://github.com/AbdallahHemdan/Sqrt_OS/contributors)
  [![GitHub issues](https://img.shields.io/github/issues/AbdallahHemdan/Sqrt_OS)](https://github.com/AbdallahHemdan/Sqrt_OS/issues)
  [![GitHub forks](https://img.shields.io/github/forks/AbdallahHemdan/Sqrt_OS)](https://github.com/AbdallahHemdan/Sqrt_OS/network)
  [![GitHub stars](https://img.shields.io/github/stars/AbdallahHemdan/Sqrt_OS)](https://github.com/AbdallahHemdan/Sqrt_OS/stargazers)
  [![GitHub license](https://img.shields.io/github/license/AbdallahHemdan/Sqrt_OS)](https://github.com/AbdallahHemdan/Sqrt_OS/blob/master/LICENSE)
  <img src="https://img.shields.io/github/languages/count/AbdallahHemdan/Sqrt_OS" />
  <img src="https://img.shields.io/github/languages/top/AbdallahHemdan/Sqrt_OS" />
  <img src="https://img.shields.io/github/languages/code-size/AbdallahHemdan/Sqrt_OS" />
  <img src="https://img.shields.io/github/issues-pr-raw/AbdallahHemdan/Sqrt_OS" />

</div>

## About
> A CPU scheduler determines an order for the execution of its scheduled processes; it
> decides which process will run according to a certain data structure that keeps track
> of the processes in the system and their status.

> A process, upon creation, has one of the three states: Running, Ready, Blocked (doing
> I/O, using other resources than CPU or waiting on unavailable resource).

## System Description

> Consider a Computer with 1-CPU and fixed size memory (1024 ==> can be changed).
> Our scheduler implemented the complementary components as sketched in the following diagrams.

📌 NOTE:

> We are supporting the following Scheduling algorithms:

1. HPF (Highest Priority First)
2. STRN (Shortest Time Remaining Next)
3. RR (Round Robin)

<div align='center'><img src="assets/diagram.png"></div>

## Used Data structures

|     Algo/Part     |       DS       |
|:-----------------:|:--------------:|
|   Processes WTA   |  Linked list   |
|    HPF - SRTN     | Priority queue |
| RR - Waiting list |     Queue      |
|       Buddy       |  Binary tree   |

## Algorithm Flow

- Algo Explanation:
    1. Recieve and process all processes which come in the current second
      1. In case of free space: Allocate space for it and push it in the ready queue
      2. Else: Push it in the waiting list

    2. Check the running process if it finishes or not
      1. True: Free its allocated space and check the waiting list
      2. False: Pass (continue the code flow)
    3. Check if the ready queue is not empty:
       - True: based on the algo, we decide that we will switch or not
    4. Wait till the next second


## Results
1. HPF Algorithm

<div align='center'><img src="assets/HPF_result_P1.png"></div>
<div align='center'><img src="assets/HPF_result_P2.png"></div>
  
<hr />

2. SRTN Algorithm

<div align='center'><img src="assets/SRTN_result_P1.png"></div>
<div align='center'><img src="assets/SRTN_result_P2.png"></div>

<hr />

3. Round Robin Algorithm

<div align='center'><img src="assets/RR_result_P1.png"></div>
<div align='center'><img src="assets/RR_result_P2.png"></div>
        
### Contributors
<table>
  <tr>
    <td align="center"><a href="https://github.com/AbdallahHemdan"><img src="https://avatars1.githubusercontent.com/u/40190772?s=460&v=4" width="150px;" alt=""/><br /><sub><b>Abdallah Hemdan</b></sub></a><br /></td>
     <td align="center"><a href="https://github.com/AdelRizq"><img src="https://avatars2.githubusercontent.com/u/40351413?s=460&v=4" width="150px;" alt=""/><br /><sub><b>Adel Mohamed</b></sub></a><br /></td>
     <td align="center"><a href="https://github.com/ahmedsherif304"><img src="https://avatars1.githubusercontent.com/u/40776441?s=460&u=9939dbf6811189204bef5360d51cccfc9765d2a7&v=4" width="150px;" alt=""/><br /><sub><b>Ahmed Sherif</b></sub></a><br /></td>
     <td align="center"><a href="https://github.com/Mahboub99"><img src="https://avatars3.githubusercontent.com/u/43186742?s=460&v=4" width="150px;" alt=""/><br /><sub><b>Ahmed Mahboub</b></sub></a><br /></td>
  </tr>
 </table>

### Licence
[MIT Licence](https://github.com/AbdallahHemdan/Sqrt_OS/blob/main/LICENSE)
