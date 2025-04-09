use std::{ops::Deref, time::Instant};

use blitzbuffers_bench::BlitzbuffersBench;
use blitzbuffers_generated::blitzbuffers::{ExpandableBufferBackend, FixedSizeBufferBackend};
use blitzbuffers_raw_first_bench::BlitzBuffersRawFirstBench;
use blitzbuffers_raw_storage_bench::BlitzBuffersRawStorageBench;
use data::{data::Entity, BENCH_DATA};

mod blitzbuffers_bench;
mod blitzbuffers_generated;
mod blitzbuffers_raw_first_bench;
mod blitzbuffers_raw_storage_bench;
mod data;
mod flatbuffers_bench;
mod flatbuffers_generated;

pub trait BufferBench {
    fn encode(&mut self, entity: &Entity) -> Vec<u8>;
    fn decode_checked(&mut self, buffer: &[u8], sum: &mut usize);
    fn decode_unchecked(&mut self, buffer: &[u8], sum: &mut usize);
}

pub fn main() {
    let bench_data = BENCH_DATA.deref().clone();
    println!("\n# Encoding");
    bench_encode(bench_data.clone());

    println!("\n# Decoding checked");
    bench_decode_checked(bench_data.clone());

    println!("\n# Decoding unchecked");
    bench_decode_unchecked(bench_data.clone());
}

fn bench_encode(data: Entity) {
    let bzb = BlitzbuffersBench::new_with_backend(ExpandableBufferBackend::new_with_default_size());
    benchmark_encode("blitzbuffers", bzb, data.clone());

    let bzb = BlitzbuffersBench::new_with_backend(FixedSizeBufferBackend::<1024>::new());
    benchmark_encode("blitzbuffers (fixed size)", bzb, data.clone());

    let bzb = BlitzBuffersRawFirstBench::new();
    benchmark_encode("blitzbuffers (raw first)", bzb, data.clone());

    let bzb = BlitzBuffersRawStorageBench::new();
    benchmark_encode("blitzbuffers (raw storage)", bzb, data.clone());

    let fbs = flatbuffers_bench::FlatbuffersBench::new();
    benchmark_encode("flatbuffers", fbs, data.clone());
}

fn benchmark_encode(name: &'static str, mut bench: impl BufferBench, data: Entity) {
    // Warm up
    let mut sum = 0;
    for _ in 0..30 {
        sum += bench.encode(&data).len();
    }

    assert!(sum > 0);

    // Run benchmark
    let mut sum = 0;
    const ITERATIONS: usize = 1_000_000;
    let start = Instant::now();
    for _ in 0..ITERATIONS {
        sum += bench.encode(&data).len();
    }
    let end = Instant::now();
    assert!(sum > 0);

    let time_taken = ((end - start).as_nanos() as f32) / (ITERATIONS as f32);
    eprintln!("{} took {:.1} ns/iter", name, time_taken);
}

fn bench_decode_checked(data: Entity) {
    let bzb = BlitzbuffersBench::new_with_backend(ExpandableBufferBackend::new_with_default_size());
    benchmark_decode_checked("blitzbuffers", bzb, data.clone());

    let fbs = flatbuffers_bench::FlatbuffersBench::new();
    benchmark_decode_checked("flatbuffers", fbs, data.clone());
}

fn benchmark_decode_checked(name: &'static str, mut bench: impl BufferBench, data: Entity) {
    let encoded = bench.encode(&data);

    let mut sum = 0;
    bench.decode_checked(&encoded, &mut sum);

    // Warm up
    let mut sum = 0;
    for _ in 0..30 {
        bench.decode_checked(&encoded, &mut sum);
    }

    assert!(sum > 0);

    // Run benchmark
    let mut sum = 0;
    const ITERATIONS: usize = 1_000_000;
    let start = Instant::now();
    for _ in 0..ITERATIONS {
        bench.decode_checked(&encoded, &mut sum);
    }
    let end = Instant::now();

    assert!(sum > 0);

    let time_taken = ((end - start).as_nanos() as f32) / (ITERATIONS as f32);
    eprintln!("{} took {:.1} ns/iter", name, time_taken);
}

fn bench_decode_unchecked(data: Entity) {
    let bzb = BlitzbuffersBench::new_with_backend(ExpandableBufferBackend::new_with_default_size());
    benchmark_decode_unchecked("blitzbuffers", bzb, data.clone());

    let fbs = flatbuffers_bench::FlatbuffersBench::new();
    benchmark_decode_unchecked("flatbuffers", fbs, data.clone());
}

fn benchmark_decode_unchecked(name: &'static str, mut bench: impl BufferBench, data: Entity) {
    let encoded = bench.encode(&data);

    let mut sum = 0;
    bench.decode_unchecked(&encoded, &mut sum);

    // Warm up
    let mut sum = 0;
    for _ in 0..30 {
        bench.decode_unchecked(&encoded, &mut sum);
    }

    assert!(sum > 0);

    // Run benchmark
    let mut sum = 0;
    const ITERATIONS: usize = 1_000_000;
    let start = Instant::now();
    for _ in 0..ITERATIONS {
        bench.decode_unchecked(&encoded, &mut sum);
    }
    let end = Instant::now();

    assert!(sum > 0);

    let time_taken = ((end - start).as_nanos() as f32) / (ITERATIONS as f32);
    eprintln!("{} took {:.1} ns/iter", name, time_taken);
}
