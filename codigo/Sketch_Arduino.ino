/************Sistema para controle e monitoramento de temperatura em um Data Center.****************************************************************
Apresentado como Trabalho de Conclusão de Cuso em Tecnologia e Análise de Desenvolvimento de Sistemas.
IFMG - Campus Bambuí. 2016.

REFERENCIAS:

  Laboratorio de Garagem, http://labdegaragem.com/profile/LaboratoriodeGaragem. 03 de Junho de 2015.
  Exemplos disponibilizados pela IDE do Projeto Arduino, http://www.arduino.cc/en/Main/Software. 06 de Junho de 2015.
  Ultimate CSS Gradient Generator, http://colorzilla.com/gradient-editor/#d3e8dc+0,9bc99e+31,8ace8e+63,24c924+100. 28 de Junho de 2015.
  Educ8*s. Arduino Project: Real time clock (RTC) and temperature monitor using the DS3231 module. Disponível em: http://educ8s.tv/arduino-project-real-time-clock-with-ds3231-module/.
  Teclado matricial membrana 4x3 com Arduino. Disponível em: http://www.arduinoecia.com.br/2015/05/teclado-matricial-membrana-4x3-arduino.html. 09 de Outubro de 2015.
  Controlando um LCD 16×2 com Arduino. Diponível em: http://blog.filipeflop.com/display/controlando-um-lcd-16x2-com-arduino.html. 09 de Outubro de 2015.

  Modificado em:   08 de dezembro de 2016.
  Autor: Wellington Leite de Oliveira.
*******************************************************************************************************************************************************/

/*******************Carrega as bibliotecas necessárias***************************************/
#include <dht.h>
#include <SPI.h>
#include <Ethernet.h>
#include <config.h>
#include <ds3231.h>
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <SD.h>

/*********Configurações do servidor web (Ethernet Shield)********************************/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //Valor do MAC
IPAddress ip(192, 168, 0, 230);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnServer(192, 168, 0, 1);
EthernetServer server(80); //Inicializa a biblioteca EthernetServer com os valores acima e configura a porta de acesso(80).

/***********************Inicialização das Variáveis***************************************/  
  //Definição da quantidade de linhas e colunas do keypad
  const byte LINHAS = 4;
  const byte COLUNAS = 3;
  //Matriz de caracteres
  char matriz_teclas[LINHAS][COLUNAS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
  };
  byte PinosLinhas[LINHAS] = {22, 23, 24, 25};//Definicão dos pinos das linhas
  byte PinosColunas[COLUNAS] = {26, 27, 28};//Definicão dos pinos das colunas
  
  //Inicializa o teclado
  Keypad meuteclado = Keypad( makeKeymap(matriz_teclas), PinosLinhas, PinosColunas, LINHAS, COLUNAS);  
  LiquidCrystal lcd(32, 33, 34, 35, 36, 37);//Define os pinos que serão utilizados para ligação ao display
  #define dht_dpin1 A1 //Define pino DATA do Sensor 1 como ligado na porta Analogica A1.
  #define dht_dpin2 A2
  #define lm_351 A4 // Pino Analogico onde está ligado ao pino 2 do LM35
  #define lm_352 A5
  #define lm_353 A7
  #define sensorGas A8
  #define BUFF_MAX 128 //Usada na função tempo para amazenar o tempo atual 
  dht DHT; //Inicializa o sensor
  uint8_t time[8];//rtc
  char recv[BUFF_MAX];
  unsigned int recv_size = 0;
  struct ts t; //Variável data e hora
  const float CELSIUS_BASE = 0.4887585532746823069403714565;//Base de conversão para Graus Celsius ((5/1023) * 100)
  
  //variáves de configuração
  String senha = "1234";//Senha padrão
  float tempIdeal = 35;//temperatura ideal
  int umidIdeal = 55;//Umidade ideal
  int nivel_sensor = 300;// Nivel de referencia para o sensor de gas inflamável
  long tempoRodizio = 10000;//Tempo de rodizio padrão: 5s para testes. (1 hora tem 3600000ms, 6h = 21600000).
  
  //Outras variáves
  float diferenca;
  int numRodizio = 0;
  float armazenaTemperatura1 = 0.0;
  float armazenaUmidade1 = 0.0;
  float armazenaTemperatura2 = 0.0;
  float armazenaUmidade2 = 0.0;
  float temperaturaLm3 = 0.0;
  float temperaturaLm4 = 0.0;
  float temperaturaLm5 = 0.0;
  int nivelGas = 1;
  long previousMillis = 0;//Variável de controle do tempo
  long previousMillis2 = 0; 
  long tempoLeitura = 1000;//Tempo em ms para fazer a leitura dos sensores
  long tempoLeitura2 = 5000;//Tempo que a luz ficará ligada quando detectar movimento
  long previousMillisRodizio = 0;// Variável para controle do tempo do rodizio
  int pinoRele1 = 2; //Pino do Arduino ligado ao IN1 da placa de relés
  int pinoRele2 = 3; //Pino do Arduino ligado ao IN2 da placa de relés
  int pinoRele3 = 5; //Pino do Arduino ligado ao IN3 da placa de relés
  int pinoRele4 = 6; //Pino do Arduino ligado ao IN4 da placa de relés
  int pinoLedVerde = 38; //Pino Led verde interface de configuração
  int pinoLedVermelho = 39; //Pino Led vermelho interface de configuração
  int pinoBeep = 40; //Pino Beep
  int estadoRele1 = HIGH; //Armazena o estado dos relés inicialmente como desligados
  int estadoRele2 = HIGH;
  int estadoRele3 = HIGH;
  int estadoRele4 = HIGH;
  int pinopir = 8;  //Pino ligado ao sensor PIR
  int movimento;
  String nomeBotao1 = "Ar-condicionado 1";
  String nomeBotao2 = "Ar-condicionado 2";
  String nomeBotao3 = "Ar-condicionado 3";
  String nomeBotao4 = "Movimento";
