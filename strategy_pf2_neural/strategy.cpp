// Strategy.cpp : Defines the entry point for the DLL application.
//
#define STRATEGY_EXPORTS
#define _STDC_

//#include "stdafx.h"
#include "strategy.h"
#include "ai.h"
#include "futbot.h"
#include "basicMove.h"
#include "ForceField.h"
#include "../strategy/base.h"
#include <math.h>

#define DELTA 10


//#define GRAVAR_DADO

//um ou outro (REDE, TECLADO, FUNCAO)
#define FUNCAO

float pIn[NO_OF_IN_UNITS];
float pOut[NO_OF_OUT_UNITS];

int chooseNextPoint(int angle, int i);
double dest[3][2];


bool mustChangeDestination(double x, double y, double destX, double destY){
    float distanceToDestination=sqrt(pow(destX-x,2) + pow(destY-y,2));
    if (distanceToDestination< 4) return true;
        else return false;
}

float distTo(int x1,int x2,int y1,int y2){
    return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}


char myMessage[200]; //big enough???

///////////////////////////////////////
// Constants that have to be optimized.
int Q; //20
int Qi; //5
///////////////////////////////////////


Environment *env;

float best_gaussian(float *y);
FILE *arq;


int Defesa(Environment *env, Robot *robot, int X)
{
	double py,px,a,b,Y;
	//Ball predictedBall;

  #ifdef BLUE
		px = GRIGHT;
	#else
		px = GLEFT;
	#endif
	py = GTOPY - (GTOPY-GBOTY)/2;

	//PredictBall(env, predictedBall);

	EquacaoReta(px,py,env->currentBall.pos.x,env->currentBall.pos.y,a,b);
   Y = a * X + b;

#ifdef BLUE
	if(env->currentBall.pos.x > X+3)
		Y = py;
	else if(env->currentBall.pos.x > X-12)
	{
		X = (double) env->currentBall.pos.x; Y = (double) env->currentBall.pos.y;
	}

#else
	if(env->currentBall.pos.x < X-3)
		Y = py;
	else if(env->currentBall.pos.x < X+12)
	{
		X = env->currentBall.pos.x; Y = env->currentBall.pos.y;
	}
#endif

   if(robot == env->home) //goleiro
	{
		if(Y > GTOPY+6) Y = GTOPY + 5;
		else if(Y < GBOTY-6) Y = GBOTY - 5;
	}

//GotoXY(robot,X,Y);


  tForce Fa;
  double dx = X - robot->pos.x;
	double dy = Y - robot->pos.y;
  Fa.setXYMod(dx,dy,1);
  return Fa.ang();

}

int chute;
int FollowBall(Environment *env, Robot *robot){
	const int K=1;

	tForce F,Fa,Fr,Fro,Fbola;
	//Ball predictedBall;
	double dx,dy,px,py,aux;
	int i;
	double j,k,a,b,ra,rb;

	//PredictBall(env, predictedBall);
	//calculo da for�a de atra�ao com a bola
	dx = env->currentBall.pos.x - robot->pos.x;
	dy = env->currentBall.pos.y - robot->pos.y;

	Fa.setXYMod(dx,dy,K); //m�dulo constante

	//para cada rob� calcular a for�a de repulsao
	for(i=0; i<3; i++)
	{
		if(robot != env->home+i)
		{
			dx = env->home[i].pos.x - robot->pos.x;
			dy = env->home[i].pos.y - robot->pos.y;

			F.setXY(dx,dy);
			Fr = Fr - F;
		}


		dx = env->opponent[i].pos.x - robot->pos.x;
		dy = env->opponent[i].pos.y - robot->pos.y;

		F.setXY(dx,dy);
		Fro = Fro - F;
	}
	if(robot == env->home + 2) {
	    dx = env->home[3].pos.x - robot->pos.x;
	    dy = env->home[3].pos.y - robot->pos.y;
	    Fr.setXY(-dx, -dy);
	}


	//para fazer o rob� encontrar o �ngulo certo para chutar a bola!
	py = (GTOPY + GBOTY)/2;
	#ifdef BLUE
		px = GLEFT;
	#else
		px = GRIGHT;
	#endif

	//encontrado o ponto atras da reta que direciona para o gol
	EquacaoReta(px,py,env->currentBall.pos.x,env->currentBall.pos.y,a,b);
	dx = 10.0 / sqrt(a*a + 1); //circunferencia de raio 6
	dy = dx * a;

	//econtrar o coeficiente angular entre o gol e o robo (ra)
	EquacaoReta(px,py,robot->pos.x,robot->pos.y,ra,rb);

	px = env->currentBall.pos.x;
	py = env->currentBall.pos.y;

	//se o rob� nao estiver na faixa permitida para chute entao
	//bola repele e ponto atr�s da bola atrai (se o ponto estiver no campo)
	//faixa permitida inclinacao da reta bola-ponto menos a a inclinacao da reta bola-robo
	//ou seja se dejejarmos uma fatia de 5� entao |ra-a|=tan(5�)
	//EquacaoReta(robot->pos.x,robot->pos.y,predictedBall.pos.x,predictedBall.pos.y,ra,rb);

	//if(py < FTOP && py > FBOT && px > FLEFTX && px < FRIGHTX)
	//if(fabs(dx)>3 || fabs(dy)>2)

    rb = fabs(fabs(ra)-fabs(a)); //subtracao dos coeficientes angulares

#ifdef BLUE
	if(robot->pos.x < env->currentBall.pos.x || rb > 0.364) //20 graus
#else
	if(robot->pos.x > env->currentBall.pos.x || rb > 0.364) //20 graus
#endif
	if(py > FBOT && py < FTOP) //se o ponto est� dentro do campo
	{
		//novo ponto de atra��o atr�s da bola
		dx = px - robot->pos.x;
		dy = py - robot->pos.y;
		Fa.setXYMod(dx,dy,K);

		//bola repele
		dx = env->currentBall.pos.x - robot->pos.x;
		dy = env->currentBall.pos.y - robot->pos.y;
		F.setXY(dx,dy); //bola repelindo
		F = F*2;
		Fr = Fr - F;  //repele na 2*intensidade dos amigos)
	}

	//For�a Resultante
	Fr = Fr  * Q;
	Fro= Fro * Qi;

	F = Fr + Fro;
	F = F + Fa;
	//Fa.setXY(((FRIGHTX + FLEFTX) / 2) - robot->pos.x, ((FTOP + FBOT) / 2) - robot->pos.y);
	//F = Fa + Fr;

	//GotoXY(robot, ((FRIGHTX + FLEFTX) / 2), ((FTOP + FBOT) / 2));
  //return 315;
	return F.ang();
}



