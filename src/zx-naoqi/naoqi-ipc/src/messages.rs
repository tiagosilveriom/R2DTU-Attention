use serde::{Serialize, Deserialize};
use super::IpcMessage;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum MessageType {
    Handshake,
    Heartbeat,
    TTSRead,
    TTSWrite,
    TTSCmd,
}

#[derive(Serialize, Deserialize, Clone, PartialEq, Eq, Debug)]
pub struct Handshake {
    pub build_version: usize,
}

impl<'a> IpcMessage<'a> for Handshake {
    const TYPE: MessageType = MessageType::Handshake;
}

#[derive(Serialize, Deserialize, Clone, PartialEq, Eq, Debug)]
pub struct Heartbeat {
    pub timestamp: usize,
}

impl<'a> IpcMessage<'a> for Heartbeat {
    const TYPE: MessageType = MessageType::Heartbeat;
}

pub enum TTSSettings {
    Language(String),
    Speed(f32),
    Pitch(f32),
    Volume(f32),
}

pub enum TTSCmd {
    Say(String),
    Interrupt
}
