/*　列車位置検出＆サーボモーター制御　Ver 6.0
 * 
 *　Serial1:19(RX),18(TX)=>RasPi １８Pinレベル変換要(未使用：GPIO）
 *　Servo1(HomeIn):2, Servo2(HomeOut):3
 *　GPIO:1=4,2=5,3=6,4=7,5=8,6=9,7=10,8=11,9=12,10=13(NG)
 *　GPIO:10=14,11=15,12=16,13=17, 14=20,15=21
 *  GPIOLED(red):H12=50,H13=51,H14=52,H15=53
 *  GPIOLED(Color):InSub(Red)=42,InSub(gren)=43,InMain(Red)=44,InMain(Gren)=45
 *  OutSub(Red)=46,OutSub(Gern)=47,OutMain(Red)=48,OutMain(Gern)=49
 *  PrarelOut:Bit0:38,Bit1:39,Bit2:40,Bit3:41
 *  BitSelect:Bit0:36,Bit1:37
 *  Bus_Location Servo 4個,  充電ステーション クランプ Servo 1個
 *
 *　JROBO itoh junichi   2020/09/04
 *                       2020/09/08 // GPIO/Pin変更
 *                       2020/09/08 // Servo+
 *                       2020/09/28 // HoleSenser+1(15)
 *                       2020/09/28 // LED+(red,Color)
 *                       2020/10/10 // RasPi通信（GPIOに変更）
 *                       2020/10/12 // 位置情報のホールド機能追加
 *                       2020/10/13 // 上記機能アルゴリズム見直し
 *                       2020/10/15 // 上記機能アルゴリズム見直し
 *                       2020/10/20 // GPIO_OUT追加
 *                       2020/10/22 // 外周列車追尾システム追加(1)
 *                       2020/10/29 // 外周列車追尾システム追加(2)
 *                       2020/10/30 // 外周列車追尾システム追加(3)
 *                       2020/11/02 // 外周列車追尾システム追加(4)
 *                       2020/11/09 // HomePoji番号変更（列車番号と合致させる）
 *                        1 2 3 4
 *               sw35     0 0 0 1
 *               sw36     0 1 1 0
 *               sw37     1 0 1 0
 *                       
*/

#include <VarSpeedServo.h>

// サーボ移動速度可変機能
VarSpeedServo HomeIn;
VarSpeedServo HomeOut;

// サーボ本線・引込線移動角度パラメーター
int posInPSub = 10;       // 0度相当（引込側）
int posInPMain = 10;     // 15度相当（本線側）
int posOutPMain = 10;     // 0度相当（本線側）
int posOutPSub = 10;     // 15度相当（引込側）
int speedMax = 1;        // 最高速
int speedMin = 1;        // (低速:1-高速:255)
// ホールセンサ位置（１－１５）
//               列車番号                              4  2  3  1
//                P番号    1 2 3 4 5 6 7  8  9  10 11|12 13 14 15
//const int digitalPin[15]={4,5,6,7,8,9,10,11,12,14,15,16,17,20,21};
//                                                                     11 12 13 14
//               列車番号                                                4  2  3  1
//                P番号      1 2 3 4 5 6 7  8  9  10 11|                20 22 21 23
//-----------------------------------------------------------------------------------
const int digitalPin[17]={0,4,5,6,7,8,9,10,11,12,14,15,16,17,22,23};
const int digitalMainPin[3]={0,16,17};
const int digitalOutPin[12]={0,4,5,6,7,8,9,10,11,12,14,15};
const int digitalSubPin[3]={0,22,23};
//               //f[15]= 0,1,2,3,4,5,,6,,7,,8,,9,10,11,12,13,14
// ホーム列車位置確認赤色LED（４ポジション）                                  4  3  2  1
const int digitalPinPS[4]={50,51,52,53};
// ポイント切替信号（赤色、緑色LED：本線側/引込線側）
const int digitalPinSev[8]={42,43,44,45,46,47,48,49};

// ホーム内列車位置によるポイント切替の為の情報作成変数
int a = 0;       // 20番ホールセンサ
int b = 0;       // 21番ホールセンサ
int c = 0;       // 22番ホールセンサ
int d = 0;       // 23番ホールセンサ
int e = 0;       // Bit演算
int e_old = 1;   // 過去のBit値

