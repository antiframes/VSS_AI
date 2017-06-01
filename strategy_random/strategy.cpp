// Strategy.cpp : Defines the entry point for the DLL application.
//
#define STRATEGY_EXPORTS
#define _STDC_

//#include "stdafx.h"
#include "strategy.h"
#include "ai.h"
#include "basicMove.h"
#include "../strategy/base.h"
#include <math.h>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>


#define DELTA 10


typedef std::chrono::high_resolution_clock Clock;


//um ou outro (REDE, TECLADO, FUNCAO)
#define FUNCAO




///////////////////////////////////////
// Constants that have to be optimized.
int Q; //20
int Qi; //5
///////////////////////////////////////

long timespent;
bool carryon=false;
Environment *env;



float distTo(int x1,int x2,int y1,int y2){
    return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}


std::vector<bool> isStanding;
double dest[3][2];
double standOrigin[3][2];

bool mustChangeDestination(double x, double y, double destX, double destY);
void sendVector(bool change1,bool change2,bool change3, base *cp);
int team=1;
void chooseNextPoint(int directionId, int i);


int main(int argc, char **argv) {
    double duration;
    env = new Environment();
    Ai *ai=new Ai();

    //int foo [3]= ai.receiveAction(*env);
    isStanding.push_back(false);
    isStanding.push_back(false);
    isStanding.push_back(false);


    base *cp = new base((int) (argv[2][0] - '0'));
    pthread_t thr;


    if(argc < 5) {
	cerr << "usage ./bin/strategy_pf2 hostname team_number Q Qi" << endl;
	exit(1);
    }

    if(argc >= 8 && strcmp(argv[3], "-set_ports") == 0) {
	sscanf(argv[4], " %d ", &(cp->p_es));
	sscanf(argv[5], " %d ", &(cp->p_er));
	printf("strategy_pf2: ports: send %d. recv %d.\n", cp->p_es, cp->p_er);
	sscanf(argv[6], " %d ", &Q);
	sscanf(argv[7], " %d ", &Qi);

    } else {
	sscanf(argv[3], " %d ", &Q);
	sscanf(argv[4], " %d ", &Qi);
	if(cp->get_team_number() == 1) {
	    cp->p_es = PORTA_E1_RECV;
	    cp->p_er = PORTA_E1_SEND;
	} else {
	    cp->p_es = PORTA_E2_RECV;
	    cp->p_er = PORTA_E2_SEND;
	}
    }

    int arg2=0;
	sscanf(argv[2], " %d ", &arg2);
    printf("%d",arg2);
    bool leftTeam = false;
    if (arg2==2)
        leftTeam=true;
    ai->leftTeam=leftTeam;
    team = arg2;

    cp->set_hostname(argv[1]);
    cp->conct();

    for (int i=0;i<N_JOGADORES_POR_TIME;i++){

    if(leftTeam) dest[i][0]=60;
    else dest[i][0]=30;
    }

    dest[0][1] = 20;
    dest[1][1] = 40;
    dest[2][1] = 60;

    if(leftTeam) dest[1][0]=90;
    else dest[1][0]=10;

    bool starting=true;
    //looper principal
    while(1) {
	cp->rcv();

    env->lastBall.pos.x = env->currentBall.pos.x;
    env->lastBall.pos.y = env->currentBall.pos.y;
    //printf("========= begin\n");
    env->currentBall.pos.x = cp->uspds_to_fira_x(cp->get_bx());
    env->currentBall.pos.y = cp->uspds_to_fira_y(cp->get_by());
    for(int i = 0; i < N_JOGADORES_POR_TIME; i++) {
        env->home[i].pos.x = cp->uspds_to_fira_x(cp->get_pax(i));
        env->home[i].pos.y = cp->uspds_to_fira_y(cp->get_pay(i));
        env->home[i].rotation = cp->uspds_to_fira_a(cp->get_paa(i));
        env->opponent[i].pos.x = cp->uspds_to_fira_x(cp->get_pbx(i));
        env->opponent[i].pos.y = cp->uspds_to_fira_y(cp->get_pby(i));

    }

    Environment *curEnvironment = new Environment();
    *curEnvironment = *env;

    if (starting){
        ai->init(curEnvironment);
        starting=false;
    }

    //int *directions = ai->receiveAction(env);

    bool change[3]={false,false,false};

        for (int i=0;i<3;i++){
            if (mustChangeDestination(env->home[i].pos.x,env->home[i].pos.y,dest[i][0],dest[i][1])){
                if(isStanding[i]){
                    isStanding[i]=false;
                    dest[i][0]=standOrigin[i][0];
                    dest[i][1]=standOrigin[i][1];
                }
                else{
                    change[i]=true;
                    ai->insertIntoBuffer(curEnvironment,i);
                    int nextAction  = ai->chooseDirection(env,i);
                    chooseNextPoint(nextAction,i);
                    ai->sendToServer(team,i,nextAction,env);

                    ai->getRewards(i);
                }
            }
        }
    
    
        if(env->currentBall.pos.x>91){
            printf("GOL DO 1\n");

            ai->sendGoalToServer(1,arg2,env);
        }
        if(env->currentBall.pos.x<9){
            printf("GOL DO 2\n");
            ai->sendGoalToServer(2,arg2,env);
        }

    //conduzir robôs ao ponto
    for (int i=0;i<N_JOGADORES_POR_TIME;i++){
        GotoXY(env->home+i, dest[i][0],dest[i][1]);
	    cp->set_vel(i, cp->fira_to_uspds_vel(env->home[i].velocityLeft), cp->fira_to_uspds_vel(env->home[i].velocityRight));
    }



	//cp->snd(change[0],change[1],change[2]);
    //sendVector(change[0],change[1],change[2],cp);
    cp->snd();
    }
    pthread_join(thr,NULL);

    printf("CLOSING\n");
	return 0;
}


