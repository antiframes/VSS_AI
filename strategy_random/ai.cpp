#include "ai.h"
#include "strategy.h"
#include <math.h>
#include <iostream>
#include <curl/curl.h>

#include <cstring>
#include <string>

#define BUFFERSIZE 10
#define ACTIONSIZE 4

float distanceTo(int x1,int x2,int y1,int y2){
    return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}


int* Ai::receiveAction(Environment *env){
    int *foo = (int*) malloc(sizeof(int)*3);
    for (int i=0;i<3;i++)
        foo[i]=chooseDirection(env,i);
    return foo;
}



int Ai::chooseDirection(Environment *env, int i){

int dir;
    bool satisfied=false;
    while (!satisfied){
        dir = (rand()%10) ;
        /*if (dir==0 || dir == 4 || dir==5 || dir==8)
            if (env->home[i].pos.y >25) satisfied=true;
        if (dir==1 || dir == 6 || dir==7 || dir==8)
            if (env->home[i].pos.y <65) satisfied=true;
        if (dir==2 || dir == 6 || dir==5 || dir==8)
            if (env->home[i].pos.x >40) satisfied=true;
        if (dir==3 || dir == 4 || dir==7 || dir==8)
            if (env->home[i].pos.x <75) satisfied=true;*/
        satisfied=true;
    }
    switch(i){
        case 0:
            actionBufferAtk[bufferPos[0]] = dir;
        case 1:
            actionBufferGk[bufferPos[1]] = dir;
        case 2:
            actionBufferDef[bufferPos[2]] = dir;
    }
    /*dir = med->getAction(i,
        env->home[0].pos.x,env->home[0].pos.y,env->home[1].pos.x,env->home[1].pos.y,env->home[2].pos.x,env->home[2].pos.y,
        env->opponent[0].pos.x,env->opponent[0].pos.y,env->opponent[1].pos.x,env->opponent[1].pos.y,env->opponent[2].pos.x,env->opponent[2].pos.y,
        env->currentBall.pos.x,env->currentBall.pos.y);*/

    //std::cout<<"direction is "<<dir<<"\n";
    return dir;
}

void Ai::toto(){
    std::cout<<"FOO\n";
}

Ai::Ai(){
}

void Ai::init(Environment *env){
    med=new NetworkMediator();

    for(int i=1;i<ACTIONSIZE;i++){
        sigma+=i;
    }

    for (int i=0;i<BUFFERSIZE;i++){
        envBufferAtk.push_back(*env);
        envBufferDef.push_back(*env);
        envBufferGk.push_back(*env);

        rewardBufferAtk.push_back(0);
        rewardBufferDef.push_back(0);
        rewardBufferGk.push_back(0);

        actionBufferAtk.push_back(0);
        actionBufferDef.push_back(0);
        actionBufferGk.push_back(0);
    }


    for (int i=0;i<3;i++){
        bufferPos[i]=0;
        //med->setWeights(i);
    }
}

void Ai::insertIntoBuffer(Environment *env, int i){
 switch (i){
     case 0:
        envBufferAtk[bufferPos[0]++]=*env;
        break;
     case 1:
        envBufferGk[bufferPos[1]++]=*env;
        break;
     case 2:
        envBufferDef[bufferPos[2]++]=*env;
        break;

 }
 if (bufferPos[i]==BUFFERSIZE) bufferPos[i]=0;

}