// Bit変化（ホールセンサ：１－１５）
int Out_val[5];
int s[3] = {0,0,0};
int out[12]={0,0,0,0,0,0,0,0,0,0,0,0};
int m[3] = {0,0,0};
int daisu = 0;  // 列車台数
int cont =0;    // 列車台数カウント用
int val[15];
int sTrainPo[3] = {0,0,0};
int outTrainPo[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
int mTrainPo[3] = {0,0,0};

int Train[5]     = {0,0,0,0,0};
//int sTrain[5]     = {0,0,0,0,0};

int Train_old[5] = {0,0,0,0,0};



int TrainInPoji1 = 0; // 列車1（23番）用ポジション
int TrainInPoji2 = 0; // 列車２（22番）用ポジション
int TrainInPoji3 = 0; // 列車３（21番）用ポジション
int TrainInPoji4 = 0; // 列車４（20番）用ポジション
// 
int TrainOutPoji1 = 0; // 列車（1番）用ポジション
int TrainOutPoji2 = 0; // 列車（2番）用ポジション
int TrainOutPoji3 = 0; // 列車（3番）用ポジション
int TrainOutPoji4 = 0; // 列車（4番）用ポジション

int TrainInFlag1 = 0; // 列車１（23番）用状態記憶フラグ
int TrainInFlag2 = 0; // 列車２（22番）用状態記憶フラグ
int TrainInFlag3 = 0; // 列車３（21番）用状態記憶フラグ
int TrainInFlag4 = 0; // 列車４（20番）用状態記憶フラグ

int TrainOutFlag1S = 0;
int TrainOutFlag1M = 0;
int TrainOutFlag2S = 0;
int TrainOutFlag2M = 0;
int TrainOutFlag3S = 0;
int TrainOutFlag3M = 0;
int TrainOutFlag4S = 0;
int TrainOutFlag4M = 0;

int TrainMainFlag = 0;
int TrainSubFlag = 0;





// RasPi-ArduinoMEGA　通信



// SETUP //////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
 
// ホーム内列車位置確認赤LED出力設定  
  for(int i=0; i < 4; i++){
    pinMode(digitalPinPS[i],  OUTPUT);
  }
// ポイント切替用信号用カラーLED出力設定
// for(int i=0; i < 8; i++){
 //   pinMode(digitalPinSev[i],  OUTPUT);
//  }
// ホールセンサ情報入力設定（センサはプルアップされている）
//  for(int i=0; i < 24; i++){
 //   pinMode(digitalPin[i],  INPUT_PULLUP);
 // }
// Bit通信(Raspi-ArduinoMEGA)
  pinMode(38,  OUTPUT);
  pinMode(39,  OUTPUT);
  pinMode(40,  OUTPUT);
  pinMode(41,  OUTPUT);
  digitalWrite(38,  HIGH);
  digitalWrite(39,  HIGH);
  digitalWrite(40,  HIGH);
  digitalWrite(41,  HIGH);

  pinMode(35,  INPUT_PULLUP);
  pinMode(36,  INPUT_PULLUP);
  pinMode(37,  INPUT_PULLUP);
  
// 初期サーボ制御（引込線優先）  
  HomeIn.attach(3);
  HomeOut.attach(2);
  delay(1300);
// 初期は引込み線側へ
//  HomeIn.write(posInPMain, speedMax);  // In（本線側）
//  digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
//  digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
  HomeIn.write(posInPSub, speedMax);     // In（引込側）
  digitalWrite(digitalPinSev[1], HIGH);  // Pin43(SubGreen)
  digitalWrite(digitalPinSev[2], HIGH);  // Pin44(MainRed)
//  HomeOut.write(posOutPMain, speedMax);// Out(本線側)
//  digitalWrite(digitalPinSev[7], HIGH);// Pin49(MainGreen)
//  digitalWrite(digitalPinSev[4], HIGH);// Pin46(SubRed)
  HomeOut.write(posOutPSub, speedMax);   // Out(引込側）
  digitalWrite(digitalPinSev[5], HIGH);  // Pin47(SubGreen)
  digitalWrite(digitalPinSev[6], HIGH);  // Pin48(MainRed)
  delay(200);
  HomeIn.detach();
  HomeOut.detach();
//digitalSub
  s[1] = digitalRead(digitalSubPin[1]);// Sub後部22
  s[2] = digitalRead(digitalSubPin[2]);// Sub先頭23
   
  if(s[1] == 1){sTrainPo[1]=1;}
  if(s[2] == 1){sTrainPo[2]=1;}
 
  if(s[1] == 0){sTrainPo[1]=0;}
  if(s[2] == 0){sTrainPo[2]=0;}

//digitalOut
  out[1] = digitalRead(digitalOutPin[1]);
  out[2] = digitalRead(digitalOutPin[2]);
  out[3] = digitalRead(digitalOutPin[3]);
  out[4] = digitalRead(digitalOutPin[4]);
  out[5] = digitalRead(digitalOutPin[5]);
  out[6] = digitalRead(digitalOutPin[6]);
  out[7] = digitalRead(digitalOutPin[7]);
  out[8] = digitalRead(digitalOutPin[8]);
  out[9] = digitalRead(digitalOutPin[9]);
  out[10] = digitalRead(digitalOutPin[10]);
  out[11] = digitalRead(digitalOutPin[11]);
 
  if(out[1] == 1){outTrainPo[1]=1;}
  if(out[2] == 1){outTrainPo[2]=1;}
  if(out[3] == 1){outTrainPo[3]=1;}
  if(out[4] == 1){outTrainPo[4]=1;}
  if(out[5] == 1){outTrainPo[5]=1;}
  if(out[6] == 1){outTrainPo[6]=1;}
  if(out[7] == 1){outTrainPo[7]=1;}
  if(out[8] == 1){outTrainPo[8]=1;}
  if(out[9] == 1){outTrainPo[9]=1;}
  if(out[10] == 1){outTrainPo[10]=1;}
  if(out[11] == 1){outTrainPo[11]=1;}
  
  if(out[1] == 0){outTrainPo[1]=0;}
  if(out[2] == 0){outTrainPo[2]=0;}
  if(out[3] == 0){outTrainPo[3]=0;}
  if(out[4] == 0){outTrainPo[4]=0;}
  if(out[5] == 0){outTrainPo[5]=0;}
  if(out[6] == 0){outTrainPo[6]=0;}
  if(out[7] == 0){outTrainPo[7]=0;}
  if(out[8] == 0){outTrainPo[8]=0;}
  if(out[9] == 0){outTrainPo[9]=0;}
  if(out[10] == 0){outTrainPo[10]=0;}
  if(out[11] == 0){outTrainPo[11]=0;}
   
//digitalMain  
  m[1] = digitalRead(digitalMainPin[1]);// Main後部20
  m[2] = digitalRead(digitalMainPin[2]);// Main先頭22
    
  if(m[1] == 1){mTrainPo[1]=1;}
  if(m[2] == 1){mTrainPo[2]=1;}
 
  if(m[1] == 0){mTrainPo[1]=0;}
  if(m[2] == 0){mTrainPo[2]=0;}

  
//列車　初期位置 
  if(s[2] == 1){Train[1] = 2;}
 if(s[1] == 1){Train[2] = 1;}
 
//if(s[1] == 1 && m[1]==1){Train[2] = 1;}
//if(m[2] == 1){Train[2] = 2;} 
//  if(s[1] == 1){Train[3] = 1;}
//if(s[1] == 1){Train[4] = 1;}

// if(m[3] == 1){Train[2] = 3;}
// if(out[1] == 1){Train[2] = 3;}
 
 //if(m[1] == 1){Train[3] = 1;}
 //if(m[1] == 1){Train[4] = 1;}  
}
//////////////////////////////////////////////////////////

