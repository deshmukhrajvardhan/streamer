for i in {1..148}
do
    curl -k https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s$i.m4s --http2 --key-type PEM --key cert.key --cert cert.crt --tlsv1.2 --resolve 10.10.3.2:443 >d

done