void Ai::getRewards(int i){
    switch(i){
        case 0:
            getRewardAtk();
            break;
        case 1:
            getRewardGk();
            break;
        case 2:
            getRewardDef();
            break;
    }

    int currentIndex = (bufferPos[1])-1;
    if (currentIndex<0) {
        currentIndex = BUFFERSIZE+currentIndex;

    }





}
void Ai::getRewardAtk(){
    int secondIndex = (bufferPos[1])-1;
    if (secondIndex<0) {
        secondIndex = BUFFERSIZE+secondIndex;

    }
    int firstIndex = (secondIndex-ACTIONSIZE);
    if (firstIndex<0){
        firstIndex = BUFFERSIZE+firstIndex;
    }



    Environment first = envBufferGk[firstIndex];
    Environment second = envBufferGk[secondIndex];

    int x = second.home[2].pos.x;
    int y = second.home[2].pos.y;

    int ballX = second.currentBall.pos.x;
    int ballY = second.currentBall.pos.y;


    int situation=0;

    int goalPos = 10;
    if (leftTeam) goalPos=90;

    int myGoalPos = 90;
    if (leftTeam) myGoalPos =10;


    if (ballX>90 || ballX<10){//houve gol
        if (leftTeam){
            if (ballX<10)// gol pró?
                situation+=200;
        }
        else if (ballX>90)// gol pró?
                situation+=200;

    }
    else{
        //bola perto do gol?
        float ballDistanceToGoal =  distanceTo(x,y,goalPos,50);
        if (ballDistanceToGoal<20) situation += 50;

        float distanceToBall = distanceTo(x,y,ballX,ballY);
        if (distanceToBall<20)//perto da bola?
            situation+= ((20-distanceToBall)*2);

        for (int i=0;i<3;i++){//muito perto de adversário?
            float distanceToOpponent = distanceTo(x,y,second.opponent[i].pos.x,second.opponent[i].pos.y);
            if (distanceToOpponent<10)
                situation-=((10-distanceToOpponent)*3);
        }

        //perto da área adversária?
        if (leftTeam){
            if (x<50) situation += ((100-x)/5);}
        else if (x>50) situation += x/5;
    }


    rewardBufferAtk[secondIndex] = situation;


   float totalReward=situation;
/* 
    for(int i=0;i<ACTIONSIZE;i++){
        int fstPos = (firstIndex+i)%BUFFERSIZE;
        int secPos = (fstPos+1)%BUFFERSIZE;

        float weight = (i+1.0)/(sigma*1.0);

        totalReward+=(rewardBufferAtk[secPos] - rewardBufferAtk[fstPos]) * weight;
    }
*/
    med->saveReward(0,
        first.home[0].pos.x,first.home[0].pos.y,first.home[1].pos.x,first.home[1].pos.y,first.home[2].pos.x,first.home[2].pos.y,
        first.opponent[0].pos.x,first.opponent[0].pos.y,first.opponent[1].pos.x,first.opponent[1].pos.y,first.opponent[2].pos.x,first.opponent[2].pos.y,
        first.currentBall.pos.x,first.currentBall.pos.y,actionBufferAtk[firstIndex],totalReward);
}

void Ai::getRewardDef(){
    int secondIndex = (bufferPos[1])-1;
    if (secondIndex<0)
        secondIndex = BUFFERSIZE+secondIndex;


    int firstIndex = (secondIndex-ACTIONSIZE);

    if (firstIndex<0)
        firstIndex = BUFFERSIZE+firstIndex;




    Environment first = envBufferDef[firstIndex];
    Environment second = envBufferDef[secondIndex];

    int x = second.home[2].pos.x;
    int y = second.home[2].pos.y;

    int ballX = second.currentBall.pos.x;
    int ballY = second.currentBall.pos.y;


    int situation=0;

    int goalPos = 10;
    if (leftTeam) goalPos=90;

    int myGoalPos = 90;
    if (leftTeam) myGoalPos =10;


    if (ballX>90 || ballX<10){//houve gol
        if (leftTeam){
            if (ballX>90)// gol contra?
                situation-=50;
            else if (ballX<10)// gol pró?
                situation+=100;
        }
        else{

        }
            if (ballX<10)// gol contra?
                situation-=50;
            else if (ballX>90)// gol pró?
                situation+=100;
        }

        float distanceToBall = distanceTo(x,y,ballX,ballY);
        if (distanceToBall<20)//perto da bola?
            situation+= ((20-distanceToBall)*2);
        for (int i=0;i<3;i++){//muito perto de adversário?
            float distanceToOpponent = distanceTo(x,y,second.opponent[i].pos.x,second.opponent[i].pos.y);
            if (distanceToOpponent<10)
                situation-=((10-distanceToOpponent)*3);
        }

        //longe da área?
        if (leftTeam){

            if (x<50) situation -= (100-x);
            else if (x>50) situation -= x;
        }



    rewardBufferDef[secondIndex] = situation;


    float totalReward=situation;

   /* for(int i=0;i<ACTIONSIZE;i++){
        int fstPos = (firstIndex+i)%BUFFERSIZE;
        int secPos = (fstPos+1)%BUFFERSIZE;

        float weight = (i+1.0)/(sigma*1.0);

        totalReward+=(rewardBufferDef[secPos] - rewardBufferDef[fstPos]) * weight;
    }
*/

    med->saveReward(2,
        first.home[0].pos.x,first.home[0].pos.y,first.home[1].pos.x,first.home[1].pos.y,first.home[2].pos.x,first.home[2].pos.y,
        first.opponent[0].pos.x,first.opponent[0].pos.y,first.opponent[1].pos.x,first.opponent[1].pos.y,first.opponent[2].pos.x,first.opponent[2].pos.y,
        first.currentBall.pos.x,first.currentBall.pos.y,actionBufferDef[firstIndex],totalReward);
}

