#include "Mediator.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <curl/curl.h>
#include <cstring>
#include <curl/curl.h>
#include "json.hpp"
#include <ctime>
using json = nlohmann::json;

NetworkMediator::NetworkMediator(){

}

int NetworkMediator::getAction(int player,
         float home0X, float home0Y, float home1X,float home1Y,float home2X,float home2Y,
                     float away0X, float away0Y,float away1X,float away1Y,float away2X,float away2Y,
                     float ballX, float ballY)
{
    float results1[6];
    float results2[10];
    
    
    float ***weights ;
    weights = getWeights(player);
    
    
    //pegar os resultados - CAMADA 1
    for (int i=0;i<6;i++){
        float result = 0;
        result+= weights[0][i][0] * home0X;
        result+= weights[0][i][1] * home0Y;
        
        result+= weights[0][i][2] * home1X;
        result+= weights[0][i][3] * home1Y;
        
        result+= weights[0][i][4] * home2X;
        result+= weights[0][i][5] * home2Y;
        
        result+= weights[0][i][6] * away0X;
        result+= weights[0][i][7] * away0Y;
        
        result+= weights[0][i][8] * away1X;
        result+= weights[0][i][9] * away1Y;
        
        result+= weights[0][i][10] * away2X;
        result+= weights[0][i][11] * away2Y;
        
        result+= weights[0][i][12] * ballX;
        result+= weights[0][i][13] * ballY;
        
        results1[i] = result;
        
    }
    
    //std::cout<<"LAYER 2\n";
     //pegar os resultados
    for (int i=0;i<10;i++){
        float result = 0;
        result+= weights[1][i][0] * results1[0];
        result+= weights[1][i][1] * results1[1];
        
        result+= weights[1][i][2] * results1[2];
        result+= weights[1][i][3] * results1[3];
        
        result+= weights[1][i][4] * results1[4];
        result+= weights[1][i][5] * results1[5];
        
        results2[i] = result;
        
    }
    
    int biggestIndex=0;
    float biggestValue=results2[0];
    
    //escolher a ação que dá o melhor resultado
    for(int i=1;i<10;i++){
        if (results2[i]>biggestValue){
            biggestIndex=i;
            biggestValue=results2[i];
        }
    }
    
    
    //biggestIndex = (rand()%10);
    
    
    return biggestIndex;
}



void NetworkMediator::saveReward(int player,
                     int home0X, int home0Y, int home1X,int home1Y,int home2X,int home2Y,
                     int away0X, int away0Y,int away1X,int away1Y,int away2X,int away2Y,
                     int ballX, int ballY,int action,int reward)
{
    std::ofstream myfile;
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    int dateDay = now->tm_mday;
    int dateMon = now->tm_mon + 1;
    int dateYear = now->tm_year + 1900;

    std::ostringstream oss;
    oss << "rewardLog"<<dateDay<<"-"<<dateMon<<"-"<<dateYear<<".txt";
    std::string filename =  oss.str();
    myfile.open (filename,std::ios::app);
    myfile <<player<<" "<<home0X<<" "<<home0Y<<" "<<home1X<<" "<<home1Y<<" "<<home2X<<" "<<home2Y<<" "<<
        away0X<<" "<<away0Y<<" "<<away1X<<" "<<away1Y<<" "<<away2X<<" "<<away2Y<<" "<<ballX<<" "<<ballY<<" "<<action<<" "<<reward<<"\n";
        

    /*myfile <<player<<" h0x:"<<home0X<<" h0y:"<<home0Y<<" h1x:"<<home1X<<" h1y:"<<home1Y<<" h2x:"<<home2X<<" h2y:"<<home2Y<<" a0x:"<<
        away0X<<" a0Y:"<<away0Y<<" a1x:"<<away1X<<" a1Y:"<<away1Y<<" a2X:"<<away2X<<" a2y:"<<away2Y<<" bX:"<<ballX<<" bY:"<<ballY<< "ACTION:"<<reward<<" REWARD:"<<reward<<"\n";*/
    /*
    myfile <<player<<" h1x:"<<home1X<<" h1y:"<<home1Y<<" bX:"<<ballX<<" bY:"<<ballY<<" ACTION:"<<reward<<" REWARD:"<<reward<<"\n";
    */
    myfile.close();
}

