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
- Can end sentence before illustration with ":".
- "vtable", all lowercase, no hyphen.
- "on-screen" when an adjective ("the on-screen player"), "on screen" when a
  place ("the player is on screen").
- Don't use a numbered list unless the order matters.
- Do not have a subheader immediately follow a header

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

## Pattern Chapter Structure

Each of the pattern chapters outside of the ones in "Design Patterns Revisited" has the same structure:

### Intent

This is a single italicized, imperative sentence explaining the core idea behind the pattern and the problem it solves.

### Motivation

This is the most narrative part of the chapter. It walks through an example problem and builds up to how the pattern solves it. It may have subheadings, illustrations, and even some code samples. It tends to be pretty large.

### The Pattern

The motivation section culminates in an explanation of the pattern and how it solves the previous problem. This one- or two-paragraph section summarizes that.

It's written in present tense with the components of the pattern itself as subjects. The major pieces of the pattern are in **bold face** the first time they are introduced here.

### When to Use It

Now that the reader sees how the pattern is useful for one problem, this short section describes which other problems are or are not a good fit for it.

It's usually only a few paragraphs long and often ends with a bullet list. It does not have any subheadings.

### Keep in Mind

This expands on the previous section and describes consequences -- mostly negative -- of using the pattern. If it's short, it will just be a few paragraphs with no subheaders. When longer, it will have a single paragraph followed by a few subheaders, one for each consequence.

### Sample Code

This long section walks step by step through a full implementation of the pattern. It will have paragraphs, illustrations, subheaders, and lots of code.

### Design Decisions

This shows how the pattern can vary along different axes within the solution space. It has a one paragraph intro followed be a series of design decisions.

Each has a subheader in the form of a question. It may be followed by a few paragraphs explaining the question.

After that, a bullet list covers some possible answers to that question. Each bullet in the list starts with a one sentence answer in **bold face**, ending with a colon.

Then there is a subsequent couple of paragraphs explaining the answer. This may be omitted.

Often, there will then be a *nested* bullet list with some ramifications of that decision. If so, each point will be one or more paragraphs, the first of which starts with an italicized sentence.

There may be code samples or illustrations anywhere in here.

Some of the decision question subsections don't follow this structure. They still have a subheader in the form of a question, but the body may just be prose or follow some other format.

### See Also

The easiest section. It's just a bullet list of short paragraphs containing links to other resources.
