use std::cell::LazyCell;

pub mod data {
    #[derive(Debug, Default, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
    pub enum EntityType {
        #[default]
        None = 0,
        Player = 1,
        Neutral = 2,
        Enemy = 3,
    }

    impl Into<EntityType> for i8 {
        fn into(self) -> EntityType {
            match self {
                0 => EntityType::None,
                1 => EntityType::Player,
                2 => EntityType::Neutral,
                3 => EntityType::Enemy,
                _ => panic!("Unknown type"),
            }
        }
    }

    #[derive(Debug, Default, Clone, PartialEq, PartialOrd)]
    pub struct Entity {
        pub entity_type: EntityType,
        pub position: (f32, f32, f32),
        pub name: String,
        pub related: Vec<Entity>,
    }

    impl Entity {
        pub fn new(
            entity_type: EntityType,
            position: (f32, f32, f32),
            name: impl AsRef<str>,
            related: Vec<Entity>,
        ) -> Self {
            Self {
                entity_type,
                position,
                name: name.as_ref().to_string(),
                related,
            }
        }
    }
}

pub const BENCH_DATA: LazyCell<data::Entity> = LazyCell::new(|| {
    data::Entity::new(
        data::EntityType::Player,
        (1f32, 2f32, 3f32),
        "Player".to_string(),
        vec![
            data::Entity::new(
                data::EntityType::Neutral,
                (4f32, 8f32, 16f32),
                "Rabbit1".to_string(),
                vec![],
            ),
            data::Entity::new(
                data::EntityType::Neutral,
                (4f32, 8f32, 16f32),
                "Rabbit2".to_string(),
                vec![],
            ),
            data::Entity::new(
                data::EntityType::Neutral,
                (4f32, 8f32, 16f32),
                "Rabbit3".to_string(),
                vec![],
            ),
            data::Entity::new(
                data::EntityType::Neutral,
                (4f32, 8f32, 16f32),
                "Rabbit4".to_string(),
                vec![],
            ),
            data::Entity::new(
                data::EntityType::Neutral,
                (4f32, 8f32, 16f32),
                "RabbitRabbitRabbitRabbit".to_string(),
                vec![],
            ),
        ],
    )
});
