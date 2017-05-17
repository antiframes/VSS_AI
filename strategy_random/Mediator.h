
class NetworkMediator{
 public:
     int getAction(int player,
         float home0X, float home0Y, float home1X,float home1Y,float home2X,float home2Y,
                     float away0X, float away0Y,float away1X,float away1Y,float away2X,float away2Y,
                     float ballX, float ballY);
     void saveReward(int player,
                     int home0X, int home0Y, int home1X,int home1Y,int home2X,int home2Y,
                     int away0X, int away0Y,int away1X,int away1Y,int away2X,int away2Y,
                     int ballX, int ballY
                    ,int action,int reward);
    void setWeights(int playerNum);
    float*** getWeights(int playerNum);
    
    float*** weights1;
    float*** weights2;
    float*** weights3;
    NetworkMediator();

 };
