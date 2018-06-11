#!/usr/local/lib/ python2.7
'''
This is a Python script which is used to automate AStream client and
cross traffic generators for SABR evaluation.
'''
from fabric.api import *
import random

import socket
socket.setdefaulttimeout(5)

import numpy as np
from scipy import *
import random
import bisect
import math


import subprocess 
import paramiko
import logging
logging.basicConfig()
import sys
import threading
import os
import time
from collections import defaultdict
user="rdeshm0"#"dbhat0"
RUN_UPERF=True
NUM_OF_RUNS = 3
zipf_dist = dict()
#client_ip = ["clnode138.clemson.cloudlab.us","clnode148.clemson.cloudlab.us"]
#client_ip = ["clnode169.clemson.cloudlab.us","clnode148.clemson.cloudlab.us"]
client_ip = ["clnode169.clemson.cloudlab.us","clnode138.clemson.cloudlab.us"]
#["clnode171.clemson.cloudlab.us"]# 
client_ports = 22 #[22,22,22,22]
#cross_ip= "clnodevm180-1.clemson.cloudlab.us"
#cross_ip= "clnode171.clemson.cloudlab.us"
cross_ip= "clnode180.clemson.cloudlab.us"
cross_port= 25611#22
# HTTP2 HTTP1 MULTICORE

# QUIC, HTTP2, HTTP1
#client_cmd = ["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP2_CL/;"]
#client_cmd = ["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;"]

# HTTP1, QUIC, iperf
#client_cmd = ["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;","sudo /dev/SQUAD/src/out/Test/quic_persistent_client --disable-certificate-verification 2>&1 & cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_quic_test_nw_buff.py -m https://www.yo.org/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r; sudo mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/QUIC_CL/"]#,
#client_cmd = ["sudo iperf -c 10.10.5.2 -t 300"]

#HTTP2
#client_cmd = ["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP2_CL/;"]

# QUIC, HTTP2, HTTP1
#client_cmd = ["sudo /dev/SQUAD/src/out/Test/quic_persistent_client --disable-certificate-verification 2>&1 & cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_quic_test_nw_buff.py -m https://www.yo.org/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r; sudo mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/QUIC_CL/","cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.7.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP2_CL/;" , "cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.8.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;"]

# HTTP2 3 apps
#client_cmd = ["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP2_CL/;","cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.7.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP2_CL/;","cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.8.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP2_CL/;"]

# QUIC 3 apps
#client_cmd = ["sudo /dev/SQUAD/src/out/Test/quic_persistent_client --disable-certificate-verification 2>&1 & cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_quic_test_nw_buff.py -m https://www.yo.org/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r; sudo mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/QUIC_CL/","sudo /dev/SQUAD/src/out/Test/quic_persistent_client --disable-certificate-verification 2>&1 & cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_quic_test_nw_buff.py -m https://www.yotwo.org/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r; sudo mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/QUIC_CL/","sudo /dev/SQUAD/src/out/Test/quic_persistent_client --disable-certificate-verification 2>&1 & cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_quic_test_nw_buff.py -m https://www.yothree.org/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r; sudo mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/QUIC_CL/"]

# HTTP1 3 apps
#client_cmd = [ "cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;","cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.8.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;"]
#client_cmd=["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.7.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;", "cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.8.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;"]
client_cmd=["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.7.1/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;","cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;"]
#client_cmd=["cd /dev/SQUAD/; sudo time taskset 0x00000001 python3 streamer/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py -m https://10.10.1.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;mv /dev/SQUAD/ASTREAM_LOGS/* /dev/SQUAD/CLIENT_LOGS/HTTP1_CL/;"]
#
#exec_command_uperf="bash /users/rdeshm0/iperf-UDP-staircase.sh" #w traffic
#exec_command_uperf="bash /users/rdeshm0/iperf-UDP-W-9.sh" #w traffic multi thread http2
#exec_command_uperf= "iperf -c 10.10.4.2 -t 300 -i 10 -u -b 3M" #on-off traffic
#exec_command_uperf= "iperf -c 10.10.5.2 -t 300 -i 10 -u -b 3M" #on-off traffic
exec_command_uperf= "iperf -c 10.10.3.1 -t 300 -i 10 -u -b 3M" #on-off traffic

#["/usr/bin/iperf -c 10.10.7.2 -t 40 -i 1 -u -b 2M", "/usr/bin/iperf -c 10.10.7.2 -t 40 -i 1 -u -b 4M", "/usr/bin/iperf -c 10.10.7.2 -t 40 -i 1 -u -b 6M", "/usr/bin/iperf -c 10.10.7.2 -t 40 -i 1 -u -b 8M"]


