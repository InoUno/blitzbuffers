# ⚡BlitzBuffers

BlitzBuffers is a serialization and deserialization format and code-generator that makes it easy to interface with the underlying data.

The main purposes of BlitzBuffers is performance, versatility, and ease of use.

## Pros

- **_Blitzingly fast_** serialization and deserialization of binary buffers.
- **Cross-language support** via code generation based on a given schema.
- **Simple, but powerful schema syntax** featuring structs, enums, tagged unions, vectors, and strings.
- **Zero-copy** serialization and deserialization - directly reads from and writes to the buffer.
- **Easy-to-use API** via the generated code.
- Code generation is done by via Python, which makes it simpler to integrate into your workflow, instead of having to deal with executables or build steps of other projects.

## Cons

- No schema versioning.
- Unset fields take up space in the buffer.
- Not many languages supported (yet)
- No endianness checks in C++ (yet) - only use it on little-endian machines

# Schema

The schema syntax and capabilities take inspiration from the [Rust language](https://www.rust-lang.org/).

<details>
<summary>Expand to see an example using the various constructs.</summary>

```bzb
namespace game

enum EntityType {
    Player
    Neutral
    Enemy
}

struct Position {
    x: f32
    y: f32
    z: f32
}

struct Entity {
    type: EntityType
    name: string
    hitpoints: u64
    position: Position
    traits: Trait[]
}

enum Trait {
    Height(u16, u16),

    Color {
        red: u8
        green: u8
        blue: u8
        alpha: OptionalByte
    }

}
```
</details>


# Code generation

Via `python`:

```
python3 -m blitzbuffers <schema-path> [-l <language> <output-file-path>]
```

# Benchmarks

A few benchmarks showing the difference between blitzbuffers and other tools/libraries with similar functionality.

## C++

Deserialization/usage is the same for blitzbuffers no matter the setup.

| Setup                     | Serialization  | Deserialization/Usage (checked) | Roundtrip |
|-|-|-|-|
| Blitzbuffers (fixed)      | 34.8 ns   | 74.7 ns               | 125 ns    |
| Blitzbuffers (expanding)  | 105 ns    | 74.8 ns               | 187 ns    |
| Flatbuffers               | 433 ns    | 176 ns                | 620 ns    |
| alpaca                    | 218 ns    | 479 ns                | 717 ns    |

## Rust

Deserialization/usage is the same for blitzbuffers no matter the setup. Checked vs. unchecked, is whether the data buffer is verified before the data in it is used.

| Setup                     | Serialization | Deserialization/Usage (checked) | Deserialization/Usage (unchecked) |
|-|-|-|-|
| Blitzbuffers (fixed)        | 77.3 ns     | 181.4 ns  | 116.8 ns
| Blitzbuffers (expanding)    | 141.1 ns    | -         | -
| Blitzbuffers (raw first)    | 593.8 ns    | -         | -
| Blitzbuffers (raw storage)  | 107.1 ns    | -         | -
| Flatbuffers                 | 410.8 ns    | 595.9 ns  | 129.9 ns