void Ai::getRewardGk(){//RECOMPENSA DO GOLEIRO

    int secondIndex = (bufferPos[1])-1;
    if (secondIndex<0) {
        secondIndex = BUFFERSIZE+secondIndex;

    }
    int firstIndex = (secondIndex-ACTIONSIZE);
    if (firstIndex<0){
        firstIndex = BUFFERSIZE+firstIndex;
    }



    Environment first = envBufferGk[firstIndex];
    Environment second = envBufferGk[secondIndex];


    //Pegar distância para o gol
    //Registrar gol contra
    int x1 = first.home[1].pos.x;
    int y1 = first.home[1].pos.y;

    int x2 = second.home[1].pos.x;
    int y2 = second.home[1].pos.y;

    int situation=0;

    int goalPos = 10;
    if (leftTeam) goalPos=90;

    float distanceToGoal = distanceTo(x2,y2,goalPos,50);
    if (second.currentBall.pos.x<goalPos)
        situation-=100;
    else if (distanceToGoal>15)
        situation-=distanceToGoal;


    rewardBufferGk[secondIndex] = situation;



    float totalReward=situation;
/*
    for(int i=0;i<ACTIONSIZE;i++){
        int fstPos = (firstIndex+i)%BUFFERSIZE;
        int secPos = (fstPos+1)%BUFFERSIZE;

        float weight = (i+1.0)/(sigma*1.0);

        totalReward+=(rewardBufferGk[secPos] - rewardBufferGk[fstPos]) * weight;
    }
*/

    med->saveReward(1,
        first.home[0].pos.x,first.home[0].pos.y,first.home[1].pos.x,first.home[1].pos.y,first.home[2].pos.x,first.home[2].pos.y,
        first.opponent[0].pos.x,first.opponent[0].pos.y,first.opponent[1].pos.x,first.opponent[1].pos.y,first.opponent[2].pos.x,first.opponent[2].pos.y,
        first.currentBall.pos.x,first.currentBall.pos.y,actionBufferGk[firstIndex],totalReward);

}

