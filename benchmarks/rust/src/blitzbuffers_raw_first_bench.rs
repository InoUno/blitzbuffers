use crate::{blitzbuffers_generated::bench_blitzbuffers::*, data::data, BufferBench};

pub struct BlitzBuffersRawFirstBench;

impl BlitzBuffersRawFirstBench {
    #[inline(always)]
    pub fn new() -> Self {
        Self {}
    }
}

impl BufferBench for BlitzBuffersRawFirstBench {
    #[inline(never)]
    fn encode(&mut self, raw_entity: &data::Entity) -> Vec<u8> {
        let mut entity = Entity::default();

        entity._type = (raw_entity.entity_type as u8).into();
        entity.position = Position {
            x: raw_entity.position.0,
            y: raw_entity.position.1,
            z: raw_entity.position.2,
        };
        entity.name = raw_entity.name.clone();

        entity
            .related
            .resize(raw_entity.related.len(), Default::default());
        for (data, inner_entity) in raw_entity.related.iter().zip(entity.related.iter_mut()) {
            inner_entity._type = (data.entity_type as u8).into();
            inner_entity.position = Position {
                x: data.position.0,
                y: data.position.1,
                z: data.position.2,
            };
            inner_entity.name = data.name.clone();
        }

        return entity.to_blitz_buffer();
    }

    #[inline(never)]
    fn decode_checked(&mut self, buffer: &[u8], sum: &mut usize) {
        let entity = Entity::view(buffer).unwrap();
        self.decode_helper(entity, sum);
    }

    #[inline(never)]
    fn decode_unchecked(&mut self, buffer: &[u8], sum: &mut usize) {
        let entity = Entity::view_unchecked(buffer);
        self.decode_helper(entity, sum);
    }
}

impl BlitzBuffersRawFirstBench {
    #[inline(always)]
    fn decode_helper(&mut self, entity: EntityViewer<'_>, sum: &mut usize) {
        *sum += entity.get_type() as usize;
        *sum += entity.get_name().len();

        let pos = entity.get_position();
        *sum += pos.get_x() as usize;
        *sum += pos.get_y() as usize;
        *sum += pos.get_z() as usize;

        for related in entity.get_related() {
            *sum += related.get_type() as usize;
            *sum += related.get_name().len();

            let pos = related.get_position();
            *sum += pos.get_x() as usize;
            *sum += pos.get_y() as usize;
            *sum += pos.get_z() as usize;
        }
    }
}