/*********************************FIM DAS VARIAVES***********************************************/

void setup()
{
  delay(1000);//Dar tempo do visor LCD ligar primeiro
  /*-------------Define os pino dos Atuadores (relés) como saída----------------------------*/
  pinMode(pinoRele1, OUTPUT);
  pinMode(pinoRele2, OUTPUT);
  pinMode(pinoRele3, OUTPUT);
  pinMode(pinoRele4, OUTPUT);
  pinMode(pinoLedVerde, OUTPUT);
  pinMode(pinoLedVermelho, OUTPUT);
  pinMode(pinoBeep, OUTPUT);  
  pinMode(dht_dpin1, INPUT);  //Sensores como entrada
  pinMode(dht_dpin2, INPUT);
  pinMode(lm_351, INPUT);
  pinMode(lm_352, INPUT);
  pinMode(lm_353, INPUT);
  pinMode(sensorGas, INPUT);
  digitalWrite(pinoRele1, HIGH); //Coloca os reles em estado desligado
  digitalWrite(pinoRele2, HIGH);
  digitalWrite(pinoRele3, HIGH);
  digitalWrite(pinoRele4, HIGH);
  /*-----------------------------------------------------------------------------*/

  Serial.begin(9600); //Inicializa a serial
  Wire.begin();
  lcd.begin(20, 4);
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);

  // Inicia a conexão Ethernet eo servidor:
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  server.begin();
  Serial.print("Este e o servidor ");
  Serial.println(Ethernet.localIP());

  //Iniciar cartão
    Serial.print("Inicializar cartão SD ...");
  // ver se o cartão está presente e pode ser inicializado
  if (!SD.begin(4)) {
    Serial.println("Cartão falhou, ou não está presente");   
    return;
  }
  Serial.println("Cartão inicializado");
}  //Fim da execução do setup

