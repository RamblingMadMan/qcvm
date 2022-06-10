# QCVM Bytecode Specification

The bytecode is separated into a header and sections, where each section is a packed contiguous array of the type required by that section.

## Header

The header of a stream of bytecode has the following structure:

| Type     | Description               | Required value |
|:---------|:--------------------------|:---------------|
| `u32`    | Version number            | `0x6`          |
| `u16`    | CRC sum                   |                |
| `u16`    | Skipped bytes             | `0x0`          |
| `u32`    | Statement section offset  |                |
| `u32`    | Statement section size    |                |
| `u32`    | Definition section offset |                |
| `u32`    | Definition section size   |                |
| `u32`    | Field section offset      |                |
| `u32`    | Field section size        |                |
| `u32`    | Function section offset   |                |
| `u32`    | Function section size     |                |
| `u32`    | String section offset     |                |
| `u32`    | String section size       |                |
| `u32`    | Global section offset     |                |
| `u32`    | Global section size       |                |


## Statements

A statement has the following structure:

| Type  | Description |
|:------|:------------|
| `u32` | Op code     |
| `u32` | Argument 1  |
| `u32` | Argument 2  |
| `u32` | Argument 3  |

## Definitions

A definition has the following structure:

| Type  | Description  |
|:------|:-------------|
| `u16` | Type code    |
| `u16` | Global index |
| `u32` | Name index   |

> The 15<sup>th</sup> bit of the type code will be set for global definition.

## Fields

A field has the following structure:

| Type  | Description |
|:------|:------------|
| `u16` | Type code   |
| `u16` | Offset      |
| `u32` | Name index  |

## Functions

A function has the following structure:

| Type     | Description                    |
|:---------|:-------------------------------|
| `i32`    | Entry point                    |
| `u32`    | First local index              |
| `u32`    | Number of locals               |
| `u32`    | "Profile" (should always be 0) |
| `u32`    | Name index                     |
| `u32`    | File name index                |
| `i32`    | Number of arguments (Max 8)    |
| `u8 * 8` | Size of each argument          |

> The entry point will be negative for builtin functions

## Strings

The string section is a sequence of null-terminated strings. This section usually starts with `0x0` so there is an empty string at index `0`.

## Globals

The globals section is a sequence of initial `u32` values for each global definition.
