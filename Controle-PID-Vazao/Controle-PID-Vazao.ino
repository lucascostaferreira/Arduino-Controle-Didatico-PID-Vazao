//definindo pino digital para a saída do sensor
#define SENSOR 8
//definindo pino PWM para o gate do MOSFET
#define GATE 10
//definindo a constante de conversão frequência-vazão
#define K 82.2
//definindo as constantes do controlador
#define Kp 10.0
#define Ti 2.0
#define Td 0.2
//definindo cálculos úteis
#define P (Kp*erro)
#define D ((Kp*Td*(erro-erroAnt))/dt)

//declarando variáveis globais
int vr;
long t0;
float vazao, vazaoRef, freq, I = 0.0, erro = 0.0, erroAnt, dt;
short pwm = 0;

//declarando função reset
void(* reset) (void) = 0;
//declarando função de periodo
unsigned long periodo(int pino);

void setup(){
  //configurando o pino digital para a saída do sensor como pino de entrada
  pinMode(SENSOR, INPUT);
  //configurando o pino PWM para o gate do MOSFET como pino de saída
  pinMode(GATE, OUTPUT);
  
  //abrindo o MOSFET para garantir que o motor permaneça desligado
  digitalWrite(GATE, LOW);
  
  //iniciando comunicação serial entre o Arduino e o computador
  Serial.begin(2000000);
  //solicitando a vazão de referência ao usuário
  Serial.print("Informe a vazao de referencia em ml/min: ");
  //aguardando sinal para iniciar as leituras
  while(!(Serial.available() > 0)){
    delay(100);
  }
  //recebendo do computador a vazão de referência
  sscanf(Serial.readStringUntil('\0').c_str(), "%d", &vr);
  //convertendo a vazão de referência para l/min
  vazaoRef = vr/1000.0;
  Serial.print("\nReferencia: ");
  Serial.println(vazaoRef);
  //limpando sinal serial recebido
  while(Serial.available() > 0){
    Serial.read();
  }
  //marcando tempo inicial
  t0 = micros();
}

void loop(){
  //calculando intervalo de tempo do loop
  dt = (micros()-t0)/1000000.0;
  //marcando tempo inicial
  t0 = micros();
  //estimando a frequência do sensor em Hz
  freq = 1000000.0/periodo(SENSOR);
  //realizando cálculos para converter a frequência do sensor em vazão
  vazao = freq/K;
  //verificando se a medição está dentro dos limites esperados
  if(vazao < 3){
    //realizando cálculos do controlador
    erroAnt = erro;
    erro = vazaoRef - vazao;
    I += (Kp/Ti)*erro*dt;
    pwm = P+I+D;
    //saturando o PWM
    if(pwm < 0)
      pwm = 0;
    if(pwm > 255)
      pwm = 255;
    //aplicando no MOSFET o PWM calculado
    analogWrite(GATE, pwm);
    //enviando dados ao computador
    Serial.print("PWM = ");
    Serial.print(pwm);
    Serial.print("\tErro = ");
    Serial.print(erro);
    Serial.print("\tVazao = ");
    Serial.println(vazao);
  }
  //verificando mudança na referência
  if(Serial.available() > 0){
    //recebendo do computador a vazão de referência
    sscanf(Serial.readStringUntil('\0').c_str(), "%d", &vr);
    //convertendo a vazão de referência para l/min
    vazaoRef = vr/1000.0;
    Serial.print("\nReferencia: ");
    Serial.println(vazaoRef);
    //limpando sinal serial recebido
    while(Serial.available() > 0){
      Serial.read();
    }
  }
}

unsigned long periodo(int pino){
  unsigned long t0, ts = micros();
  if(digitalRead(pino) == 1){
    while(digitalRead(pino) == 1){
      if(micros()-ts > 1000000)
        return 10000000;
    }
    //inicia a contagem de tempo
    t0 = micros();
    while(digitalRead(pino) == 0){
      if(micros()-ts > 1000000)
        return 10000000;
    }
    while(digitalRead(pino) == 1){
      if(micros()-ts > 1000000)
        return 10000000;
    }
  }
  else{
    while(digitalRead(pino) == 0){
      if(micros()-ts > 1000000)
        return 10000000;
    }
    //inicia a contagem de tempo
    t0 = micros();
    while(digitalRead(pino) == 1){
      if(micros()-ts > 1000000)
        return 10000000;
    }
    while(digitalRead(pino) == 0){
      if(micros()-ts > 1000000)
        return 10000000;
    }
  }
  //termina a contagem de tempo e retorna o tempo de um período em microsegundos
  return micros()-t0;
}
