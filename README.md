# Instruction Receiver

A Windows C++17 receiver for the **seven-integer text protocol** used by [Instruction-Broadcaster](https://github.com/Rick-laboratory/Instruction-Broadcaster). It can validate incoming UDP packets or explicitly apply mouse, keyboard and cursor-visibility commands.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\instruction-receiver.exe --help
```

Requires Windows, C++17 and CMake 3.16+. Ninja/MinGW-w64 places the executable directly in `build`. The original Visual Studio project is retained and updated.

## Run

```powershell
# Local validation; no keyboard or mouse actions.
.\build\Release\instruction-receiver.exe --once

# Apply packets from a selected sender on a selected local interface.
.\build\Release\instruction-receiver.exe --bind 192.168.1.20 --peer 192.168.1.10 --apply
```

Default: dry run on `127.0.0.1:9009`. `--port` accepts 0–65535; zero selects an ephemeral port. Ctrl+C stops. `--once` exits after one allowed-peer packet, returning 2 if it is malformed.

The protocol has no authentication, encryption, acknowledgements or ordering. An IP filter is not authentication. Use an input-applying instance only in a controlled environment; lost key-up packets can leave input held until cleanup.

## Wire format

Exactly seven ASCII integers, separated by whitespace:

```text
type x y code key_up cursor wheel
```

One optional trailing NUL is accepted. All unused fields must be zero.

| Type | Meaning | Example |
| --- | --- | --- |
| 0 | Mouse button flag: 2/4 left, 8/16 right, 32/64 middle | `0 0 0 2 0 0 0` |
| 0 | Signed wheel delta, conventionally ±120 | `0 0 0 0 0 0 -120` |
| 1 | Virtual key 1–254; key_up is 0 for down, 1 for up | `1 0 0 65 1 0 0` |
| 2 | Signed screen coordinates | `2 100 200 0 0 0 0` |
| 3 | Cursor 1 hides system cursors, 2 restores them | `3 0 0 0 0 2 0` |

For compatibility, mouse button packets accept key_up=0 or 1; the button flag determines the action. A packet carries one action. Truncated fields, out-of-range values, embedded NULs and extra tokens are rejected. The old sender's six-field middle-button packet is malformed.

Wheel deltas are now applied symmetrically; upward scrolling is no longer multiplied by five. The three-field protocol in Remote_Receiver and binary arrays in other experiments are different formats.

## Code guide

- [protocol.hpp](protocol.hpp) — named fields, validation, parsing and serialization.
- [UDP_Receiver.cpp](UDP_Receiver.cpp) — owned socket and exact UDP datagrams.
- [Message_Interpreter.cpp](Message_Interpreter.cpp) — Win32 input execution and state cleanup.
- [command.cpp](command.cpp) — validated internal three-field command representation.
- [main.cpp](main.cpp) — translation, execution mode and shutdown.
- [tests/receiver_test.cpp](tests/receiver_test.cpp) — protocol and loopback regressions.

The transport and Win32 helper follow the same structure as Remote_Receiver. Tests exercise no input injection. On orderly exit, pressed keys/buttons and hidden cursors are restored on a best-effort basis. Forced process termination or desktop restrictions can prevent that cleanup.

Generated Visual Studio outputs and machine-specific user settings are excluded from version control. Build fresh executables from source.

## Validation and license

Strict C++17 builds and protocol/UDP tests pass locally. Interactive input and cursor restoration were not exercised. This repository does not currently contain a standalone license file.
