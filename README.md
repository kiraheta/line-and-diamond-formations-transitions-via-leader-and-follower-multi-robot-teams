# Line and Diamond Formations Transitions via Leader and Follower Multi-Robot Teams

##Description

   The purpose of this project is to explore the use of simple task allocation 
   and formation control. Specifically, to implement a Task Manager, establish 
   communication between the Task Manager & leader robot, among the leader 
   robot & follower robots, and apply line & diamond formations.

##How to compile "communicate.h," "communicate.c" and "final.cc"

1. g++ -c communicate.c

2. g++ -o final `pkg-config --cflags playerc++` final.cc `pkg-config --libs playerc++` -lnsl communicate.o

##How to run the project:

1. Place all files inside of "worlds" directory player stage directory
2. Open a terminal for stage, for Task Manager and a terminal for each robot
3. Run "player four.cfg" to launch stage simulator on first terminal
4. Run "./final 6665", "./final 6666", "./final 6667",
    and "./final 6668" in each terminal subsequently after the second terminal
5. For the Task Manager terminal i.e. the second terminal, run
    "./final 7000"
6. You'll then see the "Enter 3 waypoints?" prompt on screen
7. Enter 3 waypoints (-11,0), (0,0) and (10,0) 
8. Program will now run successfully     

##Demo
https://youtu.be/4wiW0iyIh3I?t=16s