def cross_client(ipaddress, ports,exec_comm):
    global user

    print ipaddress, ports, exec_comm
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ipaddress,username=user,port=ports,password="SwitchAnalysis",key_filename="/users/rdeshmuk/.ssh/id_geni_ssh_rsa")
    except paramiko.AuthenticationException:
        print "[-] Authentication Exception! ..."      
         
    except paramiko.SSHException:
        print "[-] SSH Exception! ..." 
         
    works = ipaddress.strip('\n')+','+user 
    print '[+] '+ works
    #fw.write(works+'\n')
    print exec_comm
    #stdin, stdout, stderr=ssh.exec_command("apt-get update;apt-get install iperf")
    #cl_comm = "cd /groups/ch-geni-net/GIMITesting/VLC_files/www-itec.uni-klu.ac.at/rude/; " + exec_comm
    #print ipaddress, ports, cl_comm
    #stdin, stdout, stderr = ssh.exec_command("killall rude")
    #stdin,stdout,stderr= ssh.exec_command("killall iperf")
    stdin, stdout, stderr=ssh.exec_command(exec_comm)
    print stderr.read()
    print stdout.read()
    ssh.close()
def cross_client_ping(ipaddress,ports,exec_comm):
    global user
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ipaddress,username=user,port=ports,password="SwitchAnalysis",key_filename="/users/rdeshmuk/.ssh/id_geni_ssh_rsa")
    except paramiko.AuthenticationException:
        print "[-] Authentication Exception! ..."
    except paramiko.SSHException:
        print "[-] SSH Exception! ..."

    works = ipaddress.strip('\n')+','+user 
    print '[+] '+ works
    #fw.write(works+'\n')
    print exec_comm
    #print ipaddress, ports, cl_comm
    #stdin, stdout, stderr = ssh.exec_command("killall rude")
    #stdin, stdout, stderr=ssh.exec_command("sudo apt-get update; sudo apt-get install iperf")
    stdin, stdout, stderr=ssh.exec_command(exec_comm)
    print stderr.read()
    print stdout.read()
    ssh.close()

def cross_server(ipaddress):
    global user
     
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ipaddress,username=user,password="SwitchAnalysis",key_filename="/users/rdeshmuk/.ssh/id_geni_ssh_rsa")
    except paramiko.AuthenticationException:
        print "[-] Authentication Exception! ..."      
         
    except paramiko.SSHException:
        print "[-] SSH Exception! ..." 
         
    works = ipaddress.strip('\n')+','+user  
    print '[+] '+ works
    #stdin, stdout, stderr = ssh.exec_command("killall crude")
    #stdin, stdout, stderr=ssh.exec_command("apt-get update; apt-get install iperf")
    #stdin, stdout, stderr = ssh.exec_command('killall crude; cd /groups/ch-geni-net/GIMITesting/VLC_files/rude/')
    #print stdout.read()
    #print stderr.read()
    #stdin, stdout, stderr = ssh.exec_command('crude -p 8000 -l crudeserv.txt </dev/null >>/dev/null 2>&1')
    #stdin,stdout,stderr = ssh.exec_command('iperf -s -u -i 1 </dev/null >>/dev/null 2>&1')
    #stdin,stdout,stderr= ssh.exec_command("killall iperf")
    #print stdout.readlines()
    #print stderr.readlines()
    ssh.close()
#def dash_client(ipaddress, ports, zipf_index, mpd_ip):
def dash_client(ipaddress,ports,exec_cmd):
    global user
    #with settings(host_string=ipaddress, port=ports, user = "dbhat", password="<password_here>",key_filename="/users/dbhat/.ssh/id_geni_ssh_rsa"):
     #   run('sudo apt-get -f install python-pip python-dev build-essential')
     
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ipaddress,username=user,port=ports,password="SwitchAnalysis",key_filename="/users/rdeshmuk/.ssh/id_geni_ssh_rsa")
    except paramiko.AuthenticationException:
        print "[-] Authentication Exception! ..."      
         
    except paramiko.SSHException:
        print "[-] SSH Exception! ..." 
         
    works = ipaddress.strip('\n')+','+user  
    print '[+] '+ works
         
   # cmd_str='cd /proj/cdnabrtest-PG0/chromium/src; gclient runhooks; gn gen out/Default'+str(count+1)
    #stdin,stdout,stderr = ssh.exec_command('sudo apt-get -y --force-yes update; sudo apt-get -y --force-yes install vim screen;export PATH=\"$PATH:/proj/cdnabrtest-PG0/depot_tools\" ')
    #stdin,stdout,stderr = ssh.exec_command('sudo pip install urllib3 httplib2 pymongo netifaces requests numpy sortedcontainers',get_pty=True)
    #stdin,stdout,stderr=ssh.exec_command('sudo apt-get -y --force-yes install python-pip python-dev build-essential',get_pty=True)
    stdin,stdout,stderr=ssh.exec_command(exec_cmd)
    stringerr = str(stderr.readlines()[1:]) + str(ipaddress) + str(ports)
    print stringerr
    #print stdout.readlines()
    ssh.close()
