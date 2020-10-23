# Coding style

The coding style of this project is based on these basic principles, in
this order:

1. **Correctness**  
    The basic requirement for all checked-in code, documentation and any
    other resources. There is no point in having anything that does not
    do what it should or what it looks like. Even if some working
    solution does not have the preferred form and performance
    characteristics, it is better than a beautiful, consistent and fast
    but non-working one. Commit well-tested code, do code reviews with
    due diligence, keep documentation updated, avoid non-deterministic
    solutions, never introduce memory or resource leaks, beware of
    typos etc.

2. **Efficiency**  
    The code we write runs on hundreds of millions devices, some of them
    with very limited resources. At the same time, different tasks of
    different components have different priority. For example, telemetry
    is not critical functionality - it is always less important than a
    real-time VOIP call or an UI update caused by end user's action.
    Therefore we must be very conservative about the footprint of
    our code. Be mindful from the beginning to not use suboptimal
    constructs in general, like an O(n²) algorithm where an O(1) one is
    readily available, keep memory usage lean, take advantage of
    short-circuit in conditions, design component's operations
    efficiently, especially those dealing with slow storage like a hard
    disk, do not block other's threads with unnecessary I/O etc.

3. **Consistency**  
    Being consistent as much as possible makes the project both easier
    to read and contribute to. The whole code base should look like
    written by one person. It allows the brain to adapt to the repeating
    constructs and focus on the changing content only. Consistency is an
    objective measure compared to e.g. readability. The order of these
    principles guarantees that in case these two are in conflict, it is
    always better to stick with what is the de facto established
    standard in the existing surrounding code. Make sure to check code
    changes before pushing them for review, make sure it blends in with
    the rest of the file nicely, there are no differences in
    formatting etc. This applies also on the higher level - design new
    classes and methods similarly as those already present, refactor
    also the previous code for consistency if there is a better new way.

4. **Readability**  
    As a bit controversial and subjective point, it roughly means the
    following in this project: use enough white space, name identifiers
    in an understandable way, do not add unnecessary text
    or formatting,. The details are described later in this document or
    set by example in the existing code and enforced by the
    *Consistency* rule above.

## C++ code

### Rules (non-exhaustive list)

- Indentation
  - 4 spaces, no tabs.
  - No trailing white space.
  - Extra indentation for `case` statements in a `switch`.
  - Extra half indentation for class access modifiers and
        constructor initializer list opening `:`.
  - Continuation lines indented with 4 spaces extra.
- Spacing
  - Always one space after if/while/for/try/catch and before opening
        curly bracket.
  - Spaces around arithmetic and conditional operators.
  - No spaces inside parentheses of function definitions and
        function calls or control statements.
  - No space between `template` and opening angle bracket.
  - Extra horizontal white space for alignment purposes is allowed
        and recommended.
  - `*`, `&` and `&&` type modifier follow the previous token
        without a space and are separated from a variable/field name by
        a space.
- New lines
  - Generally one statement per line.
  - Exceptions can be granted, e.g. `if (...) return ...;` if
        repeated a lot in one function.
  - `template<...>` on a separate line before
        affected declaration/definition.
  - Two blank lines or a short separator between different classes
        or logical blocks in one file.
- Braces
  - "One true brace style".
  - Attached opening curly brace except for methods and multi-line
        opening if/while/for statement.
  - Curly braces even for single-line if/while/for blocks.
  - Single-line methods allowed only inline with simple content
        (typically setter/getter).
- Identifiers
  - Type names, public API methods, enumeration values are in
        `PascalCase`.
  - Class names can contain an underscore `_` to separate logical
        parts between name itself and subtype/subkind, e.g.
        `FileImpl_Windows`, `FileImpl_Linux` and `FileImpl_Mac`.
  - Other method names use `camelCase`.
  - Variable names should be generally short and in `camelCase`.
  - Member fields have prefix `m_`, static member fields have prefix
        `s_`, global variables have prefix `g_`.
  - Macros use `ALL_CAPITALS_AND_UNDERSCORES`.
  - Abstract interfaces use prefix `I` as in `IPascalCase`.
  - Mock classes use prefix `Mock`, e.g. `MockHttpClient` or
        `MockIRunner`.
