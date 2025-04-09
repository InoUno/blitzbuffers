use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct JsonEntity {
    #[serde(alias = "type")]
    pub type_: JsonEntityType,
    pub name: String,
    pub aliases: Vec<String>,
    pub numbers: Vec<Vec<Vec<u16>>>,
    pub position: JsonPositionEnum,
    pub related: Vec<JsonEntity>,
}

#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
#[repr(u8)]
pub enum JsonEntityType {
    #[default]
    Player = 0,
    Enemy = 1,
    Neutral = 2,
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
#[repr(u8)]
pub enum JsonPositionEnum {
    #[default]
    None = 0,
    NoPosition,
    Position(JsonPosition),
    TuplePosition(f32, f32, f32),
    InlinePosition {
        x: f32,
        y: f32,
        z: f32,
    },
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct JsonPosition {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}
