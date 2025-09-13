#![cfg(test)]

use crate::schema::{
    blitzbuffers,
    tests_blitzbuffers::{Entity, EntityType, Position, PositionEnum, PositionEnum_Position},
};
use blitzbuffers::PrimitiveByteFunctions;

mod schema;

#[test]
pub fn direct_to_blitz_buffer() {
    let arr = Entity {
        id: 123,
        _type: EntityType::Enemy,
        position: PositionEnum::Position(PositionEnum_Position {
            _0: Position {
                x: 1.0,
                y: 2.0,
                z: 3.0,
            },
        }),
    }
    .to_blitz_buffer();

    unsafe {
        assert_eq!(u32::read_le_bytes(&arr[0..4]), 123);
        assert_eq!(EntityType::read_le_bytes(&arr[4..5]), EntityType::Enemy);
        assert_eq!(u8::read_le_bytes(&arr[5..6]), 2u8); // Position variant
        assert_eq!(f32::read_le_bytes(&arr[6..10]), 1.0);
        assert_eq!(f32::read_le_bytes(&arr[10..14]), 2.0);
        assert_eq!(f32::read_le_bytes(&arr[14..18]), 3.0);
    }
}
