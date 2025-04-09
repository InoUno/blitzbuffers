use crate::{
    blitzbuffers_generated::bench_blitzbuffers::*, blitzbuffers_generated::blitzbuffers::*,
    data::data, BufferBench,
};

pub struct BlitzbuffersBench<Backend>
where
    Backend: BlitzBufferBackend,
{
    backend: UnsafeBlitzBufferBackend<Backend>,
}

impl<Backend> BlitzbuffersBench<Backend>
where
    Backend: BlitzBufferBackend,
{
    #[inline(always)]
    pub fn new_with_backend(backend: UnsafeBlitzBufferBackend<Backend>) -> Self {
        Self { backend }
    }
}

impl<Backend> BufferBench for BlitzbuffersBench<Backend>
where
    Backend: BlitzBufferBackend,
{
    #[inline(never)]
    fn encode(&mut self, raw_entity: &data::Entity) -> Vec<u8> {
        self.backend.clear();

        let mut entity = Entity::new_on(&self.backend);

        entity.set_type((raw_entity.entity_type as u8).into());
        entity.position().copy_from(Position {
            x: raw_entity.position.0,
            y: raw_entity.position.1,
            z: raw_entity.position.2,
        });
        entity.insert_name(&raw_entity.name);

        let related = entity.insert_related_iter_zip(raw_entity.related.iter());
        for (data, mut entity) in related {
            entity.set_type((data.entity_type as u8).into());
            entity.position().set_x(data.position.0);
            entity.position().set_y(data.position.1);
            entity.position().set_z(data.position.2);
            entity.insert_name(&data.name);
        }

        let buffer = self.backend.build();

        return buffer;
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

impl<T> BlitzbuffersBench<T>
where
    T: BlitzBufferBackend,
{
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