struct weightstring {
    char *ptr;
    size_t len;
    };
    
    void initstring(struct weightstring *s) {
        s->len = 0;
        s->ptr = (char *) malloc(s->len+1);
        if (s->ptr == NULL) {
            fprintf(stderr, "malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        s->ptr[0] = '\0';
        }

    size_t wrtfunc(void *ptr, size_t size, size_t nmemb, struct weightstring *s)
        {
        size_t new_len = s->len + size*nmemb;
        s->ptr = (char *) realloc(s->ptr, new_len+1);
        if (s->ptr == NULL) {
            fprintf(stderr, "realloc() failed\n");
            exit(EXIT_FAILURE);
        }
        std::memcpy(s->ptr+s->len, ptr, size*nmemb);
        s->ptr[new_len] = '\0';
        s->len = new_len;

        return size*nmemb;
    }


std::string makeString(int team,int change,int action, Environment * env){
        std::string s = "http://localhost:7777/send1?team=";
        s=s+std::to_string(team);
        s = s+"&l1x=";
        s = s+std::to_string(env->home[0].pos.x);
        s = s+"&l1y=";
        s = s+std::to_string(env->home[0].pos.y);
        s = s+"&l2x=";
        s = s+std::to_string(env->home[1].pos.x);
        s = s+"&l2y=";
        s = s+std::to_string(env->home[1].pos.y);
        s = s+"&l3x=";
        s = s+std::to_string(env->home[2].pos.x);
        s = s+"&l3y=";
        s = s+std::to_string(env->home[2].pos.y);
        s = s+"&r1x=";
        s = s+std::to_string(env->opponent[0].pos.x);
        s = s+"&r1y=";
        s = s+std::to_string(env->opponent[0].pos.y);
        s = s+"&r2x=";
        s = s+std::to_string(env->opponent[1].pos.x);
        s = s+"&r2y=";
        s = s+std::to_string(env->opponent[1].pos.y);
        s = s+"&r3x=";
        s = s+std::to_string(env->opponent[2].pos.x);
        s = s+"&r3y=";
        s = s+std::to_string(env->opponent[2].pos.y);
        s = s+"&ballx=";
        s = s+std::to_string(env->currentBall.pos.x);
        s = s+"&bally=";
        s = s+std::to_string(env->currentBall.pos.y);
        s = s+"&change=";
        s = s+std::to_string(change);
        s = s+"&action=";
        s = s+std::to_string(action);

        return s;
    }

std::string makeGoalString(int team,int myteam, Environment * env){
    std::string s = "http://localhost:7777/goal?team=";
    s=s+std::to_string(team);
    s = s+"&myteam=";
    s = s+std::to_string(myteam);
        s = s+"&l1x=";
        s = s+std::to_string(env->home[0].pos.x);
        s = s+"&l1y=";
        s = s+std::to_string(env->home[0].pos.y);
        s = s+"&l2x=";
        s = s+std::to_string(env->home[1].pos.x);
        s = s+"&l2y=";
        s = s+std::to_string(env->home[1].pos.y);
        s = s+"&l3x=";
        s = s+std::to_string(env->home[2].pos.x);
        s = s+"&l3y=";
        s = s+std::to_string(env->home[2].pos.y);
        s = s+"&r1x=";
        s = s+std::to_string(env->opponent[0].pos.x);
        s = s+"&r1y=";
        s = s+std::to_string(env->opponent[0].pos.y);
        s = s+"&r2x=";
        s = s+std::to_string(env->opponent[1].pos.x);
        s = s+"&r2y=";
        s = s+std::to_string(env->opponent[1].pos.y);
        s = s+"&r3x=";
        s = s+std::to_string(env->opponent[2].pos.x);
        s = s+"&r3y=";
        s = s+std::to_string(env->opponent[2].pos.y);
        s = s+"&ballx=";
        s = s+std::to_string(env->currentBall.pos.x);
        s = s+"&bally=";
        s = s+std::to_string(env->currentBall.pos.y);
    return s;
}

    
void Ai::sendToServer(int team,int change,int action, Environment * env){
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, makeString(team,change,action,env).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wrtfunc);
    struct weightstring ans;
    initstring(&ans);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ans);
    res = curl_easy_perform(curl);
}

void Ai::sendGoalToServer(int team,int myteam, Environment * env){
    CURL *curl2;
    CURLcode res2;
    curl2 = curl_easy_init();
    curl_easy_setopt(curl2, CURLOPT_URL, makeGoalString(team,myteam,env).c_str());
    curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, wrtfunc);
    struct weightstring ans2;
    initstring(&ans2);
    curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &ans2);
    res2 = curl_easy_perform(curl2);


    std::cout<<"GOOOOOOL "<<ans2.ptr<<"\n";
}