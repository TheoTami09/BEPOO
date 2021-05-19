
#include <ESP8266WiFi.h>
#include <math.h>
#include <string>

/////////////////////////server & wifi/////////////////////////////

// WiFi Router Login
const char* SSID = "Huawei M8 20";
const char* password = "bazoomdu83";

// Create the ESP Web Server on port 80
WiFiServer WebServer(80);
// Web Client
WiFiClient client;


/////////////////////////classes/////////////////////////////
class myCode{
  
  private :
  
  int code[4];
  
  protected :

  bool codeIsBlocked = false;

  long timeLocked = 0;

  int failedTry = 0;
  
  public :

  bool isCodeRight(myCode c){
    
    bool result = false;
    
    for(int i=0; i<4; i++){
      
      if(c.code[i] == code[i]){
        result = true;
      } else {
        result = false;
        break;
      }
      
    }
    
    return result;
  }

  void addFailedTry(){
    this->failedTry++;
  }

  bool checkIfBlocked(){
    if(this->failedTry < 3){
      return false;
    } else if(this->failedTry >= 3){
      this->failedTry = 3;
      this->codeIsBlocked = true;
      return true;
    }
    
  }

  void changeTimeLocked(long val){
    timeLocked = val;
  }

  long getTimeLocked(){
    return timeLocked;
  }

  void unlockCode(){
    this->codeIsBlocked = false;
    this->failedTry = 0;
    this->timeLocked = 0;
  }

  myCode();
  myCode(int c[4]);


};

myCode::myCode(){
  for(int i=0; i<4; i++){
    code[i] = -1;
  }
}

myCode::myCode(int c[4]){
  for(int i=0; i<4; i++){
    code[i]=c[i];
  }
}



class myCodeInput : myCode{
  private : 
  int codeIn[4];
  
  public :

  int * returnCode(){
    return codeIn;
  }
  
  void changeCode(int input){
    for(int i=0; i<4; i++){
      
      if(codeIn[i] == -1){
        codeIn[i] = input;
        break;
      } 
      
    }
  }

  String afficheCode(){
    String result = "";
    
    for(int i=0; i<4; i++){
      
      if(codeIn[i] == -1){
        break;
      } else {
        result += String(codeIn[i]);
      }
      
    }
    return result;
  }

  void resetCodeInput(){
    for(int i=0; i<4; i++){
    codeIn[i] = -1;
    }
  }
  
  myCodeInput();
  
};

myCodeInput::myCodeInput(){
  for(int i=0; i<4; i++){
    codeIn[i] = -1;
  }
}

bool operator==(myCode c1, myCode c2){
  return c1.isCodeRight(c2);
}
/////////////////////////constants/////////////////////////////

const int pinValInput = A0;
const int pinTouch = 15;
const int pinRedLED = 12;
const int pinGreenLED = 14;
const int pinBuzzer = 13;
bool oldValue = false;
bool validateCode = false;
bool koo = false;
int moo[4] = {1,2,3,4};
int val;

myCode cWanted(moo);
myCodeInput cInput;
String msgCode = "";



/////////////////////////fonctions/////////////////////////////

//retourne la valeur d'entrée reçue via le potentiomètre
int getValInput(){
  
    float val = analogRead(pinValInput);

    val = 10*(val/1024);

    int result = int(val);

return result;
}

//retourne vrai si le touch sensor est activé, sinon faux.
bool isTouchPressed(){
  
    if (digitalRead(pinTouch)) {
      return true;
    } else {
      return false;
    }

}

//permet de faire clignoter rapidement les led
void flashLED (char color){
  if (color == 'r'){
    digitalWrite(pinRedLED, HIGH);
    delay(500);
    digitalWrite(pinRedLED, LOW);
    delay(500);
  } else if(color == 'g'){
    digitalWrite(pinGreenLED, HIGH);
    delay(500);
    digitalWrite(pinGreenLED, LOW);
    delay(500);
  }
}


