#include "strategy.h"
#include "Mediator.h"
#include <vector>
class Ai{
 public:
     int* receiveAction(Environment *env);
     void toto();
     Ai();
    void insertIntoBuffer(Environment *env, int i);
    void init(Environment *env);
    void getRewards(int i);
    bool leftTeam;
    int chooseDirection(Environment *env, int i);
protected:
    std::vector<Environment> envBufferAtk;
    std::vector<Environment> envBufferDef;
    std::vector<Environment> envBufferGk;
    
    std::vector<int> actionBufferAtk;
    std::vector<int> actionBufferDef;
    std::vector<int> actionBufferGk;
    
    std::vector<int> rewardBufferAtk;
    std::vector<int> rewardBufferDef;
    std::vector<int> rewardBufferGk;
    
    void getRewardAtk();
    void getRewardDef();
    void getRewardGk();
    int bufferPos[3];
    
    int sigma;
    NetworkMediator *med;
 };
 
 
