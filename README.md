# Mini-Script

A small C++ script engine that parses and executes a Lisp-like script language from a text file.

## Overview

This project implements a simple interpreter in C++ that reads a script file and executes statements such as variable assignment, expressions, conditionals, loops, arrays, and output.

The engine is built with CMake and produces a single executable named `main`.

## Features

- Basic Lisp-style syntax with parenthesized forms
- `let` variable assignment
- `exp` expressions for arithmetic and comparisons
- `if` conditional execution
- `while` loops
- `write` output to console
- `array_new`, `array_set`, `array_get`, and `array_del`
- `convert` between `NUM` and `STR`
- `function` declaration and invocation

## Requirements

- CMake 4.0 or newer
- A C++ compiler compatible with the target platform (MSVC on Windows is used in this workspace)

## Build

From the project root:

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

Or using the included `build` task in VS Code.

## Run

The executable expects a script file named `src.txt` in the current working directory. The project currently runs `engine.Run("src.txt")` from `src/main.cpp`.

```powershell
cd build\Debug
main.exe
```

If you want to test with a custom script, place it as `src.txt` in the executable folder or update `src/main.cpp`.

## Script Language Syntax

The engine parses a simple parenthesized syntax. Example constructs:

- Program block:
  `( program ... )`
- Variable assignment:
  `( let name value )`
- Expression:
  `( exp left right op )`
- Output:
  `( write value )`
- Conditional:
  `( if condition body )`
- Loop:
  `( while condition body )`
- Arrays:
  `( array_new arr )`
  `( array_set arr index value )`
  `( array_get arr index )`
  `( array_del arr index )`
- Type conversion:
  `( convert NUM expr )`
  `( convert STR expr )`
- Functions:
  `( function name ( params... ) body )`

## Example

A sample script is available in `build/Debug/src.txt` and demonstrates array creation, loop-based assignment, and output.

Example script structure:

1.Hello World
```lisp
(program
	(let stra "hello" )
	(let strb "world" )
	(let eq (exp stra strb == ) )
	(let add (exp stra strb + ) )
	(if (exp eq 0 == )
		(write eq )
	)
	(write add )
	(let a 0 )
	(while  (exp a 10 < )
		(program
			(let add (exp add "a" + ) )
			(let a (exp a 1 + ) )
		)
	)
	(write add )
)
```
2.Array
```lisp
( program
    ( array_new arr )
    ( let index 0 )
    ( while ( exp index 20 < )
        ( program ( array_set arr index ( exp ( convert STR index ) "a" + ) )
        ( let index ( exp index  1 + ) )
        )
    )
    ( let index 0 )
    ( while ( exp index 20 < )
        ( program ( write ( array_get arr index ) )
        ( let index ( exp index 1 + ) )
        )
    )
)
```
More examples in /example/src.txt.
## Notes

- The `for` statement is declared but not implemented yet.
- The interpreter prints the parsed tree before execution and reports execution time.
- `src.txt` must use the supported syntax and punctuation.

## Project Structure

- `CMakeLists.txt` - build definition
- `src/main.cpp` - entry point
- `src/script.cpp` - script interpreter implementation
- `include/script.h` - interpreter declarations

## License

EPL-2.0 License