void loop()
{
    //Posiciona o cursor na coluna 3, linha 0;     
    lcd.setCursor(0, 0);
    lcd.print("Temperature Control");
    lcd.setCursor(4, 1);
    lcd.print("Data Center");
    lcd.setCursor(0, 2);
    lcd.print((armazenaTemperatura1 + armazenaTemperatura2) / 2); //media temperatura superior e inferior
    lcd.print("C");
    lcd.setCursor(13, 2);
    lcd.print((armazenaUmidade1 + armazenaUmidade2) / 2);
    lcd.print("%");
  if(diferenca < -2){
    lcd.setCursor(2, 3);
    lcd.print("Apar. Desligados");
    digitalWrite(pinoBeep, HIGH);
    digitalWrite(pinoLedVerde, HIGH);    
    digitalWrite(pinoLedVermelho, LOW);
  }else if (diferenca > 2){
    lcd.setCursor(1, 3);
    lcd.print("Todos  A. Ligados");
    digitalWrite(pinoBeep, LOW);
    digitalWrite(pinoLedVermelho, HIGH);   
    digitalWrite(pinoLedVerde, LOW); 
  }else{
    lcd.setCursor(1, 3);
    lcd.print("Estavel Rodizio");
    digitalWrite(pinoBeep, HIGH);
    digitalWrite(pinoLedVerde, HIGH);    
    digitalWrite(pinoLedVermelho, LOW);
  }

  //Verifica se alguma tecla foi pressionada
  if (meuteclado.getKey())
  {
    String senhaDigitada;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Digite a Senha");
    while(senhaDigitada.length() < 4){               
      char s = meuteclado.waitForKey();              
      senhaDigitada.concat(s);                 
      lcd.setCursor(senhaDigitada.length(), 1);
      lcd.print("*");
    }  
//Se a senha estiver correta acessa as configurações
    if (senhaDigitada.equals(senha)) {
      Serial.println("senha correta");
      configuracoes();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Senha incorreta:");      
      lcd.setCursor(0, 1);
      lcd.print(senhaDigitada);
      delay(2000);
    }   
  }
  
/**************************Leitura dos sensores************************************************/
  unsigned long currentMillis = millis();    //Tempo atual em ms
  if (currentMillis - previousMillis > tempoLeitura) //Verifica se já passou o tempo
  {
    previousMillis = currentMillis;

    DHT.read11(dht_dpin1); //Lê as informações do sensor umidade e temp 1
    armazenaTemperatura1 = DHT.temperature;
    armazenaUmidade1 = DHT.humidity;
    DHT.read11(dht_dpin2); //Lê as informações do sensor umidade e temp 2
    armazenaTemperatura2 = DHT.temperature;
    armazenaUmidade2 = DHT.humidity;
    temperaturaLm3 = analogRead(lm_351) * CELSIUS_BASE;
    temperaturaLm4 = analogRead(lm_352) * CELSIUS_BASE;
    temperaturaLm5 = analogRead(lm_353) * CELSIUS_BASE;
    nivelGas = analogRead(sensorGas);
    //Executa a função controle automatico do ambiente
    controleAutomatico();
    rtc();
    lcd.clear();
    //verifica se algum sensor fez leitura de temperatura muito acima do normal    
    if(armazenaTemperatura1 > (tempIdeal + 2) || armazenaTemperatura2 > (tempIdeal + 2) || temperaturaLm3 > (tempIdeal + 2) || temperaturaLm4 > (tempIdeal + 2) || temperaturaLm5 > (tempIdeal + 2))
    {
     gravarLog("A"); //grava log temperatura anormal
    }
    /******Verifica se existe presença de gás inflamável*******/
    if (nivelGas > nivel_sensor) {
      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Gas inflamavel");
      lcd.setCursor(5, 2);
      lcd.print("DETECTADO");
      digitalWrite(pinoBeep, LOW);
      digitalWrite(pinoLedVermelho, HIGH);   
      digitalWrite(pinoLedVerde, LOW);   
      gravarLog("G"); //grava log gas detectado  
      Serial.print("PPM gas inflamavel: ");
      Serial.println(nivelGas);  
      delay(3000);
    }
    //Ascender lâmpada no caso de presença
    movimento = digitalRead(pinopir); //Le o valor do sensor PIR
    if (movimento == LOW ){
       digitalWrite(pinoRele4, HIGH);
       gravarLog("M"); //grava log movimento detectado
    }else{   
       digitalWrite(pinoRele4, LOW);
    }  
  }

  
/**************************************************************************************************************
*                                   Inicio programação do Web Server                                          *
**************************************************************************************************************/
  EthernetClient client = server.available();// Verifica se há algum cliente conectado
  if (client)
  {
    Serial.println("Novo cliente");
    boolean currentLineIsBlank = true; // A requisição http termina com uma linha em branco
    String valorPego; //Variável para pegar os valores recebidos da página

    while (client.connected()) {
      if (client.available())
      {
        char c  = client.read();//Variável para armazenar os caracteres que forem recebidos
        valorPego.concat(c); //pega o valor após o ip do navegador
        if (valorPego.equals("GET /datalog.txt HTTP/1.1"))
        {
          //MANDAR ARQUIVO PARA O CLIENTE
          Serial.println("Enviando DATALOG para cliente web");
          File dataFile = SD.open("datalog.txt");
          client.println(F("HTTP/1.1 200 OK\n"
                   "Content-Type: application/download\n"
                   "Connection: close\n"));         
          if (dataFile) {
              while(dataFile.available() && client.available() && client.connected()) {
                client.write(dataFile.read()); 
              }
              dataFile.close();
              delay(2);
              client.stop();
              Serial.println("Conexão fechada e arquivo enviado");
            } else {
             Serial.println("ERRO AO CARREGAR ARQUIVO DO SD CARD: ");    
            }                            
        }
        if (c == '\n' && currentLineIsBlank) {
          
     /*******************Iniciando a página HTML******************************************/
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close"); // A conexão será fechada após a conclusão da resposta
            client.println("Refresh: 20");  // Atualizar a página automaticamente
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title>Temperatura Data Center IFMG</title>");                
            client.println("<style type='text/css'>");    
            
      /******************Carregar Estilo css do cartão***********************************/ 
              File cssFile = SD.open("css.css", FILE_READ);        
              // verifica se o arquivo CSS esta disponível, depois carrega byte por byte.
              while(cssFile.available()) {
                client.write(cssFile.read());                             
               // Serial.println("CARREGANDO CSS-----------------------");                
              }           
              cssFile.close();   //Fechando arquivo css                   
            client.println("</style>");         
            client.println("</head>");
            client.println("<body class='fundo'>");
  
            client.println("<div class='tudo'>");
            client.println("<div class='cabecalho'>");
            client.println("<h2>SISTEMA DE MONITORAMENTO E CONTROLE DE TEMPERATURA DO DATA CENTER IFMG - BAMBU&Iacute </h2>");
            client.println("</div>");
            client.println("<div class='menu'> ");
            
            //1º Botão
            client.print("<center><button class='buttonStatus' onclick=\"window.location.href='http://192.168.0.230/0001'\">\0</button> > Codigo: 0001 > ");           
          if (estadoRele1 == LOW) //verifica o estado do rele 1
          {
            client.print("<B><span style=\"color:red;\">");
            client.print(nomeBotao1);
            client.print(" = ON");
            client.print("</span></B></center>");
          }
          else
          {
            client.print("<B><span style=\"color: #000000;\">");
            client.print(nomeBotao1);
            client.print(" = OFF");
            client.print("</span></B></center>");
          }
          client.print("<BR>");
          
          //2º Botão
          client.print("<center><button class='buttonStatus' onclick=\"window.location.href='http://192.168.0.230/0010'\">\0</button> > Codigo: 0010 > ");          
          if (estadoRele2 == LOW)
          {
            client.print("<B><span style=\"color:red;\">");
            client.print(nomeBotao2);
            client.print(" = ON");
            client.print("</span></B></center>");
          }
          else
          {
            client.print("<B><span style=\"color: #000000;\">");
            client.print(nomeBotao2);
            client.print(" = OFF");
            client.print("</span></B></center>");
          }
          client.print("<BR>");
          
          //3º Botão
          client.print("<center><button class='buttonStatus' onclick=\"window.location.href='http://192.168.0.230/0100'\">\0</button> > Codigo: 0100 > ");          
          if (estadoRele3 == LOW)
          {
            client.print("<B><span style=\"color:red;\">");
            client.print(nomeBotao3);
            client.print(" = ON");
            client.print("</span></B></center>");
          }
          else
          {
            client.print("<B><span style=\"color: #000000;\">");
            client.print(nomeBotao3);
            client.print(" = OFF");
            client.print("</span></B></center>");
          }
          client.print("<BR>");
          
          //4º Botão
          client.print("<center><button class='buttonStatus' onclick=\"window.location.href='http://192.168.0.230/1000'\">\0</button> > Codigo: 1000 > ");          
          if (movimento == HIGH)
          {
            client.print("<B><span style=\"color:red;\">");
            client.print(nomeBotao4);
            client.print(" = SIM");
            client.print("</span></B></center>");
          }
          else
          {
            client.print("<B><span style=\"color: #000000;\">");
            client.print(nomeBotao4);
            client.print(" = NAO");
            client.print("</span></B></center>");
          }

          client.println("</div>");
          client.println("<BR>");
          client.println("<table align='center'>");
          client.println("<tr>");
          client.println("<td><h3>LOCAL</h3></td>");
          client.println("<td colspan='2'><h3>TEMPERATURA *C</h3></td>");
          client.println("<td colspan='2'><h3>UMIDADE %</h3></td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td></td>");
          client.println("<td>Superior</td>");
          client.println("<td>Inferior</td>");
          client.println("<td>Superior</td>");
          client.println("<td>Inferior</td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td>Ambiente 1</td>");
          
          // Dados do sensor 1
          client.println("<td>");
          if (armazenaTemperatura1 > tempIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(armazenaTemperatura1);
            client.print("</font>");           
          } else {
            client.println(armazenaTemperatura1);
          }
          client.println("</td>");
          
          // Dados do sensor 2
          client.println("<td>");
          if (armazenaTemperatura2 > tempIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(armazenaTemperatura2);
            client.print("</font>");           
          } else {
            client.println(armazenaTemperatura2);
          }
          client.println("</td>");
          client.println("<td>");
          
          // Dados do sensor 3
          if (armazenaUmidade1 > umidIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(armazenaUmidade1);
            client.print("</font>");            
          } else {
            client.println(armazenaUmidade1);
          }
          client.println("</td>");
          client.println("<td>");
          if ((armazenaUmidade2) > umidIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(armazenaUmidade2);
            client.print("</font>");            
          } else {
            client.println(armazenaUmidade2);
          }
          client.println("</td>");
          client.println("</tr>");

          client.println("<tr>");
          client.println("<td>Rack 1</td>");
          
          // Dados do sensor 4
          client.println("<td>");
          if (temperaturaLm3 > tempIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(temperaturaLm3);
            client.print("</font>");
          } else {
            client.println(temperaturaLm3);
          }
          client.println("</td>");
          
          // Dados do sensor 5
          client.println("<td>");
          if (temperaturaLm4 > tempIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(temperaturaLm4);
            client.print("</font>");
          } else {
            client.println(temperaturaLm4);
          }
          client.println("</td>");
          
          //Dados do sensor 6
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("</tr>");

          client.println("<tr>");
          client.println("<td>Rack 2</td>");
          
          // Dados do sensor 7
          client.println("<td>");
          if (temperaturaLm5 > tempIdeal) {
            client.print("<font color=\"#FF0000\">");
            client.println(temperaturaLm5);
            client.print("</font>");
          } else {
            client.println(temperaturaLm5);
          }
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td>Rack 3</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("<td>");
          client.println("-");
          client.println("</td>");
          client.println("</tr>");
          
/******EXIBIR PPM DE GAS INFLAMÁVEL*****/
          client.println("<tr>");
          client.println("<td>G&aacutes inflam&aacutevel</td>");
          client.println("<td colspan='4'>");
          if(nivelGas > nivel_sensor){
          client.print("<font color=\"#FF0000\">");
          client.println(nivelGas);
          client.println("ppm"); 
          client.print("</font>");    
          }else{      
          client.println(nivelGas); 
          client.println("ppm"); 
          }                   
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td>Data Log</td>");
          client.println("<td colspan='4'>");
          client.println("<a href='/datalog.txt'>DataLog.txt</a>"); // LINK PARA DOWNLOAD DATALOG
          client.println("</td>");
          client.println("</tr>");
          client.println("</table>");
          client.println("<br><br>");
          client.println("");
          
   /****Exibe a Data************/
          client.print("<BR>");
          client.print("<center> <font size=5> <font color=\"#000080\"> Data: </font></font> <font size=5>  <font color=\"#000080\">  ");
          if (t.mday < 10)
          {
            client.print("0");
          }
          client.print(t.mday);
          client.print("/");
           if (t.mon < 10)
          {
            client.print("0");
          }
          client.print(t.mon);
          client.print("/");
          client.print(t.year);
          client.print("</font></font></center>");
          
   /****Exibe a Hora*************/
          client.print("<center><font size=5> <font color=\"#000080\"> Hora: </font></font> <font size=5>  <font color=\"#000080\">  ");
          if (t.hour < 10)
          {
            client.print("0");
          }
          client.print(t.hour);
          client.print(":");
          if (t.min < 10)
          {
            client.print("0");
          }
          client.print(t.min);
          client.print(":");
          if (t.sec < 10)
          {
            client.print("0");
          }
          client.print(t.sec);
          client.print(' ');
          client.print("</font></font></center>");
          client.print("<BR><BR>");
          //Esta proxima linha é necessária para limpar o codigo ulitizado na url apos o ip.
          client.print("<meta http-equiv=\"refresh\" content=\"5; url=http://192.168.0.230/\"> "); 
          client.println("</td></tr></table>");
          client.print("</body>");
          client.println("</html>");
          break;
          /*****************************Fim da página HTML****************************/
        }
         if (c == '\n') {  
           currentLineIsBlank = true;  
         }   
         else if (c != '\r') {  
           currentLineIsBlank = false;  
         }  
      }//Fim do if de verificação de cliente disponível
    }//Fim do (while) enquanto o cliente estiver conectado

    delay(1); // Dar tempo ao navegador para receber os dados
    client.stop();// Fecha a conexão
    Serial.println("Cliente desconectado");
    Serial.println();
  }//Fim do if (se há algum cliente).
  /****************************** Fim Web Server ************************************/

}//Fim do laço loop