float best_gaussian(float *y)
{
	float min_err, b, xi, err, a=0.0, aux=0.0;
	int i;

	min_err = 100000.0;
	for (b=-180.0; b<=528.0; b+=12.0 )
		{
			err = 0.0;
			for ( i=0; i < 60; i++ )
				{
					xi = (float) (i-15) * 12.0;
					aux = (float)exp((double)(-1)*((xi - b)*(xi - b)) * 50);
					err += (y[i]  - aux)* (y[i]  - aux);
				}
			if (err < min_err)
				{
					min_err = err;
					a = b;
				}
		}

	return  a;

}

/*int goalkeeper(int i){
    int goalX,ballX,ballY;

    #ifdef BLUE
    		goalX=92;
    #else
    		goalX=8;
    #endif

    ballX = env->currentBall.pos.x;
    ballY = env->currentBall.pos.y;
     float distanceToGoal = sqrt(pow(ballX-goalX,2) + pow(ballY-40,2));
    if (distanceToGoal>20) {
        GotoXY(env->home+i,goalX,40);
    }
    else{
      GotoXY(env->home+i, ballX,ballY);
    }
    return 1;

}*/

bool leftTeam = false;
int goalkeeper(int i){
    int goalX,ballX,ballY;

        if (leftTeam)
        		goalX=90;
        else
        		goalX=10;
    ballX = env->currentBall.pos.x;
    ballY = env->currentBall.pos.y;
      float distanceToGoal = sqrt(pow(ballX-goalX,2) + pow(ballY-40,2));
     tForce Fa;
     double dx,dy;
         return -2;
    if (distanceToGoal>20)
        return -2;
    dx = env->currentBall.pos.x - env->home[i].pos.x;
    dy = env->currentBall.pos.y - env->home[i].pos.y;
Fa.setXYMod(dx,dy,1);
  return Fa.ang();
}
int defX;
int defender(int i){
    int ballX,ballY;

        if (leftTeam)
        		defX=70;
        else
        		defX=30;
    ballX = env->currentBall.pos.x;
    ballY = env->currentBall.pos.y;
     float distanceToDef = sqrt(pow(ballX-defX,2) + pow(ballY-40,2));
     tForce Fa;
     double dx,dy;
    if (distanceToDef>40) {
        //GotoXY(env->home+i,defX,40);

        dx = env->home[i].pos.x - defX;
        dy = env->home[i].pos.y- 40;
        return -1;

    }
    else{
      //return FollowBall(env, env->home+i);
      //GotoXY(env->home+i,ballX,ballY);
      //Fa.setXYMod(ballX,ballY,1);
      dx = env->currentBall.pos.x - env->home[i].pos.x;
      dy = env->currentBall.pos.y - env->home[i].pos.y;
    }

Fa.setXYMod(dx,dy,1);
  return Fa.ang();
}


// Estrategia p/ 4 robos

