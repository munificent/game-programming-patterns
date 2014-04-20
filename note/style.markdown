## Prose

- `##` headers are title caps.
- `###` subheaders are sentence caps.
- "You" refers to the programmer.
- The player is genderless and referred to as "they".
- Ownership is expressed as "owner's thing" rather than "thing of the owner".
- Contractions are fine if it helps read conversationally.
- Keyboard keys have title case and are written as full words e.g. Control-Z.
- Code blocks are preceded by a colon.
- Bullet points may be sentence fragments but start with a cap and end with a period.
- The "See Also" section consists entirely of bullet points.
- Commas before conjunctions are fine.
- American spelling.
- References to class name, methods or any code use code format.
- Italics for emphasis.
- Quote block used for sentence long quotes but double quote ("") used for references what other people call something e.g. GoF call this the "subject".
- Colon precedes block quote.
- Items in a numbered list are complete sentences with starting caps and ending period.
- Use "Naïve" instead of "Naive".
- Use smart quotes.
- Use ` -- ` for em dashes.
- Prefer present tense instead of future.
- Prefer "we" over "you" or "I".
- Use *O(n&sup2;)* for Big-O notation, not `O(n^2)`.
- Put class name in `CodeFont` when referring to the class itself.
- It's "Foo pattern", not "Foo Pattern", and the link just surrounds, "Foo".
  Ex: "Add a method to `Foo`."
- Use normal font and lowercase when referring to objects of the class.
  Ex: "Create a new foo and pass it to `bar()`."
- Capitalize when referring to pattern like proper noun, lowercase when
  referring to object implementing pattern.
  Ex: "Object Pool is a great pattern. An object pool will save you memory."
- Design Patterns and other book titles are *italicized*.
- Don't use ":" in sentence before illustration.
- "vtable", all lowercase, no hyphen.
- "on-screen" when an adjective ("the on-screen player"), "on screen" when a
  place ("the player is on screen").

## Code

- Class and enum names are Pascal case.
- Camel case method names and properties.
- Trailing underscore for fields. That way isFoo() doesn't collide with isFoo
  field.
- No "I" for interfaces.
- Do use "virtual" for overridden methods.
- Use "double" for floating point variables.
- If base class has virtual methods, give it a virtual destructor.
- Virtual destructor comes before other definitions.
- Virtual destructor {} is on the same line as the signature.
- Public before private.
- Last entry in enum doesn't have comma.
- Cases are vertically aligned if the code is one statement long.
- Each property in an initializer list gets its own line.
- The empty constructor code after initializer list gets its own line and vertically aligns with initializer list colon.
- Function code and signature on the same line is fine if code is one line long.
- Passing mutable references is fine.
- Prefer postfix increment.

## Open Questions

- Q: How are switch cases indented?
- Q: How much should const be used?
- Q: Why use ":" before code blocks but not illustrations?
- Q: Use parentheses when referring to methods/functions? `foo` or `foo()`?
- Q: `true` or true?
- Q: Are "enum", "struct", "int", etc. normal words or `code font`?
- Q: Hide fields behind getters (better style) or allow public fields (less
     boilerplate)?
- Q: Can subheaders immediately follow headers?