def kill_squad_client(ipaddress,ports):
    global user
     
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ipaddress,username=user,port=ports, password="SwitchAnalysis",key_filename="/users/rdeshmuk/.ssh/id_geni_ssh_rsa")
    except paramiko.AuthenticationException:
        print "[-] Authentication Exception! ..."      
         
    except paramiko.SSHException:
        print "[-] SSH Exception! ..." 
    
    #cl_command = "cd /users/dbhat/astream_dash/dist/; python client/dash_client.py -m http://10.10.5.2/DASH_BuckBunny/AStream_mpd/BigBuckBunny_2s_300s_4Mbps.mpd -p empirical > /dev/null &"     
    #cl_command = "cd /users/dbhat/astream_dash_spectrum/dist/; python client/dash_client.py -m http://10.10.5.2/DASH_BuckBunny/AStream_mpd/BigBuckBunny_2s_small.mpd -p empirical -r > /dev/null &" 
    #cl_command ="cd /users/dbhat/astream_dash/dist/; python client/dash_client.py -m http://10.10.5.2/DASH_BuckBunny/AStream_mpd/BigBuckBunny_2s_300s_4Mbps.mpd -p bola > /dev/null &"
    #cl_command="rm /users/dbhat/astream_dash_spectrum/dist/ASTREAM_LOGS/*"
    #cl_command="killall python"
    stdin,stdout,stderr=ssh.exec_command("sudo killall python")
    #stdin, stdout, stderr = ssh.exec_command("sudo rm -rf /users/dbhat/astream_dash*; sudo cp /groups/ch-geni-net/GIMITesting/VLC_files/astream_dash.tar.gz /users/dbhat; tar xvzf /users/dbhat/astream_dash.tar.gz")
    #stdin, stdout, stderr = ssh.exec_command("ls -l /users/dbhat")
    print stderr.readlines()
    print stdout.readlines()
    ssh.close()
def kill_cross_client(ipaddress, ports):
    global user
     
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(ipaddress,username=user,port=ports, password="SwitchAnalysis",key_filename="/users/rdeshmuk/.ssh/id_geni_ssh_rsa")
    except paramiko.AuthenticationException:
        print "[-] Authentication Exception! ..."      
         
    except paramiko.SSHException:
        print "[-] SSH Exception! ..."  
         
    works = ipaddress.strip('\n')+','+user  
    print '[+] '+ works
    #fw.write(works+'\n')
    #stdin,stdout,stderr= ssh.exec_command("killall iperf")
    stdin, stdout, stderr=ssh.exec_command("sudo killall iperf")
    print stderr.read()
    print stdout.read()
    ssh.close()

    
if __name__ == "__main__": 

	
        try:
  	  if RUN_UPERF:
	    a=list(range(len(client_ip)))
	    for j in range (0, NUM_OF_RUNS):
				#for cl_comm in exec_command_client:
				
		#concat=str(client_ip)
                try:
                    #print(a)
                    l=a.pop(0)
                        #     print(l)
                    a.append(l)
                    #threading.Thread(target=kill_cross_client,args=(bottle_neck_ip,bottle_neck_port)).start()
                    #time.sleep(10)
		    #threading.Thread(target=cross_client,args=(bottle_neck_ip,bottle_neck_port,limit_traffic_uperf)).start() #bottle_neck
                    
                    for i in a:
		    			    threading.Thread(target=cross_client,args=(cross_ip,cross_port,exec_command_uperf)).start()
					    threading.Thread(target=dash_client,args=(str(client_ip[i]),client_ports,str(client_cmd[i]))).start()

					    print("\nip:{}, port:{}, cmd:{}\n".format(client_ip[i],client_ports,client_cmd[i]))
                                            #time.sleep(400)
                    			    #threading.Thread(target=kill_cross_client,args=(cross_ip,cross_port)).start()
		    #print "Killing Cross-Traffic"
                    time.sleep(500) #(400.0) #(150.0)
                    print "Killing Cross-Traffic"
#	            threading.Thread(target=kill_cross_client,args=(bottle_neck_ip,bottle_neck_port)).start()
                except Exception, e:
		            print '[-] General Exception'

 	   
	except Exception, e:
		print '[-] General Exception'
	