//extern "C" STRATEGY_API void Strategy ( Environment *env )
int main(int argc, char **argv) {
    env= new Environment();
    Ai *ai=new Ai();
    base *cp = new base((int) (argv[2][0] - '0'));

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
    if (arg2==2)
        leftTeam=true;
    ai->leftTeam=leftTeam;

    cp->set_hostname(argv[1]);
    cp->conct();


		for (int i=0;i<N_JOGADORES_POR_TIME;i++){

    if(leftTeam) dest[i][0]=60;
    else dest[i][0]=30;

    if (i==1) {
      if(leftTeam) dest[i][0]=90;
      else dest[i][0]=10;
    }
    }

    dest[0][1] = 60;
    dest[1][1] = 40;
    dest[2][1] = 40;

		bool starting=true;
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

bool reg=false;
int nextAction[3];
  for(int i=0;i<3;i++){
			if (mustChangeDestination(env->home[i].pos.x,env->home[i].pos.y,dest[i][0],dest[i][1])){


          switch (i) {
            case 0:
              nextAction[0]  = FollowBall(env, env->home);
              reg=true;
              break;
            case 1:
              nextAction[1]=goalkeeper(1);
              if (nextAction[1]!=-2) reg=true;
              break;
            case 2:
                  nextAction[2]=defender(2);
                  if (nextAction[2]!=-1) reg=true;
              break;
          }
          if (reg) {
            ai->insertIntoBuffer(curEnvironment,i);
            int dir = chooseNextPoint(nextAction[i],i);
  					ai->registerDirection(dir,i);
  					ai->getRewards(i);
          }
				}

    }
	//printf("\n\n");
	for(int i = 0; i < N_JOGADORES_POR_TIME; i++) {
      GotoXY(env->home+i, dest[i][0],dest[i][1]);
      //Goto(env->home+i,nextAction[i]);
	    cp->set_vel(i, cp->fira_to_uspds_vel(env->home[i].velocityLeft), cp->fira_to_uspds_vel(env->home[i].velocityRight));
	}
	cp->snd();
    }

	return 0;
}

int angleToBall(int i){
  tForce Fa;
  double dx = env->currentBall.pos.x - env->home[i].pos.x;
	double dy = env->currentBall.pos.y - env->home[i].pos.y;
  Fa.setXYMod(dx,dy,1);
  return Fa.ang();
}

#define LIMIT_LEFT 92
#define LIMIT_RIGHT 15
#define LIMIT_UP 10
#define LIMIT_DOWN 75

int chooseNextPoint(int angle, int i){

    float percentage ;
    float distToBall;float distanceToCenter;

		//converter ângulo em id de direção
		int newdir = ((angle+22)/45)%8;

		//int directionId = newdir;
    int directionId =8;
    if ((sqrt(pow(angle-angleToBall(i),2)) > 45) || (angle==-1))
      directionId=newdir;

    if (angle<0 && angle>=-2)
      directionId=angle;


    switch(directionId){
            case -1://defensor
              if (leftTeam)
                dest[i][0]=70;
            	else
                dest[i][0]=30;
              dest[i][1]=40;
              break;
            case -2://goleiro
              if (leftTeam)
                dest[i][0]=90;
            	else
                dest[i][0]=10;
              dest[i][1]=40;
              break;
            case 6://norte
                dest[i][1] = env->home[i].pos.y - DELTA;
                break;
            case 2://sul
                dest[i][1] = env->home[i].pos.y + DELTA;
                break;
            case 4://leste
                dest[i][0] = env->home[i].pos.x - DELTA;
                break;
            case 0://oeste
                dest[i][0] = env->home[i].pos.x + DELTA;
                break;
            case 7://noroeste
                dest[i][0] = env->home[i].pos.x + DELTA;
                dest[i][1] = env->home[i].pos.y - DELTA;
                break;
            case 5://nordeste
                dest[i][1] = env->home[i].pos.y - DELTA;
                dest[i][0] = env->home[i].pos.x - DELTA;
                break;
            case 3://sudeste
                dest[i][1] = env->home[i].pos.y + DELTA;
                dest[i][0] = env->home[i].pos.x - DELTA;
                break;
            case 1://sudoeste
                dest[i][1] = env->home[i].pos.y + DELTA;
                dest[i][0] = env->home[i].pos.x + DELTA;
                break;
            case 8:
                distToBall = distTo(env->home[i].pos.x,env->currentBall.pos.x,env->home[i].pos.y,env->currentBall.pos.y);
                percentage = 10/distToBall;

                dest[i][1] = (env->home[i].pos.y*(1-percentage)) + (env->currentBall.pos.y*(percentage));
                dest[i][0] = (env->home[i].pos.x*(1-percentage)) + (env->currentBall.pos.x*(percentage));

                break;
            }


            if (env->home[i].pos.x>(LIMIT_LEFT - DELTA) && i!=1)
              dest[i][0]=LIMIT_LEFT-DELTA-5;
            if (env->home[i].pos.y>(LIMIT_DOWN - DELTA))
              dest[i][1]=LIMIT_DOWN-DELTA-5;
            if (env->home[i].pos.x<(LIMIT_RIGHT + DELTA) && i!=1)
                dest[i][0]=LIMIT_RIGHT + DELTA+5;
            if (env->home[i].pos.y<(LIMIT_UP + DELTA))
                dest[i][1]=LIMIT_UP + DELTA+5;

            return directionId;
}
