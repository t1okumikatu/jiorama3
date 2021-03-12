#!/usr/bin/env python3

import RPi.GPIO as GPIO # RPi.GPIOモジュールを使用
import struct, binascii, serial
import time
import threading
import queue
#import inputtrain

# ttyUSB0を開く
s = serial.Serial("/dev/ttyUSB0", 115200)#twelite
#ser=serial.Serial("/dev/ttyUSB1",9600,timeout=0)#mega
#3333ser = serial.Serial("/dev/ttyFixedB",9600,timeout=0)#mega
#ser=serial.Serial("/dev/ttyAMA0",9600,timeout=0)
#gpio_led = 20#19

# LEDとスイッチのGPIO番号
# デフォルトはRPZ-IR-Sensorの緑LEDと赤SW
# 必要に応じて変更
# コマンド0x80を送信する関数
#１号車

def sendTWELite1(s, sendto = 0x01,#gousya
        digital = [-1, -1, -1, -1],
        analog = [-1, -1, -1, -1]):
    # 先頭3バイト
    data = [sendto, 0x80, 0x01]
        
    # デジタル出力
    do = 0
    domask = 0
    for index, value in enumerate(digital):
        if value >= 0:
            domask |= 1 << index
            do |= (value & 1) << index
    data.append(do)
    data.append(domask)

    # アナログ出力
    for index, value in enumerate(analog):
        if value >= 0 and value <= 100:
            v = int(1024 * value / 100)
            data.append(v >> 8)
            data.append(v & 0xff)
        else:
            data.append(0xff)
            data.append(0xff)

    # チェックサムを計算する
    chksum = 0
    for val in data:
        chksum = (chksum + val) & 0xff
    data.append((0x100 - chksum) & 0xff)

    # 16進数文字列に変換する
    ss = struct.Struct("14B")
    outstring = str(binascii.hexlify(ss.pack(*data)), 'utf-8').upper()

    # TWE-Liteに送信する
    s.write(bytes(":" + outstring + "\r\n", 'utf-8'))
    return

def ContFFast1():
    # 前進方向
    sendTWELite1(s, digital=[0,0,1,0])
    time.sleep(0.1)
    # 前進低速
    sendTWELite1(s, analog=[0, -1, 70, -1])
    return

def ContFSlow1():
    # 前進方向
    sendTWELite1(s, digital=[0, 1, 0, 0])
    time.sleep(0.1)
    # 前進低速
    sendTWELite1(s, analog=[0, -1, 40, -1])
    return

