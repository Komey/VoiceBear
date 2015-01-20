# -*- coding: cp936 -*-
import sys
import requests 
import urllib2
import threading
import serial

import time
import thread

import os  
import string
import random
import tempfile
import unittest
import pytest

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish

from qiniu import Auth, set_default, etag, PersistentFop, build_op, op_save
from qiniu import put_data, put_file, put_stream
from qiniu import BucketManager, build_batch_copy, build_batch_rename, build_batch_move, build_batch_stat, build_batch_delete
from qiniu import urlsafe_base64_encode, urlsafe_base64_decode
from qiniu.compat import is_py2, b
from qiniu.services.storage.uploader import _form_put
import qiniu.config


def on_connect(mqttc, obj, flags, rc):
    print("rc: "+str(rc))

def on_message(mqttc, obj, msg):
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
    payload = msg.payload
    if payload[0]=='/':
        payload = payload[1:]
        print payload
        base_url = 'http://%s/%s'%('XXXX.z0.glb.clouddn.com',payload)
        dl_file(url=base_url, thread=10, save_file=payload, buffer=4096)
        
def on_publish(mqttc, obj, mid):
    print("mid: "+str(mid))

def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_log(mqttc, obj, level, string):
    print(string)

max_thread = 10
# 初始化锁
lock = threading.RLock() 
class Downloader(threading.Thread):
    def __init__(self, url, start_size, end_size, fobj, buffer):
        self.url = url
        self.buffer = buffer
        self.start_size = start_size
        self.end_size = end_size
        self.fobj = fobj
        threading.Thread.__init__(self)
    def run(self):
        with lock:
            print 'starting: %s' % self.getName()
        self._download() 
    def _download(self):
        req = urllib2.Request(self.url)
        # 添加HTTP Header(RANGE)设置下载数据的范围
        req.headers['Range'] = 'bytes=%s-%s' % (self.start_size, self.end_size)
        f = urllib2.urlopen(req)
        # 初始化当前线程文件对象偏移量
        offset = self.start_size
        while 1:
            block = f.read(self.buffer)
            # 当前线程数据获取完毕后则退出
            if not block:
                with lock:
                    print '%s done.' % self.getName()
                break
            # 写如数据的时候当然要锁住线程
            # 使用 with lock 替代传统的 lock.acquire().....lock.release()
            # 需要python >= 2.5
            with lock:
                sys.stdout.write('%s saveing block...' % self.getName())
                # 设置文件对象偏移地址
                self.fobj.seek(offset)
                # 写入获取到的数据
                self.fobj.write(block)
                offset = offset + len(block)
                sys.stdout.write('done.\n') 
 
def dl_file(url, thread=3, save_file='', buffer=1024):
    # 最大线程数量不能超过max_thread
    thread = thread if thread <= max_thread else max_thread
    # 获取文件的大小
    req = urllib2.urlopen(url)
    size = int(req.info().getheaders('Content-Length')[0])
    # 初始化文件对象
    fobj = open(save_file, 'wb')
    # 根据线程数量计算 每个线程负责的http Range 大小
    avg_size, pad_size = divmod(size, thread)
    plist = []
    for i in xrange(thread):
        start_size = i*avg_size
        end_size = start_size + avg_size - 1
        if i == thread - 1:
            # 最后一个线程加上pad_size
            end_size = end_size + pad_size + 1
        t = Downloader(url, start_size, end_size, fobj, buffer)
        plist.append(t) 
     #  开始搬砖
    for t in plist:
        t.start() 
     # 等待所有线程结束
    for t in plist:
        t.join() 
     # 结束当然记得关闭文件对象
    fobj.close()
    print 'Download completed!'
    play_file(save_file)
    
def play_file(key):
    print "Start Play!"
    f = open(key,'rb')
    size=os.path.getsize(key)
    index = 0;
    while(index < size):
        ser.write(f.read(32))
        index +=32
    print "Done!"
    

def ul_file(key):
    access_key = 'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'
    secret_key = 'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'
    bucket_name = 'bear'
    q = Auth(access_key, secret_key)
    localfile = key
    mime_type = ""
    
    token = q.upload_token(bucket_name, key)
    ret, info = put_file(token, key, localfile, mime_type=mime_type, check_crc=True)
    payload = ret['key']
    try:
        assert ret['hash'] == etag(key)
        os.remove(key)
        print "Send to qiniu Done! "+ key
        publish.single("Phone",payload,qos=2, hostname="iot1.chinacloudapp.cn",port=6666)
    except AssertionError:
        print 'Upload Error'
        
ser = serial.Serial()
wtime = 5
hread = 0
fname = str(random.randint(10, 100))+'.ogg'
f = open(fname, 'wb')
print 'Creat new file '+fname
ctime = time.time()

def timer():
    while(1):
        global hread,f,ctime,fname
        ntime = time.time()
        if(ntime - ctime > wtime):
            if (hread == 1):
                time.sleep(1)
                if(hread == 1):
                    f.close()
                    _fname = fname
                    fname = str(random.randint(10, 100))+'.ogg'
                    f = open(fname, 'wb')
                    print 'Creat new file '+fname
                    hread = 0
                    thread.start_new_thread(ul_file,(_fname,)) 
            
def read_Ser():
    ser.flushInput()
    global hread,f,ctime
    while(1):
        try:
            b = ser.read(1)
            f.write(b)
            ctime = time.time()
            hread = 1
        except Exception,ex:
            print Exception,":",ex
        
def ser_int():
    ser.baudrate = 9600 
    ser.port = 1
    ser.open()
    print ser.portstr

def ser_start():
    thread.start_new_thread(read_Ser,())  
    thread.start_new_thread(timer,())  
    

def mqtt_int():
    mqttc = mqtt.Client(client_id="bear", clean_session=True, userdata=None)
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect
    mqttc.on_publish = on_publish
    mqttc.on_subscribe = on_subscribe
    # Uncomment to enable debug messages
    mqttc.on_log = on_log
    mqttc.connect("iot1.chinacloudapp.cn", 6666, 60)
    mqttc.subscribe("Bear", 2)

    mqttc.loop_forever()


if __name__ == '__main__':
    print 'Hello !'
    ser_int()
    ser_start()
    mqtt_int()
    

