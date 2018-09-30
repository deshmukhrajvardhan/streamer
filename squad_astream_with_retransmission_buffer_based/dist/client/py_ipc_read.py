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


def write_msg(list_cmd, mq):
    for i in list_cmd:
        print(i)
        mq.send(i, True)
    return

def read_msg(p_no, mq):
    message = 'start'
    m = ['start']
    total_chunk_sizes = []
    chunk_sizes = []
    print("Py master reading chunks")
    while m[0] != b'end':
        (message, prior) = mq.receive()
        m = message.split(b':')
#        print("Reply:{}, content-length:{}\n".format(message,m[1]))
        chunk_sizes.append(float(m[1]))

    content_length = chunk_sizes.pop()
    print("{} sum:{}, content_length:{}".format(p_no,sum(chunk_sizes), content_length))
    return


def get_mpd(url):
    """ Module to download the MPD from the URL and save it to file"""
    key_c_orig_r = 262144
    key_c_orig_w = 262145
    key_c_retx_r = 362146
    key_c_retx_w = 462146


    ############################################## read chunks ###################################
    try:
        mq_orig_r = sysv_ipc.MessageQueue(key_c_orig_w, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: mq_orig_r Queue not created")

    try:
        mq_retx_r = sysv_ipc.MessageQueue(key_c_retx_w, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: mq_orig_r Queue not created")

    thread_orig = threading.Thread(target=read_msg, args=("P1",mq_orig_r))
    thread_retx = threading.Thread(target=read_msg, args=("P2",mq_retx_r))
    thread_orig.start()
#    thread_orig.join()
    thread_retx.start()
    thread_retx.join()



get_mpd(" http://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s.mpd")

