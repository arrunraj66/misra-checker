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
