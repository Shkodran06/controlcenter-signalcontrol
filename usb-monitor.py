"""Local read-only USB bridge with automatic reconnection. Requires pyserial."""
import json, threading, time, pathlib, argparse
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
import serial

class USBReader:
 def __init__(self,port):
  self.port=port;self.connection=None;self.latest=None;self.last=0;self.retry_at=0;self.lock=threading.Lock()
 def clear(self):
  with self.lock:self.latest=None;self.last=0
 def close(self):
  if self.connection:
   try:self.connection.close()
   except (serial.SerialException,OSError):pass
  self.connection=None;self.clear()
 def poll(self):
  if self.connection is None:
   if time.monotonic()<self.retry_at:return
   try:self.connection=serial.Serial(self.port,115200,timeout=1)
   except (serial.SerialException,OSError):self.retry_at=time.monotonic()+1;return
  try:
   line=self.connection.readline(65536)
   if not line:return
   try:message=json.loads(line)
   except (ValueError,UnicodeDecodeError):return
   if isinstance(message,dict) and message.get('type')=='status':
    with self.lock:self.latest=message;self.last=time.monotonic()
  except (serial.SerialException,OSError):
   self.close();self.retry_at=time.monotonic()+1
 def snapshot(self):
  with self.lock:return self.latest if time.monotonic()-self.last<2 else None

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--port',required=True);parser.add_argument('--http-port',type=int,default=8082);args=parser.parse_args()
 reader=USBReader(args.port);stop=threading.Event()
 def read_loop():
  while not stop.is_set():reader.poll();stop.wait(.05)
 class Handler(SimpleHTTPRequestHandler):
  def __init__(self,*a,**kw):super().__init__(*a,directory=str(pathlib.Path(__file__).parent),**kw)
  def do_GET(self):
   if self.path.split('?')[0]=='/api/status':
    body=json.dumps(reader.snapshot()).encode();self.send_response(200);self.send_header('Content-Type','application/json');self.send_header('Cache-Control','no-store');self.send_header('Content-Length',str(len(body)));self.end_headers();self.wfile.write(body)
   else:super().do_GET()
  def log_message(self,*a):pass
 worker=threading.Thread(target=read_loop,daemon=True);worker.start()
 print(f'Monitor USB: http://localhost:{args.http_port} — riconnessione automatica attiva',flush=True)
 try:ThreadingHTTPServer(('127.0.0.1',args.http_port),Handler).serve_forever()
 except KeyboardInterrupt:pass
 finally:stop.set();worker.join(timeout=2);reader.close()
if __name__=='__main__':main()
