mod json_entity;
mod schema;

use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

use json_entity::JsonEntity;
use json_entity::JsonEntityType;
use json_entity::JsonPositionEnum;
use schema::blitzbuffers::*;
use schema::tests_blitzbuffers::*;

fn main() {
    let mut args = std::env::args();
    if args.len() <= 1 {
        generate(get_general_data());
        return;
    };

    // Skip executable arg
    args.next();

    // Match on choice arg
    match args.next().unwrap().as_ref() {
        "c" => {
            if args.len() == 0 {
                check(get_general_data());
                return;
            }
            for path_string in args {
                check(PathBuf::from(path_string));
            }
        }
        "g" => {
            if args.len() == 0 {
                generate(get_general_data());
                return;
            }
            for path_string in args {
                generate(PathBuf::from(path_string));
            }
        }
        v => {
            panic!("Unexpected argument: {v}");
        }
    }
}

fn get_general_data() -> PathBuf {
    let path = PathBuf::from(std::env!("CARGO_MANIFEST_DIR")).join("../data/general");
    path.canonicalize().unwrap()
}

fn generate(data_path: PathBuf) {
    let data_file_path = data_path.join("data.json");
    let data_file = File::open(data_file_path).unwrap();

    let out_path = data_path.join("rust.data");

    let mut out_file = File::create(&out_path).unwrap();

    let data: JsonEntity = serde_json::from_reader(data_file).unwrap();

    let backend = ExpandableBufferBackend::new_with_default_size();
    let mut builder = Entity::new_on(&backend);
    write_entity(&data, &mut builder);

    let out_data = backend.build();
    out_file.write_all(&out_data).unwrap();

    println!("Generated {}", out_path.display());
}

fn write_entity<Backend: BlitzBufferBackend>(
    from: &JsonEntity,
    builder: &mut EntityBuilder<'_, Backend>,
) {
    let ty = match from.type_ {
        JsonEntityType::Player => EntityType::Player,
        JsonEntityType::Enemy => EntityType::Enemy,
        JsonEntityType::Neutral => EntityType::Neutral,
    };
    builder.set_type(ty);
    builder.insert_name(&from.name);
    builder.copy_from_aliases(&from.aliases);
    builder.copy_from_numbers(&from.numbers);

    match &from.position {
        JsonPositionEnum::None => {
            builder.position().clear();
        }
        JsonPositionEnum::NoPosition => {
            builder.position().make_NoPosition();
        }
        JsonPositionEnum::Position(json_position) => {
            let mut pos = builder.position().make_Position()._0();
            pos.set_x(json_position.x);
            pos.set_y(json_position.y);
            pos.set_z(json_position.z);
        }
        JsonPositionEnum::TuplePosition(x, y, z) => {
            let mut pos = builder.position().make_TuplePosition();
            pos.set_0(*x);
            pos.set_1(*y);
            pos.set_2(*z);
        }
        JsonPositionEnum::InlinePosition { x, y, z } => {
            let mut pos = builder.position().make_InlinePosition();
            pos.set_x(*x);
            pos.set_y(*y);
            pos.set_z(*z);
        }
    };

    if from.related.is_empty() {
        return;
    }

    for (from, mut to) in builder.insert_related_iter_zip(from.related.iter()) {
        write_entity(from, &mut to);
    }
}

fn check(data_path: PathBuf) {
    let data_file_path = data_path.join("data.json");
    let data_file = File::open(data_file_path).unwrap();

    let data: JsonEntity = serde_json::from_reader(data_file).unwrap();

    let backend = ExpandableBufferBackend::new_with_default_size();
    let mut builder = Entity::new_on(&backend);
    write_entity(&data, &mut builder);

    let org_data = backend.build();

    let org_entity = match Entity::view(&org_data) {
        Ok(entity) => entity,
        Err(err) => {
            eprintln!("Invalid entity: {}", err);
            return;
        }
    };

    let paths = std::fs::read_dir(data_path).unwrap();

    for entry in paths.map(|res| res.unwrap()) {
        let path = entry.path();
        if path.extension().unwrap().eq("data") {
            let bytes = std::fs::read(&path).unwrap();
            let encoded = match Entity::view(&bytes) {
                Ok(entity) => entity,
                Err(err) => {
                    eprintln!("Invalid entity: {}", err);
                    return;
                }
            };
            if let Err(mut errs) = compare_entities(&org_entity, &encoded) {
                errs.reverse();
                eprintln!(
                    "Data file {} does not match:\n{}",
                    path.display(),
                    errs.join("\n")
                );
                return;
            }
            println!("Checked {}", path.display());
        }
    }
}

fn compare_entities(
    original: &EntityViewer<'_>,
    encoded: &EntityViewer<'_>,
) -> Result<(), Vec<String>> {
    if original.get_type() != encoded.get_type() {
        return Err(vec![format!(
            "Types are not equal. Original: {:?} -- Encoded: {:?}",
            original.get_type(),
            encoded.get_type()
        )]);
    }

    if original.get_name() != encoded.get_name() {
        return Err(vec![format!(
            "Names are not equal. Original: {:?} -- Encoded: {:?}",
            original.get_name(),
            encoded.get_name()
        )]);
    }

    if original.get_position() != encoded.get_position() {
        return Err(vec![format!(
            "Positions are not equal. Original: {:?} -- Encoded: {:?}",
            original.get_position(),
            encoded.get_position()
        )]);
    }

    if original.get_aliases() != encoded.get_aliases() {
        return Err(vec![format!(
            "Aliases are not equal. Original: {:?} -- Encoded: {:?}",
            original.get_aliases(),
            encoded.get_aliases()
        )]);
    }

    if original.get_numbers() != encoded.get_numbers() {
        return Err(vec![format!(
            "Numbers are not equal. Original: {:?} -- Encoded: {:?}",
            original.get_numbers(),
            encoded.get_numbers()
        )]);
    }

    for (idx, (org, enc)) in original
        .get_related()
        .into_iter()
        .zip(encoded.get_related().into_iter())
        .enumerate()
    {
        if let Err(mut errs) = compare_entities(&org, &enc) {
            errs.push(format!("Related entity {idx} is not equal"));
            return Err(errs);
        }
    }

    Ok(())
}
