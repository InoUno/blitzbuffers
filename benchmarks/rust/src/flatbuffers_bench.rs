use flatbuffers::FlatBufferBuilder;

use crate::{
    data::data,
    flatbuffers_generated::bench_flatbuffers::{
        root_as_entity, root_as_entity_unchecked, Entity, EntityArgs, EntityType, Position,
    },
    BufferBench,
};

pub struct FlatbuffersBench<'fbb> {
    fbb: flatbuffers::FlatBufferBuilder<'fbb>,
}

impl<'fbb> FlatbuffersBench<'fbb> {
    pub fn new() -> Self {
        Self {
            fbb: FlatBufferBuilder::with_capacity(1024),
        }
    }
}

impl<'fbb> BufferBench for FlatbuffersBench<'fbb> {
    #[inline(never)]
    fn encode(&mut self, raw_entity: &data::Entity) -> Vec<u8> {
        self.fbb.reset();

        let related_len = raw_entity.related.len();
        let mut offsets = Vec::with_capacity(related_len);

        for i in 0..related_len {
            let related = &raw_entity.related[i];
            let pos = Position::new(related.position.0, related.position.1, related.position.2);
            let name = self.fbb.create_string(&related.name);
            let entity = Entity::create(
                &mut self.fbb,
                &EntityArgs {
                    type_: EntityType::ENUM_VALUES[related.entity_type as usize],
                    position: Some(&pos),
                    name: Some(name),
                    related: None,
                },
            );

            offsets.push(entity);
        }

        let pos = Position::new(
            raw_entity.position.0,
            raw_entity.position.1,
            raw_entity.position.2,
        );
        let name = self.fbb.create_string(&raw_entity.name);
        let related = self.fbb.create_vector(&offsets);
        let entity = Entity::create(
            &mut self.fbb,
            &EntityArgs {
                type_: EntityType::ENUM_VALUES[raw_entity.entity_type as usize],
                position: Some(&pos),
                name: Some(name),
                related: Some(related),
            },
        );

        self.fbb.finish(entity, None);

        let buffer = self.fbb.finished_data();
        return buffer.to_vec();
    }

    #[inline(never)]
    fn decode_checked(&mut self, buffer: &[u8], sum: &mut usize) {
        let entity = root_as_entity(buffer).unwrap();
        self.decode_helper(entity, sum);
    }

    #[inline(never)]
    fn decode_unchecked(&mut self, buffer: &[u8], sum: &mut usize) {
        let entity = unsafe { root_as_entity_unchecked(buffer) };
        self.decode_helper(entity, sum);
    }
}

impl FlatbuffersBench<'_> {
    #[inline(always)]
    fn decode_helper(&mut self, entity: Entity<'_>, sum: &mut usize) {
        *sum += entity.type_().0 as usize;
        *sum += entity.name().unwrap_or_default().len();
        entity.position().map(|pos| {
            *sum += pos.x() as usize;
            *sum += pos.y() as usize;
            *sum += pos.z() as usize;
        });

        let related_vec = entity.related().unwrap_or_default();
        for related in related_vec {
            *sum += related.type_().0 as usize;
            *sum += related.name().unwrap_or_default().len();
            related.position().map(|pos| {
                *sum += pos.x() as usize;
                *sum += pos.y() as usize;
                *sum += pos.z() as usize;
            });
        }
    }
}