// LOOP //////////////////////////////////////////////////
void loop() {
  //列車走行情報
  /*
  if(Train[1] > 0 && Train[1] < 15){
  Serial.print("sTrain[1_3]poji=");
  Serial.print(Train[1]-1);
  Serial.println(Train[3]-1);
}
if(Train[3] > 0 && Train[3] < 15){
  //Serial.print("sTrain[3]poji=");
 // Serial.println(sTrain[3]-1);
  // Serial.println("");
}

if(Train[2] > 0 && Train[2] < 15){
  Serial.print("mTrain[2_4]poji=");
  Serial.print(Train[2]-1);
}

if(Train[4] > 0 && Train[4] < 15){
  //Serial.print("mTrain[4]poji=");
  Serial.println(Train[4]-1);
}
 */
 //S 
  Serial.print("TrainOutFlag1_2_3_4_S==");
  Serial.print(TrainOutFlag1S);
  Serial.print(TrainOutFlag2S);
  Serial.print(TrainOutFlag3S);
  Serial.println(TrainOutFlag4S);
  //M
  Serial.print("TrainOutFlag1_2_3_4_M==");
  Serial.print(TrainOutFlag1M);
  Serial.print(TrainOutFlag2M);
  Serial.print(TrainOutFlag3M);
  Serial.println(TrainOutFlag4M);

  
  
 
  //列車位置 TrainPo
 // Serial.print("TrainPo[1][2][3][4]==");
 // Serial.print(sTrainPo[1][2][3][4]=");
  
 // 列車位置確認モニター（テスト用）
  Serial.print("Train[1][2][3][4]=");
  Serial.print(Train[1]);
  Serial.print(" ");
  Serial.print(Train[2]);
  Serial.print(" ");
  Serial.print(Train[3]);
  Serial.print(" ");
  Serial.println(Train[4]);
  
  /*
  
  Serial.print("mTrainPo[1-13]==");
  Serial.print(mTrainPo[1]);
  Serial.print(mTrainPo[2]);
  Serial.print(mTrainPo[3]);
  Serial.print(mTrainPo[4]);
  Serial.print(mTrainPo[5]);
  Serial.print(mTrainPo[6]);
  Serial.print(mTrainPo[7]);
  Serial.print(mTrainPo[8]);
  Serial.print(mTrainPo[9]);
  Serial.print(mTrainPo[10]);
  Serial.print(mTrainPo[11]);
  Serial.print(mTrainPo[12]);
  Serial.println(mTrainPo[13]);

  Serial.print("sTrainPo[1-13]==");
  Serial.print(sTrainPo[1]);
  Serial.print(sTrainPo[2]);
  Serial.print(sTrainPo[3]);
  Serial.print(sTrainPo[4]);
  Serial.print(sTrainPo[5]);
  Serial.print(sTrainPo[6]);
  Serial.print(sTrainPo[7]);
  Serial.print(sTrainPo[8]);
  Serial.print(sTrainPo[9]);
  Serial.print(sTrainPo[10]);
  Serial.print(sTrainPo[11]);
  Serial.print(sTrainPo[12]);
  Serial.println(sTrainPo[13]);
 */ 
// ポイント切替の必要時サーボ制御へ
  //if(e_old != e){
   // ServoCont();
   // }
  //digitalMain
  m[1] = digitalRead(digitalMainPin[1]);// Main後部20
  m[2] = digitalRead(digitalMainPin[2]);// Main先頭22
 

  if(m[1] == 1){mTrainPo[1]=1;}
  if(m[2] == 1){mTrainPo[2]=1;}
 

  if(m[1] == 0){mTrainPo[1]=0;}
  if(m[2] == 0){mTrainPo[2]=0;}
  //digitalOut
  out[1] = digitalRead(digitalOutPin[1]);
  out[2] = digitalRead(digitalOutPin[2]);
  out[3] = digitalRead(digitalOutPin[3]);
  out[4] = digitalRead(digitalOutPin[4]);
  out[5] = digitalRead(digitalOutPin[5]);
  out[6] = digitalRead(digitalOutPin[6]);
  out[7] = digitalRead(digitalOutPin[7]);
  out[8] = digitalRead(digitalOutPin[8]);
  out[9] = digitalRead(digitalOutPin[9]);
  out[10] = digitalRead(digitalOutPin[10]);
  out[11] = digitalRead(digitalOutPin[11]);
 
  if(out[1] == 1){outTrainPo[1]=1;}
  if(out[2] == 1){outTrainPo[2]=1;}
  if(out[3] == 1){outTrainPo[3]=1;}
  if(out[4] == 1){outTrainPo[4]=1;}
  if(out[5] == 1){outTrainPo[5]=1;}
  if(out[6] == 1){outTrainPo[6]=1;}
  if(out[7] == 1){outTrainPo[7]=1;}
  if(out[8] == 1){outTrainPo[8]=1;}
  if(out[9] == 1){outTrainPo[9]=1;}
  if(out[10] == 1){outTrainPo[10]=1;}
  if(out[11] == 1){outTrainPo[11]=1;}
  
  if(out[1] == 0){outTrainPo[1]=0;}
  if(out[2] == 0){outTrainPo[2]=0;}
  if(out[3] == 0){outTrainPo[3]=0;}
  if(out[4] == 0){outTrainPo[4]=0;}
  if(out[5] == 0){outTrainPo[5]=0;}
  if(out[6] == 0){outTrainPo[6]=0;}
  if(out[7] == 0){outTrainPo[7]=0;}
  if(out[8] == 0){outTrainPo[8]=0;}
  if(out[9] == 0){outTrainPo[9]=0;}
  if(out[10] == 0){outTrainPo[10]=0;}
  if(out[11] == 0){outTrainPo[11]=0;}
   
  //digitalSub
  s[1] = digitalRead(digitalSubPin[1]);// Sub後部22
  s[2] = digitalRead(digitalSubPin[2]);// Sub先頭23
 
  
  Serial.print("s[2]s[1]==");
  Serial.print(s[2]);
  Serial.println(s[1]);

  Serial.print("out[1-11]==");
  Serial.print(out[1]);
  Serial.print(out[2]);
  Serial.print(out[3]);
  Serial.print(out[4]);
  Serial.print(out[5]);
  Serial.print(out[6]);
  Serial.print(out[7]);
  Serial.print(out[8]);
  Serial.print(out[9]);
  Serial.print(out[10]);
  Serial.println(out[11]);
     
  Serial.print("m[2]m[1]==");
  Serial.print(m[2]);
  Serial.println(m[1]);
   
  if(s[1] == 1){sTrainPo[1]=1;}
  if(s[2] == 1){sTrainPo[2]=1;}
 
  
  if(s[1] == 0){sTrainPo[1]=0;}
  if(s[2] == 0){sTrainPo[2]=0;}
 

// ホーム内位置情報変数クリアー
/*
a = 0;
b = 0;
c = 0;
d = 0;
e_old = e;
e = 0;*/

//列車衝突防止
if(Train[1]>3 && Train[1]<14 && Train[2]>3 && Train[2]<14){
//collision();
}
// ポイント切替状態表示信号カラーLED制御
HomeSignal();
// 列車移動引き継ぎ情報
HomePoji();
//列車発射信号
Outflag1234(); 
// 列車追跡
Outside_TrainSerch();

//TrainPP();
//Train1Main();
//motor_reset();
   //Train1Fast(); 
    delay(1);
  delay(200);
}

