use std::io::{self, Read, Write};
use std::fs;
use std::fmt;
use std::error;

use bincode;
use serde::{Serialize, Deserialize};
use ipc_channel::ipc::{self, IpcOneShotServer, IpcReceiver, IpcSender, TryRecvError, IpcError};

pub mod messages;
pub use messages::*;

#[derive(Debug)]
pub enum NaoQiIpcError {
    Bincode(bincode::Error),
    Io(io::Error),
    Empty,
    NotConnected,
}

impl fmt::Display for NaoQiIpcError {

    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use NaoQiIpcError::*;
        match self {
            Bincode(e) => e.fmt(f),
            Io(e) => e.fmt(f),
            Empty => write!(f, "Empty channel"),
            NotConnected => write!(f, "Channel not connected"),
        }
    }
}

impl error::Error for NaoQiIpcError {
    
    fn source(&self) -> Option<&(dyn error::Error + 'static)> {
        use NaoQiIpcError::*;
        match self {
            Bincode(e) => e.source(),
            Io(e) => e.source(),
            Empty => None,
            NotConnected => None,
        }
    }
}

impl From<bincode::Error> for NaoQiIpcError {

    fn from(e: bincode::Error) -> Self {
        Self::Bincode(e)
    }
}

impl From<io::Error> for NaoQiIpcError {

    fn from(e: io::Error) -> Self {
        Self::Io(e)
    }
}

impl From<IpcError> for NaoQiIpcError {

    fn from(e: IpcError) -> Self {
        match e {
            IpcError::Bincode(x) => Self::Bincode(x),
            IpcError::Io(x) => Self::Io(x),
            IpcError::Disconnected => Self::NotConnected,
        }
    }
}

impl From<TryRecvError> for NaoQiIpcError {

    fn from(e: TryRecvError) -> Self {
        match e {
            TryRecvError::IpcError(x) => Self::from(x),
            TryRecvError::Empty => Self::Empty,
        }
    }
}

pub type Result<T> = std::result::Result<T, NaoQiIpcError>;

pub trait IpcMessage<'de>: serde::Serialize + serde::Deserialize<'de> {
    const TYPE: MessageType;
}

#[derive(Debug, Serialize, Deserialize)]
pub struct GenericMessage {
    msg_type: MessageType,
    content: Vec<u8>,
}

impl GenericMessage {

    pub fn new(msg_type: MessageType, content: Vec<u8>) -> GenericMessage {
        GenericMessage {
            msg_type,
            content,
        }
    }

    pub fn downcast<'a, M: IpcMessage<'a>>(&'a self) -> Result<M> {
        Ok(bincode::deserialize(&self.content)?)
    }
}

type IpcRx = IpcReceiver<GenericMessage>;
type IpcTx = IpcSender<GenericMessage>;

pub struct IpcChannel {
    rx: Option<IpcRx>,
    tx: Option<IpcTx>,
}

impl IpcChannel {

    pub fn new() -> Self {
        IpcChannel {
            rx: None,
            tx: None,
        }
    }

    fn handshake(&mut self) -> Result<()> {
        let handshake_send = Handshake { build_version: 0 };
        self.send(&handshake_send)?;
        let handshake_recv = self.rx.as_ref().unwrap().recv()?;

        if handshake_recv.msg_type == MessageType::Handshake {
            Ok(())
        } else {
            Err(NaoQiIpcError::Io(io::Error::new(io::ErrorKind::InvalidData, "Invalid Handshake received")))
        }
    }

    pub fn accept(&mut self) -> Result<()> {
        let (negotiation_channel, negotiation_key) = IpcOneShotServer::new()?;
        self.write_negotiation_key(&negotiation_key)?;
        let (_, (tx, rx)) = negotiation_channel.accept()?;
        self.rx = Some(rx);
        self.tx = Some(tx);

        self.handshake()?;

        Ok(())
    }

    pub fn connect(&mut self) -> Result<()> {
        let negotiation_key = self.read_negotiation_key()?;
        let negotiation_channel = IpcSender::connect(negotiation_key)?;
        
        let (tx0, rx0) = ipc::channel()?;
        let (tx1, rx1) = ipc::channel()?;
        negotiation_channel.send((tx0, rx1))?;
        self.rx = Some(rx0);
        self.tx = Some(tx1);

        self.handshake()?;

        Ok(())
    }

    pub fn send<'a, M: IpcMessage<'a>>(&mut self, msg: &M) -> Result<()> {
        let msg_encoding: Vec<u8> = bincode::serialize(msg)?;
        let generic = GenericMessage::new(M::TYPE, msg_encoding);
        if let Some(tx) = &self.tx {
            tx.send(generic)?;
            Ok(())
        } else {
            Err(NaoQiIpcError::NotConnected)
        }
    }

    pub fn receive(&mut self) -> Result<GenericMessage> {
        if let Some(rx) = &self.rx {
            Ok(rx.try_recv()?)
        } else {
            Err(NaoQiIpcError::NotConnected)
        }
    }

    const NEGOTIATION_KEY_PATH: &'static str = "/tmp/naoqi-negotiation-key";
    const NEGOTIATION_KEY_TMP_PATH: &'static str = "/tmp/naoqi-negotiation-key-tmp";

    fn read_negotiation_key(&self) -> Result<String> {
        let mut file = fs::File::open(Self::NEGOTIATION_KEY_PATH)?;
        let mut buf = String::new();
        file.read_to_string(&mut buf)?;
        fs::remove_file(Self::NEGOTIATION_KEY_PATH)?;

        Ok(buf)
    }

    fn write_negotiation_key(&self, key: &str) -> Result<()> {
        //Scope to force closure of file
        {
            let mut file = fs::File::create(Self::NEGOTIATION_KEY_TMP_PATH)?;
            file.write(key.as_bytes())?;
            file.sync_all()?;

        }
        fs::rename(Self::NEGOTIATION_KEY_TMP_PATH, Self::NEGOTIATION_KEY_PATH)?;

        Ok(())
    }

}


#[cfg(test)]
mod tests {
    use super::*;
    use std::thread;
    use std::time;

    #[test]
    fn send_and_receive() {


        let t1 = thread::spawn(|| {
            let mut ch = IpcChannel::new();
            ch.accept().unwrap();

            let mut success = false;
            for _ in 0..10 {
                match ch.receive() {
                    Ok(generic_recv_msg) => {
                        assert_eq!(generic_recv_msg.msg_type, MessageType::Heartbeat);
                        let recv_msg = generic_recv_msg.downcast::<Heartbeat>().unwrap();
                        assert_eq!(recv_msg.timestamp, 42);
                        success = true;
                        break;
                    },
                    Err(NaoQiIpcError::Empty) | Err(NaoQiIpcError::NotConnected) => {},
                    Err(e) => panic!("{:?}", e),
                }
                thread::sleep(time::Duration::from_millis(5));
            }
            assert!(success);
        });

        let t2 = thread::spawn(|| {
            let mut ch = IpcChannel::new();

            let mut success = false;
            for _ in 0..10 {
                if ch.connect().is_ok() {
                    success = true;
                    break;
                }
                thread::sleep(time::Duration::from_millis(5));
            }
            assert!(success);
            thread::sleep(time::Duration::from_millis(10));
            let send_msg = Heartbeat { timestamp: 42 };
            ch.send(&send_msg).unwrap();
        });


        t1.join().unwrap();
        t2.join().unwrap();
    }
}
