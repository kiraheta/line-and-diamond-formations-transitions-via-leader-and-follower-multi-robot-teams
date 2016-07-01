/*
* To compile:
*
* g++ -c communicate.c
*
* g++ -o final `pkg-config --cflags playerc++` final.cc `pkg-config --libs playerc++` -lnsl communicate.o
*
*/
#include <libplayerc++/playerc++.h>  
#include <iostream>                  
#include "args.h"                    
#include <time.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <queue>
#include "communicate.h"

using namespace PlayerCc;
using namespace std;

struct TaskManager
{
	double x, y;
};

int port;
/****************************************************************/
 double startClock(){ //This function starts & keeps track of start time
     
    clock_t t1;
    t1 = clock();
    
    return t1;
}

void endClock(double t1){ //This function prints overall runtime of program
    
    clock_t t2 = clock();
    double diff ((1.5*(double)t2)-(double)t1);
    double time = diff/CLOCKS_PER_SEC;
    cout << "Runtime in seconds for completing tasks: " << time << endl;
}

/****************************************************************/

void getWaypoints (queue <double> &xWaypoints, queue <double> &yWaypoints ){

   double temp1[xWaypoints.size()];
   double temp2[yWaypoints.size()];
   int size = xWaypoints.size();
   double x, y;

   // User enter commands     
   cout << "Enter 3 waypoints composed of - x position y position" << endl;
   for (int i = 0; i<3; i++) {
		cin >> x >> y;
		xWaypoints.push (x);
	   yWaypoints.push (y);
	}
	  
	for (int j=0; j<3; j++) {
	   cout << j << ". Waypoint: " << "( " << xWaypoints.front() << ", "
	        << yWaypoints.front() << " )" << endl;
	        
	          temp1[j] = xWaypoints.front(); xWaypoints.pop(); xWaypoints.push(temp1[j]); 
	          temp2[j] = yWaypoints.front(); yWaypoints.pop(); yWaypoints.push(temp2[j]); 
	}
}

/****************************************************************/
/* send command to the others */

//send_cmdTM(bfdTM, sendX, SendY, setFormation);
void send_cmdTM(int bfd, double x, double y, char setFormation)
{ // int x = 99;

	// create a message
	int nbytes = 0;
	char *msg = NULL;
	char *char_from = NULL;
	char *char_x = NULL;
	char *char_y = NULL;
	char *char_formation = NULL;

	msg = (char *) malloc(60*sizeof(char));
	char_from = (char *) malloc(10*sizeof(char));
	char_x = (char*) malloc(sizeof(double));
	char_y = (char*) malloc(sizeof(double));
	char_formation = (char*) malloc(10*sizeof(char));

	itoa((int)port, char_from);
	sprintf(char_x, "%f", x);
	sprintf(char_y, "%f", y);
	sprintf(char_formation, "%c", setFormation);

	strcpy(msg, "M");
	strcat(msg, "$");
	strcat(msg, char_x);
	strcat(msg, "|");
	strcat(msg, char_y);
	strcat(msg, ":");
	strcat(msg, char_formation);
	strcat(msg, "!");
	printf("sending message: %s\n", msg);
	nbytes = talk_to_all(bfd, msg, H);
}
/****************************************************************/
void s_cmd(int bfd, char *cmd)
{
   // create a message
	 int nbytes = 0;
	 nbytes = talk_to_all(bfd, cmd, H);
}
/****************************************************************/
/* send command to the others */ 
void send_cmd(int bfd){
   int x = 99;

   // create a message
   int nbytes = 0;
   char *msg = NULL;
   char *char_from = NULL;
   char *char_x = NULL;

   msg = (char *) malloc(60*sizeof(char));
   char_from = (char *) malloc(10*sizeof(char));
   char_x = (char *) malloc(10*sizeof(char));

   itoa((int)port, char_from);
   itoa((int)x, char_x);

   strcpy(msg, "C");
   strcat(msg, char_from);
   strcat(msg, "$");
   strcat(msg, char_x);
   strcat(msg, "!");

   printf("sending message: %s\n", msg);
   nbytes = talk_to_all(bfd, msg, H);
}
/****************************************************************/
void parse_msg(char *msg)
{
   char *ptr, *token;

   token = strtok(msg, "!");
   while (token != NULL) {
      if (token[0] = 'C') {
         printf("This is a C type of message\n");
      }
      ptr = strstr(token, "C");
      ptr++;
      printf("port number %d\n", atoi(ptr));
      ptr = strstr(token, "$");
      ptr++;
      printf("x position is %d\n", atoi(ptr));
      token = strtok(NULL, "!");
  }
}
/****************************************************************/
void parse_msgTM(char *msg, double *x, double *y, char *formation, char *kind){
	//passes by reference to set position, speed, and yaw to objects in main using the parse method
   char *ptr, *token;

   token = strtok(msg, "!");
   while (token != NULL) {
  	   *kind = token[0];
       if (*kind == 'L') {
       }
       if(*kind == 'M'){
       }
       ptr = strstr(token, "$");
       ptr++;
       *x = atof(ptr);
       ptr = strstr(token, "|");
       ptr++;
       *y = atof(ptr);
       ptr = strstr(token, ":");
       ptr++;
       *formation = *ptr;
       token = strtok(NULL, "!");
  }
}

