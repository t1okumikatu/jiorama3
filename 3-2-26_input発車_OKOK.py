# -*- coding: utf-8 -*-
# etnryboard.pyを読み込む
#from entryboard import *
import time
import RPi.GPIO as GPIO


sw35=20
sw36=27 #SCLK 22-->27
sw37=16 #MISO 21-->16
a=0

 
# 動作モード設定
GPIO.setmode(GPIO.BCM)
# GPIOモード設定
GPIO.setup(sw35, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(sw36, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(sw37, GPIO.OUT, initial=GPIO.LOW)
# LED1に1を出力
# press(キー)
GPIO.cleanup(sw35)
GPIO.cleanup(sw36)
GPIO.cleanup(sw37)

while(1):
    
  number = input('数値を入力>>>')
  if number.isdecimal():
     number = int(number)
     print(number)
   
  if number==1:
     GPIO.setup(sw35, GPIO.OUT, initial=GPIO.LOW)
     GPIO.setup(sw36, GPIO.OUT, initial=GPIO.LOW)
     GPIO.setup(sw37, GPIO.OUT, initial=GPIO.LOW)
     GPIO.output(sw35, GPIO.LOW)
     GPIO.output(sw36, GPIO.LOW)
     GPIO.output(sw37, GPIO.HIGH)
     print(sw35,sw36,sw37)
     time.sleep(2.0)
     GPIO.cleanup(sw35)
     GPIO.cleanup(sw36)
     GPIO.cleanup(sw37)
     
   #time.sleep(2)
  if number==2:
     GPIO.setup(sw35, GPIO.OUT, initial=GPIO.LOW)
     GPIO.setup(sw36, GPIO.OUT, initial=GPIO.LOW)
     GPIO.setup(sw37, GPIO.OUT, initial=GPIO.LOW)
     GPIO.output(sw35, GPIO.LOW)
     GPIO.output(sw36, GPIO.HIGH)
     GPIO.output(sw37, GPIO.LOW)
     print(sw35,sw36,sw37)
     time.sleep(2)
     GPIO.cleanup(sw35)
     GPIO.cleanup(sw36)
     GPIO.cleanup(sw37)
     
   #time.sleep(2)
  if number==3:
     GPIO.setup(sw35, GPIO.OUT, initial=GPIO.LOW)
     GPIO.setup(sw36, GPIO.OUT, initial=GPIO.LOW)
     GPIO.setup(sw37, GPIO.OUT, initial=GPIO.LOW)
     GPIO.output(sw35, GPIO.LOW)
     GPIO.output(sw36, GPIO.HIGH)
     GPIO.output(sw37, GPIO.HIGH)
     print(sw35,sw36,sw37)
     time.sleep(2)
     GPIO.cleanup(sw35)
     GPIO.cleanup(sw36)
     GPIO.cleanup(sw37)
     
   #time.sleep(2)
  if number==4:
   GPIO.setup(sw35, GPIO.OUT, initial=GPIO.LOW)
   GPIO.setup(sw36, GPIO.OUT, initial=GPIO.LOW)
   GPIO.setup(sw37, GPIO.OUT, initial=GPIO.LOW)
   GPIO.output(sw35, GPIO.HIGH)
   GPIO.output(sw36, GPIO.LOW)
   GPIO.output(sw37, GPIO.LOW)
   print(sw35,sw36,sw37)
   time.sleep(2)
   GPIO.cleanup(sw35)
   GPIO.cleanup(sw36)
   GPIO.cleanup(sw37)
  if number==5:
   GPIO.setup(sw35, GPIO.OUT, initial=GPIO.LOW)
   GPIO.setup(sw36, GPIO.OUT, initial=GPIO.LOW)
   GPIO.setup(sw37, GPIO.OUT, initial=GPIO.LOW)
   GPIO.output(sw35, GPIO.HIGH)
   GPIO.output(sw36, GPIO.LOW)
   GPIO.output(sw37, GPIO.HIGH)
   print(sw35,sw36,sw37)
   time.sleep(2)
   GPIO.cleanup(sw35)
   GPIO.cleanup(sw36)
   GPIO.cleanup(sw37)
  if number==9:
     break
   
   #time.sleep(2)

   

# 2秒待つ
   #time.sleep(2)

# 終了処理

GPIO.cleanup(sw35)
GPIO.cleanup(sw36)
GPIO.cleanup(sw37)