/***********************Função Controle automatico do ambiente***********************/
void controleAutomatico() {
  Serial.println("EXECUTOU A FUNCAO CONTROLE AUTOMATICO");
  unsigned long currentMillis = millis();    //Tempo atual em ms
  // Calcula a temperatura media e subtrai da temperatura ideal
  diferenca = ((armazenaTemperatura1 + armazenaTemperatura2) / 2) - tempIdeal; 
  Serial.println("-----------Diferença da temperatura atual com a ideal");
  Serial.println(diferenca); 
  //se a diferença estiver for 2 abaixo e até 2 acima da temperatura ideal entra no rodizio. 
  //Do contrário Liga todos ou desliga todos.
  if (diferenca <= 2 && diferenca >= -2) { 
    //primeiro verifica se todos estavam ligados, se tiverem é alterado o estado antes de iniciar o rodizio
    if (estadoRele1 == LOW && estadoRele2 == LOW && estadoRele3 == LOW) {
      acionaReles('1');
      acionaReles('2');
      acionaReles('3');
    }
    if (estadoRele1 == HIGH && estadoRele2 == HIGH && estadoRele3 == HIGH) {
      acionaReles('1'); //Liga 1
      acionaReles('2'); //Liga 2
    }
    if ((currentMillis - previousMillisRodizio > tempoRodizio) && estadoRele1 == LOW && estadoRele3 == LOW && numRodizio == 0) {
      acionaReles('3'); //Desliza 3
      acionaReles('2'); //Liga 2
      previousMillisRodizio = currentMillis;
      Serial.println("ACIONA 1 E 2");
    }
    if ((currentMillis - previousMillisRodizio > tempoRodizio) && numRodizio == 0) {
      previousMillisRodizio = currentMillis;
      acionaReles('1'); //Desliga 1
      acionaReles('3'); //Liga 3
      Serial.println("ACIONA 2 E 3");
      numRodizio = 1;
    }
    if (numRodizio == 1 && (currentMillis - previousMillisRodizio > tempoRodizio)) {
      acionaReles('2'); // Desliga 2
      acionaReles('1'); //Liga 1
      numRodizio = 0;
      previousMillisRodizio = currentMillis;
      Serial.println("ACIONA 3 E 1");
    }    
  } else if ((estadoRele1 == LOW || estadoRele2 == LOW || estadoRele3 == LOW ) && (diferenca < 2)) {
    acionaReles('5'); //Desligar todos
  } else if ((estadoRele1 == HIGH || estadoRele2 == HIGH || estadoRele3 == HIGH ) && (diferenca > 2)) {
    acionaReles('6'); //Ligar todos
  }
}