#define LIMIT_LEFT 96
#define LIMIT_RIGHT 8
#define LIMIT_UP 4
#define LIMIT_DOWN 80


void chooseNextPoint(int directionId, int i){
    float percentage ;
    float distToBall;float distanceToCenter;

    switch(directionId){
            case 0://norte
                dest[i][1] = env->home[i].pos.y - DELTA;
                break;
            case 1://sul
                dest[i][1] = env->home[i].pos.y + DELTA;
                break;
            case 2://leste
                dest[i][0] = env->home[i].pos.x - DELTA;
                break;
            case 3://oeste
                dest[i][0] = env->home[i].pos.x + DELTA;
                break;
            case 4://noroeste
                dest[i][0] = env->home[i].pos.x + DELTA;
                dest[i][1] = env->home[i].pos.y - DELTA;
                break;
            case 5://nordeste
                dest[i][1] = env->home[i].pos.y - DELTA;
                dest[i][0] = env->home[i].pos.x - DELTA;
                break;
            case 6://sudeste
                dest[i][1] = env->home[i].pos.y + DELTA;
                dest[i][0] = env->home[i].pos.x - DELTA;
                break;
            case 7://sudoeste
                dest[i][1] = env->home[i].pos.y + DELTA;
                dest[i][0] = env->home[i].pos.x + DELTA;
                break;
            case 8:
                distToBall = distTo(env->home[i].pos.x,env->currentBall.pos.x,env->home[i].pos.y,env->currentBall.pos.y);
                percentage = 10/distToBall;

                dest[i][1] = (env->home[i].pos.y*(1-percentage)) + (env->currentBall.pos.y*(percentage));
                dest[i][0] = (env->home[i].pos.x*(1-percentage)) + (env->currentBall.pos.x*(percentage));
                break;
            case 9:
                distanceToCenter = distTo(env->home[i].pos.x,50,env->home[i].pos.y,50);
                percentage = 5/distanceToCenter;

                dest[i][1] = (env->home[i].pos.y*(1-percentage)) + (50*(percentage));
                dest[i][0] = (env->home[i].pos.x*(1-percentage)) + (50*(percentage));

                standOrigin[i][0]=env->home[i].pos.x;
                standOrigin[i][1]=env->home[i].pos.y;

                isStanding[i]=true;
                break;
            }


            if (dest[i][0]>(LIMIT_LEFT - DELTA))
                dest[i][0]=LIMIT_LEFT-DELTA;
            if (dest[i][1]>(LIMIT_DOWN - DELTA))
                dest[i][1]=LIMIT_DOWN-DELTA;
            if (dest[i][0]<(LIMIT_RIGHT + DELTA))
                dest[i][0]=LIMIT_RIGHT + DELTA;
            if (dest[i][1]<(LIMIT_UP + DELTA))
                dest[i][1]=LIMIT_UP + DELTA;

}




bool mustChangeDestination(double x, double y, double destX, double destY){
    return
        (sqrt(pow(destX-x,2) + pow(destY-y,2))< 3)
        ;
}

void sendVector(bool change1,bool change2,bool change3, base *cp){
    if (!change1 && !change2 && !change3){
        cp->snd();return;
    }
    float *data;
    data = (float *) malloc(sizeof(float) * 17);
    data[0] = env->home[0].pos.x;
    data[1] = env->home[0].pos.y;
    data[2] = env->home[1].pos.x;
    data[3] = env->home[1].pos.y;
    data[4] = env->home[2].pos.x;
    data[5] = env->home[2].pos.y;
    data[6] = env->opponent[0].pos.x;
    data[7] = env->opponent[0].pos.y;
    data[8] = env->opponent[1].pos.x;
    data[9] = env->opponent[1].pos.y;
    data[10] = env->opponent[2].pos.x;
    data[11] = env->opponent[2].pos.y;
    data[12] = env->currentBall.pos.x;
    data[13] = env->currentBall.pos.y;
    data[14] = 0;
    if(change1) data[14]=1;
    data[15] = 0;
    if(change2) data[15]=1;
    data[16] = 0;
    if(change3) data[16]=1;

    cp->snd(data);
}