//loop 終わり 
//void digitalMainPin[15](){
 // if(digitalMainPin[1]==1){
    
//  }
//}



//motor reset
//void motor_reset(){
//  digitalWrite(38,  LOW);
 // digitalWrite(39,  LOW);
//  digitalWrite(40,  LOW);
//  digitalWrite(41,  LOW);  
//}

//collision

void collision(){
  //Train[1]-------------------------------
  if(Train[1]-2<Train[1]-1 && Train[1]-2>Train[1]-3){
 
  if(outTrainPo[Train[1]-2]==1 && outTrainPo[Train[1]-3]==1 ){
  Train1Stop();
  Serial.println("482collision_Train1Stop();");
  }
  if( outTrainPo[Train[1]-1]==1 && outTrainPo[Train[1]-2]==0 && outTrainPo[Train[1]-3]==1 ){
  Train1Slow();
  TrainOutFlag1S = 1;
  TrainOutFlag1M = 1;
  Serial.println("488collision_Train1Slow();");
  }
  if( outTrainPo[Train[1]-1]==0 && outTrainPo[Train[1]-2]==0 && outTrainPo[Train[1]-3]==1 ){
  Train1Fast();
  Serial.println("492collision_Train1Fast();");
  }
  
}

//Train[2]4
if(Train[2]<Train[2]+1 && Train[2]>Train[2]-1){
 
  if(outTrainPo[Train[2]]==1 && outTrainPo[Train[2]-1]==1 ){
  Train2Stop();
  Serial.println("502Train2Stop();");
  }
  if( outTrainPo[Train[2]+1]==1 && outTrainPo[Train[2]]==0 && outTrainPo[Train[2]-1]==1 ){
  Train2Slow();
  TrainOutFlag2S = 1;
  TrainOutFlag2M = 1;
  Serial.println("508Train2Slow();");
  }
  if( outTrainPo[Train[2]+1]==0 && outTrainPo[Train[2]]==0 && outTrainPo[Train[2]-1]==1 ){
  Train2Fast();
  Serial.println("512Train2Fast();");
  }
  
}
//Train[3]
if(Train[3]<Train[3]+1 && Train[3]>Train[3]-1){
 
  if(outTrainPo[Train[3]]==1 && outTrainPo[Train[3]-1]==1 ){
  Train3Stop();
  Serial.println("521Train3Stop();");
  }
  if( outTrainPo[Train[3]+1]==1 && outTrainPo[Train[3]]==0 && outTrainPo[Train[3]-1]==1 ){
  Train3Slow();
  TrainOutFlag3S = 1;
  TrainOutFlag3M = 1;
  Serial.println("527Train3Slow();");
  }
  if( outTrainPo[Train[3]+1]==0 && outTrainPo[Train[3]]==0 && outTrainPo[Train[3]-1]==1 ){
  Train3Fast();
  Serial.println("531Train3Fast();");
  }
  
}

  
  //Train[4]-------------------------
 if(Train[4]<Train[4]+1 && Train[4]>Train[4]-1){
 
  if(outTrainPo[Train[4]]==1 && outTrainPo[Train[4]-1]==1 ){
  Train4Stop();
  Serial.println("542Train4Stop();");
  }
  if( outTrainPo[Train[4]+1]==1 && outTrainPo[Train[4]]==0 && outTrainPo[Train[4]-1]==1 ){
  Train4Slow();
  TrainOutFlag4S = 1;
  TrainOutFlag4M = 1;
  Serial.println("548Train4Slow();");
  }
  if( outTrainPo[Train[4]+1]==0 && outTrainPo[Train[4]]==0 && outTrainPo[Train[4]-1]==1 ){
  Train4Fast();
  Serial.println("552Train4Fast();");
  }
  
}
}

