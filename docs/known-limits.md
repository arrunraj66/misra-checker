# Known limits of implemented detectors

Original engineering notes. Each limit is covered by a fixture so a change in
behaviour is visible.

## Rule 14.4 (controlling expression)
Implemented: `_Bool` values; relational, equality and logical operators; `!`;
`<stdbool.h>` `true`/`false`; comma (category of the right operand); `?:` when
both result operands are Boolean. Integer promotions are looked through.
Not implemented: the full essential-type model (character, enum, signed,
unsigned, floating) that rules 10.x need. An enum constant or a `_Bool`
bit-field read through a macro the checker cannot see into is judged by its
C type only.

## Rule 13.4 (assignment result)
The result counts as used when its enclosing context consumes it: operands,
call arguments, conditions, return, initializers and the last statement of a
GNU statement expression. It does not count as used as an expression statement,
a `for` init/increment, a cast to `void`, or the left operand of a comma. The
value of a comma or `?:` expression is followed outward, so `a = 1, b = 2;` is
clean and `x = c ? (y = 1) : 0;` is not. Increment and decrement operators are
not assignments for this rule.

## Rule 8.3 (consistent declarations)
Types are compared as element type plus array bounds. `int a[]` is compatible
with `int a[4]` and `int a[5]`, while `int a[4]` and `int a[5]` conflict; every
declaration is compared with every earlier one because compatibility is not
transitive. Parameter names are compared positionally. Function types are
compared as canonical text, so equal function-pointer types spelled through
different typedefs agree, but compatible-yet-distinct types (for example enums
and their underlying integer type) are reported.

## Section 10 rules (essential types) — 10.1, 10.2, 10.3, 10.4, 10.6, 10.7, 10.8
The model lives in `src/essential_type.{hpp,cpp}` and was written
independently from the rule intent. **Rule 10.5 is deliberately not
implemented**: its cast table could not be reconstructed with confidence, and a
guessed table would be worse than a gap. Validate every table below against the
licensed MISRA text before relying on a result.

Model:
* Categories: Boolean, Character (plain `char`), Enum (named enumerations and
  typedef-named anonymous ones), Signed, Unsigned, Floating. Pointers,
  aggregates, `void` and wide characters are `Other`; operations involving
  `Other` operands are not checked.
* Integer promotions and implicit conversions are looked through; an explicit
  cast takes the target type. `<stdbool.h>` `true`/`false` are Boolean.
  Constants of an unnamed enumeration are `int`, whatever type the compiler
  chose underneath. Bit-fields use their declared width.
* The result of an arithmetic or bitwise operator takes the type of its
  operands (the wider if both agree); a constant operand adopts the other
  operand's type. This reproduces the "essential, not promoted" behaviour.

Interpretations and leniencies (each is a place a licensed review may differ):
* 10.1: a non-negative signed *constant* is accepted as an operand of shifts,
  `~`, `&`, `|`, `^`. Operands of `Other` category are accepted.
* 10.3: assignment, initialization, argument passing to a prototyped function
  and `return` are all treated as assignment. Initializer lists are not
  checked. Compound assignments are checked by 10.1/10.4, not 10.3. An integer
  constant may initialize an object of the same category if it fits, or an
  unsigned object if non-negative and it fits; it may **not** initialize a
  Boolean, Character or Floating object (so `char c = 0;` and `float f = 1.0;`
  are reported).
* 10.4: `?:` result operands and compound assignments count as operators that
  perform the usual arithmetic conversions; shifts do not.
* 10.6/10.7/10.8: a *composite* expression is a non-constant arithmetic,
  bitwise, shift or unary `+ - ~` result. A cast result, comparison, call,
  member access and `?:` are not composite.

## Rules 5.2 and 5.3 (scopes)
Block scopes are tracked in the frontend: file scope, function scope (the
parameters and the outermost block share one scope), nested blocks, `for`
statements, prototype scope, and one scope per struct/union for members.
* Only project identifiers are modelled. A local that hides a name declared in
  a system header (for example a variable called `index`) is not reported.
* An outer declaration hides only what follows it. Within one file this is
  decided by source order; across files a header is assumed to precede its
  includer.
* Block-scope function declarations and `extern` declarations re-declare an
  existing entity and are not treated as hiding it.
* Parameters named inside a function *type* (function-pointer typedefs and
  declarators) are in their own prototype scope and are not recorded.