/********* Função que executa comandos dos relés**************/
void acionaReles(int op) {
  /*--------- Inicio Switch case para executar os comandos dos relês  ------------------*/
  switch (op)
  {
    case '1':
      estadoRele1 = !estadoRele1;
      //Aciona rele 1
      digitalWrite(pinoRele1, estadoRele1);
      if (estadoRele1 == LOW)
        Serial.println("Rele 1 Ligado com sucesso");
      else
        Serial.println("Rele 1 Desligado com sucesso");
      break;

    case '2':
      estadoRele2 = !estadoRele2;
      //Aciona rele 2
      digitalWrite(pinoRele2, estadoRele2);
      if (estadoRele2 == LOW)
        Serial.println("Rele 2 Ligado com sucesso");
      else
        Serial.println("Rele 2 Desligado com sucesso");
      break;

    case '3':
      estadoRele3 = !estadoRele3;
      //Aciona rele 3
      digitalWrite(pinoRele3, estadoRele3);
      if (estadoRele3 == LOW)
        Serial.println("Rele 3 Ligado com sucesso");
      else
        Serial.println("Rele 3 Desligado com sucesso");
      break;

    case '4':
      estadoRele4 = !estadoRele4;
      digitalWrite(pinoRele4, estadoRele4);
      if (estadoRele4 == LOW)
        Serial.println("Rele 4 Ligado com sucesso");        
      else
        Serial.println("Rele 4 Desligado com sucesso");
      break;

    case '5':
      if (estadoRele1 == LOW || estadoRele2 == LOW || estadoRele3 == LOW) {//se algum rele estiver ligado, então desliga todos
        digitalWrite(pinoRele1, HIGH);
        digitalWrite(pinoRele2, HIGH);
        digitalWrite(pinoRele3, HIGH);
        estadoRele1 = HIGH;
        estadoRele2 = HIGH;
        estadoRele3 = HIGH;
        gravarLog("Todos os aparelhos foram desligados");
        Serial.println("TODOS OS RELES FORAM DESACIONADOS");
      }
      break;

    case '6':
      if (estadoRele1 == HIGH || estadoRele2 == HIGH || estadoRele3 == HIGH) {//se algum rele estiver desligado, então liga todos
        digitalWrite(pinoRele1, LOW);
        digitalWrite(pinoRele2, LOW);
        digitalWrite(pinoRele3, LOW);
        estadoRele1 = LOW;
        estadoRele2 = LOW;
        estadoRele3 = LOW;
        gravarLog("Todos os aparelhos foram ligados");
        Serial.println("TODOS OS RELES FORAM ACIONADOS");
      }
      break;
  }
  /*------------Fim Switch Case---------------------------------------------------------------*/
}
/********Função para controle do Real Time Clock ***** Crédito: Educ8*s *************/
void rtc()
{
  char in;
  char tempF[6];
  float temperature;
  char buff[BUFF_MAX];
  unsigned long now = millis();

  DS3231_get(&t); //Get time
  parse_cmd("C", 1);
  temperature = DS3231_get_treg(); //Get temperature
  dtostrf(temperature, 5, 1, tempF);

  if (Serial.available() > 0) {
    in = Serial.read();

    if ((in == 10 || in == 13) && (recv_size > 0)) {
      parse_cmd(recv, recv_size);
      recv_size = 0;
      recv[0] = 0;
    } else if (in < 48 || in > 122) {
      ;       // ignore ~[0-9A-Za-z]
    } else if (recv_size > BUFF_MAX - 2) {   // drop lines that are too long
      // drop
      recv_size = 0;
      recv[0] = 0;
    } else if (recv_size < BUFF_MAX - 2) {
      recv[recv_size] = in;
      recv[recv_size + 1] = 0;
      recv_size += 1;
    }
  }
}