// シリアル通信でパソコンから入力された文字を変数に保存する
void Outflag1234(){
  
  int sw = Serial.read();
  
  if (sw == '1') {
   TrainOutFlag1S = 1; 
   Train1Slow();
   TrainInFlag1 = 0; 
   
   
   
   
   //Train1Slow();
   Serial.println("654TrainOutFlag1111111=");
   //Serial.println(TrainOutFlag1);
  }
  if(digitalRead(35)==0 && digitalRead(36)==0 && digitalRead(37)==1){
    TrainOutFlag1S = 1; 
    TrainInFlag1 ==0;
    Train1Slow();
    
    //Train1Slow();
   //Serial.println("Train[1]start");
  
  }
  if (sw == 'A') {
   TrainOutFlag1M=1; 
   
   Train1Slow();
   //TrainOutFlag2=0; 
   //Serial.print("TrainOutFlag22222222=");
  
  }
  if(digitalRead(35)==0 && digitalRead(36)==1 && digitalRead(37)==0){
    //Serial.println(digitalRead(35));
    //Serial.println(digitalRead(36));
    //Serial.println(digitalRead(37));
    TrainOutFlag1M=1; 
    Train1Slow();
   //Serial.println("Train[2]start");
  }

  
  if (sw == '2') {
   TrainOutFlag2S = 1; 
   //TrainInFlag3 = 0; 
   Train2Slow();
   //Serial.print("TrainOutFlag333333=");
   //Serial.println(TrainOutFlag3);
  }
  if(digitalRead(35)==0 && digitalRead(36)==1 && digitalRead(37)==1){
    TrainOutFlag2S = 1;
    //TrainInFlag3 = 0;  
    Train2Slow();
   //Serial.println("Train[3]start");
  }
  if (sw == 'B') {
   TrainOutFlag2M = 1; 
    Train2Slow();
  
}
  if(digitalRead(35)==1 && digitalRead(36)==0 && digitalRead(37)==0){
    TrainOutFlag2M = 1; 
     Train2Slow();
  // Serial.println("Train[4]start");
}
if(digitalRead(35)==1 && digitalRead(36)==0 && digitalRead(37)==1){
   software_reset(); 
   Train1Stop();
}

  
  if (sw == '3') {
   TrainOutFlag3S = 1; 
  
   //TrainInFlag1 = 0; 
    Train3Slow();
   
   
   
   //Train1Slow();
   //Serial.print("TrainOutFlag1111111=");
   //Serial.println(TrainOutFlag1);
  }
  if(digitalRead(35)==0 && digitalRead(36)==0 && digitalRead(37)==1){
    TrainOutFlag3S = 1; 
    //TrainInFlag1 ==0;
    Train3Slow();
    
    //Train1Slow();
   //Serial.println("Train[1]start");
  
  }
  if (sw == 'C') {
   TrainOutFlag3M=1; 
   
   Train3Slow();
   //TrainOutFlag2=0; 
   //Serial.print("TrainOutFlag22222222=");
  
  }
  if(digitalRead(35)==0 && digitalRead(36)==1 && digitalRead(37)==0){
    //Serial.println(digitalRead(35));
    //Serial.println(digitalRead(36));
    //Serial.println(digitalRead(37));
    TrainOutFlag3M=1; 
    Train3Slow();
   //Serial.println("Train[2]start");
  }

  
  if (sw == '4') {
   TrainOutFlag4S = 1; 
   //TrainInFlag3 = 0; 
   Train4Slow();
   //Serial.print("TrainOutFlag333333=");
   //Serial.println(TrainOutFlag3);
  }
  if(digitalRead(35)==0 && digitalRead(36)==1 && digitalRead(37)==1){
    TrainOutFlag4S = 1;
    //TrainInFlag3 = 0;  
    Train4Slow();
   //Serial.println("Train[3]start");
  }
  if (sw == 'D') {
   TrainOutFlag4M = 1; 
  
}
  if(digitalRead(35)==1 && digitalRead(36)==0 && digitalRead(37)==0){
    TrainOutFlag4M = 1; 
  // Serial.println("Train[4]start");
}
  if(sw=='5'){
    software_reset(); 
  }
if(digitalRead(35)==1 && digitalRead(36)==0 && digitalRead(37)==1){
   software_reset(); 
   
}
  if(sw=='6'){
   Train123StopF(); 
  }
}


void software_reset() {
 // digitalWrite(13, LOW);
   asm volatile ("  jmp 0");  
} 


void HomePoji(){    // HomePojiからOutPojiへ引継ぎ

//[23]----------------------------------------
     if(Train[1] > 0 && Train[1] < 5 ){
      if(TrainOutFlag1S==1  ){
     Train1Sub_Home(); //2~5
     Serial.println("716Train1Sub_Home()");
      }
      if(TrainOutFlag1M==1){ 
     Train1Main_Home();
     Serial.println("800Train1Main_Home()");
     }
     }
  //[22]======================================
   if(Train[2] > 0 && Train[2] < 5 ){ //1-4
    if(TrainOutFlag2M==1){
    Train2Main_Home();
    Serial.println("738Train2Main_Home()");
    }
    if(TrainOutFlag2S==1){
    Train2Sub_Home();//2~5
    Serial.println("813Train2Sub_Home()");
    }
   }
}
  //[21]======================================
 
  //[20]==========================================
 

  //============================================
//[23]
void  Train1Sub_Home(){
    Serial.println("825Train1Sub_Home()");
   if(TrainOutFlag1S==1 && digitalRead(digitalSubPin[Train[1]])==1){ 
    Train[1]=Train[1]+1;
    TrainOutFlag1M = 0;
    TrainOutFlag1S = 0;
    Serial.println("748Train1Sub_Home[2]()");
   //if(s[2]==1 ){
   //  Train[1]=2;
   //  TrainOutFlag1S = 0;
   //  Train1Stop();
   //  Serial.println("835Train1Stop();"); 
  //  }
   }
    TrainOutFlag1S = 1;   
        
   
    /*
   if(TrainOutFlag1S ==0  && digitalRead(digitalSubPin[Train[1]])==1){
      TrainOutFlag1S = 0;
     Train1Stop();
     Serial.println("774Train1Stop();"); 
    }
    if(TrainOutFlag1S ==0  && digitalRead(digitalSubPin[Train[1]])==0){
      TrainOutFlag1S = 0;
      Train1Slow();
      Serial.println("849Train1Slow();"); 
    } 
    //if(TrainOutFlag1S ==0  && digitalRead(digitalMainPin[Train[1]])==1){
    //  TrainOutFlag1S = 0;
   // }   
   */
    if(Train[1]==1 && TrainOutFlag1S ==1 ){
      Train[1]=2;
      Train1Stop();
      Serial.print("777Train1Stop();");
      TrainOutFlag1S = 0;
      //Serial.println("788Train1Stop();");
      } 
    if(Train[1]==3 && s[2]==1){
       Train1Stop();
      Serial.print("783Train1Stop();");
       TrainOutFlag1S = 0;
    }
      
      Serial.print("T1S===Train[1]");
      Serial.println(Train[1]);
  }

  void  Train1Main_Home(){
    Serial.println("792Train1Main_Home()");
   if(TrainOutFlag1M==1 && digitalRead(digitalMainPin[Train[1]])==1){
    Train[1] = Train[1] + 1;
    TrainOutFlag1M = 0;
    //TrainOutFlag1S = 0;
   }
    TrainOutFlag1M = 1;
 // if(Train[1]==14 && TrainOutFlag1S == 1 ){
     //Train[1]=1;
  //   TrainOutFlag1S = 0;
  //   Train1Stop();
  //   Serial.println("823Train1Stop();"); 
 //   }
     if(Train[1]==1 && TrainOutFlag1M ==1  ){
      Train[1]=2;
      Train1Stop();
      Serial.println("808Train1Stop();"); 
      TrainOutFlag1M = 0;
     }
     if(Train[1]==3 && m[2]==1 ){
      Train1Stop();
      Serial.println("813Train1Stop();"); 
     TrainOutFlag1M = 0;
    }
       
      Serial.print("T1M===Train[1]");
      Serial.println(Train[1]);
  }
