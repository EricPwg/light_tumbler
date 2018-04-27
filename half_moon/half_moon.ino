#include <MaxMatrix.h>
#include <math.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 accelgyro;

int hold_state;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t lax, lay, laz;


const byte DATA1 = 11;  // 資料輸入線
const byte CS1= 10;  // 晶片選擇線
const byte CLK1 = 9;// 時脈線
const byte DATA2 = 11;  // 資料輸入線
const byte CS2= 10;  // 晶片選擇線
const byte CLK2 = 9;  // 時脈線
const byte maxInUse = 6;   // 代表串連兩個MAX7219模組
MaxMatrix m1(DATA1, CS1, CLK1, maxInUse);
//MaxMatrix m2(DATA2, CS2, CLK2, maxInUse);

long long int light_data1[] = {0, 0, 0, 0, 0, 0, 0, 0};
long long int light_data2[] = {0, 0, 0, 0, 0, 0, 0, 0};

short LED_R2[16] = {3, 7, 2, 4, 0, 5, 1, 6, 27, 31, 26, 28, 24, 29, 25, 30};
short LED_G2[16] = {14, 8, 10, 11, 22, 21, 20, 19, 38, 32, 34, 35, 46, 45, 44, 43};
short LED_B2[16] = {9, 13, 12, 15, 17, 16, 18, 23, 33, 37, 36, 39, 41, 40, 42, 47};

short LED_R1[16] = {27, 31, 26, 28, 24, 29, 25, 30, 3, 7, 2, 4, 0, 5, 1, 6};
short LED_G1[16] = {33, 32, 34, 35, 41, 40, 42, 43, 9, 8, 10, 11, 22, 16, 18, 19};
short LED_B1[16] = {38, 37, 36, 39, 46, 45, 44, 47, 14, 13, 12, 15, 17, 21, 20, 23};

short vector1_x[16] = {70, 19, 31, -27, -49, -81, -87, -81, -49, -27, 19, 31, 70, 100, 60, 0};
short vector1_y[16] = {51, 57, 95, 82, 35, 59, 0, -59, -35, -82, -57, -95, -51, 0, 0, 0};
short vector1_z[16] = {50, 80, 7, 50, 80, 7, 50, 7, 80, 50, 80, 7, 50, 7, 80, 100};

void cal_color(int num, int x, int y, int z, int *r, int *g, int *b){
  int costheta = (x*vector1_x[num]+y*vector1_y[num]+z*vector1_z[num])/10;
  int rt = (costheta-500)*8/500;
  int gt = abs(costheta-500)*8/200;
  int bt = (500-costheta)*8/500;
  *r = (rt > 8 || rt < 0) ? 0 : rt;
  *g = (gt > 8 || gt < 0) ? 0 : gt;
  *b = (bt > 8 || bt < 0) ? 0 : bt;
}

void reset1(){
  for (int i=0;i<8;i++){
    light_data1[i]  = (long long int)0;
  }
}

void reset2(){
  for (int i=0;i<8;i++){
    light_data2[i]  = (long long int)0;
  }
}

void set_color1(int LED_NUM, int R, int G, int B){
  for (int i=0;i<R;i++){
    light_data1[i] |= ((long long int)1 << LED_R1[LED_NUM]);
  }
  for (int i=0;i<G;i++){
    light_data1[i] |= ((long long int)1 << LED_G1[LED_NUM]);
  }
  for (int i=0;i<B;i++){
    light_data1[i] |= ((long long int)1 << LED_B1[LED_NUM]);
  }
}

void set_color2(int LED_NUM, int R, int G, int B){
  for (int i=0;i<R;i++){
    light_data2[i] |= ((long long int)1 << LED_R2[LED_NUM]);
  }
  for (int i=0;i<G;i++){
    light_data2[i] |= ((long long int)1 << LED_G2[LED_NUM]);
  }
  for (int i=0;i<B;i++){
    light_data2[i] |= ((long long int)1 << LED_B2[LED_NUM]);
  }
}

void show1(){
  for (int i=0;i<6;i++){
    byte ttt[] = {8, 8, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int j=0;j<8;j++){
      ttt[j+2] = (light_data1[j] >> (i*8));
      //Serial.print(ttt[j+2]);
      //Serial.print(" ");
    }
    //Serial.println();
    m1.writeSprite(i*8, 0, ttt);
  }
}

void show2(){
  for (int i=0;i<6;i++){
    byte ttt[] = {8, 8, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int j=0;j<8;j++){
      ttt[j+2] = (light_data2[j] >> (i*8));
      //Serial.print(ttt[j+2]);
      //Serial.print(" ");
    }
    //Serial.println();
    //m2.writeSprite(i*8, 0, ttt);
  }
}

bool if_side(){
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  if (abs(ax-lax) < 2000 && abs(ay-lay) < 2000 && abs(az-laz) < 2000){
    hold_state++;
  }
  else {
    hold_state = 0;
  }
  lax = ax;
  lay = ay;
  laz = az;
  
  //Serial.println(hold_state);
  if (hold_state < 250) return true;
  else return false;
}

void side(){
  ax/=160;
  ay/=160;
  az/=160;
  reset1();
  for (int i=0;i<16;i++){
    int r, g, b;
    cal_color(i, ax, ay, az, &r, &g, &b);
    set_color1(i, r, g, b);
  }
  //m1.init();
  show1();
}

void setup() {
  hold_state = 0;
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    accelgyro.initialize();
  // put your setup code here, to run once:
  m1.init();
  //m2.init();
  m1.setIntensity(20);
  //m2.setIntensity(20);
  //delay(1000);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  side();
  //delay(10);
  /*
  if (Serial.available()){
    int t = Serial.parseInt();
    reset1();
    set_color1(t, 5, 5, 5);
    show1();
  }
  */
  
  while(!if_side()){
    for (int i=0;i<16;i++){
      set_color1(i, 6, 0, 0);
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
    for (int i=0;i<16;i++){
      set_color1(i, 0, 6, 0);
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
    for (int i=0;i<16;i++){
      set_color1(i, 0, 0, 6);
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
  
    for (int i=0;i<16;i++){
      set_color1(i, i%3, i%6, i%4);
      show1();
      delay(50);
      if (if_side()) break;
    }
    for (int i=0;i<16;i++){
      set_color1(i, i%6, i%4, i%3);
      show1();
      delay(50);
      if (if_side()) break;
    }
    reset1();
    for (int j=8;j>0;j--){
      for (int i=0;i<16;i++){
        set_color1(i, j, j, j);
      }
      show1();
      reset1();
      delay(50);
      if (if_side()) break;
    }
    reset1();
    for (int j = 1;j<9;j++){
      for (int i=0;i<16;i++){
        set_color1(i, j, 0, 0);
      }
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
    for (int j = 8;j > 0;j--){
      for (int i=0;i<16;i++){
        set_color1(i, j, 0, 0);
      }
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
  
    for (int j = 1;j<9;j++){
      for (int i=0;i<16;i++){
        set_color1(i, 0, j, 0);
      }
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
    for (int j = 8;j > 0;j--){
      for (int i=0;i<16;i++){
        set_color1(i, 0, j, 0);
      }
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
  
    for (int j = 1;j<9;j++){
      for (int i=0;i<16;i++){
        set_color1(i, 0, 0, j);
      }
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
    for (int j = 8;j > 0;j--){
      for (int i=0;i<16;i++){
        set_color1(i, 0, 0, j);
      }
      show1();
      delay(50);
      if (if_side()) break;
      reset1();
    }
  }
}

