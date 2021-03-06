#ifndef FORCEFIELD_H
#define FORCEFIELD_H

/**
Exemplo:
tForce FResultante,Faux,Frepsulsao,Fatracao;

//Calculando a for�a resultante
Frepulsao.setXYMod(0,0,0); //zerando a for�a
for(int i=0; i<7; i++)
{
    readsensor(i,distX,distY);
    Faux.setXY(distX,distY);
    Frepulsao += Faux;
}
Fatracao.setXYMod(MetaX,MetaY,1); //setXYMod pois o m�dulo deve ser contante
FResultante = Fatracao - (Frepulsao*Q);
//sinal negativo para inverter a dire��o como repusao

//fazendo as leituras
FResultante.mod() retorna o m�dulo
FResultante.ang() retorna o angulo
FResultante.fdx() componente X
FResultante.fdy() componente Y
*/
class tVector
{
  public:
    tVector(){initialize();}
    tVector(const tVector &C);
    
    tVector& operator+(const tVector &B);
    tVector& operator-(const tVector &B);
    void operator+=(const tVector &C);
    void operator=(const tVector &C);
    tVector& operator*(double value);

    //input data
    inline virtual void setXY(double distX, double distY){x=distX;y=distY;}
    inline virtual void setMT(double mod, double direction){module=mod; theta=direction;}
    //output data
    inline double ang(){return theta;}
    inline double mod(){return module;}
    inline double fdx(){return x;}
    inline double fdy(){return y;}

  protected:
	 void initialize();
    double x,y;      //componentes
    double module;   //m�dulo do vetor
    double theta;    //dire��o do vetor
};

class tForce:public tVector
{
	public:
      void setXY(double distX, double distY);
      void setMT(double mod, double direction);
	  void setXYMod(double distX, double distY, double Modulo);
	  void operator=(tVector &C){tVector::operator =(C);}
      tForce():tVector(){};
};

#endif
