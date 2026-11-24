# Qwen Local Lab

Raw outputs from local Qwen3.8 Flash Next IQ3_S experiments. No manual fixes to generated source. Review and test the code independently; this is an experiment, not a production release.

## Environment

- AMD Ryzen 7 5700X, 64 GB DDR4-3200
- AMD Radeon RX 7800 XT 16 GB (inference), R7 260X (desktop)
- Ubuntu 24.04, Strata 0.2.20, engine 0.1.66 HIP
- INT8 KV, MTP, 1024 MiB VRAM reserve; monitor reported 128K context

## Experiments — 2026-11-24

| Project | Reported output tokens | Decode tok/s |
|---|---:|---:|
| [Nova website](website-nova/) | 11,080 | 60.8 |
| [PulseMonitor](pulse-monitor/) | 19,673 | 61.3 |
| [C++ calculator](calculator-cpp/) | 78,385 | 70.4 |

Token counts and speeds are Strata UI counters for full responses, not counts of source code alone. The calculator request took 1115.8 s according to the monitor. Session average: 67.3 decode tok/s, 1119 prefill tok/s; 110,931 output tokens, 79,973 reused prompt tokens, 4419 cached experts, 15.2/16 GB VRAM. Context reached 120K/128K. These are observed session metrics, not a controlled benchmark.

## Policy

Generated source is preserved verbatim from the exported responses (surrounding chat text removed). Bugs are intentionally left as generated. PulseMonitor is a front-end demo with simulated metrics, not a hardware collector. More experiments can be added as separate directories.