def ContRSlow1():
    # 前進方向
    sendTWELite1(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite1(s, analog=[60, 0, -1, -1])
    return

def ContRFast1():
    # 前進方向
    sendTWELite1(s, digital=[0,-1,-1,-1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite1(s, analog=[0,80, -1, -1])
    return

def ContStop1():
    # 停止
    sendTWELite1(s, analog=[100, -1, 100, -1])
    time.sleep(0.1)
    # 消灯
    sendTWELite1(s, digital=[0,0,0, 1])
    return
def ContStop1F():
    # 停止
    sendTWELite1(s, analog=[0, -1, 0, -1])
    time.sleep(0.1)
    # 消灯
    sendTWELite1(s, digital=[0,1,1, 1])
    return
#2号車
def sendTWELite2(s, sendto = 0x02,
        digital = [-1, -1, -1, -1],
        analog = [-1, -1, -1, -1]):
    # 先頭3バイト
    data = [sendto, 0x80, 0x01]
        
    # デジタル出力
    do = 0
    domask = 0
    for index, value in enumerate(digital):
        if value >= 0:
            domask |= 1 << index
            do |= (value & 1) << index
    data.append(do)
    data.append(domask)

    # アナログ出力
    for index, value in enumerate(analog):
        if value >= 0 and value <= 100:
            v = int(1024 * value / 100)
            data.append(v >> 8)
            data.append(v & 0xff)
        else:
            data.append(0xff)
            data.append(0xff)

    # チェックサムを計算する
    chksum = 0
    for val in data:
        chksum = (chksum + val) & 0xff
    data.append((0x100 - chksum) & 0xff)

    # 16進数文字列に変換する
    ss = struct.Struct("14B")
    outstring = str(binascii.hexlify(ss.pack(*data)), 'utf-8').upper()

    # TWE-Liteに送信する
    s.write(bytes(":" + outstring + "\r\n", 'utf-8'))
    return

def ContFFast2():
    # 前進方向
    sendTWELite2(s, digital=[0, 0, 1, 0])
    time.sleep(0.2)
    # 前進低速
    sendTWELite2(s, analog=[0, -1, 70, -1])
    return

def ContFSlow2():
    # 前進方向
    sendTWELite2(s, digital=[0, 1, 0, 0])
    time.sleep(0.2)
    # 前進低速
    sendTWELite2(s, analog=[0, -1, 40, -1])
    return

def ContRSlow2():
    # 前進方向
    sendTWELite2(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite2(s, analog=[60, 0, -1, -1])
    return

def ContRFast2():
    # 前進方向
    sendTWELite2(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite2(s, analog=[80, 0, -1, -1])
    return

def ContStop2():
    # 停止
    sendTWELite2(s, analog=[100, -1, 100, -1])
    time.sleep(0.2)
    # 消灯
    sendTWELite2(s, digital=[0, 0, 0, 1])
    return
def ContStop123F():
    # 停止
    sendTWELite1(s, analog=[0, 0, 0, 0])
    time.sleep(0.2)
    # 消灯
    sendTWELite1(s, digital=[0, 1, 1, 1])
    sendTWELite2(s, analog=[0, 0, 0, 0])
    time.sleep(0.2)
    # 消灯
    sendTWELite2(s, digital=[0, 1, 1, 1])
    sendTWELite3(s, analog=[0, 0, 0, 0])
    time.sleep(0.2)
    # 消灯
    sendTWELite3(s, digital=[0, 1, 1, 1])
    return
#3号車
def sendTWELite3(s, sendto = 0x03,
        digital = [-1, -1, -1, -1],
        analog = [-1, -1, -1, -1]):
    # 先頭3バイト
    data = [sendto, 0x80, 0x01]
        
    # デジタル出力
    do = 0
    domask = 0
    for index, value in enumerate(digital):
        if value >= 0:
            domask |= 1 << index
            do |= (value & 1) << index
    data.append(do)
    data.append(domask)

    # アナログ出力
    for index, value in enumerate(analog):
        if value >= 0 and value <= 100:
            v = int(1024 * value / 100)
            data.append(v >> 8)
            data.append(v & 0xff)
        else:
            data.append(0xff)
            data.append(0xff)

    # チェックサムを計算する
    chksum = 0
    for val in data:
        chksum = (chksum + val) & 0xff
    data.append((0x100 - chksum) & 0xff)

    # 16進数文字列に変換する
    ss = struct.Struct("14B")
    outstring = str(binascii.hexlify(ss.pack(*data)), 'utf-8').upper()

    # TWE-Liteに送信する
    s.write(bytes(":" + outstring + "\r\n", 'utf-8'))
    return

def ContFFast3():
    # 前進方向
    sendTWELite3(s, digital=[1, 0, 1, 0])
    time.sleep(0.1)
    # 前進低速
    sendTWELite3(s, analog=[0, -1, 70, -1])
    return

def ContFSlow3():
    # 前進方向
    sendTWELite3(s, digital=[1, 1, 0, 0])
    time.sleep(0.1)
    # 前進低速
    sendTWELite3(s, analog=[0, -1, 40, -1])
    return

def ContRSlow3():
    # 前進方向
    sendTWELite3(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite3(s, analog=[40, -1, 0, -1])
    return

def ContRFast3():
    # 前進方向
    sendTWELite3(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite3(s, analog=[60, -1, 0, -1])
    return

def ContStop3():
    # 停止
    sendTWELite3(s, analog=[100, -1, 100, -1])
    time.sleep(0.1)
    # 消灯
    sendTWELite3(s, digital=[1, 0, 0, 1])
    return
#4号車
def sendTWELite4(s, sendto = 0x04,
        digital = [-1, -1, -1, -1],
        analog = [-1, -1, -1, -1]):
    # 先頭3バイト
    data = [sendto, 0x80, 0x01]
        
    # デジタル出力
    do = 0
    domask = 0
    for index, value in enumerate(digital):
        if value >= 0:
            domask |= 1 << index
            do |= (value & 1) << index
    data.append(do)
    data.append(domask)

    # アナログ出力
    for index, value in enumerate(analog):
        if value >= 0 and value <= 100:
            v = int(1024 * value / 100)
            data.append(v >> 8)
            data.append(v & 0xff)
        else:
            data.append(0xff)
            data.append(0xff)

    # チェックサムを計算する
    chksum = 0
    for val in data:
        chksum = (chksum + val) & 0xff
    data.append((0x100 - chksum) & 0xff)

    # 16進数文字列に変換する
    ss = struct.Struct("14B")
    outstring = str(binascii.hexlify(ss.pack(*data)), 'utf-8').upper()

    # TWE-Liteに送信する
    s.write(bytes(":" + outstring + "\r\n", 'utf-8'))
    return

def ContFFast4():
    # 前進方向
    sendTWELite4(s, digital=[1, 0, 1, 0])
    time.sleep(0.1)
    # 前進低速
    sendTWELite4(s, analog=[0, 80, -1, -1])
    return

def ContFSlow4():
    # 前進方向
    sendTWELite4(s, digital=[1, 0, 1, 0])
    time.sleep(0.1)
    # 前進低速
    sendTWELite4(s, analog=[0, 60, -1, -1])
    return

def ContRSlow4():
    # 前進方向
    sendTWELite4(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite4(s, analog=[60, 0, -1, -1])
    return

def ContRFast4():
    # 前進方向
    sendTWELite4(s, digital=[0, 1, 0, 1])
    time.sleep(0.1)
    # 前進低速
    sendTWELite4(s, analog=[80, 0, -1, -1])
    return

def ContStop4():
    # 停止
    sendTWELite4(s, analog=[100, 100, -1, -1])
    time.sleep(0.1)
    # 消灯
    sendTWELite4(s, digital=[1, 1, 1, 1])
    return



gpio_sw41 = 9  #RXD 10--->>9
gpio_sw40 = 11  
gpio_sw39 = 13
gpio_sw38 = 18


while True:
    a=0
    b=0
    c=0
    d=0
    e=0

# GPIO番号指定の準備
    
    GPIO.setmode(GPIO.BCM)
# LEDピンを出力に設定
    
# スイッチピンを入力、プルアップに設定
    GPIO.setup(gpio_sw38, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(gpio_sw39, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(gpio_sw40, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(gpio_sw41, GPIO.IN, pull_up_down=GPIO.PUD_UP)
# 出力

    

# スイッチの状態を取得
    sw38 = GPIO.input(gpio_sw38)
    sw39 = GPIO.input(gpio_sw39)
    sw40 = GPIO.input(gpio_sw40)
    sw41 = GPIO.input(gpio_sw41)
    
    #sw36 = GPIO.output(gpio_sw36)
    #sw37 = GPIO.output(gpio_sw37)

# スイッチが押されていた場合(ON)
#while True:
    #s.write(bytes(":0080010F0F0000000000000000E9\r\n",'utf-8'))
    #time.sleep(1)
    #GPIO.output(sw36,0)
    #ContFSlow2()
    #ContFFast2()
    #ContStop2F()
    #ContStop2()
    #ContFSlow1()
    #ContStop123F()
    print(sw41,sw40,sw39,sw38)
    if sw41==1:
       bitnum=0b1000
       a=bitnum
       #print(a)
    if sw40==1:
       bitnum=0b0100
       b=bitnum
       #print(b)
    if sw39==1:
       bitnum=0b0010
       c=bitnum
       #print(c)
    if sw38==1:
       bitnum=0b0001
       d=bitnum
       #print(d)
          
   
  
       #time.sleep(0.5)

    e=a+b+c+d

    print(e)
    #Train1
    if e==2:
       #ContFSlow1()
       ContStop123F()
       print("ContStop123F()")
       GPIO.cleanup(gpio_sw38)
       GPIO.cleanup(gpio_sw39)
       GPIO.cleanup(gpio_sw40)
       GPIO.cleanup(gpio_sw41)

    if e==3:
       #ContFSlow1()
       ContFSlow1()
       print("ContFSlow1()")
       GPIO.cleanup(gpio_sw38)
       GPIO.cleanup(gpio_sw39)
       GPIO.cleanup(gpio_sw40)
       GPIO.cleanup(gpio_sw41)

    #Train2
    if e==4:
       ContFFast1()
       print("ContFFast1()")
       GPIO.cleanup(gpio_sw38)
       GPIO.cleanup(gpio_sw39)
       GPIO.cleanup(gpio_sw40)
       GPIO.cleanup(gpio_sw41)

    #Train2
    if e==5:
       ContStop1()
       print("ContStop1()")
       GPIO.cleanup(gpio_sw38)
       GPIO.cleanup(gpio_sw39)
       GPIO.cleanup(gpio_sw40)
       GPIO.cleanup(gpio_sw41)

    #Train2
       
    if e==6:
       #ContFSlow2()
       ContFSlow2()
       print("ContFSlow2()")
    if e==7:
       ContFFast2()
       print("ContFFast2()")
    if e==8:
       ContStop2()
       print("ContStop2()")
       
    #Train3
    if e==9:
       ContFSlow3()
       print("ContFSlow3()")
    if e==10:
       ContFFast3()
       print("ContFFast3()")
    if e==11:
       ContStop3()
       print("ContStop3()")
    #Train4
       
    if e==12:
       ContFSlow4()
       print("ContFSlow4()")
    if e==13:
       ContFFast4()
       print("ContFFast4()")
    if e==14:
       ContStop4()
       print("ContStop4()")
       
#else:
#time.sleep(2);    
# 後処理 GPIOを解放
    #GPIO.cleanup(gpio_led)
GPIO.cleanup(gpio_sw38)
GPIO.cleanup(gpio_sw39)
GPIO.cleanup(gpio_sw40)
GPIO.cleanup(gpio_sw41)
s.close()