void parse_cmd(char *cmd, int cmdsize)
{
  uint8_t i;
  uint8_t reg_val;
  char buff[BUFF_MAX];
  // struct ts t;

  //snprintf(buff, BUFF_MAX, "cmd was '%s' %d\n", cmd, cmdsize);
 // Serial.print(buff);

  // TssmmhhWDDMMYYYY aka set time
  if (cmd[0] == 84 && cmdsize == 16) {
    //T355720619112011
    t.sec = inp2toi(cmd, 1);
    t.min = inp2toi(cmd, 3);
    t.hour = inp2toi(cmd, 5);
    t.wday = inp2toi(cmd, 7);
    t.mday = inp2toi(cmd, 8);
    t.mon = inp2toi(cmd, 10);
    t.year = inp2toi(cmd, 12) * 100 + inp2toi(cmd, 14);
    DS3231_set(t);
    Serial.println("OK");
  } else if (cmd[0] == 49 && cmdsize == 1) {  // "1" get alarm 1
    DS3231_get_a1(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 50 && cmdsize == 1) {  // "2" get alarm 1
    DS3231_get_a2(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 51 && cmdsize == 1) {  // "3" get aging register
    Serial.print("aging reg is ");
    Serial.println(DS3231_get_aging(), DEC);
  } else if (cmd[0] == 65 && cmdsize == 9) {  // "A" set alarm 1
    DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
    //ASSMMHHDD
    for (i = 0; i < 4; i++) {
      time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // ss, mm, hh, dd
    }
    byte flags[5] = { 0, 0, 0, 0, 0 };
    DS3231_set_a1(time[0], time[1], time[2], time[3], flags);
    DS3231_get_a1(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 66 && cmdsize == 7) {  // "B" Set Alarm 2
    DS3231_set_creg(DS3231_INTCN | DS3231_A2IE);
    //BMMHHDD
    for (i = 0; i < 4; i++) {
      time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // mm, hh, dd
    }
    byte flags[5] = { 0, 0, 0, 0 };
    DS3231_set_a2(time[0], time[1], time[2], flags);
    DS3231_get_a2(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 67 && cmdsize == 1) {  // "C" - get temperature register
    Serial.print("temperature DS3231 is ");
    Serial.println(DS3231_get_treg(), DEC);
  } else if (cmd[0] == 68 && cmdsize == 1) {  // "D" - reset status register alarm flags
    reg_val = DS3231_get_sreg();
    reg_val &= B11111100;
    DS3231_set_sreg(reg_val);
  } else if (cmd[0] == 70 && cmdsize == 1) {  // "F" - custom fct
    reg_val = DS3231_get_addr(0x5);
    Serial.print("orig ");
    Serial.print(reg_val, DEC);
    Serial.print("month is ");
    Serial.println(bcdtodec(reg_val & 0x1F), DEC);
  } else if (cmd[0] == 71 && cmdsize == 1) {  // "G" - set aging status register
    DS3231_set_aging(0);
  } else if (cmd[0] == 83 && cmdsize == 1) {  // "S" - get status register
    Serial.print("status reg is ");
    Serial.println(DS3231_get_sreg(), DEC);
  } else {
    Serial.print("unknown command prefix ");
    Serial.println(cmd[0]);
    Serial.println(cmd[0], DEC);
  }
}

/**** Função faz leitura do teclado numérico e configura opções do sistema**/
void configuracoes() {
    gravarLog("C"); // acesso as configurações locais
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1 - Tempo Rodizio");
    lcd.setCursor(0, 1);
    lcd.print("2 - Temperat. Ideal");
    lcd.setCursor(0, 2);
    lcd.print("3 - Alterar Senha");
    lcd.setCursor(0, 3);
    lcd.print("4 - Data e Hora");
  char s = meuteclado.waitForKey();
  //atribuir as conf escolhidas
  //altera o tempo de rodizio
  if (s == '1') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp de Rodizio");
    lcd.setCursor(0, 1);
    lcd.print("Apenas N horas:");
    lcd.setCursor(0, 2);
    lcd.print("Apenas 2 Digitos:");
    String novoTempR;
    while (novoTempR.length() < 2) {
      char s = meuteclado.waitForKey();
      novoTempR.concat(s);
      lcd.setCursor(novoTempR.length(), 3);
      lcd.print(novoTempR);
    }
    tempoRodizio = (novoTempR.toFloat()) * 3600000;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Novo Rodizio");
      lcd.setCursor(0, 1);
      lcd.print(tempoRodizio / 3600000);
      lcd.print(" em ");
      lcd.print(tempoRodizio / 3600000);
      lcd.setCursor(6, 1);
      lcd.print(" Horas");     
    delay(2000);
    //altera a temperatura ideal
  } else if (s == '2') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temperatura Ideal");
      lcd.setCursor(0, 1);
      lcd.print("Apenas 2 Digitos:");
    String novaTempIdeal;
    while (novaTempIdeal.length() < 2) {
      char s = meuteclado.waitForKey();
      novaTempIdeal.concat(s);
      lcd.setCursor(novaTempIdeal.length(), 2);
      lcd.print(novaTempIdeal);
    }
    tempIdeal = novaTempIdeal.toFloat();
    Serial.println(tempIdeal);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp. Alterada:");
      lcd.setCursor(0, 1);
      lcd.print(tempIdeal);
    delay(2000);
    //se caso 3 altera a senha
  } else if (s == '3') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Nova Senha:");
    String novaSenha;
    while (novaSenha.length() < 4) {
      char s = meuteclado.waitForKey();
      novaSenha.concat(s);
      lcd.setCursor(novaSenha.length(), 1);
      lcd.print("*");
    }
    senha = novaSenha;
    //Opção alterar a data e hora
  } else if (s == '4') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Nova Data/Hora:");    
      lcd.setCursor(0, 1);
      lcd.print("Formato:");
      // TssmmhhWDDMMYYYY aka set time
      lcd.setCursor(0, 2);
      lcd.print("ssmmhhSDDMMYYYY");
    String novaData = "T";
    while (novaData.length() < 16) {
      char s = meuteclado.waitForKey();
      novaData.concat(s);
      lcd.setCursor(novaData.length()-2, 3);
      lcd.print(s);      
    }
    char nData[16];
    for (int i = 0; i < novaData.length(); i++){
       nData[i] = novaData.charAt(i);
    }       
    parse_cmd(nData,16); //Usado somente para atualizar o RTC.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Nova Data/Hora:"); 
      lcd.setCursor(0, 1);
      lcd.print(novaData);    
    Serial.println(nData);
    delay(2000);
  }
}