- Namespaces
  - No global `using namespace xxx` for `std` or other
        external libraries.
  - Comment at the end of namespace block in form
        `} // namespace Identifier`.
  - Multiple namespaces opening on one line.
- Types
  - No C-style casts.
  - Convert integers to enum values with `static_cast<Enum>(x)`.
  - `int` and (just) `unsigned` for basic signed/unsigned
        integer type.
  - stdint.h types (`int8_t`, `uint64_t` etc.) where a particular
        bit width is needed.
  - `class` is preferred to `struct`, the latter is allowed only if
        there are no access modifiers and the structure is used for data
        storage only, it does not contain complex methods etc.
  - Overridden virtual methods are marked in child classes with both
        `virtual` modifier and `override` specifier.
  - `const` used on the right side of the associated type, i.e.
        `char const*`, `std::string const&` etc.
- Tests
  - Names of unit test classes and source files based on the
        class/component being tested with suffix `Tests`.
  - If the original class contains an underscore and a
        subtype/subkind in its name, the subtype/subkind is moved at the
        end (e.g. `FileImpl_Windows` → `FileImplTests_Windows`).
  - `EXPECT_THAT()`/`ASSERT_THAT()` instead of `xxx_EQ()`,
        `xxx_TRUE()` etc. It prints the same level of detail (nice
        description, expected value, actual value), but reads more
        naturally and it is consistent with more complex matchers.
  - Explicit `Eq(...)` matcher used only if needed (e.g. comparing
        `std::string` to a string constant), otherwise it can be omitted
        for readability.
  - Mock call expectation actions (after
        `WillOnce`/`WillRepeatedly`) on a new line (or lines in case of
        `DoAll`).
- Pointers
  - Use raw pointers to express non-ownership. Never delete a raw pointer.
  - Use [`std::unique_ptr<T>`](https://en.cppreference.com/w/cpp/memory/unique_ptr)
         to express single ownership.
  - Use [`std::shared_ptr<T>`](https://en.cppreference.com/w/cpp/memory/shared_ptr)
         to express shared ownership between components.
    - Use [`std::weak_ptr<T>`](https://en.cppreference.com/w/cpp/memory/weak_ptr)
             to avoid reference cycles.
  - Only pass smart pointers (unique_ptr, shared_ptr, weak_ptr) as parameters to
         explicitly express lifetime semantics.
  - If required by a platform (e.g, Microsoft COM), use the appropriate
         ref-counted type, but do not expose those types to platform agnostic code,
         prefer to use an abstraction.
- Exceptions
  - All [user-declared destructors](https://en.cppreference.com/w/cpp/language/destructor) must be marked [noexcept](https://en.cppreference.com/w/cpp/language/noexcept_spec).

## Python code

- PEP8
- Except E501 (line too long) in a reasonable extent - i.e. 100-120
    characters, no important logic hidden behind the right margin.

## CMake scripts

- Indentation: 2 spaces, no tabs. No trailing white space.
- Command parentheses always after command name without any space.
- Use original CMake distribution files as a reference.

## Markdown documentation

- GitHub Flavored Markdown
- Trailing white space (two spaces at end of line) is allowed as an
    official means to insert a hard line break.
- Everything is in English, with clear full sentences and without colloquial
    phrases.  
- See existing .md files for reference.

## Git commits

- Use the common best practices for Git commit messages:
    <http://chris.beams.io/posts/git-commit/>
- Keep individual commits small and atomic
    (<http://www.freshconsulting.com/atomic-commits/>), update
    documentation and tests together with the relevant code change.
- Do white-space-only changes in a separate commit for clarity (null
    output for `git diff -w` etc.).
- Squash small fixes and code-review changes into original commits.
- Integrate changes from development branches to `master` using rebase
    and fast-forward merge.