/****************************************************************/
/* main function */
int main(int argc, char** argv){
   
   clock_t t1=startClock(); //begin clock

   bool forLeader, forFollower, listeningTM = false, forTM = false, leaderCounter=true, lineDiamond = true, firstWaypoint = false, secondWaypoint = false, thirdWaypoint = false, done = false;
   int lfd, bfd, lfdTM, bfdTM;
   int nbytes = 0, counter = 0, waypoint = 0, lCounter = 0;
   double x, y, sendX, sendY, withinRange = 0.03;
   double wayX, wayY, xLeader, yLeader, yawLeader, follX, follY;
   char msg[MAXBUF], msgTM[MAXBUF], msgL[MAXBUF], setFormation = 'l', getFormation, kindOfMsg;
   char *setCmd = (char *) malloc(10*sizeof(char));
   queue <double> xWaypoints;
   queue <double> yWaypoints;
   
   /*   Begin program   */
   if (argc < 2) { 
       printf("mytest port\n");
       exit(1);
     }
     port = atoi(argv[1]); // converts comm line param from string to int
 
   try
   { 
   
      PlayerClient robot(gHostname, port);
      Position2dProxy pp(&robot, gIndex);
      LaserProxy lp(&robot, gIndex);
      
      std::cout << robot << std::endl;

      pp.SetMotorEnable (true);
      
      // check port input
		if ((port != 6665) && (port != 7000)){
	      lfd = create_listen(PORT_H,H);      //follower listens
 	      bfd = create_broadcast(PORT_R,H);   //follower broadcasts
		   forLeader = false; forFollower = true;}
		else if (port == 6665) {
		   lfd = create_listen (PORT_R,H);     //leader listens
		   bfd = create_broadcast (PORT_H,H);  //leader broadcasts
		   forLeader = true;}
		else if (port == 7000) {
	   	lfdTM = create_listen(PORT_H,H);      //TM listens
 	   	bfdTM = create_broadcast(PORT_R,H);   //TM broadcasts
		   forTM = true;}
		else {cout << "Wrong port. Try again!" << endl; exit(1);}
			
 	   // get Waypoints from user
		if (forTM) {
		   getWaypoints(xWaypoints, yWaypoints);
		} // end of if
		
/****************************************************************/		
	while(1){	
		
		// TM sends waypoints
		if (forTM) {
		
		   //do {
		   if (leaderCounter){
			   if ((!xWaypoints.empty()) || (!yWaypoints.empty())) {
			      
			      if (lineDiamond) {
			      	setFormation = 'l';
			      }	else { setFormation = 'd'; }
			      
			      sendX = xWaypoints.front(); sendY = yWaypoints.front();
			      cout << "Waypoint: ( " << sendX << ", " << sendY << " )" << endl; 
			      
			      xWaypoints.pop(); yWaypoints.pop();
			      
		         //broadcast waypoint to leader
		         send_cmdTM(bfdTM, sendX, sendY, setFormation);
		         
		         if (lineDiamond)
		            lineDiamond = false;
		         else lineDiamond = true;
		         
		         forTM = false; 
		         listeningTM = true;
		         leaderCounter = false;
		         
					} 
					else {
						cout << "No more waypoints." << endl;
						// send last broadcast to leader to stop
				      *setCmd = 'h';
							s_cmd(bfdTM, setCmd);
				      endClock(t1);
				      exit(1);
					}
				}
		}
		   
		   if (forLeader) {
				if(counter<3) {
				   //cout << "Leader is listening!" << endl;
				   nbytes =listen_to_robot(lfd, msgTM);  //leader is listening
					if(nbytes == 0) continue;
					   cout << "Leader is listening." << endl;
						cout << "msg: " << msgTM << endl;
						//exit(1);
						parse_msgTM(msgTM, &xLeader, &yLeader, &getFormation, &kindOfMsg);
						
						if (waypoint==0){
						   firstWaypoint = true;
							waypoint++; counter++;}
						else if (waypoint==1){
						   secondWaypoint = true;
							waypoint++; counter++;}
						else if (waypoint == 2){
							thirdWaypoint = true;
							waypoint++; counter++;}
						else { done = true;}
				} else {
				   //cout << "Leader is listening again!" << endl;
				   nbytes =listen_to_robot(lfd, msgTM);  //leader is listening
				   if (strcmp(msgTM, "h")==0){
		         cout << "All waypoints reached. Program was successful!" << endl;
		         pp.SetSpeed(0.0, 0.0);
		         //sleep(1);
		         exit(1);
		         }
				}		  
			} //end for if
			
			if (forFollower) {
				//cout << "Follower " << port << " is listening!" << endl;
				   nbytes =listen_to_robot(lfd, msgL);  //leader is listening
					if(nbytes == 0) continue;
					   cout << "Follower " << port <<" received Leader message: " << msgL << endl;
					   //cout << "Current formation: " << setFormation << endl;
					   if (strcmp(msgL, "a")==0){
 					      //cout << "Hi!" << endl;
 					      counter++;
 					      if (counter==3) {
 					      pp.SetSpeed(0.0,0.0);
 					      cout << "Success!" << endl;
 					      exit(0);
 					      }
					   } else {parse_msgTM(msgL, &xLeader, &yLeader, &getFormation, &kindOfMsg);}
					   cout << "Current formation: " << getFormation << endl;
/****************************************************************/				
					if (port == 6666) {
					   
					   robot.Read();
					   x = pp.GetXPos(); y = pp.GetYPos();
					
						if(getFormation == 'l'){
							
							//cout << "I'm inside get Formation" << endl;
							
							follX = xLeader;
							follY = yLeader+1;
							
							if (getFormation == 'l') {
							   if (xLeader - x >= 0.3) {
								   pp.SetSpeed(10,0);
							   }
							else
								pp.GoTo(follX,follY,0);
						   }
					   }
					   
					   if(getFormation == 'd'){
							follX = xLeader - 1;
							follY = yLeader + 1;
							
							if(xLeader - x >= 1)
								pp.GoTo(follX,follY,0);
							else
								pp.SetSpeed(0.2,0.0);
			         }
			       }
/****************************************************************/
                if (port == 6667) {
					   
					   robot.Read();
					   x = pp.GetXPos(); y = pp.GetYPos();
					
						if(getFormation == 'l'){
							
							//cout << "I'm inside get Formation" << endl;
							
							follX = xLeader;
							follY = yLeader-1;
							
							if (getFormation == 'l') {
							   if (xLeader - x >= 0.3) {
								   pp.SetSpeed(10,0);
							   }
							else
								pp.GoTo(follX,follY,0);
						   }
					   }
					   
					   if(getFormation == 'd'){
							follX = xLeader - 1;
							follY = yLeader - 1;
							
							if(xLeader - x >= 1)
								pp.GoTo(follX,follY,0);
							else
								pp.SetSpeed(0.2,0.0);
			         }
			       }
/****************************************************************/
                if (port == 6668) {
					   
					   robot.Read();
					   x = pp.GetXPos(); y = pp.GetYPos();
					
						if(getFormation == 'l'){
							
							//cout << "I'm inside get Formation" << endl;
							
							follX = xLeader;
							follY = yLeader-2;
							
							if (getFormation == 'l') {
							   if (xLeader - x >= 2) {
								   pp.SetSpeed(2,0);
							   }
							else
								pp.GoTo(follX,follY,0);
						   }
					   }
					   
					   if(getFormation == 'd'){
							follX = xLeader - 2;
							follY = yLeader;
							
							if(xLeader - x >= 2)
								pp.GoTo(follX,follY,0);
							else
								pp.SetSpeed(0.0,0.0);
								
							if(xLeader - x >= 2.8 && yLeader - y <= 0.1){
										pp.SetSpeed(0.6,0.0);
									}	
			         }
			       }
/****************************************************************/
		   } // end of if (forFollower)
		
				while (firstWaypoint || secondWaypoint || thirdWaypoint) {
					   
					   robot.Read();
					   
					   // Leader sends message
					   send_cmdTM(bfd, pp.GetXPos(), pp.GetYPos(), getFormation);
						//pp.GoTo(order[0].x, order[0].y, 0);
						//cout << "Leader sent msg: " << getFormation << endl;
						//exit(1);
						
						if (getFormation == 'l')
							pp.SetSpeed(0.2, 0); //OG
					   else {
					      if (lCounter<60){
					   	   pp.SetSpeed (0.5,0);
					   	   lCounter++;}
					   	else
					   	   pp.SetSpeed(0.2,0);
					   }
					   
					   x = pp.GetXPos(); y = pp.GetYPos();
		            cout << "Current speed & yaw: " << "(" << pp.GetXSpeed() << ", " << pp.GetYaw() << ")" << endl;
		            cout << "Waypoint " << waypoint << ": ( " << xLeader << ", " << yLeader << ")" << endl;        

						if((xLeader-withinRange <= x) && (x <= xLeader+withinRange)) { 
						   xWaypoints.pop(); yWaypoints.pop();
						   cout << "Arrived" << endl;
							pp.SetSpeed(0,0);
					
							// Send message to TM letting it know Leader arrived to waypoint
							*setCmd = 'a';
							s_cmd(bfd, setCmd);
							
							if (waypoint == 1){
							   firstWaypoint = false;}
							else if (waypoint == 2){
								secondWaypoint = false;}
							else if (waypoint == 3) {
							   thirdWaypoint = false;}
							   
							break;
						} //end for if
				} //end for while
				  
			if (listeningTM) {		
			   //cout <<  "TM is listening!" << endl;
			   //exit(1);
				nbytes =listen_to_robot(lfdTM, msg);  //TM is listening
		      if(nbytes == 0) continue;
		      //cout << "\nTask Manager is listening." << endl;
		      //sleep(2);
		      if (strcmp(msg, "a")==0){
		         cout << "TM got msg: " << msg << endl;
		         forTM = true;
		         listeningTM = false;
		         leaderCounter = true;
		      }
		   } // end of if (listeningTM)
   } // end of while   
   } // end of try
		 catch (PlayerCc::PlayerError & e){
		   std::cerr << e << std::endl;
		   return -1;
		 }
} // end of main	