//[22]
  void  Train2Sub_Home(){
      Serial.println("Train2Sub_Home()");
   if(TrainOutFlag2S==1 && digitalRead(digitalSubPin[Train[2]])==1){
    Train[2] = Train[2] + 1;
    TrainOutFlag2S = 0;
  


     if(s[1]==1 ){
     Train[2]=2;
     TrainOutFlag2S = 1;
     Train2Stop();
     Serial.println("834Train2Stop();"); 
    }
     if(Train[2]==3 ){
     TrainOutFlag2S = 0;
     Train2Stop();
     Serial.println("838Train2Stop();"); 
    }
   }
   
   /*  
     if(TrainOutFlag2S ==0  && digitalRead(digitalSubPin[Train[2]])==1){
      TrainOutFlag2S = 1;//Home
      Train2Stop();
      Serial.println("887Train2Stop();"); 
   }  
    
     if(TrainOutFlag2S ==0  && digitalRead(digitalSubPin[Train[2]])==0){
      TrainOutFlag2S =1;
       Train2Slow();
       Serial.println("893Train2Slow();"); 
    } 
  */  
    
  
    //TrainOutFlag1S = 1;
    Serial.print("T2S===");
    Serial.println(Train[2]);
  }
   void Train2Main_Home(){
    Serial.println("T882rain2Main_Home()");
   if(TrainOutFlag2M==1 && digitalRead(digitalMainPin[Train[2]])==1){
    Train[2] = Train[2] + 1;
    TrainOutFlag2M =0;
    Serial.println("885Train2Main_Home()");
   }
   TrainOutFlag2M =1;
 /*  
    if(TrainOutFlag2M ==0  && digitalRead(digitalMainPin[Train[2]])==1){
      TrainOutFlag2M = 1;
      Train2Stop();
      Serial.println("892Train2Stop();"); 
       }
    if(TrainOutFlag2M ==1  && digitalRead(digitalMainPin[Train[2]])==0){
      TrainOutFlag2M = 1;
      Train2Slow();
       Serial.println("897Train2Slow();"); 
       }
  */
      if(Train[2]==1 && TrainOutFlag2M ==1 ){
      Train[2]=2;
      Train2Stop();
      Serial.println("910Train2Stop();");
       TrainOutFlag2M = 0;
      } 
       if(Train[2]==3 && m[2]==1){
       Train2Stop();
      Serial.print("915Train2Stop();");
       TrainOutFlag2M = 0;
    }
      
    Serial.print("T2M===");
    Serial.println(Train[2]);
  }

 
  
  
  
  