/////////////////////////setup & main/////////////////////////////

void setup() {
  //setup des pins des led et du buzzer
  pinMode(pinRedLED, OUTPUT);
  pinMode(pinGreenLED, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, LOW);
  digitalWrite(pinRedLED, LOW);
  digitalWrite(pinGreenLED, LOW);
  
  Serial.begin(115200);
  delay(10);
  Serial.println();

  //connexion au réseau WiFi
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.print("Connexion à ");
  Serial.println(SSID);
  WiFi.begin(SSID, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connecté au WiFi");

  // Start the Web Server
  WebServer.begin();
  Serial.println("Serveur Web démarré");

  // Print the IP address
  Serial.print("Adresse de connexion à l'ESP8266 ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

void loop() {
  //on regarde si quelqu'un s'est connecté
  client = WebServer.available();
  if (!client) {
    return;
  }

  //on attend que quelqu'un envoie des données

  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r\n');
  Serial.println(request);
  client.flush();

  //pour la detection de page web actuelle
  if (request.indexOf("/CODE_LOCKED") != -1) {
    cWanted.unlockCode(),
    flashLED('g');
    flashLED('r');
    koo = true;
  } 
  if (request.indexOf("/MAIN") != -1) {
    koo = false;
  }
  
  
  //code html pour la page web
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>ESP8266 Demo</title>");
  client.println("<meta http-equiv=\"refresh\" content=\"1\">"); //to refresh the page every second
  client.println("</head>");
  client.println("<body>");

  
   val = getValInput();
  //si on appuie sur le touch pad et que la valeur n'est pas 10 
  //(qu'on définit comme etant celle de validation de code)
  //on change le code entré
  if(isTouchPressed() and val != 10){
    if(!oldValue){
      cInput.changeCode(val);
    }

    
   //sinon on effectue la comparaison entre code, avec la surcharge de l'opérateur '=='
  } else if(isTouchPressed() and val == 10){
    myCode cAux(cInput.returnCode());
    if(cWanted==cAux){
      client.println("CODE OK");
      flashLED('g');
      cInput.resetCodeInput();
      client.print("<br>");
    } else {
      client.println("ERREUR");
      flashLED('r');
      cWanted.addFailedTry();
      cInput.resetCodeInput();
      cWanted.changeTimeLocked(millis());
      client.print("<br>");
    }
  }
  oldValue = isTouchPressed(); //sert à éviter un appui multiple, 
                              //on doit retirer le doigt du touch sensor pour de nouveau en entrer un

  //si le code est bloqué (3 tentatives échouées) on affiche le code correspondant et actionne le buzzer
  //les cas 'else' correspondent à l'affichage relatif aux différentes pages du site web
  if(cWanted.checkIfBlocked() ){
    client.println("Code bloqué ! <a href=\"/CODE_LOCKED\">UNLOCK</a></br> ");
    client.println("Date de blocage :");
    client.println(cWanted.getTimeLocked());
    client.print("<br>");
    digitalWrite(pinBuzzer, LOW);
    delay(500);
    digitalWrite(pinBuzzer, HIGH);
    delay(500);
    digitalWrite(pinBuzzer, LOW);

  } else if(!koo){
  if(val != 10){
  client.println("Valeur en entrée : ");
  client.print(val);
  client.print("<br>");
  } else {
  client.println("Valider code?");
  client.print("<br>");
  }
  
  client.println("Code entré : ");
  client.print(cInput.afficheCode());
  client.print("<br>");
  
  client.println("Bouton pressé : ");
  client.print(isTouchPressed());
  client.print("<br>");
  } else if(koo){
    cWanted.unlockCode(),
    flashLED('g');
    flashLED('r');
    client.println("<a href=\"/MAIN\">Back to main menu</a></br>");
  }
    
  client.println("</body>");
  client.println("</html>");

  delay(1);

}
