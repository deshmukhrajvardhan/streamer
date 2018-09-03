import hyper
import urllib

DOWNLOAD_CHUNK = 15000

def download_vid():
    ssl_context = hyper.tls.init_context()
    ssl_context.load_cert_chain(certfile='~/cert.crt', keyfile='~/cert.key')
    ssl_context.load_verify_locations(cafile='~/cert.pem')
    parse_url = urllib.parse.urlparse("https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets")
    connection = hyper.HTTP20Connection(parse_url.netloc, ssl_context=ssl_context,force_proto='h2', secure=True,port=443)
    connection.network_buffer_size= int(DOWNLOAD_CHUNK)
    
    for i in range(1,150):
        segment_uri="https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s{}.m4s".format(i)
        print("Downloading: {}\n".format(i))
        try:
            seg_resp_conn = connection.request('GET',segment_uri)
            seg_conn = connection.get_response(seg_resp_conn)
            with open("~/vid.m4s",'wb') as segment_file_handle:
                segment_data = seg_conn.read(DOWNLOAD_CHUNK)
                while segment_data:
                    segment_file_handle.write(segment_data)
                    segment_data = seg_conn.read(DOWNLOAD_CHUNK)
            seg_conn.close()
                    
        except hyper.http20.exceptions.HTTP20Error as error:
            print("Unable to download DASH Segment {} HTTP Error:{} ".format(segment_uri, str(error.code)))
            return None
    

download_vid()