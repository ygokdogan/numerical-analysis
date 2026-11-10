# Numerical Analysis | C

[Türkçe](README.md) | **English**

**A terminal application for root finding, linear algebra, numerical differentiation, integration, and interpolation.**

Developed for a Structured Programming course, this project performs numerical computations on user-supplied functions and matrices. Mathematical expressions are parsed at runtime, and each method reports its results as roots, error estimates, iteration counts, matrices, or solution vectors.

No external numerical computing library is required. The algorithms, expression parser, and memory management are implemented in C.

## Available methods

| Menu | Method | Purpose |
| :---: | --- | --- |
| 1 | Bisection | Find a root by repeatedly halving a sign-changing interval |
| 2 | Regula-Falsi | Find a root using linear interpolation |
| 3 | Newton-Raphson | Find a root using an initial estimate and a numerical derivative |
| 4 | Gauss-Jordan | Compute the inverse of an N × N matrix |
| 5 | Cholesky (LU)* | Solve a linear system through lower and upper triangular factorization |
| 6 | Gauss-Seidel | Solve a linear system iteratively |
| 7 | Numerical differentiation | Compute forward, backward, and central differences together |
| 8 | Simpson's 1/3 and 3/8 | Compute two numerical integration approximations together |
| 9 | Trapezoidal rule | Approximate a definite integral using trapezoids |
| 10 | Gregory-Newton | Interpolate equally spaced data using forward differences |

\* The menu labels this method “Cholesky (LU).” The implementation performs **Crout-style LU factorization** with a unit diagonal in `U`, rather than the classical `A = L Lᵀ` Cholesky factorization.

## Build and run

Requirements: GCC or Clang with C99 support and a terminal. Run these commands from the project directory.

### Linux / macOS

```bash
cc -std=c99 -O2 numerical_analysis.c -o program -lm
./program
```

### Windows (MinGW-w64 / GCC)

```powershell
gcc -std=c99 -O2 numerical_analysis.c -o program.exe -lm
.\program.exe
```

The `-lm` option links the math library. Once the application starts, enter a method number and follow the parameter prompts. Select `0` in the main menu to exit.

## Function syntax

Enter the expression alone, without an `f(x) =` prefix.

| Element | Supported syntax |
| --- | --- |
| Variable and constant | `x`, `pi` |
| Arithmetic operators | `+`, `-`, `*`, `/`, `^` |
| Grouping | `(` and `)` |
| Trigonometric functions | `sin`, `cos`, `tan`, `cot` |
| Inverse trigonometric functions | `arcsin`, `arccos`, `arctan` |
| Exponential and logarithmic functions | `exp(x)`, `ln(x)`, `log(x)` |
| Logarithms with a specified base | `log_5(x)`; use `log_x(8)` for a variable base |

Example expressions:

```text
x^3 - 2*x - 5
sin(x) + x^2 - 3
exp(x) - 4
ln(x) + log_2(x)
cos(pi*x)
```

- Write multiplication explicitly: `2*x` and `2*(x+1)`.
- Use a decimal point: `0.001`.
- Use lowercase function names. Trigonometric angles are in radians.
- `ln` is the natural logarithm; `log` is the base-10 logarithm.
- Use `x^0.5` for a square root; `sqrt` is not defined.

## Example: finding √2

Select **1 — Bisection Method** from the main menu and enter these values in order:

| Parameter | Value |
| --- | --- |
| Function | `x^2 - 2` |
| Decimal places | `6` |
| Lower bound | `1` |
| Upper bound | `2` |
| Tolerance | `0.000001` |
| Maximum iterations | `100` |

Output:

```text
--- RESULT ---
  Root      : 1.414214
  Error     : 0.000001
  Iteration : 20
```

For integration, select **8 — Simpson's Methods** and use `x^2`, 6 decimal places, the interval `[0, 1]`, and `n = 6`. Both methods return `0.333333`.

## How it works

```text
Function text → Token sequence → Postfix expression → Stack evaluation
                                                           ↓
                                                    Numerical method
                                                           ↓
                                                     Terminal output
```

The expression parser converts infix notation to postfix notation while respecting operator precedence and parentheses. The resulting expression is evaluated at different values of `x` by the root-finding, differentiation, and integration methods. Matrices and interpolation tables use dynamically allocated memory.

## Project files

| File / directory | Contents |
| --- | --- |
| [`numerical_analysis.c`](numerical_analysis.c) | Algorithms, expression parser, and terminal menu |
| [`screenshots/`](screenshots/) | Application screenshots |
| [`.gitignore`](.gitignore) | Ignore rules for build outputs, local settings, and archives |
| [`README.md`](README.md) | Turkish documentation |
| [`README.en.md`](README.en.md) | English documentation |

## Usage notes

- For Bisection and Regula-Falsi, choose a continuous function and an interval whose endpoints have function values with opposite signs.
- Newton-Raphson uses a numerical derivative. The initial estimate and nonzero derivative step `h` affect convergence.
- Gauss-Seidel does not converge for every system; diagonally dominant matrices are a useful starting point. The LU implementation does not perform pivoting.
- For Simpson's 1/3 rule, an odd `n` is increased by one to make it even. The Simpson's 3/8 implementation divides each interval into three subintervals internally, so a positive `n` does not need to be a multiple of 3.
- Gregory-Newton interpolation requires equally spaced `x` values.
- The decimal-place setting controls output formatting; convergence tolerance is a separate parameter.
- Input validation is limited. Use valid expressions, positive dimensions, and parameters appropriate for the selected method. Respect the domains of logarithms, division, and other operations.

This project is intended for studying numerical methods implemented in C and exploring them through example problems.