struct weightstring {
  char *ptr;
  size_t len;
};

void init_string(struct weightstring *s) {
  s->len = 0;
  s->ptr = (char *) malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct weightstring *s)
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

float** parseLayer(struct weightstring *layer,bool l1){
  int x=10,y=7;
  if (l1){
    x=6;y=15;
  }
  float ** layerArray;
  layerArray = (float **) malloc(sizeof(float*)*x);
  for (int i=0;i<x;i++){
    layerArray[i] = (float *) malloc(sizeof(float)*y);
  }

  float curNum = 0;
  int numDigit = 0;

  int i=0,j=0;
  int curPos = 2;

  auto j3 = json::parse(layer->ptr);
    for (json::iterator it2 = j3.begin(); it2 != j3.end(); ++it2) {
        j=0;
        for (json::iterator it = it2->begin(); it != it2->end(); ++it) {
            layerArray[i][j++] = *it;
            }
        i++;
    }
    

  return layerArray;
}

float*** NetworkMediator::getWeights(int playerNum){
    switch(playerNum){
        case 0:
            return weights1;
            break;
        case 1:
            return weights2;
            break;
        case 2:
            return weights3;
            break;
    }
}

void NetworkMediator::setWeights(int playerNum){
    float ***weights;
    weights = (float***) malloc(sizeof(float **) * 2);
    weights[0] = (float**) malloc(sizeof(float *) * 6);
    weights[1] = (float**) malloc(sizeof(float *) * 10);
    for (int i=0;i<6;i++)
        weights[0][i] = (float*) malloc(sizeof(float ) * 14);
    for (int i=0;i<10;i++)
        weights[1][i] = (float*) malloc(sizeof(float ) * 6);
    
    /*std::ifstream weightFile;
    weightFile.open("weights.txt");
    std::string line;
    for (int i=0;i<10;i++)
        for (int j=0; j<14;j++){
            std::getline(weightFile, line);
            weights[i][j] = std::stof(line);
        }
    weightFile.close();*/
    
    
    //pegando dados do servidor
    
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        struct weightstring layer1,layer2;
        init_string(&layer1);
        
        switch(playerNum){
        case 0:
            curl_easy_setopt(curl, CURLOPT_URL, "https://verysmallsize-eb3a1.firebaseio.com/p0/layer1.json");
            break;
        case 1:
            curl_easy_setopt(curl, CURLOPT_URL, "https://verysmallsize-eb3a1.firebaseio.com/p1/layer1.json");
            break;
        case 2:
            curl_easy_setopt(curl, CURLOPT_URL, "https://verysmallsize-eb3a1.firebaseio.com/p2/layer1.json");
            break;
        }

        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &layer1);
        res = curl_easy_perform(curl);


        float** l1 = parseLayer(&layer1,true);
        
        free(layer1.ptr);
        
        
        init_string(&layer2);

        
        switch(playerNum){
        case 0:
            curl_easy_setopt(curl, CURLOPT_URL, "https://verysmallsize-eb3a1.firebaseio.com/p0/layer2.json");
            break;
        case 1:
            curl_easy_setopt(curl, CURLOPT_URL, "https://verysmallsize-eb3a1.firebaseio.com/p1/layer2.json");
            break;
        case 2:
            curl_easy_setopt(curl, CURLOPT_URL, "https://verysmallsize-eb3a1.firebaseio.com/p2/layer2.json");
            break;
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &layer2);
        res = curl_easy_perform(curl);

        
        float** l2 = parseLayer(&layer2,false);
        
        free(layer2.ptr);
        

        /* always cleanup */
        curl_easy_cleanup(curl);
        
        
        
        weights[0] = l1;
        weights[2] = l2;
        
        switch(playerNum){
        case 0:
            weights1 = weights;
            break;
        case 1:
            weights2 = weights;
            break;
        case 2:
            weights3 = weights;
            break;
        }
    }
  
  //todo: converter string em pesos e tirar fonte do arquivo
  
}
