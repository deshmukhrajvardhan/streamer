import sysv_ipc
import threading
import sys
import errno
import timeit
from string import ascii_letters, digits
import read_mpd
import urllib
import random
import os

def write_msg(list_cmd,mq):
        for i in list_cmd:
                print (i)
                mq.send(i, True)
        return


def get_mpd(url):
    """ Module to download the MPD from the URL and save it to file"""
    key_c_orig_r = 262144
    key_c_orig_w = 262145
    key_c_retx_r=362146
    key_c_retx_w=462146
    keyr1=562146
    keyr2=662146
    try:
        mq_orig_r=sysv_ipc.MessageQueue(key_c_orig_w, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: mq_orig_r Queue not created")

    try:
        mq_retx_r=sysv_ipc.MessageQueue(key_c_retx_w, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: mq_orig_r Queue not created")

    message='start'
    m=['start']
    total_chunk_sizes=[]
    chunk_sizes=[]
    while m[0] != b'end':
        (message,prior)=mq_orig_r.receive()
        m=message.split(b':')
#        print("Reply:{}, content-length:{}\n".format(message,m[1]))
        chunk_sizes.append(float(m[1]))

    content_length = chunk_sizes.pop()
    print("P1 sum:{}, content_length:{}".format(sum(chunk_sizes),content_length))
  #  if sum(chunk_sizes) == content_length:
  #      total_chunk_sizes.append(chunk_sizes)
  #  else:
  #      print("Not all chunks in pipe 1")

    chunk_sizes=[]
    message='start'
    m=['start']
    while m[0] != b'end':
        (message,prior)=mq_retx_r.receive()
        m=message.split(b':')
 #       print("retx-Reply:{}, content-length:{}\n".format(message,m[1]))
        chunk_sizes.append(float(m[1]))

    content_length = chunk_sizes.pop()
    print("P2 sum:{}, content_length:{}".format(sum(chunk_sizes),content_length))

#    if sum(chunk_sizes) == content_length:
#        total_chunk_sizes.append(chunk_sizes)
#    else:
#        print("Not all chunks in pipe 2")


    mpd_file = url.split('/')[-1]
    mpd_file=os.path.abspath(mpd_file)
    try:
        mqw1=sysv_ipc.MessageQueue(keyw1, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    try:
        mqr1=sysv_ipc.MessageQueue(keyr1, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    try:
        mqr1=sysv_ipc.MessageQueue(keyr1, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    parse_url = urllib.parse.urlparse(url)
    cmd1=["CREATE_STREAM",parse_url.path,mpd_file]
    print (parse_url.path)
    print (mpd_file)
    print (str(message[0],'utf-8', errors='ignore').split('\x00')[0])
    if "CONN_CREATED" in (str(message[0],'utf-8', errors='ignore').split('\x00')[0]):
        print ("CONNECTION ESTABLISHED")
        thread3=threading.Thread(target=write_msg,args=(cmd1,mqw1))
        thread3.start()
        while True:
          message=mqr1.receive()
          print("Received msg:{}".format(message[0]))
          if "DONE" in str(message[0]):
                   break
        try:
              mqr1.remove()
              mqw1.remove()
        except sysv_ipc.ExistentialError:
              print ("Existen.. error")
        #process3.join()
        thread3.join()
        return mpd_file
    else:
        return ("MPD error")
        
get_mpd(" http://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s.mpd")