//hint
// 外周列車追跡   /////////////////////////////////
void Outside_TrainSerch(){
//[23]---------------------------------------
if(Train[1] > 3 && Train[1] < 14   ){ //3-15
    if(TrainOutFlag1S==1  ){
   Train1Out();//5+6
   Serial.println("918Train1Out()");
  }
  if(TrainOutFlag1M==1){
   Train1Main();
   Serial.println("916Train1Main()");
}
}  
  
  
//--------------------------------------------
//[22]-----------------------------------------
if(Train[2] > 3 && Train[2] < 14 ){ //3-11
  if(TrainOutFlag2S==1){
   Train2Out();//5+6
   Serial.println("936Train2Sub()");
  }
   if(TrainOutFlag2M==1){
   Train2Main();
   Serial.println("932Train2Main()");
}
}    
  
//[21]=========================================
  if(Train[3] > 0 && Train[3] < 14){
   if(TrainOutFlag3S==1){
     Train3Sub();
     if(s[13]==1){
      TrainOutFlag3S=0;
     }
     //TrainOutFlag1S=1;
     //TrainOutFlag1M=0;
   }
   if(TrainOutFlag3M==1){
     Train3Main();
     if(m[13]==1){
      TrainOutFlag3M=0;
     }
     //TrainOutFlag1M=1;
     //TrainOutFlag1S=0;
   }
   // Train1Main();
   
  }

    
  

//==============================================
//[20]-------------------------------------------
   if(Train[4] > 0 && Train[4] < 14){
   if(TrainOutFlag4S==1){
     Train4Sub();
     if(s[13]==1){
      TrainOutFlag4S=0;
     }
     //TrainOutFlag1S=1;
     //TrainOutFlag1M=0;
   }
   if(TrainOutFlag4M==1){
     Train4Main();
     if(m[13]==1){
      TrainOutFlag4M=0;
     }
     //TrainOutFlag1M=1;
     //TrainOutFlag1S=0;
   }
   // Train1Main();
   
  }
}
    
  
// 列車位置加算ルーチン
//[23]----------------------------------------------
 void Train1Out(){
    Serial.println("993Train1Out()");
   if(TrainOutFlag1S==1 && digitalRead(digitalOutPin[Train[1]-2])==1){
    Train[1] = Train[1] + 1;
    TrainOutFlag1S = 0;
   }
   TrainOutFlag1S = 1;
   //collision
    if(TrainOutFlag1S ==1  && digitalRead(digitalOutPin[Train[1]-2])==1){
      TrainOutFlag1S = 0;
      Train1Stop();//追いついたらstop
      Serial.print(" 1001Train1Stop();//追いついたらstop");
    }
     if(TrainOutFlag1S ==0  && digitalRead(digitalOutPin[Train[1]-2])==0){
      TrainOutFlag1S = 1;//s[5]からTrainOutFlag1S = 1  OK
      Train1Slow();
      Serial.print(" 996Train1Slow");
    }
    
    //coliision
    if(Train[1]==14 && TrainOutFlag1S ==1){
      Train1Stop();
       Serial.print(" 1010Train1Stop();");
      TrainOutFlag1S =0;
      Train[1]=1;
    }
      
      
    Serial.print("T1S===");
    Serial.println(Train[1]);
  }
 
  
  
  void Train1Main(){//out
    TrainOutFlag1M = 1;
    Serial.println("1019Train1Main()");
   if(TrainOutFlag1M==1 && digitalRead(digitalOutPin[Train[1]-2])==1){
    Train[1] = Train[1] + 1;
    TrainOutFlag1M = 0;
   }
   TrainOutFlag1M = 1;
    if(Train[1]==14 && TrainOutFlag1M ==1){
      Train1Stop();
      Serial.print(" 1027Train1Stop();");
      TrainOutFlag1M =0;
      Train[1]=1;
    }
  
    
    
    Serial.print("T1M===");
    Serial.println(Train[1]);
  }

   
  //[22]-------------------------------------------------
   void Train2Out(){ //前列車にマージしない OUT
    Serial.println("Train2Out()");
   if(TrainOutFlag2S==1 && digitalRead(digitalOutPin[Train[2]-2])==1){
    Train[2] = Train[2] + 1;
    TrainOutFlag2S = 0;
   }
   TrainOutFlag2S = 1;
   // if(TrainOutFlag2S ==0  && digitalRead(digitalSubPin[Train[2]])==0){
   //   TrainOutFlag2S = 1;//stop_front-train_start_outflag2=1
   //   Train2Slow();
   // }
   // if(TrainOutFlag2S ==0  && digitalRead(digitalSubPin[Train[2]])==1){
   //   TrainOutFlag2S = 0;
   //   Train2Stop();
   // } 
    //if(TrainOutFlag2S ==0  && digitalRead(digitalMainPin[Train[2]])==1){
   //   TrainOutFlag2S = 0;
   // }   
   
    if(Train[2]==14 && TrainOutFlag2S==1 ){
      Train2Stop();
      Serial.print("1073Train2Stop();");
      TrainOutFlag2S = 0;
      Train[2]=1;
    } 
       
    Serial.print("T2S===");
    Serial.println(Train[2]);
  }
  //
  void Train2Main(){  //OUT
    // TrainOutFlag2M = 1;
      Serial.println("1074Train2Main()");
   if(TrainOutFlag2M==1 && digitalRead(digitalOutPin[Train[2]-2])==1){
    Train[2] = Train[2] + 1;
    TrainOutFlag2M = 0;
    }
     TrainOutFlag2M = 1;
     //collision
       if(TrainOutFlag2M ==1  && digitalRead(digitalOutPin[Train[1]-2])==1){
      TrainOutFlag2M = 0;
      Train2Stop();//追いついたらstop
      Serial.print(" 1083Train2Stop();//追いついたらstop");
     
     if(TrainOutFlag2M ==0  && digitalRead(digitalOutPin[Train[1]-2])==0){
      TrainOutFlag2M = 1;//s[5]からTrainOutFlag1S = 1  OK
      Train2Slow();
      Serial.print("1088Train2Slow");
    }
    }
    //collision

     
     if(Train[2]==14 && TrainOutFlag2M ==1){
      Train2Stop();
      Serial.print(" 1087Train2Stop();");
      TrainOutFlag2M =0;
      Train[2]=1;
    }
   
       
    Serial.print("T2M===");
    Serial.println(Train[2]);
  }

 
  
  //[21]==============================================
    void Train3Sub(){ //OUT
    Serial.println("Train3Sub()");
   if(TrainOutFlag3S==1 && digitalRead(digitalSubPin[Train[3]])==1){
    Train[3] = Train[3] + 1;
    TrainOutFlag3S = 0;
    if(Train[3]==14 ){
     Train[3]=1;
     TrainOutFlag3S = 0;
      
   }
    if(TrainOutFlag3S ==0  && digitalRead(digitalSubPin[Train[3]])==0){
      TrainOutFlag3S = 1;
    }  
    if(TrainOutFlag3S ==0  && digitalRead(digitalSubPin[Train[3]])==1){
      TrainOutFlag3S = 0;
    }
    if(TrainOutFlag3S ==0  && digitalRead(digitalMainPin[Train[3]])==1){
      TrainOutFlag3S = 0;
    } 
    if(Train[3]==4 ){
      TrainOutFlag3S = 0;
    } 
    }
    Serial.print("T3S===");
    Serial.println(Train[3]);
  }
  void Train3Main(){
   if(TrainOutFlag3M==1 && digitalRead(digitalMainPin[Train[3]])==1){
    Train[3] = Train[3] + 1;
    TrainOutFlag3M = 0;
    if(Train[3]==14 ){
     Train[3]=1;
     TrainOutFlag3M = 0;
      
   }
    if(TrainOutFlag3M ==0  && digitalRead(digitalMainPin[Train[3]])==0){
      TrainOutFlag3M = 1;
       }
    if(TrainOutFlag3M ==0  && digitalRead(digitalMainPin[Train[3]])==1){
      TrainOutFlag3M = 0;
       }
    if(TrainOutFlag3M ==0  && digitalRead(digitalSubPin[Train[3]])==1){
      TrainOutFlag3M = 0;
    }
    if(Train[3]==4 ){
      TrainOutFlag3M = 0;
    } 
    }
    Serial.print("T3M===");
    Serial.println(Train[3]);
  }

  
  
  //==================================================
  //[20]==============================================
    void Train4Sub(){
    Serial.println("Train4Sub()");
   if(TrainOutFlag4S==1 && digitalRead(digitalSubPin[Train[4]])==1){
    Train[4] = Train[4] + 1;
    TrainOutFlag4S = 0;
    if(Train[4]==14 ){
     Train[4]=1;
     TrainOutFlag4S = 0;
      
   }
    if(TrainOutFlag4S ==0  && digitalRead(digitalSubPin[Train[4]])==0){
      TrainOutFlag4S = 1;
    }  
    if(TrainOutFlag4S ==0  && digitalRead(digitalSubPin[Train[4]])==1){
      TrainOutFlag4S = 0;
    } 
    if(TrainOutFlag4S ==0  && digitalRead(digitalMainPin[Train[4]])==1){
      TrainOutFlag4S = 0;
    }
    if(Train[4]==4 ){
      TrainOutFlag4S = 0;
    } 
    }
    Serial.print("T4S===");
    Serial.println(Train[4]);
  }
  void Train4Main(){
   if(TrainOutFlag4M==1 && digitalRead(digitalMainPin[Train[4]])==1){
    Train[4] = Train[4] + 1;
    TrainOutFlag4M = 0;
    if(Train[4]==14 ){
     Train[4]=1;
     TrainOutFlag4M = 0;
      
   }
    if(TrainOutFlag4M ==0  && digitalRead(digitalMainPin[Train[4]])==0){
      TrainOutFlag4M = 1;
    }
    if(TrainOutFlag4M ==0  && digitalRead(digitalMainPin[Train[4]])==1){
      TrainOutFlag4M = 0;
    }
    if(TrainOutFlag4M ==0  && digitalRead(digitalSubPin[Train[4]])==1){
      TrainOutFlag4M = 0;
    }
    if(Train[4]==4 ){
      TrainOutFlag4M = 0;
    } 
    }
    Serial.print("T4M===");
    Serial.println(Train[4]);
  }