/*** Função que grava um log  de eventos no cartão SD*****/  
void gravarLog(String evento){
  
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    //Serial.println(dataFile.size()+" bytes");
    Serial.println("------------------------------------------------");
    if (dataFile) {
       // dataFile.print("Data: ");
         if (t.mday < 10)
        {
          dataFile.print("0");
        }
        dataFile.print(t.mday);        
        dataFile.print("/");
        if (t.mon < 10)
        {
          dataFile.print("0");
        }
        dataFile.print(t.mon);
        dataFile.print("/");
        dataFile.print(t.year);
        dataFile.print(" ");
        if (t.hour < 10)
        {
          dataFile.print("0");
        }
        dataFile.print(t.hour);
        dataFile.print(":");
        if (t.min < 10)
        {
          dataFile.print("0");
        }
        dataFile.print(t.min);
        dataFile.print(":");
        if (t.sec < 10)
        {
          dataFile.print("0");
        }
        dataFile.print(t.sec);
      dataFile.print(" ");
      dataFile.print(armazenaTemperatura1);
      dataFile.print("C ");
      dataFile.print(armazenaTemperatura2 );
      dataFile.print("C ");
      dataFile.print(temperaturaLm3);
      dataFile.print("C ");
      dataFile.print(temperaturaLm4 );
      dataFile.print("C ");
      dataFile.print(armazenaUmidade1);
      dataFile.print("% ");
      dataFile.print(armazenaUmidade2);
      dataFile.print("% ");
      dataFile.print(nivelGas);
      dataFile.print("ppm ");
      dataFile.println(evento);  
      dataFile.close();    
      Serial.println("Arquivo datalog fechado");      
    }  
    else {
      Serial.println("erro ao abrir datalog.txt, não foi possivel gravar o evento");
    }  
}

