#include <LiquidCrystal_I2C.h>
#include <Adafruit_I2CDevice.h>
#include <Keypad.h>
#include "RTClib.h"
#include <Stepper.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

RTC_DS3231 rtc;

//var teclado
const byte linhas = 4;
const byte colunas = 4;
const char teclasMatriz[linhas][colunas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pinLinha[linhas] = {9, 8, 7, 6};
byte pinColuna[colunas] = {5, 4, 3, 2};

Keypad teclado = Keypad(makeKeymap(teclasMatriz), pinLinha, pinColuna, linhas, colunas);

//var rtc
byte h, m, s;
byte hp[2], mp[2], sp[2];
byte dia[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool alm[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int almHrs[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int almMin[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte local = 0;
int w;
//var tempo
unsigned long millisAnteriores = 0;
const int intervalo = 500;

//vars de modificação
byte i[6] = {0, 0, 0, 0, 0, 0};
int telas = 1;

int hrloka[2] = {-1, -1};
int minloka[2] = {-1, -1};
char loka;

char key;

bool mute = 0;

byte som[8] = {B00010, B00110, B11110, B11110, B11110, B11110, B00110, B00010};
byte x[8] = {B00000, B00000, B10001, B01010, B00100, B01010, B10001, B00000};
byte coracao[8] = {B0000, B0000, B01010, B10101, B10001, B01010, B00100, B00000};

float STEPS_PER_REV = 32; 
float GEAR_RED = 6;
float STEPS_PER_OUT_REV = STEPS_PER_REV * GEAR_RED;
int StepsRequired;

Stepper steppermotor(STEPS_PER_REV, 10, 11, 12, 13);
 

void setup() {
  rtc.begin();
  DateTime now = rtc.now();
  if(rtc.lostPower())
  {
    rtc.adjust(rtc.now());
  }
  pinMode(1, OUTPUT);
  
  
  lcd.init();
  lcd.backlight();
  lcd.print("Ola, mundo!");
  delay(1000);
  lcd.createChar(1, som);
  lcd.createChar(2, x);
  lcd.createChar(3, coracao);
  lcd.clear();
}

void loop() {
  //Tempo
  unsigned long millisCorridos = millis();
  if(millisCorridos > 4294967294)
  {
    millisCorridos = 0;
    millisAnteriores = 0;
  }
  //rtc
  DateTime now = rtc.now();
  h = now.hour(), DEC;
  m = now.minute(), DEC;
  s = now.second(), DEC;

  //pitar
  i[0] = 0;
  i[1] = 0;
  i[2] = 0;
  i[3] = 0;
  i[4] = 0;
  i[5] = 0;
  telas = 0;

  key = teclado.getKey();

  //função girar
  for( i[0] = 0; i[0]<18; i[0]++)
  {
    if(h == almHrs[i[0]] && m == almMin[i[0]] && s == 0 && dia[i[0]] == 0)
    {
      w = (i[0]) - local;
      if (w < 0)
      {
        w = 19 + w;
      }
      local = (i[0]);
      for(i[3]; i[3]<=w; i[3]++)
      {
        girar();
      }
      if(alm[i[0]] == 1)
      {
        while(i[1]<1)
        {
          key = teclado.getKey();
          pitar();
          if(key == "A"){
            almHrs[i[0]] = 0;
            almMin[i[0]] = 0;
            if(h < 12)
            {
              lcd.clear();
              lcd.setCursor(4,0);
              lcd.print("Tenha um");
              lcd.setCursor(4,1);
              lcd.print("bom dia");
              lcd.write(3);
            }else if(h > 11 && h < 19){
              lcd.clear();
              lcd.setCursor(2,0);
              lcd.print("Tenha uma");
              lcd.setCursor(3,1);
              lcd.print("boa tarde");
              lcd.write(3);
            }else{
              lcd.clear();
              lcd.setCursor(3,0);
              lcd.print("Tenha uma");
              lcd.setCursor(3,1);
              lcd.print("boa noite");
              lcd.write(3);
            }
            delay(2000);
            lcd.clear();
            i[1]++;
          }
        }
      }
    }
  }
  if(millisCorridos - millisAnteriores >= intervalo)
  {
    millisAnteriores = millisCorridos;
    mostrarTemp();
  }
  if(key == '*')
  {
    if(mute == 0)
    {
      mute = 1;
    }else{
      mute = 0;
    }
  }
  if(key == 'B')
  {
    lcd.clear();
    lcd.setCursor(5,0);
    lcd.print("Ajustar");
    lcd.setCursor(5,1);
    lcd.print("Alarme");
    delay(1500);
    while(i[2]<1)
    {
      key = teclado.getKey();
      if(key == '*')
      {
        telas--;
      }
      if(key == '#')
      {
        telas++;
      }
      if(key == 'A')
      {
        if(alm[telas] == 0)
        {
          alm[telas] = 1;
          lcd.clear();
          lcd.print("Alarme ");
          lcd.print(telas+1);
          lcd.setCursor(0,1);
          lcd.print("Ligado");
          delay(1500);
          i[2]++;
        }else{
          alm[telas] = 0;
          lcd.clear();
          lcd.print("Alarme ");
          lcd.print(telas+1);
          lcd.setCursor(0,1);
          lcd.print("Desligado");
          delay(1500);
          i[2]++;
        }
      }
      if(key == 'D')
      {
        lcd.clear();
        lcd.print("Saindo");
        delay(100);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        delay(500);
        i[2]++;
      }
      if(telas > 18)
      {
        telas = 0;
      }
      if(telas < 0)
      {
        telas = 18;
      }
      delay(250);
      lcd.clear();
      lcd.print("Alarme ");
      lcd.print(telas+1);
      lcd.print(": ");
      if(almHrs[telas] < 10)
      {
        lcd.print("0");
        lcd.print(almHrs[telas]);
      }else{
        lcd.print(almHrs[telas]);
      }
      lcd.print(":");
      if(almMin[telas] < 10)
      {
        lcd.print("0");
        lcd.print(almMin[telas]);
      }else{
        lcd.print(almMin[telas]);
      }
      lcd.setCursor(0,1);
      if(alm[telas] == 0)
      {
        lcd.print("Desligado");
      }else{
        lcd.print("Ligado");
      }
    }
  }
  if(key == 'C')
  {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Configurar");
    lcd.setCursor(5,1);
    lcd.print("Alarme");
    delay(1500);
    while(i[2]<1)
    {
      key = teclado.getKey();
      if(key == '*')
      {
        telas--;
      }
      if(key == '#')
      {
        telas++;
      }
      delay(250);
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("alarme:");
      lcd.setCursor(7,1);
      if(telas<9)
      {
        lcd.setCursor(7,1);
        lcd.print("0");
        lcd.print(telas+1);
        }else{
        lcd.setCursor(7,1);
        lcd.print(telas+1);
      }

      if(telas > 18)
      {
        telas = 0;
      }
      if(telas < 0)
      {
        telas = 18;
      }
      if(key == 'D')
      {
        lcd.clear();
        lcd.print("Saindo");
        delay(100);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        delay(500);
        lcd.print(".");
        delay(500);
        i[2]++;
      }
      if(key == 'A')
      {
        while(i[3]<1)
        {
          key = teclado.getKey();
          i[5] = 0;
          loka = 0;
          delay(250);
          lcd.clear();
          lcd.setCursor(5,0);
          lcd.print("Horas:");
          lcd.setCursor(7,1);
          if(hrloka[0] == -1 && hrloka[1] == -1)
          {
            lcd.print("00");
            delay(200);
          }else if(hrloka[0] != -1 && hrloka[1] == -1)
          {
            lcd.print(hrloka[0]);
            lcd.print("0");
          }else{
            lcd.print(hrloka[0]);
            lcd.print(hrloka[1]);
          }
          if(hrloka[0] > 2)
          {
            hrloka[0] = -1;
          }
          if(hrloka[0] == 2)
          {
            if(hrloka[1] > 3)
            {
              hrloka[1] = -1;
            }
          }
          for(i[5] = 0; i[5]<10; i[5]++)
          {
            if(i[5] == 0){
              loka = '0';
            }else if(i[5] == 1){
              loka = '1';
            }else if(i[5] == 2){
              loka = '2';
            }else if(i[5] == 3){
              loka = '3';
            }else if(i[5] == 4){
              loka = '4';
            }else if(i[5] == 5){
              loka = '5';
            }else if(i[5] == 6){
              loka = '6';
            }else if(i[5] == 7){
              loka = '7';
            }else if(i[5] == 8){
              loka = '8';
            }else if(i[5] == 9){
              loka = '9';
            }
            if(key == loka)
            {
              if(hrloka[0] == -1)
              {
                hrloka[0] = i[5];
              }else{
                hrloka[1] = i[5];
              }
            }
          }
          if(key == 'D')
          {
            lcd.clear();
            lcd.print("Saindo");
            delay(100);
            lcd.print(".");
            delay(500);
            lcd.print(".");
            delay(500);
            lcd.print(".");
            delay(500);
            i[3]++;
            i[2]++;
            hrloka[0] = -1;
            hrloka[1] = -1;
            minloka[0] = -1;
            minloka[1] = -1;
          }
          if(key == 'A')
          {
            almHrs[telas] = hrloka[0]*10 + hrloka[1];
            while(i[4]<1)
            {
              key = teclado.getKey();
              i[5] = 0;
              loka = 0;
              delay(250); 
              lcd.clear();
              lcd.setCursor(4,0);
              lcd.print("minutos:");
              lcd.setCursor(7,1);
              if(minloka[0] == -1 && minloka[1] == -1)
              {
                lcd.print("00");
              }else if(minloka[0] != -1 && minloka[1] == -1)
              {
                lcd.print(minloka[0]);
                lcd.print("0");
              }else{
                lcd.print(minloka[0]);
                lcd.print(minloka[1]);
              }
              if(minloka[0] > 5)
              {
                minloka[0] = -1;
              }
              for(i[1] = 0; i[1]<10; i[1]++)
              {
                if(i[1] == 0){
                  loka = '0';
                }else if(i[1] == 1){
                  loka = '1';
                }else if(i[1] == 2){
                  loka = '2';
                }else if(i[1] == 3){
                  loka = '3';
                }else if(i[1] == 4){
                  loka = '4';
                }else if(i[1] == 5){
                  loka = '5';
                }else if(i[1] == 6){
                  loka = '6';
                }else if(i[1] == 7){
                  loka = '7';
                }else if(i[1] == 8){
                  loka = '8';
                }else if(i[1] == 9){
                  loka = '9';
                }
                if(key == loka)
                {
                  if(minloka[0] == -1)
                  {
                    minloka[0] = i[1];
                  }else{
                    minloka[1] = i[1];
                  }
                }
              }
              if(key == 'D')
              {
                lcd.clear();
                lcd.print("Saindo");
                delay(100);
                lcd.print(".");
                delay(500);
                lcd.print(".");
                delay(500);
                lcd.print(".");
                delay(500);
                i[4]++;
                i[3]++;
                i[2]++;
                hrloka[0] = -1;
                hrloka[1] = -1;
                minloka[0] = -1;
                minloka[1] = -1;
              }
              if(key == 'A')
              {
                almMin[telas] = minloka[0]*10 + minloka[1];
                while (i[5]<1){
                  key = teclado.getKey();
                  delay(250);
                  lcd.clear();
                  lcd.setCursor(6,0);
                  lcd.print("Dia:");
                  if(dia[telas] == 0)
                  {
                    lcd.setCursor(6,1);
                    lcd.print("Hoje");
                  }else if(dia[telas] == 1){
                    lcd.setCursor(5,1);
                    lcd.print("Amanha");
                  }else if(dia[telas] == 2){
                    lcd.setCursor(1,1);
                    lcd.print("Daqui a 2 dias");
                  }
                  if(dia[telas] > 2)
                  {
                    dia[telas] = 0;
                  }
                  if(dia[telas] < 0)
                  {
                    dia[telas] = 2;
                  }
                  if(key == '*')
                  {
                    dia[telas] --; 
                  }
                  if(key == '#')
                  {
                    dia[telas] ++;
                  }
                  if(key == 'D')
                  {
                    lcd.clear();
                    lcd.print("Saindo");
                    delay(100);
                    lcd.print(".");
                    delay(500);
                    lcd.print(".");
                    delay(500);
                    lcd.print(".");
                    delay(500);
                    i[5]++;
                    i[4]++;
                    i[3]++;
                    i[2]++;
                    hrloka[0] = -1;
                    hrloka[1] = -1;
                    minloka[0] = -1;
                    minloka[1] = -1;
                  }
                  if(key == 'A')
                  {
                    hrloka[0] = -1;
                    hrloka[1] = -1;
                    minloka[0] = -1;
                    minloka[1] = -1;
                    lcd.clear();
                    lcd.print("Programado ");
                    lcd.write(3);
                    delay(1500);
                    lcd.clear();
                    i[5]++;
                    i[4]++;
                    i[3]++;
                    i[2]++;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if(mute == 1)
  {
    lcd.setCursor(14,1);
    lcd.write(1);
    lcd.write(2);
  }
  if(h == 0 && m == 0 && s == 0)
  {
    for(byte p = 0; p < 20; p++)
    {
      dia[p] = dia[p] -1;
      if(dia[p] < 0)
      {
        dia[p] = 0;
      }
    }
  }
}

void pitar() {
  if(mute == 0)
  {
    digitalWrite(1, HIGH);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Hora de se");
    lcd.setCursor(4, 1);
    lcd.print("medicar!");
    delay(500);
    digitalWrite(1, LOW);
    lcd.clear();
    delay(500);
  }else{
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Hora de se");
    lcd.setCursor(4, 1);
    lcd.print("medicar!");
    delay(500);
    lcd.clear();
    delay(500);
  }
  
}

void mostrarTemp() {
  lcd.clear();
  lcd.setCursor(4, 0);
  if(h<10)
  {
    lcd.print("0");
    lcd.print(h, DEC);
  }else{
    lcd.print(h, DEC);
  }
  lcd.print(":");
  if(m<10)
  {
    lcd.print("0");
    lcd.print(m, DEC);
  }else{
    lcd.print(m, DEC);
  }
  lcd.print(":");
  if(s<10)
  {
    lcd.print("0");
    lcd.print(s, DEC);
  }else{
    lcd.print(s, DEC);
  }
}

void girar()
{
  StepsRequired  =  STEPS_PER_OUT_REV / 2;   
  steppermotor.setSpeed(100);  
  steppermotor.step(StepsRequired );
  delay(2000);
}