// SERVO CONT //////////////////////////////////////////////////
// ポイント制御（ホーム内列車ポジションによりホーム入口ポイントを切替）
void ServoCont(){
  HomeIn.attach(2);
  delay(100);
  switch (e){
    case 0:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 1:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 2:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 3:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 4:
       HomeIn.write(posInPMain, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
       digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
       break;
    case 5:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 6:
       HomeIn.write(posInPMain, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
       digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
       break;
    case 7:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 8:
       HomeIn.write(posInPMain, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
       digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
       break;
    case 9:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 10:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 11:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    case 12:
       HomeIn.write(posInPMain, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
       digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
       break;
    case 13:
       HomeIn.write(posInPMain, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
       digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
       break;
    case 14:
       HomeIn.write(posInPMain, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[3], HIGH);// Pin45(MainGreen)
       digitalWrite(digitalPinSev[0], HIGH);// Pin42(SubRed)
       break;
    case 15:
       HomeIn.write(posInPSub, speedMax);
       digitalWrite(digitalPinSev[1], LOW); // Off
       digitalWrite(digitalPinSev[2], LOW); // Off
       digitalWrite(digitalPinSev[0], LOW); // Off
       digitalWrite(digitalPinSev[3], LOW); // Off
       digitalWrite(digitalPinSev[1], HIGH);// Pin43(SubGreen)
       digitalWrite(digitalPinSev[2], HIGH);// Pin44(MainRed)
       break;
    default:
       break;
  }
  delay(100);
  HomeIn.detach();
}

void Train1Slow(){
   digitalWrite(38,  HIGH);//3
   digitalWrite(39,  HIGH);//Train[1] Slow
   digitalWrite(40,  LOW);
   digitalWrite(41,  LOW); 
  // Serial.println("Train11111Slow");
}
void Train2Slow(){
   digitalWrite(38,  LOW);//6
   digitalWrite(39,  HIGH);//Train[2] Slow
   digitalWrite(40,  HIGH);
   digitalWrite(41,  LOW); 
   //Serial.println("Train222222Slow");
}
void Train3Slow(){
   digitalWrite(38,  HIGH);//9
   digitalWrite(39,  LOW);//Train[3] Slow
   digitalWrite(40,  LOW);
   digitalWrite(41,  HIGH); 
}
void Train4Slow(){
   digitalWrite(38,  LOW);//12
   digitalWrite(39,  LOW);//Train[4] Slow
   digitalWrite(40,  HIGH);
   digitalWrite(41,  HIGH); 
}
void Train1Stop(){
   digitalWrite(38,  HIGH);//5
   digitalWrite(39,  LOW);//Train[1] Stop
   digitalWrite(40,  HIGH);
   digitalWrite(41,  LOW); 
   Serial.println("1483Train11111Stop");
}
void Train2Stop(){
   digitalWrite(38,  LOW);//8
   digitalWrite(39,  LOW);//Train[2] Stop
   digitalWrite(40,  LOW);
   digitalWrite(41,  HIGH); 
   //Serial.println("Train22222Stop");
}
void Train123StopF(){
   digitalWrite(38,  LOW);//2
   digitalWrite(39,  HIGH);//Train[2] StopF
   digitalWrite(40,  LOW);
   digitalWrite(41,  LOW); 
   Serial.println("Train123123Stop");
}
void Train3Stop(){
   digitalWrite(38,  HIGH);//11
   digitalWrite(39,  HIGH);//Train[3] Stop
   digitalWrite(40,  LOW);
   digitalWrite(41,  HIGH); 
}
void Train4Stop(){
   digitalWrite(38,  LOW);//14
   digitalWrite(39,  HIGH);//Train[4] Stop
   digitalWrite(40,  HIGH);
   digitalWrite(41,  HIGH); 
}

void Train1Fast(){
   digitalWrite(38,  LOW);//4
   digitalWrite(39,  LOW);//Train[1] Fast
   digitalWrite(40,  HIGH);
   digitalWrite(41,  LOW); 
}
void Train2Fast(){                                                                                                    
   digitalWrite(38,  HIGH);//7
   digitalWrite(39,  HIGH);//Train[2] Fast
   digitalWrite(40,  HIGH);
   digitalWrite(41,  LOW); 
}
void Train3Fast(){
   digitalWrite(38,  LOW);//10
   digitalWrite(39,  HIGH);//Train[3] Fast
   digitalWrite(40,  LOW);
   digitalWrite(41,  HIGH); 
}
void Train4Fast(){
   digitalWrite(38,  HIGH);  //13
   digitalWrite(39,  LOW);  //Train[4] Fast
   digitalWrite(40,  HIGH);
   digitalWrite(41,  HIGH); 
}


// HOMESIGNAL /////////////////////////////////////////////////
// HomePoint 赤LED制御（列車位置確認）

void HomeSignal(){
  int Buf15 = digitalRead(digitalMainPin[1]);
  
  if(Buf15 == HIGH){
    digitalWrite(digitalPinPS[0], HIGH);
  }else{
    digitalWrite(digitalPinPS[0], LOW);
  }
  
  int Buf14 = digitalRead(digitalMainPin[2]);
  if(Buf14 == HIGH){
    digitalWrite(digitalPinPS[2], HIGH);
  }else{
    digitalWrite(digitalPinPS[2], LOW);
  }
  
  int Buf13 = digitalRead(digitalSubPin[1]);
  if(Buf13 == HIGH){
    digitalWrite(digitalPinPS[1], HIGH);
  }else{
    digitalWrite(digitalPinPS[1], LOW);
  }

  int Buf12 = digitalRead(digitalSubPin[2]);
  if(Buf12 == HIGH){ //52-23
    digitalWrite(digitalPinPS[3], HIGH);
  }else{
    digitalWrite(digitalPinPS[3], LOW);
  }
}


  
